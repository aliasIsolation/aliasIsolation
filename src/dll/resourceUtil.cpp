#include "resourceUtil.h"

#include <map>

namespace
{
	std::map<CComPtr<ID3D11Texture2D>, CComPtr<ID3D11RenderTargetView>>		textureToRtvMap;
	std::map<CComPtr<ID3D11Texture2D>, CComPtr<ID3D11ShaderResourceView>>	textureToSrvMap;
	std::map<CComPtr<ID3D11Texture2D>, CComPtr<ID3D11UnorderedAccessView>>	textureToUavMap;
}


extern ID3D11Device* g_device;


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
		g_device->CreateShaderResourceView(tex, &desc, &srv);

		if (!srv) {
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
