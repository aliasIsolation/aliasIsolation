#pragma warning(disable: 4996)	// 'sprintf': This function or variable may be unsafe.

#include <windows.h>
#include <d3d11.h>
#include <cstdio>

#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "MinHook.h"

#include "common.h"
#include "vtables.h"
#include "profiler.h"
#include "shaderRegistry.h"
#include "resourceUtil.h"
#include "fnTypes.h"
#include "frameConstants.h"
#include "alienResources.h"
#include "taa.h"
#include "post.h"
#include "hookedFns.h"
#include "shaderHooks.h"
#include "dllParams.h"


extern SharedDllParams	g_dllParams;

ID3D11Device*			g_device = nullptr;
ID3D11DeviceContext*	g_deferred_context = nullptr;

FrameConstants			g_frameConstants;
AlienResources			g_alienResources;

extern ShaderHandle		g_sharpenPsHandle;


// ----------------------------------------------------------------------------------------------------------------

void enableShaderHooks()
{
	MH_EnableHook(g_d3dHooks.PSSetShader);
	MH_EnableHook(g_d3dHooks.VSSetShader);
	MH_EnableHook(g_d3dHooks.Draw);

	taaEnableApiHooks();
}

void disableShaderHooks()
{
	MH_DisableHook(g_d3dHooks.PSSetShader);
	MH_DisableHook(g_d3dHooks.VSSetShader);
	MH_DisableHook(g_d3dHooks.Draw);

	taaDisableApiHooks();
}

// ----------------------------------------------------------------------------------------------------------------

void finishFrame() {
	Profiler::GlobalProfiler.EndFrame();
	g_frameConstants.taaRanThisFrame = false;
	ShaderRegistry::releaseUnused();
	g_frameConstants.prevViewProjNoJitter = g_frameConstants.currViewProjNoJitter;
}

// ----------------------------------------------------------------------------------------------------------------

HRESULT WINAPI CreateSamplerState_hook(
		ID3D11Device* device,
  const	D3D11_SAMPLER_DESC *pSamplerDesc,
		ID3D11SamplerState **ppSamplerState
) {
	D3D11_SAMPLER_DESC desc = *pSamplerDesc;

	// Only g-buffer pass objects have anisotropy, and those are also the ones that need the bias.
	if (desc.MaxAnisotropy > 1.0f) {
		desc.MipLODBias -= 0.5f;
	}

	return g_d3dHookOrig.CreateSamplerState(device, &desc, ppSamplerState);
}

// ----------------------------------------------------------------------------------------------------------------

HRESULT WINAPI Draw_hook(
	ID3D11DeviceContext* context,
	UINT VertexCount,
	UINT StartVertexLocation
) {
	HRESULT result;
	auto origDrawFn = [&]() { result = g_d3dHookOrig.Draw(context, VertexCount, StartVertexLocation); };

	ID3D11VertexShader*	currentVs = nullptr;
	ID3D11PixelShader*	currentPs = nullptr;
	context->VSGetShader(&currentVs, nullptr, nullptr);
	context->PSGetShader(&currentPs, nullptr, nullptr);

	taaOnDraw(context, currentVs, currentPs);

	if (caOnDraw(context, currentVs, currentPs, origDrawFn)) {
		finishFrame();
		return result;
	}

	origDrawFn();
	return result;
}

// ----------------------------------------------------------------------------------------------------------------

void STDMETHODCALLTYPE PSSetShader_hook( 
    ID3D11DeviceContext * context,
    /* [annotation] */ 
    _In_opt_  ID3D11PixelShader *pPixelShader,
    /* [annotation] */ 
    _In_reads_opt_(NumClassInstances)  ID3D11ClassInstance *const *ppClassInstances,
    UINT NumClassInstances)
{
	ID3D11PixelShader *shader = pPixelShader;
	auto replaced = getReplacedShader(pPixelShader);
	if (replaced == g_sharpenPsHandle && g_sharpenPsHandle.isValid() ) {
		modifySharpenPass(context);
		shader = ShaderRegistry::getPs(replaced);
	}

	g_d3dHookOrig.PSSetShader(context, shader, ppClassInstances, NumClassInstances);

	if (g_alienResources.cameraMotionPs == pPixelShader && g_alienResources.cameraMotionPs != nullptr)
	{
		context->PSGetConstantBuffers(2, 1, &g_alienResources.cbDefaultPSC);

		ID3D11RenderTargetView* rtv;
		context->OMGetRenderTargets(1, &rtv, nullptr);

		ID3D11Resource* rtRes = nullptr;
		if (rtv) rtv->GetResource(&rtRes);

		g_alienResources.velocitySrv = srvFromTex((ID3D11Texture2D*)rtRes);
	}
}

