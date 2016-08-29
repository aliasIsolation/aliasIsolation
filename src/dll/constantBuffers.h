#pragma once

#include "glm/glm.hpp"


struct CbDefaultXSC {
	glm::mat4 ViewProj;
	glm::mat4 ViewMatrix;
	glm::mat4 SecondaryProj;
	glm::mat4 SecondaryViewProj;
	glm::mat4 SecondaryInvViewProj;
	glm::vec4 ConstantColour;
	glm::vec4 Time;
	glm::vec4 CameraPosition;
	glm::vec4 InvProjectionParams;
	glm::vec4 SecondaryInvProjectionParams;
	glm::vec4 ShaderDebugParams;
	glm::vec4 ToneMappingDebugParams;
	glm::vec4 HDR_EncodeScale2;
	glm::vec4 EmissiveSurfaceMultiplier;
	glm::vec4 AlphaLight_OffsetScale;
	glm::vec4 TessellationScaleFactor;
	glm::vec4 FogNearColour;
	glm::vec4 FogFarColour;
	glm::vec4 FogParams;
	glm::vec4 AdvanceEnvironmentShaderDebugParams;
	glm::vec4 SMAA_RTMetrics;
};

struct CbDefaultVSC {
	glm::mat4 ProjMatrix;
	glm::mat4 TextureTransform;
	glm::mat4 InvViewProj;
	glm::mat4 PrevViewProj;
	glm::mat4 PrevSecViewProj;
	glm::vec4 TextureScaleBias;
	glm::vec4 RenderTargetSizeVS;
	glm::vec4 RenderTargetResolutionVS;
	glm::vec4 MorphTargetParams;
	glm::vec4 OnePixelStepForS0VS;
	glm::vec4 ScreenRes;
	glm::vec4 ClearSurfaceColor;
	glm::vec4 ClearSurfaceDepth;
	glm::vec4 VertexDepthClamp;
	glm::vec4 FlareScreenAspect;
	glm::vec4 DecalParams;
	glm::vec4 UserClipPlane0;
	glm::vec4 UserClipPlane1;
	glm::vec4 UserClipPlane2;
	glm::vec4 UserClipPlane3;
	glm::vec4 DecalScaleParams;
	glm::vec4 MipLevel;
};

struct CbDefaultPSC {
	glm::mat4 AlphaLight_WorldtoClipMatrix;
	glm::mat4 AlphaLight_CliptoWorldMatrix;
	glm::mat4 ProjectorMatrix;
	glm::mat4 MotionBlurCurrInvViewProjection;
	glm::mat4 MotionBlurPrevViewProjection;
	glm::mat4 MotionBlurPrevSecViewProjection;
	glm::mat4 Spotlight0_Transform;
	glm::vec4 TextureAnimation;
	glm::vec4 AmbientDiffuse;
	glm::vec4 EnvironmentDebugParams;
	glm::vec4 ShadowFilterESMWeights;
	glm::vec4 LegacyDofParams;
	glm::vec4 OnePixelStepForS0;
	glm::vec4 RenderTargetSize;
	glm::vec4 ModelTrackerID;
	glm::vec4 Sharpness_Bloom_Controls;
	glm::vec4 Blur_Direction;
	glm::vec4 LightMeterDebugParams;
	glm::vec4 SourceResolution;
	glm::vec4 HDR_EncodeScale;
	glm::vec4 OutputGamma;
	glm::vec4 AlphaLight_ScaleParams;
	glm::vec4 WrinkleMapWeights;
	glm::vec4 RadiosityCubeMapIdx;
	glm::vec4 HairShadingParams;
	glm::vec4 SkinShadingParams;
	glm::vec4 HDR_EncodeScale3;
	glm::vec4 ScreenResolution;
	glm::vec4 VelocityParams;
	glm::vec4 DeferredConstColor;
	glm::vec4 Spotlight0_Shadow_Params;
	glm::vec4 Spotlight0_ShadowMapDimensions;
	glm::vec4 ShadowFilterType;
	glm::vec4 Spotlight0_ReverseZPerspective;
	glm::vec4 SpacesuitVisorParams;
	glm::vec4 Directional_Light_Colour;
	glm::vec4 Directional_Light_Direction;
	glm::vec4 EnvironmentMap_Params;
	glm::vec4 Spotlight0_Shadow_Bias_GoboScale;
	glm::vec4 ScreenSpaceLightShaftParams1;
	glm::vec4 ScreenSpaceLightShaftParams2;
	glm::vec4 ScreenSpaceLightPosition;
	glm::vec4 LensAndScreenCenter;
	glm::vec4 ScaleAndScaleIn;
	glm::vec4 HmdWarpParam;
	glm::vec4 ChromAbParam;
	glm::vec4 SMAA_SubsampleIndices;
};
