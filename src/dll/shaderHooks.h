#pragma once

#include "common.h"

struct ID3D11PixelShader;
struct ID3D11VertexShader;

ShaderHandle getReplacedShader(ID3D11PixelShader* ps);
ShaderHandle getReplacedShader(ID3D11VertexShader* vs);
