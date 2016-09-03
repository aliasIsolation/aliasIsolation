#pragma once

enum class IUnknownVTbl : short
{
    // IUnknown
    QueryInterface = 0,
    AddRef = 1,
    Release = 2,
};

enum class D3D11DeviceVTbl : short
{
    // IUnknown
    QueryInterface = 0,
    AddRef = 1,
    Release = 2,

    // ID3D11Device
    CreateBuffer = 3,
    CreateTexture1D = 4,
    CreateTexture2D = 5,
    CreateTexture3D = 6,
    CreateShaderResourceView = 7,
    CreateUnorderedAccessView = 8,
    CreateRenderTargetView = 9,
    CreateDepthStencilView = 10,
    CreateInputLayout = 11,
    CreateVertexShader = 12,
    CreateGeometryShader = 13,
    CreateGeometryShaderWithStreamOutput = 14,
    CreatePixelShader = 15,
    CreateHullShader = 16,
    CreateDomainShader = 17,
    CreateComputeShader = 18,
    CreateClassLinkage = 19,
    CreateBlendState = 20,
    CreateDepthStencilState = 21,
    CreateRasterizerState = 22,
    CreateSamplerState = 23,
    CreateQuery = 24,
    CreatePredicate = 25,
    CreateCounter = 26,
    CreateDeferredContext = 27,
    OpenSharedResource = 28,
    CheckFormatSupport = 29,
    CheckMultisampleQualityLevels = 30,
    CheckCounterInfo = 31,
    CheckCounter = 32,
    CheckFeatureSupport = 33,
    GetPrivateData = 34,
    SetPrivateData = 35,
    SetPrivateDataInterface = 36,
    GetFeatureLevel = 37,
    GetCreationFlags = 38,
    GetDeviceRemovedReason = 39,
    GetImmediateContext = 40,
    SetExceptionMode = 41,
    GetExceptionMode = 42,
};

enum class D3D11DeviceContextVTbl : short
{
	// IUnknown
	QueryInterface = 0,
	AddRef = 1,
	Release = 2,

	// ID3D11DeviceChild
	GetDevice,
	GetPrivateData,
	SetPrivateData, 
	SetPrivateDataInterface,

	// ID3D11DeviceContext
	VSSetConstantBuffers,
	PSSetShaderResources,
	PSSetShader,
	PSSetSamplers,
	VSSetShader,
	DrawIndexed,
	Draw,
	Map,
	Unmap,
	PSSetConstantBuffers,
	IASetInputLayout,
	IASetVertexBuffers,
	IASetIndexBuffer,
	DrawIndexedInstanced,
	DrawInstanced,
	GSSetConstantBuffers,
	GSSetShader,
	IASetPrimitiveTopology,
	VSSetShaderResources,
	VSSetSamplers,
	Begin,
	End,
	GetData,
	SetPredication,
	GSSetShaderResources,
	GSSetSamplers,
	OMSetRenderTargets,
	OMSetRenderTargetsAndUnorderedAccessViews,
	OMSetBlendState,
	OMSetDepthStencilState,
	SOSetTargets,
	DrawAuto,
	DrawIndexedInstancedIndirect,
	DrawInstancedIndirect,
	Dispatch,
	DispatchIndirect,
	RSSetState,
	RSSetViewports,
	RSSetScissorRects,
	CopySubresourceRegion,
	CopyResource,
	UpdateSubresource,
	CopyStructureCount,
	ClearRenderTargetView,
	ClearUnorderedAccessViewUint,
	ClearUnorderedAccessViewFloat,
	ClearDepthStencilView,
	GenerateMips,
	SetResourceMinLOD,
	GetResourceMinLOD,
	ResolveSubresource,
	ExecuteCommandList,
	HSSetShaderResources,
	HSSetShader,
	HSSetSamplers,
	HSSetConstantBuffers,
	DSSetShaderResources,
	DSSetShader,
	DSSetSamplers,
	DSSetConstantBuffers,
	CSSetShaderResources,
	CSSetUnorderedAccessViews,
	CSSetShader,
	CSSetSamplers,
	CSSetConstantBuffers,
	VSGetConstantBuffers,
	PSGetShaderResources,
	PSGetShader,
	PSGetSamplers,
	VSGetShader,
	PSGetConstantBuffers,
	IAGetInputLayout,
	IAGetVertexBuffers,
	IAGetIndexBuffer,
	GSGetConstantBuffers,
	GSGetShader,
	IAGetPrimitiveTopology,
	VSGetShaderResources,
	VSGetSamplers,
	GetPredication,
	GSGetShaderResources,
	GSGetSamplers,
	OMGetRenderTargets,
	OMGetRenderTargetsAndUnorderedAccessViews,
	OMGetBlendState,
	OMGetDepthStencilState,
	SOGetTargets,
	RSGetState,
	RSGetViewports,
	RSGetScissorRects,
	HSGetShaderResources,
	HSGetShader,
	HSGetSamplers,
	HSGetConstantBuffers,
	DSGetShaderResources,
	DSGetShader,
	DSGetSamplers,
	DSGetConstantBuffers,
	CSGetShaderResources,
	CSGetUnorderedAccessViews,
	CSGetShader,
	CSGetSamplers,
	CSGetConstantBuffers,
	ClearState,
	Flush,
	GetType,
	GetContextFlags,
	FinishCommandList,
};

enum class IDXGISwapChainVtbl {
    QueryInterface,
    AddRef,
    Release,
    SetPrivateData,
    SetPrivateDataInterface,
    GetPrivateData,
    GetParent,
    GetDevice,
    Present,
    GetBuffer,
    SetFullscreenState,
    GetFullscreenState,
    GetDesc,
    ResizeBuffers,
    ResizeTarget,
    GetContainingOutput,
    GetFrameStatistics,
    GetLastPresentCount,
};