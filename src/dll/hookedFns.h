#pragma once

#include "fnTypes.h"


struct FnHookHandles {
	void* CreateSamplerState = nullptr;
	void* Draw = nullptr;
	void* PSSetShader = nullptr;
	void* VSSetShader = nullptr;
	void* ResizeBuffers = nullptr;
	void* CreatePixelShader = nullptr;
	void* CreateVertexShader = nullptr;
	void* D3D11CreateDeviceAndSwapChain = nullptr;
};

struct HookedFns {
	CreateSamplerState_t	CreateSamplerState = nullptr;
	Draw_t					Draw = nullptr;
	PSSetShader_t			PSSetShader = nullptr;
	VSSetShader_t			VSSetShader = nullptr;
	Present_t				Present = nullptr;
	ResizeBuffers_t			ResizeBuffers = nullptr;
	CreatePixelShader_t		CreatePixelShader = nullptr;
	CreateVertexShader_t	CreateVertexShader = nullptr;
};


extern FnHookHandles	g_d3dHooks;
extern HookedFns		g_d3dHookOrig;


HRESULT WINAPI CreatePixelShader_hook(void* thisptr, const char* bytecode, SIZE_T bytecodeLength, void* classLinkage, ID3D11PixelShader** pixelShader);
HRESULT WINAPI CreateVertexShader_hook(void* thisptr, const char* bytecode, SIZE_T bytecodeLength, void* classLinkage, ID3D11VertexShader** vertexShader);
