#pragma once

#include <d3d11.h>
#include <atlbase.h>


CComPtr<ID3D11Texture2D>			texFromView(const CComPtr<ID3D11ShaderResourceView>& texView);
CComPtr<ID3D11RenderTargetView>		rtvFromTex(const CComPtr<ID3D11Texture2D>& tex);
CComPtr<ID3D11RenderTargetView>		rtvFromSrv(const CComPtr<ID3D11ShaderResourceView>& texView);
CComPtr<ID3D11ShaderResourceView>	srvFromTex(const CComPtr<ID3D11Texture2D>& tex);
CComPtr<ID3D11UnorderedAccessView>	uavFromTex(const CComPtr<ID3D11Texture2D>& tex);
void								releaseResourceViews();
