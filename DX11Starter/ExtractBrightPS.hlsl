static const float3 GrayScaleIntensity = { 0.299f, 0.587f, 0.114f };

Texture2D ColorTexture : register(t0);
//Texture2D BloomTexture : register(t1);
SamplerState Sampler : register(s0);

cbuffer externalData : register(b0)
{
	float BloomTreshold;
};

struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD;
};

float4 main(VertexToPixel input) : SV_TARGET{

	float4 color = ColorTexture.Sample(Sampler, input.uv);
	float intensity = dot(color.rgb, GrayScaleIntensity);

	//return color;
	return (intensity > BloomTreshold ? color : float4(0, 0, 0, 1));
}