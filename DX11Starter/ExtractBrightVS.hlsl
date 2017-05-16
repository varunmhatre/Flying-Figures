/*struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD;
};

struct VertexShaderInput
{
	
	float3 position		: POSITION;     // XYZ position
	float3 normal       : NORMAL;
	float2 uv           : TEXCOORD;
};

VertexToPixel main(VertexShaderInput input)
{
	VertexToPixel output;
	output.position = input.position;
	output.uv = input.uv;
	return output;
}*/


struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

// The entry point for our vertex shader
VertexToPixel main(uint id : SV_VertexID)
{
	// Set up output
	VertexToPixel output;

	// Calculate the UV (0,0) to (2,2) via the ID
	output.uv = float2(
		(id << 1) & 2,  // id % 2 * 2
		id & 2);

	// Adjust the position based on the UV
	output.position = float4(output.uv, 0, 1);
	output.position.x = output.position.x * 2 - 1;
	output.position.y = output.position.y * -2 + 1;

	return output;
}