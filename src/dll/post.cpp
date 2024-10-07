#include "post.h"

#include "glm/glm.hpp"
#include "tracy/Tracy.hpp"
#include "tracy/TracyD3D11.hpp"
#include <d3d11.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "alienResources.h"
#include "frameConstants.h"
#include "profiler.h"
#include "resourceUtil.h"
#include "settings.h"
#include "shaderRegistry.h"
#include "utilities.h"

extern TracyD3D11Ctx g_tracyD3D11Ctx;
extern ID3D11Device *g_device;
extern ID3D11DeviceContext *g_deferred_context;
extern Settings g_settings;

ShaderHandle g_sharpenPsHandle;

namespace
{
CComPtr<ID3D11Texture2D> g_bloomMergeTex = nullptr;
CComPtr<ID3D11Resource> g_origBloomMergeResource = nullptr;
} // namespace

// Hook for Cinematic Tools or whatever might want to render an overlay.
// Hooking Present mostly works, but it's a highly contended hook, and apps have
// a tendency to remove each other's hooks. For example, Fraps nukes the Present
// hook of Cinematic Tools.
extern "C" __declspec(dllexport) void __cdecl aliasIsolation_hookableOverlayRender(ID3D11Device * /*device*/,
                                                                                   ID3D11DeviceContext * /*context*/)
{
// We can't compile inline assembly for a 64-bit build target.
#ifndef _WIN64
    __asm {nop;
nop;
nop;
nop;
nop;
nop;
nop;
nop;
nop;
nop;
nop;
nop;
    }
#endif
}

static ID3D11Buffer *createConstantBuffer(unsigned sizeBytes)
{
    ZoneScopedN(__FUNCTION__);

    // Fill in a buffer description.
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeBytes;
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    // Create the buffer.
    ID3D11Buffer *res = nullptr;
    DX_CHECK(g_device->CreateBuffer(&cbDesc, nullptr, &res));

    return res;
}

static void updateConstantBuffer(ID3D11DeviceContext *context, ID3D11Buffer *constantBuffer, void *data,
                                 unsigned sizeBytes)
{
    ZoneScopedN(__FUNCTION__);

    D3D11_MAPPED_SUBRESOURCE mapped = {};
    context->Map(constantBuffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mapped);
    if (mapped.pData)
    {
        memcpy(mapped.pData, data, sizeBytes);
    }
    context->Unmap(constantBuffer, 0);
}

void modifySharpenPass(ID3D11DeviceContext *const context)
{
    ZoneScopedN(__FUNCTION__);

    static ID3D11Texture2D *logoTex;
    static ID3D11Buffer *cb;
    static bool resourcesCreated = false;

    struct Constants
    {
        float logoIntensity;
        float sharpenAmount;
        // this must be a 4 byte type, as a regular bool can potentially break the
        // HLSL shader logic by not initialising all of the memory.
        BOOL sharpenEnabled;
        float pad;
    } constants = {
        // Fade off the logo with time
        (g_frameConstants.gameTime > 0.0f ? (1.f - glm::smoothstep(10.0f, 12.0f, g_frameConstants.gameTime)) : 0.0f),
        g_settings.sharpening, g_settings.sharpeningEnabled
        // TODO! Is pad using initialised memory? If it isn't, this could cause
        // cache misses.
    };

    if (!resourcesCreated)
    {
        resourcesCreated = true;

        {
            const std::string logoPath = getDataFilePath("textures\\aliasIsolationLogo.png", false);
            int x, y, n;
            unsigned char *data = stbi_load(logoPath.c_str(), &x, &y, &n, 4);
            if (data && 4 == n)
            {
                D3D11_TEXTURE2D_DESC texDesc;
                ZeroMemory(&texDesc, sizeof(texDesc));
                texDesc.Width = x;
                texDesc.Height = y;
                texDesc.MipLevels = 1;
                texDesc.ArraySize = 1;
                texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                texDesc.SampleDesc.Count = 1;
                texDesc.SampleDesc.Quality = 0;
                texDesc.Usage = D3D11_USAGE_IMMUTABLE;
                texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

                D3D11_SUBRESOURCE_DATA sub;
                ZeroMemory(&sub, sizeof(sub));
                sub.pSysMem = data;
                sub.SysMemPitch = x * n;
                sub.SysMemSlicePitch = x * y * n;

                DX_CHECK(g_device->CreateTexture2D(&texDesc, &sub, &logoTex));
            }
            stbi_image_free(data);
        }

        cb = createConstantBuffer(sizeof(constants));
    }

    // Update constants
    updateConstantBuffer(context, cb, &constants, sizeof(constants));

    CComPtr<ID3D11ShaderResourceView> resources[] = {
        logoTex ? srvFromTex(logoTex) : nullptr,
    };

    context->PSSetConstantBuffers(10, 1, &cb);
    context->PSSetShaderResources(20, sizeof(resources) / sizeof(*resources), &resources[0].p);
}

