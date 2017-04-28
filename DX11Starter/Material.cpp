#include "Material.h"



Material::Material(SimplePixelShader* p, SimpleVertexShader* v, ID3D11ShaderResourceView* srv, ID3D11SamplerState* ssp)
{
	PixelShader = p;
	VertexShader = v;
	SRV = srv;
	SSP = ssp;
}


Material::~Material()
{
}

SimplePixelShader* Material::GetPixelShader()
{
	return PixelShader;
}

SimpleVertexShader* Material::GetVertexShader()
{
	return VertexShader;
}

ID3D11ShaderResourceView* Material::GetSRV()
{
	return SRV;
}

ID3D11SamplerState* Material::GetSSP()
{
	return SSP;
}


