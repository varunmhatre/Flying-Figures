#pragma once

#include <DirectXMath.h>

struct DirectionalLight
{
	DirectX::XMFLOAT4 AmbientColor;
	DirectX::XMFLOAT4 DiffuseColor;
	DirectX::XMFLOAT3 Direction;
};


struct PointLight
{
	DirectX::XMFLOAT4 Color;
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 CameraPos;
};
