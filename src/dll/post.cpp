#include "post.h"

#include <d3d11.h>
#include "glm/glm.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "frameConstants.h"
#include "resourceUtil.h"
#include "shaderRegistry.h"
#include "alienResources.h"
#include "dllParams.h"
#include "profiler.h"

extern ID3D11Device*		g_device;
extern ID3D11DeviceContext*	g_deferred_context;
extern SharedDllParams		g_dllParams;

ShaderHandle				g_sharpenPsHandle;

namespace
{
	ID3D11Texture2D*	g_bloomMergeTex = nullptr;
	ID3D11Resource*		g_origBloomMergeResource = nullptr;
}


void modifySharpenPass(ID3D11DeviceContext *const context)
{
	static ID3D11Texture2D*	logoTex;
	static ID3D11Buffer*	cb;
	static bool				resourcesCreated = false;

	struct Constants {
		float		logoIntensity;
		unsigned	pad[3];
	} constants;

	// Fade off the logo with time
	constants.logoIntensity	= g_frameConstants.gameTime > 0.0f ? (1.f - glm::smoothstep(10.0f, 12.0f, g_frameConstants.gameTime)) : 0.0f;

	if (!resourcesCreated) {
		resourcesCreated = true;

		{
			const std::string logoPath = std::string(g_dllParams.aliasIsolationRootDir) + "/data/textures/aliasIsolationLogo.png";
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
				sub.pSysMem = data;
				sub.SysMemPitch = x * n;
				sub.SysMemSlicePitch = x * y * n;

				DX_CHECK(g_device->CreateTexture2D(&texDesc, &sub, &logoTex));
			}
			stbi_image_free(data);
		}
	
		{
			// Fill in a buffer description.
			D3D11_BUFFER_DESC cbDesc;
			cbDesc.ByteWidth = sizeof(constants);
			cbDesc.Usage = D3D11_USAGE_DYNAMIC;
			cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			cbDesc.MiscFlags = 0;
			cbDesc.StructureByteStride = 0;

			DX_CHECK(g_device->CreateBuffer(&cbDesc, nullptr, &cb));
		}
	}

	// Update constants
	{
		D3D11_MAPPED_SUBRESOURCE res;
		res.pData = 0;

		context->Map(cb, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
		if (res.pData) {
			memcpy(res.pData, &constants, sizeof(constants));
		}
		context->Unmap(cb, 0);
	}

	ID3D11ShaderResourceView* resources[] = {
		logoTex ? srvFromTex(logoTex) : nullptr,
	};

	context->PSSetConstantBuffers(10, 1, &cb);
	context->PSSetShaderResources(20, sizeof(resources)/sizeof(*resources), resources);
}


void modifyBloomMerge(ID3D11DeviceContext *const context)
{
	static bool				resourcesCreated = false;
	static glm::uvec2		resourcesResolution;

	const uint screenWidth = g_frameConstants.screenWidth;
	const uint screenHeight = g_frameConstants.screenHeight;

	if (!resourcesCreated || resourcesResolution != glm::uvec2(screenWidth, screenHeight)) {
		resourcesCreated = true;
		resourcesResolution = glm::uvec2(screenWidth, screenHeight);

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
			texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

			DX_CHECK(g_device->CreateTexture2D(&texDesc, nullptr, &g_bloomMergeTex));
		}
	}

	ID3D11RenderTargetView* origRtv = nullptr;
	context->OMGetRenderTargets(1, &origRtv, nullptr);
	if (origRtv) {
		origRtv->GetResource(&g_origBloomMergeResource);
		origRtv->Release();
		if (g_origBloomMergeResource) g_origBloomMergeResource->Release();
	}

	ID3D11RenderTargetView* rtvs[] = { rtvFromTex(g_bloomMergeTex) };
	context->OMSetRenderTargets(1, rtvs, nullptr);

	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(viewport));
	viewport.Width = (float)screenWidth;
	viewport.Height = (float)screenHeight;

	context->RSSetViewports(1, &viewport);
}


void modifyGlareForPost(ID3D11DeviceContext *const context)
{
	enum { NumSrvs = 16 };
	ID3D11ShaderResourceView* origSrvs[NumSrvs] = { nullptr };

	context->PSGetShaderResources(0, NumSrvs, origSrvs);
	for (int i = 0; i < NumSrvs; ++i) {
		ID3D11Resource* res = nullptr;
		if (origSrvs[i]) {
			origSrvs[i]->GetResource(&res);
			if (res && res == g_origBloomMergeResource) {
				ID3D11ShaderResourceView* srvs[] = { srvFromTex(g_bloomMergeTex) };
				context->PSSetShaderResources(i, 1, srvs);
			}
		}
	}
}

bool caOnDraw(ID3D11DeviceContext* context, ID3D11VertexShader* currentVs, ID3D11PixelShader* currentPs, std::function<void()> origDrawFn)
{
	// Insert chromatic aberration after the sharpening pass
	if (g_sharpenPsHandle.isValid() && ShaderRegistry::getPs(g_sharpenPsHandle) == currentPs && currentPs != nullptr) {
		static ShaderHandle		chromaticAberrationPsHandle;
		static bool				resourcesCreated = false;
		static glm::uvec2		resourcesResolution;
		static ID3D11Texture2D*	tempTex;

		const uint screenWidth = g_frameConstants.screenWidth;
		const uint screenHeight = g_frameConstants.screenHeight;

		if (!resourcesCreated || resourcesResolution != glm::uvec2(screenWidth, screenHeight)) {
			resourcesCreated = true;
			resourcesResolution = glm::uvec2(screenWidth, screenHeight);

			chromaticAberrationPsHandle = ShaderRegistry::addPixelShader("chromaticAberration_ps.hlsl");

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
		}

		ID3D11RenderTargetView* targetRtv = nullptr;
		context->OMGetRenderTargets(1, &targetRtv, nullptr);
		if (targetRtv) targetRtv->Release();

		ID3D11RenderTargetView* tempRtv = rtvFromTex(tempTex);
		context->OMSetRenderTargets(1, &tempRtv, nullptr);

		// Draw the sharpen pass now into the temporary RT
		origDrawFn();

		context->OMSetRenderTargets(1, &targetRtv, nullptr);

		ID3D11ShaderResourceView* srvs[] = {
			srvFromTex(tempTex)
		};

		context->PSSetShader(ShaderRegistry::getPs(chromaticAberrationPsHandle), nullptr, 0);
		context->PSSetShaderResources(0, 1, srvs);

		ProfileBlock profile("ca");
		context->Draw(3, 0);

		return true;
	}

	return false;
}
