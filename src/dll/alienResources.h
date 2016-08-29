#pragma once

#include "constantBuffers.h"

struct ID3D11ShaderResourceView;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11Buffer;


struct AlienResources
{
	ID3D11VertexShader*			smaaVertexShader = nullptr;
	ID3D11VertexShader*			rgbmEncodeVs = nullptr;

	ID3D11PixelShader*			rgbmEncodePs = nullptr;
	ID3D11PixelShader*			dofEncodePs = nullptr;
	ID3D11PixelShader*			cameraMotionPs = nullptr;

	ID3D11ShaderResourceView*	velocitySrv = nullptr;
	ID3D11ShaderResourceView*	mainTexView = nullptr;

	ID3D11Buffer*				cbDefaultXSC = nullptr;
	ID3D11Buffer*				cbDefaultVSC = nullptr;
	ID3D11Buffer*				cbDefaultPSC = nullptr;

	CbDefaultXSC*				mappedCbDefaultXSC = nullptr;
	CbDefaultVSC*				mappedCbDefaultVSC = nullptr;
	CbDefaultPSC*				mappedCbDefaultPSC = nullptr;
};

extern AlienResources g_alienResources;
