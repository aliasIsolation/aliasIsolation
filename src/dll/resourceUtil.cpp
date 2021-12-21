#include "resourceUtil.h"

#include <map>

namespace
{
	std::map<CComPtr<ID3D11Texture2D>, CComPtr<ID3D11RenderTargetView>>		textureToRtvMap;
	std::map<CComPtr<ID3D11Texture2D>, CComPtr<ID3D11ShaderResourceView>>	textureToSrvMap;
	std::map<CComPtr<ID3D11Texture2D>, CComPtr<ID3D11UnorderedAccessView>>	textureToUavMap;
}


extern ID3D11Device* g_device;


void ErrorDescription(HRESULT hr)
{
	if (FACILITY_WINDOWS == HRESULT_FACILITY(hr))
	{
		hr = HRESULT_CODE(hr);
	}

	TCHAR* szErrMsg;

	if (FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, 
		hr, 
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&szErrMsg, 0, NULL) != 0)
	{
		_tprintf(TEXT("[aliasIsolation::resourceUtil] %s"), szErrMsg);
		LocalFree(szErrMsg);
	}
	else
		_tprintf(TEXT("[aliasIsolation::resourceUtil - Could not find a description for error # %#x.]\n"), hr);
}

CComPtr<ID3D11Texture2D> texFromView(const CComPtr<ID3D11ShaderResourceView>& texView) {
	CComPtr<ID3D11Resource> res = nullptr;
	texView->GetResource(&res.p);

	D3D11_RESOURCE_DIMENSION resType;
	res->GetType(&resType);

	if (resType != D3D11_RESOURCE_DIMENSION_TEXTURE2D) {
		DebugBreak();
	}

	ID3D11Texture2D *const tex = (ID3D11Texture2D*)res.p;
	return CComPtr<ID3D11Texture2D>(tex);
}

CComPtr<ID3D11RenderTargetView> rtvFromTex(const CComPtr<ID3D11Texture2D>& tex) {
	decltype(textureToRtvMap)::iterator it = textureToRtvMap.find(tex);

	if (it != textureToRtvMap.end()) {
		return it->second;
	} else {
		D3D11_RENDER_TARGET_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(desc));

		D3D11_TEXTURE2D_DESC texDesc;
		tex->GetDesc(&texDesc);

		desc.Format = texDesc.Format;
		desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;

		CComPtr<ID3D11RenderTargetView> rtv = nullptr;
		g_device->CreateRenderTargetView(tex, &desc, &rtv);

		if (!rtv) {
			DebugBreak();
		}

		textureToRtvMap[tex] = rtv;
		return rtv;
	}
}

CComPtr<ID3D11RenderTargetView> rtvFromSrv(const CComPtr<ID3D11ShaderResourceView>& texView) {
	CComPtr<ID3D11Resource> res = nullptr;
	texView->GetResource(&res);

	D3D11_RESOURCE_DIMENSION resType;
	res->GetType(&resType);

	if (resType != D3D11_RESOURCE_DIMENSION_TEXTURE2D) {
		DebugBreak();
	}

	CComPtr<ID3D11Texture2D> tex = (ID3D11Texture2D*)res.p;
	return rtvFromTex(tex);
}

CComPtr<ID3D11ShaderResourceView> srvFromTex(const CComPtr<ID3D11Texture2D>& tex) {
	decltype(textureToSrvMap)::iterator it = textureToSrvMap.find(tex);

	if (it != textureToSrvMap.end()) {
		return it->second;
	} else {
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(desc));

		D3D11_TEXTURE2D_DESC texDesc;
		tex->GetDesc(&texDesc);

		desc.Format = texDesc.Format;
		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipLevels = -1;

		CComPtr<ID3D11ShaderResourceView> srv = nullptr;
		HRESULT createSRVResult = g_device->CreateShaderResourceView(tex, &desc, &srv);

		if (!SUCCEEDED(createSRVResult) && !srv) {
			HRESULT deviceRemovedReason = g_device->GetDeviceRemovedReason();

			// Convert the HRESULT code to a human-readable error message.
			ErrorDescription(createSRVResult);
			ErrorDescription(deviceRemovedReason);
			DebugBreak();
		}

		textureToSrvMap[tex] = srv.p;
		return srv;
	}
}

CComPtr<ID3D11UnorderedAccessView> uavFromTex(const CComPtr<ID3D11Texture2D>& tex) {
	decltype(textureToUavMap)::iterator it = textureToUavMap.find(tex);

	if (it != textureToUavMap.end()) {
		return it->second;
	} else {
		D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(desc));

		D3D11_TEXTURE2D_DESC texDesc;
		tex->GetDesc(&texDesc);

		desc.Format = texDesc.Format;
		desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;

		CComPtr<ID3D11UnorderedAccessView> uav = nullptr;
		g_device->CreateUnorderedAccessView(tex, &desc, &uav);

		if (!uav) {
			DebugBreak();
		}

		textureToUavMap[tex] = uav;
		return uav;
	}
}

void releaseResourceViews() {
	textureToRtvMap.clear();
	textureToSrvMap.clear();
	textureToUavMap.clear();
}
