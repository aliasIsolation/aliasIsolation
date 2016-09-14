#pragma once

#include <d3d11.h>


typedef HRESULT (WINAPI *CreatePixelShader_t)(void*, const void*, SIZE_T, void*, ID3D11PixelShader**);
typedef HRESULT (WINAPI *CreateVertexShader_t)(void*, const void*, SIZE_T, void*, ID3D11VertexShader**);

typedef HRESULT (WINAPI *CreateSamplerState_t)(
		ID3D11Device* This,
  const	D3D11_SAMPLER_DESC *pSamplerDesc,
		ID3D11SamplerState **ppSamplerState
);

typedef HRESULT (WINAPI *Draw_t)(
	ID3D11DeviceContext* This,
	UINT VertexCount,
	UINT StartVertexLocation
);

typedef void (STDMETHODCALLTYPE *PSSetShader_t)(
    ID3D11DeviceContext * This,
    /* [annotation] */ 
    _In_opt_  ID3D11PixelShader *pPixelShader,
    /* [annotation] */ 
    _In_reads_opt_(NumClassInstances)  ID3D11ClassInstance *const *ppClassInstances,
    UINT NumClassInstances);

typedef HRESULT (WINAPI* Map_t)(
	ID3D11DeviceContext *This,
    _In_  ID3D11Resource *pResource,
    _In_  UINT Subresource,
    _In_  D3D11_MAP MapType,
    _In_  UINT MapFlags,
    _Out_  D3D11_MAPPED_SUBRESOURCE *pMappedResource);

typedef HRESULT (WINAPI* Unmap_t)(
	ID3D11DeviceContext *This,
    _In_  ID3D11Resource *pResource,
    _In_  UINT Subresource);

typedef void (STDMETHODCALLTYPE *VSSetShader_t)( 
    ID3D11DeviceContext * This,
    /* [annotation] */ 
    _In_opt_  ID3D11VertexShader *pVertexShader,
    /* [annotation] */ 
    _In_reads_opt_(NumClassInstances)  ID3D11ClassInstance *const *ppClassInstances,
    UINT NumClassInstances);

typedef HRESULT (WINAPI *Present_t)(
	IDXGISwapChain* This,
	UINT SyncInterval,
	UINT Flags
);

typedef HRESULT (WINAPI *ResizeBuffers_t)(
	IDXGISwapChain* This,
	UINT        BufferCount,
	UINT        Width,
	UINT        Height,
	DXGI_FORMAT NewFormat,
	UINT        SwapChainFlags
);

typedef HRESULT (WINAPI* D3D11CreateDeviceAndSwapChain_t)(
      void		          *pAdapter,
      D3D_DRIVER_TYPE      DriverType,
      HMODULE              Software,
      UINT                 Flags,
const void			      *pFeatureLevels,
      UINT                 FeatureLevels,
      UINT                 SDKVersion,
const DXGI_SWAP_CHAIN_DESC *pSwapChainDesc,
      IDXGISwapChain		**ppSwapChain,
      ID3D11Device		**ppDevice,
      void				*pFeatureLevel,
      ID3D11DeviceContext	**ppImmediateContext
);

typedef BOOL (WINAPI *CreateProcessW_t)(
  LPCTSTR lpApplicationName,
  LPTSTR lpCommandLine,
  LPSECURITY_ATTRIBUTES lpProcessAttributes,
  LPSECURITY_ATTRIBUTES lpThreadAttributes,
  BOOL bInheritHandles,
  DWORD dwCreationFlags,
  LPVOID lpEnvironment,
  LPCTSTR lpCurrentDirectory,
  LPSTARTUPINFOW lpStartupInfo,
  LPPROCESS_INFORMATION lpProcessInformation
);

typedef BOOL (WINAPI *CreateProcessA_t)(
	LPCSTR lpApplicationName,
	LPSTR lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	BOOL bInheritHandles,
	DWORD dwCreationFlags,
	LPVOID lpEnvironment,
	LPCSTR lpCurrentDirectory,
	LPSTARTUPINFOA lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation
);