// ----------------------------------------------------------------------------------------------------------------

void STDMETHODCALLTYPE VSSetShader_hook( 
    ID3D11DeviceContext * context,
    /* [annotation] */ 
    _In_opt_  ID3D11VertexShader *pVertexShader,
    /* [annotation] */ 
    _In_reads_opt_(NumClassInstances)  ID3D11ClassInstance *const *ppClassInstances,
    UINT NumClassInstances)
{
	ID3D11VertexShader *shader = pVertexShader;
	auto replaced = getReplacedShader(pVertexShader);
	if (replaced.isValid()) {
		shader = ShaderRegistry::getVs(replaced);
	}

	g_d3dHookOrig.VSSetShader(context, shader, ppClassInstances, NumClassInstances);

	if (g_alienResources.smaaVertexShader == pVertexShader && g_alienResources.smaaVertexShader != nullptr)
	{
		context->VSGetConstantBuffers(0, 1, &g_alienResources.cbDefaultXSC);
	}
	else if (g_alienResources.rgbmEncodeVs == pVertexShader && g_alienResources.rgbmEncodeVs != nullptr)
	{
		context->VSGetConstantBuffers(1, 1, &g_alienResources.cbDefaultVSC);
	}
}

// ----------------------------------------------------------------------------------------------------------------

