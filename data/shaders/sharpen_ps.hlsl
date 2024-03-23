// No HLSL available - function stub generated

SamplerState LinearSampler : register(s0);
SamplerState PointSampler  : register(s1);

Texture2D<float3> colorTex : register(t0);
Texture2D<float4> logoTex  : register(t20);

cbuffer Constants : register(b10) {
	float logoIntensity;
	float sharpenAmount;
    bool sharpenEnabled;
	float pad;
}


struct PSInput
{
	float4 param0 : SV_Position;
	float2 param1 : TEXCOORD0;
	float4 param2 : TEXCOORD1;
};

struct PSOutput
{
	float4 param0 : SV_Target0;
	float4 param1 : SV_Target1;
};


float3 linearToSrgb(float3 C_lin) {
    return
		(C_lin <= 0.0031308)
    	? (C_lin * 12.92)
    	: (1.055 * pow(max(0.0.xxx, C_lin), 1.0 / 2.4) - 0.055);
}

float calculateLuma(float3 col)
{
	return dot(float3(0.299, 0.587, 0.114), col);
}

PSOutput mainPS(in PSInput IN)
{
	PSOutput OUT = (PSOutput)0;

	uint screenWidth, screenHeight;
	colorTex.GetDimensions(screenWidth, screenHeight);
	const float2 texelSize = 1.0.xx / float2(screenWidth, screenHeight);

	float3 center = colorTex.SampleLevel(PointSampler, IN.param1 + float2(0,  0) * texelSize, 0).xyz;
    float3 outColor;
	
    if (sharpenEnabled)
    {
        float3 neighbors[4] =
        {
            colorTex.SampleLevel(PointSampler, IN.param1 + float2(1, 1) * texelSize, 0).xyz,
			colorTex.SampleLevel(PointSampler, IN.param1 + float2(-1, 1) * texelSize, 0).xyz,
			colorTex.SampleLevel(PointSampler, IN.param1 + float2(1, -1) * texelSize, 0).xyz,
			colorTex.SampleLevel(PointSampler, IN.param1 + float2(-1, -1) * texelSize, 0).xyz
        };

        float neighborDiff = 0;

		[unroll]
        for (uint i = 0; i < 4; ++i)
        {
            neighborDiff += calculateLuma(abs(neighbors[i] - center));
        }

        float sharpening = (1 - saturate(2 * neighborDiff)) * sharpenAmount;

        float3 sharpened = float3(
			0.0.xxx
			+ neighbors[0] * -sharpening
			+ neighbors[1] * -sharpening
			+ neighbors[2] * -sharpening
			+ neighbors[3] * -sharpening
			+ center * 5
		) * 1.0 / (5.0 + sharpening * -4.0);
		
        outColor = linearToSrgb(sharpened);
    }
    else
    {
        outColor = linearToSrgb(center);
    }
	
	if (logoIntensity > 0.0)
	{
		uint logoWidth, logoHeight;
		logoTex.GetDimensions(logoWidth, logoHeight);

		float4 logoColor = logoTex.Sample(PointSampler, IN.param0.xy / float2(logoWidth, logoHeight));
		outColor = lerp(outColor, logoColor.rgb, 0.75 * logoColor.a * logoIntensity);
	}

	OUT.param0 = float4(outColor, 1.0);

	return OUT;
}
