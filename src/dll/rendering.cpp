#pragma warning(disable: 4996)	// 'sprintf': This function or variable may be unsafe.

#include <windows.h>
#include <d3d11.h>
#include <cstdio>
#include <atlbase.h>

#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "MinHook.h"
#include "methodHook.h"

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
#include "settings.h"
#include "menu.h"


extern SharedDllParams	g_dllParams;

ID3D11Device*			g_device = nullptr;
ID3D11DeviceContext*	g_deferred_context = nullptr;
Settings				g_settings;

FrameConstants			g_frameConstants;
AlienResources			g_alienResources;

extern ShaderHandle		g_sharpenPsHandle;

// ----------------------------------------------------------------------------------------------------------------

bool g_shaderHooksEnabled = false;

void enableShaderHooks()
{
	if (!g_shaderHooksEnabled)
	{
		enableMethodHook(g_d3dHooks.PSSetShader);
		enableMethodHook(g_d3dHooks.VSSetShader);
		enableMethodHook(g_d3dHooks.Draw);

		taaEnableApiHooks();
		g_shaderHooksEnabled = true;
	}
}

void disableShaderHooks()
{
	if (g_shaderHooksEnabled)
	{
		disableMethodHook(g_d3dHooks.PSSetShader);
		disableMethodHook(g_d3dHooks.VSSetShader);
		disableMethodHook(g_d3dHooks.Draw);

		taaDisableApiHooks();

		g_shaderHooksEnabled = false;
	}
}

// ----------------------------------------------------------------------------------------------------------------

