#include "Camera.h"


// for test
# include <iostream>
using namespace std;


Camera::Camera()
{
	// set default values
	Xrotation = 0.0f;
	Yrotation = 0.0f;
	XMStoreFloat3(&CameraPosition, XMVectorSet(0, 0, -5, 0));
	XMStoreFloat3(&CameraDirection, XMVectorSet(0, 0, 1, 0));

}


Camera::~Camera()
{
}

XMFLOAT4X4 Camera::GetViewMatrix()
{
	return ViewMatrix;
}

XMFLOAT4X4 Camera::GetProjectionMatrix()
{
	return ProjectionMatrix;
}

XMVECTOR Camera::GetCameraPosition()
{
	return XMLoadFloat3(&CameraPosition);
}

XMVECTOR Camera::GetCameraDirection()
{
	return XMLoadFloat3(&CameraDirection);
}

void Camera::Update(float deltaTime)
{
	XMVECTOR quad=XMQuaternionRotationRollPitchYaw(Xrotation,Yrotation,0);//create a rotation matrix based on the current X and Y rotation values
	XMVECTOR z=XMVectorSet(0, 0, 1, 0);      // a unit vector on the z axis
	XMVECTOR dir = XMVector3Rotate(z, quad); // apply quaternion to the default forward vector
	XMStoreFloat3(&CameraDirection, dir);    // set the direction
	XMVECTOR pos = GetCameraPosition();
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMMATRIX V = XMMatrixLookToLH(
		pos,  // camera's current position
		dir,  // direction
		up);  // up vector

	// -----for test-----
	/*cout << ViewMatrix._11 << endl;
	cout << ViewMatrix._12 << endl;
	cout << ViewMatrix._13 << endl;
	cout << ViewMatrix._14 << endl;
	cout << ViewMatrix._21 << endl;
	cout << ViewMatrix._22 << endl;
	cout << ViewMatrix._23 << endl;
	cout << ViewMatrix._24 << endl;
	cout << ViewMatrix._31 << endl;
	cout << ViewMatrix._32 << endl;
	cout << ViewMatrix._33 << endl;
	cout << ViewMatrix._34 << endl;
	cout << ViewMatrix._41 << endl;
	cout << ViewMatrix._42 << endl;
	cout << ViewMatrix._43 << endl;
	cout << ViewMatrix._44 << endl;

	cout << Xrotation << endl;*/
	// -----test ending-----

	
	XMStoreFloat4x4(&ViewMatrix, XMMatrixTranspose(V));  // store the result as the camera's current view matrix


	/* input logic here */
	if (GetAsyncKeyState('W') & 0x8000)
	{
		/* move forward */
		XMVECTOR pos = GetCameraPosition();
		XMVECTOR dir = GetCameraDirection();
		pos = XMVectorAdd(pos, dir * 2 * deltaTime);
		XMStoreFloat3(&CameraPosition, pos);
		// amount of the movement is 2
	}
	if (GetAsyncKeyState('S') & 0x8000)
	{
		/* move backwards */
		XMVECTOR pos = GetCameraPosition();
		XMVECTOR dir = GetCameraDirection();
		pos = XMVectorAdd(pos, -dir * 2 * deltaTime);
		XMStoreFloat3(&CameraPosition, pos);
		// amount of the movement is 2
	}
	if (GetAsyncKeyState('A') & 0x8000)
	{
		/* strafe left */
		XMVECTOR pos = GetCameraPosition();
		XMVECTOR dir = GetCameraDirection();
		XMVECTOR up = XMVectorSet(0, 1, 0, 0);
		XMVECTOR left = XMVector3Cross(dir, up);
		pos = XMVectorAdd(pos, left * 2 * deltaTime);
		XMStoreFloat3(&CameraPosition, pos);
		// amount of the movement is 2
	}
	if (GetAsyncKeyState('D') & 0x8000)
	{
		/* strafe right */
		XMVECTOR pos = GetCameraPosition();
		XMVECTOR dir = GetCameraDirection();
		XMVECTOR up = XMVectorSet(0, 1, 0, 0);
		XMVECTOR right = -(XMVector3Cross(dir, up));
		pos = XMVectorAdd(pos, right * 2*deltaTime);
		XMStoreFloat3(&CameraPosition, pos);
		// amount of the movement is 2
	}
	if (GetAsyncKeyState(VK_SPACE) & 0x8000)
	{
		/* move up along the world's Y axis */
		CameraPosition.y += 2 * deltaTime;
		// amount of the movement is 2
	}
	if (GetAsyncKeyState('X') & 0x8000)
	{
		/* move down along the world's Y axis */
		CameraPosition.y -= 2 * deltaTime;
		// amount of the movement is 2
	}

}

void Camera::UpdateProjMatrix(unsigned int w, unsigned int h)
{
	XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.25f * 3.1415926535f,	// Field of View Angle
		(float)w / h,	// Aspect ratio
		0.1f,				  	// Near clip plane distance
		100.0f);			  	// Far clip plane distance
	XMStoreFloat4x4(&ProjectionMatrix, XMMatrixTranspose(P)); // Transpose for HLSL! // Transpose for HLSL!

	BoundingFrustum::CreateFromMatrix(frustum, P);
}

/*
void Camera::RotateMouseOnX(int y1, int y2)
{
	float speed = 0.0005;
	Xrotation += (y1 - y2) * speed ; // y1 means current mouse position; y2 means previous position
	cout << "x"<< Xrotation << endl;
	// can be modified better: http://gamedev.stackexchange.com/questions/19507/how-should-i-implement-a-first-person-camera
}

void Camera::RotateMouseOnY(int x1, int x2)
{
	float speed = 0.002;
	Yrotation += (x1 - x2) * speed; // x1 means current mouse position; x2 means previous position
	cout << "y" << Yrotation << endl;
	// can be modified better: http://gamedev.stackexchange.com/questions/19507/how-should-i-implement-a-first-person-camera
}
*/

void Camera::Rotate(float x, float y)
{
	Xrotation += x;
	Yrotation += y;

	Xrotation = max(min(Xrotation, 1.570796327f), -1.570796327f);

	XMStoreFloat4(&CameraRotation, XMQuaternionRotationRollPitchYaw(Xrotation, Yrotation, 0));

}

BoundingFrustum Camera::GetFrustum()
{
	return frustum;
}
