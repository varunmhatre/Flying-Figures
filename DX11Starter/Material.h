#pragma once
#include "SimpleShader.h"
#include <DirectXMath.h>
using namespace DirectX;

class Material
{

public:
	Material(SimplePixelShader* p, SimpleVertexShader* v, ID3D11ShaderResourceView* srv, ID3D11SamplerState* ssp);
	~Material();
	SimplePixelShader* GetPixelShader();
	SimpleVertexShader* GetVertexShader();

	ID3D11ShaderResourceView* GetSRV();
	ID3D11SamplerState* GetSSP();

private:
	SimplePixelShader* PixelShader;// pixel shader pointer
	SimpleVertexShader* VertexShader;// vertex shader pointer
	ID3D11ShaderResourceView* SRV;  // shader resource view pointer
	ID3D11SamplerState*  SSP; // sampler state pointer
};

