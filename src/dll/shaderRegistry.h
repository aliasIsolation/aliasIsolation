#pragma once

#include <string>

#include "common.h"


struct ID3D11PixelShader;
struct ID3D11VertexShader;
struct ID3D11ComputeShader;


namespace ShaderRegistry
{
	ShaderHandle addPixelShader(std::string path);
	ShaderHandle addVertexShader(std::string path);
	ShaderHandle addComputeShader(std::string path);

	void reloadModified();

	ID3D11PixelShader* getPs(ShaderHandle h);
	ID3D11VertexShader* getVs(ShaderHandle h);
	ID3D11ComputeShader* getCs(ShaderHandle h);

	void releaseUnused();

	void startCompilerThread();
	void stopCompilerThread();
}
