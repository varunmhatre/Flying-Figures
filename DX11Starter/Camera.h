#pragma once

#include <DirectXMath.h>
#include <Windows.h>
#include <DirectXCollision.h>
using namespace DirectX;


class Camera
{
public:
	Camera();
	~Camera();
	void Update(float deltaTime);
	void UpdateProjMatrix(unsigned int w, unsigned int h);
	//void RotateMouseOnX(int y1, int y2);
	//void RotateMouseOnY(int x1, int x2);
	XMFLOAT4X4 GetViewMatrix();
	XMFLOAT4X4 GetProjectionMatrix();
	XMVECTOR GetCameraPosition();
	XMVECTOR  GetCameraDirection();
	void Rotate(float y, float x);
	//void Input(Camera c);
	BoundingFrustum GetFrustum();

private:
	//-----------------fields-----------------------
	// XMFLOAT4X4 WorldMatrix;   // world matrix
	// XMFLOAT3 position;        // individual position
	// XMFLOAT3 rotation;        // rotation vector
	// XMFLOAT3 scale;           // scale vector
	//----------------------------------------------

	XMFLOAT4X4 ViewMatrix;    
	XMFLOAT4X4 ProjectionMatrix;

	//-------Look To view Matrix--------
	// XMFLOAT4X4 LookToMatrix;  // "look to" matrix
	XMFLOAT3 CameraPosition;  // camera's position
	XMFLOAT3 CameraDirection; // camera's direction
	XMFLOAT4 CameraRotation;  // for camera movement
	float Xrotation;  // rotation around X axis
	float Yrotation;  // rotation around Y axis

	BoundingFrustum frustum;
};