// const char *const s_caString = "Chromatic Aberration";

bool caOnDraw(ID3D11DeviceContext *context, ID3D11PixelShader *currentPs, std::function<void()> origDrawFn)
{
    ZoneScopedN(__FUNCTION__);

    // Insert chromatic aberration after the sharpening pass
    if (g_sharpenPsHandle.isValid() && ShaderRegistry::getPs(g_sharpenPsHandle) == currentPs && currentPs != nullptr)
    {
        if (g_settings.chromaticAberrationEnabled)
        {
            TracyD3D11Zone(g_tracyD3D11Ctx, "Chromatic Aberration (ON)");
            // FrameMarkStart(s_caString);

            static ShaderHandle chromaticAberrationPsHandle;
            static bool resourcesCreated = false;
            static glm::uvec2 resourcesResolution;
            static ID3D11Texture2D *tempTex;
            static ID3D11Buffer *constantBuffer;

            const uint screenWidth = g_frameConstants.screenWidth;
            const uint screenHeight = g_frameConstants.screenHeight;

            struct Constants
            {
                float caAmount;
                float pad[3];
            } constants = {g_settings.chromaticAberration};

            if (!resourcesCreated || resourcesResolution != glm::uvec2(screenWidth, screenHeight))
            {
                resourcesCreated = true;
                resourcesResolution = glm::uvec2(screenWidth, screenHeight);

                chromaticAberrationPsHandle = ShaderRegistry::addPixelShader("chromaticAberration_ps.cso");

                {
                    D3D11_TEXTURE2D_DESC texDesc;
                    ZeroMemory(&texDesc, sizeof(texDesc));
                    texDesc.Width = screenWidth;
                    texDesc.Height = screenHeight;
                    texDesc.MipLevels = 1;
                    texDesc.ArraySize = 1;
                    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                    texDesc.SampleDesc.Count = 1;
                    texDesc.SampleDesc.Quality = 0;
                    texDesc.Usage = D3D11_USAGE_DEFAULT;
                    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

                    DX_CHECK(g_device->CreateTexture2D(&texDesc, nullptr, &tempTex));
                }

                constantBuffer = createConstantBuffer(sizeof(Constants));
            }

            updateConstantBuffer(context, constantBuffer, &constants, sizeof(constants));

            CComPtr<ID3D11RenderTargetView> targetRtv = nullptr;
            context->OMGetRenderTargets(1, &targetRtv.p, nullptr);

            CComPtr<ID3D11RenderTargetView> tempRtv = rtvFromTex(tempTex);
            context->OMSetRenderTargets(1, &tempRtv.p, nullptr);

            // Draw the sharpen pass now into the temporary RT
            origDrawFn();

            context->OMSetRenderTargets(1, &targetRtv.p, nullptr);

            CComPtr<ID3D11ShaderResourceView> srvs[] = {srvFromTex(tempTex)};

            context->PSSetShader(ShaderRegistry::getPs(chromaticAberrationPsHandle), nullptr, 0);
            context->PSSetConstantBuffers(0, 1, &constantBuffer);
            context->PSSetShaderResources(0, 1, &srvs[0].p);

            // ProfileBlock profile("Chromatic Aberration");
            context->Draw(3, 0);

            aliasIsolation_hookableOverlayRender(g_device, context);

            // FrameMarkEnd(s_caString);
        }
        else
        {
            TracyD3D11Zone(g_tracyD3D11Ctx, "Chromatic Aberration (OFF)");

            // Draw the current frame without CA.
            origDrawFn();
        }

        return true;
    }

    return false;
}
