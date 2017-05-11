#pragma once
#include <DirectXMath.h>
#include "Mesh.h"
#include "Material.h"
#include "Physics.h"
using namespace DirectX;

class Entity
{
public:
	Entity(Mesh* m, std::string s, Physics *); // accept Mesh pointer and set default values
	~Entity();
	XMFLOAT4X4 GetMatrix();   // get world matrix
	void SetMatrix();         // set world matrix
	XMMATRIX GetTrans();      // get translation matrix
	void SetTrans(XMFLOAT3 p);
	XMFLOAT3 getPos();
	// set translation matrix
	XMMATRIX GetScale();      // get scale matrix
	void SetScale(XMFLOAT3 s);// set scale matrix
	XMMATRIX GetRot();        // get rotation matrix
	void SetRot(XMFLOAT3 r);          // set rotation matrix 

									  //XMMATRIX GetTransMatrix(); // get tranformation matrix
									  //void SetTransMatrix();  // set transformation matrix

									  // get methods for drawing
	//ID3D11Buffer * const GetVertexBuffer();
	//ID3D11Buffer * const GetIndexBuffer();
	//int GetIndexCount();
	Mesh* GetMesh();
	void SetMesh(Mesh * );
	Material* GetMaterial();
	std::string GetName();

	//void PrepareMaterial(XMFLOAT4X4 viewmatrix, XMFLOAT4X4 projectionmatrix);

	Physics *phy;

	XMFLOAT3 getdecalsPos();

	bool isdecal;

private:

	XMFLOAT4X4 world_matrix; // world matrix
	XMFLOAT4X4 transform_matrix; // transformation matrix

	XMFLOAT3 position; // individual position
	XMFLOAT3 rotation; // rotation vector
	XMFLOAT3 scale; // scale vector

	Mesh * mesh;  // a pointer to a Mesh object

	Material * material; // keep track of which material it will be using
	std::string name;

	XMFLOAT3 decalsPos;
};

