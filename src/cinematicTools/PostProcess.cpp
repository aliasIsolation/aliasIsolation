#include "PostProcess.h"
#include "Tools\Log.h"

void PostProcess::Toggle()
{
	m_enabled = !m_enabled;
	Log::Write("Post process override: " + to_string(m_enabled));
}

void PostProcess::PostProcessHook(void* This)
{
	if (m_enabled)
	{
		PostProcessSettings* settings = (PostProcessSettings*)((int)This + 0x1918);
		if (!m_initialized)
		{
			m_settings.m_bloom = settings->m_bloom;
			m_settings.m_brightness = settings->m_brightness;
			m_settings.m_contrast = settings->m_contrast;
			m_settings.m_dofFocusDistance = settings->m_dofFocusDistance;
			m_settings.m_dofScale = settings->m_dofScale;
			m_settings.m_dofStrength = settings->m_dofStrength;
			m_settings.m_gamma = settings->m_gamma;
			m_settings.m_saturation = settings->m_saturation;
			m_settings.m_tint = settings->m_tint;
			m_settings.m_vignetteStrength = settings->m_vignetteStrength;
			m_settings.m_chromaticStrength = settings->m_chromaticStrength;
			m_settings.m_sharpness = settings->m_sharpness;
			m_settings.m_radialDistortFactor = settings->m_radialDistortFactor;
			m_settings.m_radialDistortConstraint = settings->m_radialDistortConstraint;
			m_settings.m_radialDistortScalar = settings->m_radialDistortScalar;
			m_settings.m_lenseThing = settings->m_lenseThing;
			m_settings.m_filmGrain = settings->m_filmGrain;
			m_settings.m_filmGrain2 = settings->m_filmGrain2;
			m_settings.m_filmGrainLights = settings->m_filmGrainLights;
			m_initialized = true;
		}
		settings->m_bloom = m_settings.m_bloom;
		settings->m_brightness = m_settings.m_brightness;
		settings->m_contrast = m_settings.m_contrast;
		settings->m_dofFocusDistance = m_settings.m_dofFocusDistance;
		settings->m_dofScale = m_settings.m_dofScale;
		settings->m_dofStrength = m_settings.m_dofStrength;
		settings->m_gamma = m_settings.m_gamma;
		settings->m_saturation = m_settings.m_saturation;
		settings->m_tint = m_settings.m_tint;
		settings->m_vignetteStrength = m_settings.m_vignetteStrength;
		settings->m_chromaticStrength = m_settings.m_chromaticStrength;
		settings->m_sharpness = m_settings.m_sharpness;
		settings->m_radialDistortFactor = m_settings.m_radialDistortFactor;
		settings->m_radialDistortConstraint = m_settings.m_radialDistortConstraint;
		settings->m_radialDistortScalar = m_settings.m_radialDistortScalar;
		settings->m_lenseThing = m_settings.m_lenseThing;
		settings->m_filmGrain = m_settings.m_filmGrain;
		settings->m_filmGrain2 = m_settings.m_filmGrain2;
		settings->m_filmGrainLights = m_settings.m_filmGrainLights;
	}
}