void finishFrame() {
	Profiler::GlobalProfiler.EndFrame();
	g_frameConstants.taaRanThisFrame = false;
	ShaderRegistry::releaseUnused();
	g_frameConstants.prevViewProjNoJitter = g_frameConstants.currViewProjNoJitter;
	loadSettings(&g_settings);
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

	CComPtr<ID3D11VertexShader> currentVs = nullptr;
	CComPtr<ID3D11PixelShader>	currentPs = nullptr;
	context->VSGetShader(&currentVs.p, nullptr, nullptr);
	context->PSGetShader(&currentPs.p, nullptr, nullptr);

	if (currentPs == g_alienResources.cameraMotionPs && g_alienResources.cameraMotionPs != nullptr)
	{
		CComPtr<ID3D11ShaderResourceView> depthSrv = nullptr;
		context->PSGetShaderResources(8, 1, &depthSrv);
		g_alienResources.depthSrv = depthSrv;
	}

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
		g_alienResources.cbDefaultPSC = nullptr;
		context->PSGetConstantBuffers(2, 1, &g_alienResources.cbDefaultPSC);

		CComPtr<ID3D11RenderTargetView> rtv;
		context->OMGetRenderTargets(1, &rtv.p, nullptr);
		g_alienResources.velocityRtv = rtv;

		CComPtr<ID3D11Resource> rtRes = nullptr;
		if (rtv)
		{
			rtv->GetResource(&rtRes.p);
		}
		else
		{
			LOG_MSG("[aliasIsolation::rendering] FATAL ERROR - rtv->GetResource(&rtRes.p) - Failed to get resource from render target view!\n", "");
			DebugBreak();
		}

		g_alienResources.velocitySrv = srvFromTex((ID3D11Texture2D*)rtRes.p);
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
		g_alienResources.cbDefaultXSC = nullptr;
		context->VSGetConstantBuffers(0, 1, &g_alienResources.cbDefaultXSC);
	}
	else if (g_alienResources.rgbmEncodeVs == pVertexShader && g_alienResources.rgbmEncodeVs != nullptr)
	{
		g_alienResources.cbDefaultVSC = nullptr;
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

HRESULT WINAPI Present_hook(
	IDXGISwapChain* swapChain,
	UINT        SyncInterval,
	UINT        Flags
) {
	Menu::DrawMenu();

	return g_d3dHookOrig.Present(swapChain, SyncInterval, Flags);
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
	//MessageBoxA(NULL, "D3D11CreateDeviceAndSwapChain_hook called", NULL, NULL);

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

	// We only care about the first call to this function. Other hook-based mods will sometimes create their own devices,
	// but we don't care about those, and don't want to inject into those.
	if (g_device)
	{
		return res;
	}

	g_frameConstants.screenWidth = pSwapChainDesc->BufferDesc.Width;
	g_frameConstants.screenHeight = pSwapChainDesc->BufferDesc.Height;

	g_device = *ppDevice;

	DX_CHECK(g_device->CreateDeferredContext(0, &g_deferred_context));

#define HOOK_DEVICE_METHOD(NAME) \
	g_d3dHooks.NAME = hookMethod(*(void***)*ppDevice, int(D3D11DeviceVTbl::NAME), &NAME##_hook, (void**)&g_d3dHookOrig.NAME)
#define HOOK_CONTEXT_METHOD(NAME) \
	g_d3dHooks.NAME = hookMethod(*(void***)*ppImmediateContext, int(D3D11DeviceContextVTbl::NAME), &NAME##_hook, (void**)&g_d3dHookOrig.NAME)
#define HOOK_SWAPCHAIN_METHOD(NAME) \
	g_d3dHooks.NAME = hookMethod(*(void***)*ppSwapChain, int(IDXGISwapChainVtbl::NAME), &NAME##_hook, (void**)&g_d3dHookOrig.NAME)

	HOOK_DEVICE_METHOD(CreatePixelShader);
	HOOK_DEVICE_METHOD(CreateVertexShader);
	HOOK_DEVICE_METHOD(CreateSamplerState);

	HOOK_CONTEXT_METHOD(PSSetShader);
	HOOK_CONTEXT_METHOD(VSSetShader);
	HOOK_CONTEXT_METHOD(Draw);

	HOOK_SWAPCHAIN_METHOD(ResizeBuffers);
	HOOK_SWAPCHAIN_METHOD(Present);

	taaHookApi(*ppImmediateContext);

#undef HOOK_DEVICE_METHOD
#undef HOOK_CONTEXT_METHOD
#undef HOOK_SWAPCHAIN_METHOD

	enableShaderHooks();

	enableMethodHook(g_d3dHooks.CreatePixelShader);
	enableMethodHook(g_d3dHooks.CreateVertexShader);
	enableMethodHook(g_d3dHooks.CreateSamplerState);
	enableMethodHook(g_d3dHooks.ResizeBuffers);
	enableMethodHook(g_d3dHooks.Present);

#if _DEBUG
	if (AllocConsole())
	{
		freopen("CONIN$", "r", stdin);
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
	}
#endif

	Profiler::GlobalProfiler.Initialize(g_device, *ppImmediateContext);

	// Initialise the Alias Isolation menu.
	Menu::InitMenu(*ppSwapChain);

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
		LOG_MSG("[aliasIsolation::rendering] GetModuleHandleA(\"d3d11\") failed: MH_ERROR_MODULE_NOT_FOUND\n", "");
		return;
	}

	if (ret != MH_OK)
	{
		char buf[256];
		sprintf_s(buf, "MH_CreateHookApi() failed: %d\n", ret);
		MessageBoxA(NULL, buf, NULL, NULL);
		return;
	}

	MH_CHECK(MH_EnableHook(g_d3dHooks.D3D11CreateDeviceAndSwapChain));
	LOG_MSG("[aliasIsolation::rendering] MH_EnableHook(g_d3dHooks.D3d11CreateDeviceAndSwapChain)\n", "");

	ShaderRegistry::startCompilerThread();
	LOG_MSG("[aliasIsolation::rendering] ShaderRegistry::startCompilerThread()\n", "");
}

void unhookRendering()
{
	ShaderRegistry::stopCompilerThread();
	disableShaderHooks();
	MH_DisableHook(g_d3dHooks.D3D11CreateDeviceAndSwapChain);
	disableMethodHook(g_d3dHooks.CreatePixelShader);
	disableMethodHook(g_d3dHooks.CreateVertexShader);
	disableMethodHook(g_d3dHooks.CreateSamplerState);
	disableMethodHook(g_d3dHooks.ResizeBuffers);
}
