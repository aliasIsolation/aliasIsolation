#include "taa.h"

#include <functional>
#include <vector>

#include "glm/glm.hpp"
#include "methodHook.h"
#include "tracy/Tracy.hpp"
#include "tracy/TracyD3D11.hpp"
#include <atlbase.h>
#include <d3d11.h>

#include "alienResources.h"
#include "fnTypes.h"
#include "frameConstants.h"
#include "math.h"
#include "profiler.h"
#include "resourceUtil.h"
#include "shaderRegistry.h"
#include "vtables.h"

extern TracyD3D11Ctx g_tracyD3D11Ctx;
extern ID3D11Device *g_device;
extern ID3D11DeviceContext *g_deferred_context;

static void renderTaa(ID3D11DeviceContext *const context, ID3D11ShaderResourceView *mainTexView)
{
    ZoneScopedN(__FUNCTION__);

    static ID3D11Texture2D *accumTex[2];
    static ID3D11SamplerState *pointSampler;
    static ID3D11SamplerState *linearSampler;
    static ID3D11Buffer *cb = NULL;
    static ShaderHandle taaCsHandle;

    static bool resourcesCreated = false;
    static glm::uvec2 resourcesResolution;

    struct Constants
    {
        unsigned screenWidth;
        unsigned screenHeight;
        float invScreenWidth;
        float invScreenHeight;
    } constants;
    ZeroMemory(&constants, sizeof(constants));

    const uint screenWidth = g_frameConstants.screenWidth;
    const uint screenHeight = g_frameConstants.screenHeight;

    // Fade off the logo with time
    constants.screenWidth = screenWidth;
    constants.screenHeight = screenHeight;
    constants.invScreenWidth = 1.f / screenWidth;
    constants.invScreenHeight = 1.f / screenHeight;

    if (!resourcesCreated || resourcesResolution != glm::uvec2(screenWidth, screenHeight))
    {
        resourcesCreated = true;
        resourcesResolution = glm::uvec2(screenWidth, screenHeight);

        taaCsHandle = ShaderRegistry::addComputeShader("taa_cs.cso");

        {
            D3D11_TEXTURE2D_DESC texDesc;
            ZeroMemory(&texDesc, sizeof(texDesc));
            texDesc.Width = screenWidth;
            texDesc.Height = screenHeight;
            texDesc.MipLevels = 1;
            texDesc.ArraySize = 1;
            texDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
            texDesc.SampleDesc.Count = 1;
            texDesc.SampleDesc.Quality = 0;
            texDesc.Usage = D3D11_USAGE_DEFAULT;
            texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS;

            DX_CHECK(g_device->CreateTexture2D(&texDesc, nullptr, &accumTex[0]));
            DX_CHECK(g_device->CreateTexture2D(&texDesc, nullptr, &accumTex[1]));
        }

        {
            D3D11_SAMPLER_DESC samplerDesc;
            ZeroMemory(&samplerDesc, sizeof(samplerDesc));
            samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
            samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
            samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
            samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
            samplerDesc.MaxAnisotropy = 1;

            DX_CHECK(g_device->CreateSamplerState(&samplerDesc, &pointSampler));

            samplerDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
            DX_CHECK(g_device->CreateSamplerState(&samplerDesc, &linearSampler));
        }

        {
            // Fill in a buffer description.
            D3D11_BUFFER_DESC cbDesc;
            ZeroMemory(&cbDesc, sizeof(cbDesc));
            cbDesc.ByteWidth = sizeof(constants);
            cbDesc.Usage = D3D11_USAGE_DYNAMIC;
            cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            cbDesc.MiscFlags = 0;
            cbDesc.StructureByteStride = 0;

            DX_CHECK(g_device->CreateBuffer(&cbDesc, nullptr, &cb));
        }
    }

    static uint accumIdx = 0;
    context->ClearState();

    // Update constants
    {
        D3D11_MAPPED_SUBRESOURCE res;
        ZeroMemory(&res, sizeof(res));

        if (!cb || cb == NULL)
        {
            NULL_FAILED(cb);
        }
        else
        {
            DX_CHECK(context->Map(cb, 0, D3D11_MAP_WRITE_DISCARD, 0, &res));
            if (res.pData)
            {
                memcpy(res.pData, &constants, sizeof(constants));
            }
            context->Unmap(cb, 0);
        }
    }

    CComPtr<ID3D11ShaderResourceView> resources[] = {mainTexView, srvFromTex(accumTex[1 - accumIdx]),
                                                     g_alienResources.velocitySrv, g_alienResources.depthSrv};

    CComPtr<ID3D11UnorderedAccessView> uavs[] = {uavFromTex(accumTex[accumIdx])};

    context->CSSetUnorderedAccessViews(0, sizeof(uavs) / sizeof(*uavs), &uavs[0].p, nullptr);
    context->CSSetShader(ShaderRegistry::getCs(taaCsHandle), nullptr, 0);
    context->CSSetConstantBuffers(0, 1, &cb);
    context->CSSetShaderResources(0, sizeof(resources) / sizeof(*resources), &resources[0].p);
    context->CSSetSamplers(0, 1, &linearSampler);
    context->CSSetSamplers(1, 1, &pointSampler);

    unsigned groupSizeX = 8;
    unsigned groupSizeY = 8;

    context->Dispatch((g_frameConstants.screenWidth + groupSizeX - 1) / groupSizeX,
                      (g_frameConstants.screenHeight + groupSizeY - 1) / groupSizeY, 1);

    {
        CComPtr<ID3D11Resource> res = nullptr;
        mainTexView->GetResource(&res);
        context->CopyResource(res, accumTex[accumIdx]);
    }

    accumIdx = (accumIdx + 1) % 2;
}

