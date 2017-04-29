#pragma once
#include "DXCore.h"
#include <DirectXMath.h>
using namespace DirectX;
class Physics
{
	XMFLOAT4X4 worldMatrix;
	//XMFLOAT3 pos;
	XMFLOAT3 translation;
	XMFLOAT3 rotate;
	XMFLOAT3 scale;
public:
	


	Physics();
	~Physics();

	void setTranslation(float, float, float);
	void setScale(float, float, float);
	void setRotate(float, float, float);
	XMFLOAT3 getTranslation();
	XMFLOAT3 getScale();
	XMFLOAT3 getRotate();

	void setWorld(XMFLOAT3, XMFLOAT3, XMFLOAT3);
	XMFLOAT4X4 getWorld();
};

