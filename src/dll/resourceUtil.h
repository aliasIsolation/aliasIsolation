#pragma once

#include <d3d11.h>


ID3D11Texture2D*			texFromView(ID3D11ShaderResourceView* texView);
ID3D11RenderTargetView*		rtvFromTex(ID3D11Texture2D* tex);
ID3D11RenderTargetView*		rtvFromSrv(ID3D11ShaderResourceView* texView);
ID3D11ShaderResourceView*	srvFromTex(ID3D11Texture2D* tex);
ID3D11UnorderedAccessView*	uavFromTex(ID3D11Texture2D* tex);
void						releaseResourceViews();
