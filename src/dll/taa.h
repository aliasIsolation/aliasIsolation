#pragma once

struct ID3D11DeviceContext;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11DeviceContext;

bool taaOnDraw(ID3D11DeviceContext* context, ID3D11VertexShader* currentVs, ID3D11PixelShader* currentPs);


void taaHookApi(ID3D11DeviceContext* immediateContext);

void taaEnableApiHooks();
void taaDisableApiHooks();
