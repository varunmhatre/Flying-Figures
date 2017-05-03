#include "Physics.h"



Physics::Physics()
{
	translation = XMFLOAT3(0, 0, 0);
	scale = XMFLOAT3(0, 0, 0);
	rotate = XMFLOAT3(0, 0, 0);
}


Physics::~Physics()
{
}

void Physics::setTranslation(float x, float y, float z) {
	//x = x - 3.65;
	//y = -0.1*x*x + 1;  //parabola to simulate gravity

	x = sin(x / 1.9f)*5.0f;			//circle to have objects appear again in cycles
	y = cos(y / 1.9f)*5.0f - 4.5f;	//circle to have objects appear again in cycles

	translation = XMFLOAT3(x, y, z);
	//XMMATRIX translation = XMMatrixTranslation(x, y, z);
	//XMStoreFloat4x4(&translationMatrix, translation);
}

void Physics::setScale(float x, float y, float z) {
	scale = XMFLOAT3(x, y, z);
	//XMMATRIX scale = XMMatrixScaling(x, y, z);
	//XMStoreFloat4x4(&scaleMatrix, scale);
}
void Physics::setRotate(float x, float y, float z) {
	rotate = XMFLOAT3(x, y, z);
	//XMMATRIX scale = XMMatrixScaling(x, y, z);
	//XMStoreFloat4x4(&scaleMatrix, scale);
}
XMFLOAT3 Physics::getTranslation() {
	return translation;
}
XMFLOAT3 Physics::getScale() {
	return scale;
}
XMFLOAT3 Physics::getRotate() {
	return rotate;
}

void Physics::setWorld(XMFLOAT3 sm, XMFLOAT3 rm, XMFLOAT3 tm) {
	XMMATRIX translation = XMMatrixTranslation(tm.x, tm.y, tm.z);
	XMMATRIX scale = XMMatrixScaling(sm.x, sm.y, sm.z);
	XMMATRIX rotation = XMMatrixRotationZ(rm.z);
	rotation = XMMatrixMultiply(rotation, XMMatrixRotationX(rm.x));
	rotation = XMMatrixMultiply(rotation, XMMatrixRotationY(rm.y));

	XMVECTOR rot = XMLoadFloat3(&rm);
	XMMATRIX rotation0 = XMMatrixRotationRollPitchYawFromVector(rot);

	//XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(scale * rotation * translation));
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(scale * rotation0 * translation));
}


XMFLOAT4X4 Physics::getWorld() {

	return worldMatrix;
}