static void insertRendering(ID3D11DeviceContext *const context, const std::function<void(ID3D11DeviceContext *)> &fn)
{
    ZoneScopedN(__FUNCTION__);

    fn(g_deferred_context);

    CComPtr<ID3D11CommandList> commandList = nullptr;
    g_deferred_context->FinishCommandList(true, &commandList);
    context->ExecuteCommandList(commandList, true);
}

//const char *const s_taaString = "Temporal Anti-Aliasing";

bool taaOnDraw(ID3D11DeviceContext *context, ID3D11VertexShader *currentVs, ID3D11PixelShader *currentPs)
{
    ZoneScopedN(__FUNCTION__);

#ifndef ALIASISOLATION_NO_TAA_PASS
    // Run TAA before the DoF encode if it's present in the frame; otherwise run it before the RGBM encode
    bool shouldRunTaa = g_alienResources.dofEncodePs == currentPs ||
                        (!g_frameConstants.taaRanThisFrame && g_alienResources.rgbmEncodeVs == currentVs &&
                         g_alienResources.rgbmEncodePs == currentPs);

    if (shouldRunTaa)
    {
        CComPtr<ID3D11ShaderResourceView> mainTexView = nullptr;
        context->PSGetShaderResources(0, 1, &mainTexView.p);

        if (mainTexView)
        {
            TracyD3D11Zone(g_tracyD3D11Ctx, "Temporal Anti-Aliasing");
            // FrameMarkStart(s_taaString);

            CComPtr<ID3D11Texture2D> const mainTex = texFromView(mainTexView);
            D3D11_TEXTURE2D_DESC mainTexDesc;
            mainTex->GetDesc(&mainTexDesc);

            if (mainTexDesc.Format == DXGI_FORMAT_R11G11B10_FLOAT &&
                !!(mainTexDesc.BindFlags & D3D11_BIND_RENDER_TARGET))
            {
                g_alienResources.mainTexView = mainTexView;

                insertRendering(context, [&](ID3D11DeviceContext *dev) { renderTaa(dev, mainTexView); });

                g_frameConstants.taaRanThisFrame = true;
                ++g_frameConstants.taaSampleIdx;
            }

            // FrameMarkEnd(s_taaString);

            return true;
        }
    }
#endif

    return false;
}

// ----------------------------------------------------------------------------------------------------------------

Map_t Map_orig = nullptr;
Unmap_t Unmap_orig = nullptr;

HRESULT WINAPI Map_hook(ID3D11DeviceContext *context, _In_ ID3D11Resource *pResource, _In_ UINT Subresource,
                        _In_ D3D11_MAP MapType, _In_ UINT MapFlags, _Out_ D3D11_MAPPED_SUBRESOURCE *pMappedResource)
{
    ZoneScopedN(__FUNCTION__);

    HRESULT res = Map_orig(context, pResource, Subresource, MapType, MapFlags, pMappedResource);

    if (g_alienResources.cbDefaultXSC == pResource)
    {
        g_alienResources.mappedCbDefaultXSC = (CbDefaultXSC *)pMappedResource->pData;
    }
    else if (g_alienResources.cbDefaultVSC == pResource)
    {
        g_alienResources.mappedCbDefaultVSC = (CbDefaultVSC *)pMappedResource->pData;
    }
    else if (g_alienResources.cbDefaultPSC == pResource)
    {
        g_alienResources.mappedCbDefaultPSC = (CbDefaultPSC *)pMappedResource->pData;
    }

    return res;
}

