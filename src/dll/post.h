#pragma once

#include <functional>

struct ID3D11DeviceContext;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11DeviceContext;

bool caOnDraw(ID3D11DeviceContext* context, ID3D11VertexShader* currentVs, ID3D11PixelShader* currentPs, std::function<void()> origDrawFn);
void modifySharpenPass(ID3D11DeviceContext *const context);
