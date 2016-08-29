#include "resourceUtil.h"

#include <map>

namespace
{
	std::map<ID3D11Texture2D*, ID3D11RenderTargetView*>		textureToRtvMap;
	std::map<ID3D11Texture2D*, ID3D11ShaderResourceView*>	textureToSrvMap;
	std::map<ID3D11Texture2D*, ID3D11UnorderedAccessView*>	textureToUavMap;
}


extern ID3D11Device* g_device;


ID3D11Texture2D* texFromView(ID3D11ShaderResourceView* texView) {
	ID3D11Resource* res = nullptr;
	texView->GetResource(&res);

	D3D11_RESOURCE_DIMENSION resType;
	res->GetType(&resType);

	if (resType != D3D11_RESOURCE_DIMENSION_TEXTURE2D) {
		abort();
	}

	ID3D11Texture2D *const tex = (ID3D11Texture2D*)res;
	return tex;
}

ID3D11RenderTargetView* rtvFromTex(ID3D11Texture2D* tex) {
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

		ID3D11RenderTargetView* rtv = nullptr;
		g_device->CreateRenderTargetView(tex, &desc, &rtv);

		if (!rtv) {
			abort();
		}

		textureToRtvMap[tex] = rtv;
		return rtv;
	}
}

ID3D11RenderTargetView* rtvFromSrv(ID3D11ShaderResourceView* texView) {
	ID3D11Resource* res = nullptr;
	texView->GetResource(&res);

	D3D11_RESOURCE_DIMENSION resType;
	res->GetType(&resType);

	if (resType != D3D11_RESOURCE_DIMENSION_TEXTURE2D) {
		abort();
	}

	ID3D11Texture2D *const tex = (ID3D11Texture2D*)res;
	return rtvFromTex(tex);
}

ID3D11ShaderResourceView* srvFromTex(ID3D11Texture2D* tex) {
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

		ID3D11ShaderResourceView* srv = nullptr;
		g_device->CreateShaderResourceView(tex, &desc, &srv);

		if (!srv) {
			abort();
		}

		textureToSrvMap[tex] = srv;
		return srv;
	}
}

ID3D11UnorderedAccessView* uavFromTex(ID3D11Texture2D* tex) {
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

		ID3D11UnorderedAccessView* uav = nullptr;
		g_device->CreateUnorderedAccessView(tex, &desc, &uav);

		if (!uav) {
			abort();
		}

		textureToUavMap[tex] = uav;
		return uav;
	}
}

void releaseResourceViews() {
	for (auto it : textureToRtvMap) {
		if (it.second) it.second->Release();
	}
	textureToRtvMap.clear();

	for (auto it : textureToSrvMap) {
		if (it.second) it.second->Release();
	}
	textureToSrvMap.clear();

	for (auto it : textureToUavMap) {
		if (it.second) it.second->Release();
	}
	textureToUavMap.clear();
}