HRESULT WINAPI ResizeBuffers_hook(
	IDXGISwapChain* swapChain,
	UINT        BufferCount,
	UINT        Width,
	UINT        Height,
	DXGI_FORMAT NewFormat,
	UINT        SwapChainFlags
) {
	g_alienResources.cbDefaultXSC = nullptr;
	g_alienResources.cbDefaultVSC = nullptr;
	g_alienResources.cbDefaultPSC = nullptr;
	g_alienResources.mappedCbDefaultXSC = nullptr;
	g_alienResources.mappedCbDefaultVSC = nullptr;
	g_alienResources.mappedCbDefaultPSC = nullptr;
	g_alienResources.velocitySrv = nullptr;
	g_alienResources.mainTexView = nullptr;

	g_frameConstants.screenWidth	= Width;
	g_frameConstants.screenHeight	= Height;

	releaseResourceViews();

	return g_d3dHookOrig.ResizeBuffers(swapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
}

// ----------------------------------------------------------------------------------------------------------------

D3D11CreateDeviceAndSwapChain_t	D3D11CreateDeviceAndSwapChain_orig = nullptr;


HRESULT WINAPI D3D11CreateDeviceAndSwapChain_hook(
      void		          *pAdapter,
      D3D_DRIVER_TYPE      DriverType,
      HMODULE              Software,
      UINT                 Flags,
const void			      *pFeatureLevels,
      UINT                 FeatureLevels,
      UINT                 SDKVersion,
const DXGI_SWAP_CHAIN_DESC *pSwapChainDesc,
      IDXGISwapChain       **ppSwapChain,
      ID3D11Device         **ppDevice,
      void    *pFeatureLevel,
      ID3D11DeviceContext  **ppImmediateContext
) {
	g_frameConstants.screenWidth = pSwapChainDesc->BufferDesc.Width;
	g_frameConstants.screenHeight = pSwapChainDesc->BufferDesc.Height;

	HRESULT res = D3D11CreateDeviceAndSwapChain_orig(
		pAdapter,
		DriverType,
		Software,
		Flags,
		pFeatureLevels,
		FeatureLevels,
		SDKVersion,
		pSwapChainDesc,
		ppSwapChain,
		ppDevice,
		pFeatureLevel,
		ppImmediateContext
	);

	g_device = *ppDevice;

	DX_CHECK(g_device->CreateDeferredContext(0, &g_deferred_context));

	g_d3dHooks.CreatePixelShader = (CreatePixelShader_t)(*(void***)*ppDevice)[int(D3D11DeviceVTbl::CreatePixelShader)];
	MH_CHECK(MH_CreateHook(g_d3dHooks.CreatePixelShader, &CreatePixelShader_hook, (LPVOID*)&g_d3dHookOrig.CreatePixelShader));

	g_d3dHooks.CreateVertexShader = (CreateVertexShader_t)(*(void***)*ppDevice)[int(D3D11DeviceVTbl::CreateVertexShader)];
	MH_CHECK(MH_CreateHook(g_d3dHooks.CreateVertexShader, &CreateVertexShader_hook, (LPVOID*)&g_d3dHookOrig.CreateVertexShader));

	g_d3dHooks.CreateSamplerState = (CreateSamplerState_t)(*(void***)*ppDevice)[int(D3D11DeviceVTbl::CreateSamplerState)];
	MH_CHECK(MH_CreateHook(g_d3dHooks.CreateSamplerState, &CreateSamplerState_hook, (LPVOID*)&g_d3dHookOrig.CreateSamplerState));

	g_d3dHooks.PSSetShader = (PSSetShader_t)(*(void***)*ppImmediateContext)[int(D3D11DeviceContextVTbl::PSSetShader)];
	MH_CHECK(MH_CreateHook(g_d3dHooks.PSSetShader, &PSSetShader_hook, (LPVOID*)&g_d3dHookOrig.PSSetShader));

	g_d3dHooks.VSSetShader = (VSSetShader_t)(*(void***)*ppImmediateContext)[int(D3D11DeviceContextVTbl::VSSetShader)];
	MH_CHECK(MH_CreateHook(g_d3dHooks.VSSetShader, &VSSetShader_hook, (LPVOID*)&g_d3dHookOrig.VSSetShader));

	g_d3dHooks.Draw = (Draw_t)(*(void***)*ppImmediateContext)[int(D3D11DeviceContextVTbl::Draw)];
	MH_CHECK(MH_CreateHook(g_d3dHooks.Draw, &Draw_hook, (LPVOID*)&g_d3dHookOrig.Draw));

	g_d3dHooks.ResizeBuffers = (ResizeBuffers_t)(*(void***)*ppSwapChain)[int(IDXGISwapChainVtbl::ResizeBuffers)];
	MH_CHECK(MH_CreateHook(g_d3dHooks.ResizeBuffers, &ResizeBuffers_hook, (LPVOID*)&g_d3dHookOrig.ResizeBuffers));

	taaHookApi(*ppImmediateContext);
	enableShaderHooks();

	MH_EnableHook(g_d3dHooks.CreatePixelShader);
	MH_EnableHook(g_d3dHooks.CreateVertexShader);
	MH_EnableHook(g_d3dHooks.CreateSamplerState);
	MH_EnableHook(g_d3dHooks.ResizeBuffers);

#if _DEBUG
	if (AllocConsole())
	{
		freopen("CONIN$", "r", stdin);
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
	}
#endif

	Profiler::GlobalProfiler.Initialize(g_device, *ppImmediateContext);

	return res;
}

// ----------------------------------------------------------------------------------------------------------------

void hookRendering()
{
	int ret;

	HMODULE hModule = GetModuleHandleA("d3d11");
	if (hModule)
	{
		g_d3dHooks.D3D11CreateDeviceAndSwapChain = (LPVOID)GetProcAddress(hModule, "D3D11CreateDeviceAndSwapChain");
		if (g_d3dHooks.D3D11CreateDeviceAndSwapChain)
		{
			ret = MH_CreateHook(g_d3dHooks.D3D11CreateDeviceAndSwapChain, &D3D11CreateDeviceAndSwapChain_hook, (LPVOID*)&D3D11CreateDeviceAndSwapChain_orig);
		}
		else
		{
			ret = MH_ERROR_FUNCTION_NOT_FOUND;
		}
	} 
	else
	{
		//ret = MH_ERROR_MODULE_NOT_FOUND;
		return;
	}

	if (ret != MH_OK)
	{
		char buf[256];
		sprintf(buf, "MH_CreateHookApi() failed: %d", ret);
		MessageBoxA(NULL, buf, NULL, NULL);
		return;
	}

	MH_CHECK(MH_EnableHook(g_d3dHooks.D3D11CreateDeviceAndSwapChain));

	ShaderRegistry::startCompilerThread();
}

void unhookRendering()
{
	ShaderRegistry::stopCompilerThread();
	disableShaderHooks();
	MH_DisableHook(g_d3dHooks.D3D11CreateDeviceAndSwapChain);
	MH_DisableHook(g_d3dHooks.CreatePixelShader);
	MH_DisableHook(g_d3dHooks.CreateVertexShader);
	MH_DisableHook(g_d3dHooks.CreateSamplerState);
	MH_DisableHook(g_d3dHooks.ResizeBuffers);
}
