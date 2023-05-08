#include <windows.h>
#include <d3d11.h>
#include <unordered_map>

#include "shaderRegistry.h"
#include "hookedFns.h"
#include "alienResources.h"


FnHookHandles	g_d3dHooks;
HookedFns		g_d3dHookOrig;

extern ShaderHandle	g_sharpenPsHandle;


namespace {
	std::unordered_map<const ID3D11PixelShader*,  ShaderHandle>	replacedPixelShaders;
	std::unordered_map<const ID3D11VertexShader*, ShaderHandle>	replacedVertexShaders;
}


void replacePixelShader(const ID3D11PixelShader* orig, ShaderHandle replaced)
{
	replacedPixelShaders[orig] = replaced;
}

void replaceVertexShader(const ID3D11VertexShader* orig, ShaderHandle replaced)
{
	replacedVertexShaders[orig] = replaced;
}

ShaderHandle getReplacedShader(ID3D11PixelShader* ps)
{
	auto replaced = replacedPixelShaders.find(ps);
	if (replaced != replacedPixelShaders.end()) {
		return replaced->second;
	} else {
		return ShaderHandle();
	}
}

ShaderHandle getReplacedShader(ID3D11VertexShader* vs)
{
	auto replaced = replacedVertexShaders.find(vs);
	if (replaced != replacedVertexShaders.end()) {
		return replaced->second;
	} else {
		return ShaderHandle();
	}
}

// ------------------------------------------------------------------------------------------------
// Just in case we get the same pointer for a new shader following a level reload
void clearPixelShaderReplacement(const ID3D11PixelShader* orig)
{
	if (replacedPixelShaders.find(orig) != replacedPixelShaders.end()) {
		replacedPixelShaders.erase(orig);
	}

	if (replacedVertexShaders.find((const ID3D11VertexShader*)orig) != replacedVertexShaders.end()) {
		replacedVertexShaders.erase((const ID3D11VertexShader*)orig);
	}
}

void clearVertexShaderReplacement(const ID3D11VertexShader* orig)
{
	if (replacedVertexShaders.find(orig) != replacedVertexShaders.end()) {
		replacedVertexShaders.erase(orig);
	}

	if (replacedPixelShaders.find((const ID3D11PixelShader*)orig) != replacedPixelShaders.end()) {
		replacedPixelShaders.erase((const ID3D11PixelShader*)orig);
	}
}

// ----------------------------------------------------------------------------------------------------------------


HRESULT WINAPI CreatePixelShader_hook(void* thisptr, const char* bytecode, SIZE_T bytecodeLength, void* classLinkage, ID3D11PixelShader** pixelShader)
{
	HRESULT res = g_d3dHookOrig.CreatePixelShader(thisptr, bytecode, bytecodeLength, classLinkage, pixelShader);
	clearPixelShaderReplacement(*pixelShader);

	if (bytecodeLength > 0) {
		if ('D' == bytecode[0] && 'X' == bytecode[1] && 'B' == bytecode[2] && 'C' == bytecode[3])
		{
			const unsigned *const hash = (unsigned*)(bytecode+4);

			// RGBM encode pixel shader
			if (hash[0] == 0x4fcfc7f7 && hash[1] == 0x5c5e12cf && hash[2] == 0x059e8b33 && hash[3] == 0x11f8489b)
			{
				g_alienResources.rgbmEncodePs = *pixelShader;
				return res;
			}

			// RGBM encode pixel shader
			if (hash[0] == 0x29ed6504 && hash[1] == 0x77d5438c && hash[2] == 0xe9c206c8 && hash[3] == 0xb1f27ba2)
			{
				g_alienResources.dofEncodePs = *pixelShader;
				return res;
			}

			if (hash[0] == 0x1fb3edd4 && hash[1] == 0xe984323b && hash[2] == 0x11bcf154 && hash[3] == 0x5a029c94)
			{
				g_alienResources.cameraMotionPs = *pixelShader;
				return res;
			}

			// Replace the original SMAA spatial pass by a post-sharpening filter
			if (hash[0] == 0x02b5231b && hash[1] == 0x8b3879b8 && hash[2] == 0x7db9bc8d && hash[3] == 0xf46a9d78)
			{
				g_sharpenPsHandle = ShaderRegistry::addPixelShader("sharpen_ps.hlsl");
				replacePixelShader(*pixelShader, g_sharpenPsHandle);
				return res;
			}

			// Replace the shadow-map linearize shader with one with better numerical stability. The original flickers in vanilla Alien.
			if (hash[0] == 0x3c7b9d08 && hash[1] == 0x7b3adf54 && hash[2] == 0x3bfc6b9d && hash[3] == 0x1b0ec92e)
			{
				replacePixelShader(*pixelShader, ShaderRegistry::addPixelShader("shadowLinearize_ps.hlsl"));
				return res;
			}

			// Ditto, for shadow downsampling.
			if (hash[0] == 0x8d485646 && hash[1] == 0x7d707454 && hash[2] == 0x95bf6cb1 && hash[3] == 0x09b85460)
			{
				replacePixelShader(*pixelShader, ShaderRegistry::addPixelShader("shadowDownsample_ps.hlsl"));
				return res;
			}

			// Replace the bloom merge PS
			if (hash[0] == 0xc0fbe484 && hash[1] == 0x2d952c03 && hash[2] == 0x993de248 && hash[3] == 0x56ad9263)
			{
				replacePixelShader(*pixelShader, ShaderRegistry::addPixelShader("bloomMerge_ps.hlsl"));
				return res;
			}
		}
	}

	return res;
}

// ----------------------------------------------------------------------------------------------------------------

HRESULT WINAPI CreateVertexShader_hook(void* thisptr, const char* bytecode, SIZE_T bytecodeLength, void* classLinkage, ID3D11VertexShader** vertexShader)
{
	HRESULT res = g_d3dHookOrig.CreateVertexShader(thisptr, bytecode, bytecodeLength, classLinkage, vertexShader);
	clearVertexShaderReplacement(*vertexShader);

	if (bytecodeLength > 0) {
		if ('D' == bytecode[0] && 'X' == bytecode[1] && 'B' == bytecode[2] && 'C' == bytecode[3])
		{
			const unsigned *const hash = (unsigned*)(bytecode+4);

			// Replace the VS for the main post pass to kill barrel distortion
			if (hash[0] == 0x40af2521 && hash[1] == 0x0abe7ef9 && hash[2] == 0x8cafc30b && hash[3] == 0xa48433af)
			{
				replaceVertexShader(*vertexShader, ShaderRegistry::addVertexShader("mainPost_vs.hlsl"));
				return res;
			}

#ifndef ALIASISOLATION_NO_SMAA_VS
			// SMAA vertex shader
			if (hash[0] == 0x021e3541 && hash[1] == 0xc8808c25 && hash[2] == 0x81beb6be && hash[3] == 0x342bfddd)
			{
				g_alienResources.smaaVertexShader = *vertexShader;
				return res;
			}
#endif

#ifndef ALIASISOLATION_NO_RGBM_VS
			// RGBM encode vertex shader
			if (hash[0] == 0xa851671d && hash[1] == 0xbe79cf68 && hash[2] == 0x2e6d9376 && hash[3] == 0x567ba13c)
			{
				g_alienResources.rgbmEncodeVs = *vertexShader;
				return res;
			}
#endif
		}
	}

	return res;
}
