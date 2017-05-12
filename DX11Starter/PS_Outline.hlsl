
// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage


SamplerState Sampler	: register(s0);
// --------------------

struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 position		: SV_POSITION;
	float3 normal		: NORMAL;
	float3 worldPos		: WORLDPOS;

};

float4 main(VertexToPixel input) : SV_TARGET
{
	input.normal = normalize(input.normal);

	return float4(1,1,0,1);

}

