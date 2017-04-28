

struct DirectionalLight
{
	float4 AmbientColor;
	float4 DiffuseColor;
	float3 Direction;
};

struct PointLight {
	float4 pointLightColor;
	float3 pointLightPosition;
	float i;
	float3 cameraPosition;
};

cbuffer lightData : register(b0)
{
	DirectionalLight directionalLight2;
	PointLight pl;
};


Texture2D Texture		: register(t0);
Texture2D ShadowMap		: register(t1);
SamplerState Sampler	: register(s0);
SamplerComparisonState ShadowSampler : register(s1);

Texture2D diffuseTexture  : register(t2);
Texture2D projectionTexture  : register(t3);
Texture2D NormalMap		: register(t4);

struct VertexToPixel
{

	float4 position		: SV_POSITION;
	float3 normal		: NORMAL;
	float3 worldPos		: WORLDPOS;
	float2 uv			: TEXCOORD;      // uv coordination
	float4 posForShadow	: TEXCOORD1;
	float4 viewposition : TEXCOORD2;
	float3 tangent		: TANGENT;

};

float4 directionallightcalculation(float3 n, DirectionalLight light)
{
	float3 ToLight = -light.Direction;
	float lightAmount = saturate(dot(n, ToLight));
	float4 result = light.DiffuseColor * lightAmount + light.AmbientColor;
	return result;
}

// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	float2 projectTexCoord;
	float4 projectionColor;

	projectTexCoord.x = input.viewposition.x / input.viewposition.w / 2.0f + 0.5f;
	projectTexCoord.y = -input.viewposition.y / input.viewposition.w / 2.0f + 0.5f;


	float4 surfaceColor = diffuseTexture.Sample(Sampler, input.uv);

	float4 textureColor = Texture.Sample(Sampler, input.uv);

	input.normal = normalize(input.normal);
	input.tangent = normalize(input.tangent);


	float3 normalFromMap = NormalMap.Sample(Sampler, input.uv).xyz * 2 - 1;

	// Transform from tangent to world space
	float3 N = input.normal;
	float3 T = normalize(input.tangent - N * dot(input.tangent, N));
	float3 B = cross(T, N);

	float3x3 TBN = float3x3(T, B, N);
	input.normal = normalize(mul(normalFromMap, TBN));


	if ((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y))
	{

		// Sample the color value from the projection texture using the sampler at the projected texture coordinate location.
		projectionColor = projectionTexture.Sample(Sampler, projectTexCoord);

		// Set the output color of this pixel to the projection texture overriding the regular color value.
		if (projectionColor.x != 1 && projectionColor.y != 1 && projectionColor.z != 1)
			textureColor = projectionColor;
	}

	
	 float4 second_directional = directionallightcalculation(input.normal, directionalLight2);
	 
	//directional light 
	float lightAmountDL = saturate(dot(input.normal, normalize(-directionalLight2.Direction)));
	
	float4 dlight = directionalLight2.DiffuseColor* lightAmountDL*textureColor + directionalLight2.AmbientColor;
	
	//point light
	float3 dirToPointLight = normalize(pl.pointLightPosition - input.worldPos);
	float lightAmountPL = saturate(dot(input.normal, dirToPointLight));
	//float4 plight = pl.pointLightColor * lightAmountPL*surfaceColor;
	float4 plight = pl.pointLightColor * lightAmountPL*textureColor;
	// Specular highlight for point light
	float3 toCamera = normalize(pl.cameraPosition - input.worldPos);
	float3 refl = reflect(-dirToPointLight, N);
	float specular = pow(saturate(dot(refl, toCamera)), 28);


	 // shadow stuff

	 // Figure out this pixel's UV in the SHADOW MAP
	 float2 shadowUV = input.posForShadow.xy / input.posForShadow.w * 0.5f + 0.5f;
	 shadowUV.y = 1.0f - shadowUV.y; // Flip the Y since UV coords and screen coords are different

									 // Calculate this pixel's actual depth from the light
	 float depthFromLight = input.posForShadow.z / input.posForShadow.w;

	 // Sample the shadow map
	 float shadowAmount = ShadowMap.SampleCmpLevelZero(ShadowSampler, shadowUV, depthFromLight);

	 float4 diffuseColor = Texture.Sample(Sampler, input.uv);


	 float4 result = shadowAmount*diffuseColor;

	 return result;


}
