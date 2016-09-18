#pragma once
#include <Windows.h>
#include <DirectXMath.h>

using namespace DirectX;

class PostProcessSettings
{
public:
	float m_contrast;
	float m_saturation;
	XMFLOAT3 m_tint;
	BYTE Pad0[0x8];
	float m_brightness;
	float m_gamma;
	BYTE Pad1[0x8];
	float m_bloom;
	BYTE Pad6[0x18];
	float m_lenseThing;
	BYTE Pad7[0x4C];
	float m_filmGrain;
	float m_filmGrain2;
	float m_filmGrainLights;
	BYTE Pad2[0x2C];
	float m_vignetteStrength;
	float m_chromaticStrength;
	BYTE Pad3[0x8];
	float m_radialDistortFactor;
	float m_radialDistortConstraint;
	float m_radialDistortScalar;
	BYTE Pad4[0x30];
	float m_sharpness;
	BYTE Pad5[0x8];
	float m_dofStrength;
	float m_dofFocusDistance;
	float m_dofScale;
};

class CustomPostProcess
{
public:
	float m_contrast;
	float m_saturation;
	XMFLOAT3 m_tint;
	float m_brightness;
	float m_gamma;
	float m_bloom;
	float m_dofStrength;
	float m_dofFocusDistance;
	float m_dofScale;
	float m_vignetteStrength;
	float m_chromaticStrength;
	float m_sharpness;
	float m_radialDistortFactor;
	float m_radialDistortConstraint;
	float m_radialDistortScalar;
	float m_lenseThing;
	float m_filmGrain;
	float m_filmGrain2;
	float m_filmGrainLights;
};

class PostProcess
{
public:
	void PostProcessHook(void* This);
	void Toggle();

	bool* isEnabled() { return &m_enabled; }

	CustomPostProcess m_settings;

private:
	bool m_initialized;
	bool m_enabled;
};