static glm::vec2 getFrameJitter()
{
    ZoneScopedN(__FUNCTION__);

    if (g_frameConstants.taaRanThisFrame)
    {
        // We're post TAA in the frame. Must not add jitter or stuff will shake.
        return glm::vec2(0, 0);
    }

    glm::vec2 sampleOffset = hammersley2d((g_frameConstants.taaSampleIdx * 7u) % 16u, 16, 238308531);
    sampleOffset -= 0.5f;
    sampleOffset.x *= 2.0f / g_frameConstants.screenWidth;
    sampleOffset.y *= 2.0f / g_frameConstants.screenHeight;
    return sampleOffset;
}

// Cache of jittered projection matrices calculated for the shared pixel and vertex constant buffers.
// The math to calculate them is quite expensive, so we cache them and only update when needed.
struct
{
    glm::mat4 SecondaryProj;
    glm::mat4 ViewProj;
    glm::mat4 SecondaryViewProj;
} g_defaultXSC_cache;
uint taaSampleIdxForXSC_cache = 99U;
bool g_disableXSC_cache = true;

HRESULT WINAPI Unmap_hook(ID3D11DeviceContext *context, _In_ ID3D11Resource *pResource, _In_ UINT Subresource)
{
    ZoneScopedN(__FUNCTION__);

    CComPtr<ID3D11RenderTargetView> rtv;
    context->OMGetRenderTargets(1, &rtv, nullptr);

    CComPtr<ID3D11Resource> rtRes = nullptr;
    if (rtv)
        rtv->GetResource(&rtRes);

    ID3D11Texture2D *const rtTex = (ID3D11Texture2D *)rtRes.p;
    D3D11_TEXTURE2D_DESC rtTexDesc;
    ZeroMemory(&rtTexDesc, sizeof(D3D11_TEXTURE2D_DESC));

    if (rtTex)
        rtTex->GetDesc(&rtTexDesc);

    UINT numViewports = 1;
    D3D11_VIEWPORT viewport;
    context->RSGetViewports(&numViewports, &viewport);

    CComPtr<ID3D11BlendState> blendState = nullptr;
    context->OMGetBlendState(&blendState, nullptr, nullptr);

    D3D11_BLEND_DESC blendDesc;
    ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
    if (blendState)
    {
        blendState->GetDesc(&blendDesc);
    }

    const uint screenWidth = g_frameConstants.screenWidth;
    const uint screenHeight = g_frameConstants.screenHeight;

    // Only want TAA jitter in full-screen render passes
    if (rtTex && rtTexDesc.Width == screenWidth && rtTexDesc.Height == screenHeight &&
        viewport.Width == (float)screenWidth && viewport.Height == (float)screenHeight)
    {
#ifndef ALIASISOLATION_NO_SMAA_JITTER_ADD
        if (g_alienResources.cbDefaultXSC == pResource && g_alienResources.mappedCbDefaultXSC)
        {
            const glm::vec2 sampleOffset = getFrameJitter();

            CbDefaultXSC *const xsc = g_alienResources.mappedCbDefaultXSC;

            if (glm::mat3(xsc->SecondaryProj) != glm::mat3()) // Planar reflections have identity SecondaryProj
            {
                g_frameConstants.gameTime = xsc->Time.x;

                // We cannot jitter shadow matrices, or we get terrible flickering due to moving acne.
                // The way we detect shadow rendering is by comparing "CameraPosition" from the constant buffer
                // with one reconstructed from the view matrix. They will only match for main viewport rendering,
                // as "CameraPosition" is not updated for shadow views.
                const glm::vec3 viewCameraPos =
                    glm::mat3(xsc->ViewMatrix) *
                    glm::vec3(-xsc->ViewMatrix[0][3], -xsc->ViewMatrix[1][3], -xsc->ViewMatrix[2][3]);

                // Standard views have the CameraPosition matching the view matrix
                if (glm::length(viewCameraPos - glm::vec3(xsc->CameraPosition)) < 0.01f)
                {
                    // Only re-calculate the matrices if ViewProj changed. Otherwise we use the ones from
                    // g_defaultXSC_cache.
                    if (g_disableXSC_cache || g_frameConstants.currViewProjNoJitter != xsc->ViewProj ||
                        g_frameConstants.taaSampleIdx != taaSampleIdxForXSC_cache)
                    {
                        ZoneScopedN("TAA Matrix Calculation");

                        taaSampleIdxForXSC_cache = g_frameConstants.taaSampleIdx;

                        g_frameConstants.currViewMatrix = xsc->ViewMatrix;
                        g_frameConstants.currViewProjNoJitter = xsc->ViewProj;
                        g_frameConstants.currInvViewProjNoJitter =
                            glm::inverse(glm::dmat4(g_frameConstants.currViewProjNoJitter));

                        glm::mat4 jitterAdd;
                        jitterAdd[0][3] = sampleOffset.x;
                        jitterAdd[1][3] = sampleOffset.y;

                        g_defaultXSC_cache.SecondaryProj = xsc->SecondaryProj * jitterAdd;
                        g_defaultXSC_cache.ViewProj = xsc->ViewProj * jitterAdd;
                        g_defaultXSC_cache.SecondaryViewProj = xsc->SecondaryViewProj * jitterAdd;
                    }

                    xsc->SecondaryProj = g_defaultXSC_cache.SecondaryProj;
                    xsc->ViewProj = g_defaultXSC_cache.ViewProj;
                    xsc->SecondaryViewProj = g_defaultXSC_cache.SecondaryViewProj;
                }
            }

            g_alienResources.mappedCbDefaultXSC = nullptr;
        }
#endif
#ifndef ALIASISOLATION_NO_JITTER_ADD
        if (g_alienResources.cbDefaultVSC == pResource && g_alienResources.mappedCbDefaultVSC)
        {
            const glm::vec2 sampleOffset = getFrameJitter();

            CbDefaultVSC *const vsc = g_alienResources.mappedCbDefaultVSC;

            if (vsc->ProjMatrix[0][2] != sampleOffset.x || vsc->ProjMatrix[1][2] != sampleOffset.y)
            {
                glm::mat4 jitterAdd;
                jitterAdd[0][3] = sampleOffset.x;
                jitterAdd[1][3] = sampleOffset.y;

                vsc->ProjMatrix = vsc->ProjMatrix * jitterAdd;
                vsc->PrevViewProj = vsc->PrevViewProj * jitterAdd;
                vsc->PrevSecViewProj = vsc->PrevSecViewProj * jitterAdd;
            }

            g_alienResources.mappedCbDefaultVSC = nullptr;
        }
#endif
#ifndef ALIASISOLATION_NO_JITTER_REMOVE
        if (g_alienResources.cbDefaultPSC == pResource && g_alienResources.mappedCbDefaultPSC)
        {
            CbDefaultPSC *const psc = g_alienResources.mappedCbDefaultPSC;

            // Multiply the two matrices used in velocity vector calculation.
            // We can do it on the CPU using doubles, whereas the GPU uses floats, and loses a lot of precision.
            // Using single precision in the velocity calculation causes the screen to slightly shake.
            // For this calculation we use double precision for higher accuracy.
            psc->MotionBlurCurrInvViewProjection =
                glm::mat4(g_frameConstants.currInvViewProjNoJitter * glm::dmat4(g_frameConstants.prevViewProjNoJitter));
            psc->MotionBlurPrevViewProjection = glm::mat4();

            const glm::vec2 sampleOffset = getFrameJitter();

            glm::dmat4 jitterRemove;
            jitterRemove[0][3] = -1.0f * sampleOffset.x;
            jitterRemove[1][3] = -1.0f * sampleOffset.y;

            // Using single-precision on this calculation was introducing shadow flickering, due to the jitter not being
            // accurately removed from the shadow pass.
            const glm::dmat4 shadowJitterRemove = glm::dmat4(g_frameConstants.currViewProjNoJitter) * jitterRemove *
                                                  g_frameConstants.currInvViewProjNoJitter;
            psc->Spotlight0_Transform = glm::mat4(shadowJitterRemove) * psc->Spotlight0_Transform;

            g_alienResources.mappedCbDefaultPSC = nullptr;
        }
#endif
    }

    return Unmap_orig(context, pResource, Subresource);
}

namespace
{
void *Map = nullptr;
void *Unmap = nullptr;
} // namespace

void taaHookApi(ID3D11DeviceContext *immediateContext)
{
    ZoneScopedN(__FUNCTION__);

#define HOOK_CONTEXT_METHOD(NAME)                                                                                      \
    NAME = hookMethod(*(void ***)immediateContext, int(D3D11DeviceContextVTbl::NAME), &NAME##_hook,                    \
                      (void **)&NAME##_orig)

    HOOK_CONTEXT_METHOD(Map);
    HOOK_CONTEXT_METHOD(Unmap);

#undef HOOK_CONTEXT_METHOD
}

void taaEnableApiHooks()
{
    ZoneScopedN(__FUNCTION__);

    enableMethodHook(Map);
    enableMethodHook(Unmap);
}

void taaDisableApiHooks()
{
    ZoneScopedN(__FUNCTION__);

    disableMethodHook(Map);
    disableMethodHook(Unmap);
}
