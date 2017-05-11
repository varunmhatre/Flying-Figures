#include "Entity.h"



Entity::Entity(Mesh* m, std::string s, Physics *ph)
{
	this->mesh = m;
	//this->material = ma;
	// set default values for the transformations (raw position, rotation & scale values)
	
	position = XMFLOAT3(0.0f, 0.0f, 0.0f); // default individual position
	rotation = XMFLOAT3(0.0f, 0.0f, 0.0f); // default rotation vector
	scale = XMFLOAT3(1.0f, 1.0f, 1.0f); // default scale vector
										//XMMATRIX trans = XMMatrixIdentity();
										//XMMATRIX rot = XMMatrixIdentity();
										//XMMATRIX scale = XMMatrixIdentity();

										// set default for matrix
										//XMStoreFloat4x4(&transform_matrix, XMMatrixTranspose(XMMatrixIdentity()));
	XMStoreFloat4x4(&world_matrix, XMMatrixTranspose(XMMatrixIdentity())); //transpose to HLSL
	name = s;
	phy = ph;
	isdecal = false;
}

Entity::~Entity()
{
	//delete phy;

}

// get and set world matrix for individual entity
XMFLOAT4X4 Entity::GetMatrix()
{
	SetMatrix();
	return world_matrix;
}

void Entity::SetMatrix()
{
	XMStoreFloat4x4(&world_matrix, XMMatrixTranspose(GetTrans()*GetRot()*GetScale()));
}

// get and set translation matrix 
// Where translation is a 3D vector that represent the position where we want to move our space to.
// A translation matrix leaves all the axis rotated exactly as the active space.
XMMATRIX Entity::GetTrans()
{
	return XMMatrixTranslation(position.x, position.y, position.z);
}
void Entity::SetTrans(XMFLOAT3 p)
{
	position = p;
	XMStoreFloat4x4(&world_matrix, XMMatrixTranspose(GetTrans()));
	decalsPos = p;
	decalsPos.z = p.z - 2;

}

std::string Entity::GetName()
{
	return name;
}

XMFLOAT3 Entity::getPos()
{
	return position;
}


XMFLOAT3 Entity::getdecalsPos()
{
	return decalsPos;
}

// get and set scale matrix
// Where scale is a 3D vector that represent the scale along each axis
XMMATRIX Entity::GetScale()
{
	return XMMatrixScaling(scale.x, scale.y, scale.z);
}
void Entity::SetScale(XMFLOAT3 s)
{
	scale = s;
	XMStoreFloat4x4(&world_matrix, XMMatrixTranspose(GetScale()));
}

// get and set rotation matrix
// Where rotation is the angle we want to use for our rotation.
XMMATRIX Entity::GetRot()
{

	return XMMatrixRotationX(rotation.x)*XMMatrixRotationY(rotation.y)*XMMatrixRotationZ(rotation.z);

}
void Entity::SetRot(XMFLOAT3 r)
{
	rotation = r;
	XMStoreFloat4x4(&world_matrix, XMMatrixTranspose(GetRot()));
}


// "get" methods for drawing
/*
ID3D11Buffer * const Entity::GetVertexBuffer()
{
	return mesh->vbuffer;
}
ID3D11Buffer * const Entity::GetIndexBuffer()
{
	return mesh->ibuffer;
}
int Entity::GetIndexCount()
{
	return mesh->num_index;
}
*/
Mesh* Entity::GetMesh() 
{
	return mesh;
}

void Entity::SetMesh(Mesh * m)
{
	mesh = m;
}

Material* Entity::GetMaterial()
{
	return material;
}

//void Entity::PrepareMaterial(XMFLOAT4X4 viewmatrix, XMFLOAT4X4 projectionmatrix)
//{
//	// pass matrices into materials' simple vertex shader
//	material ->GetVertexShader()->SetMatrix4x4("world", this->GetMatrix());
//	material ->GetVertexShader()->SetMatrix4x4("view", viewmatrix);
//	material ->GetVertexShader()->SetMatrix4x4("projection", projectionmatrix);
//
//	// Set the vertex and pixel shaders to use for the next Draw() command
//	//  - These don't technically need to be set every frame...YET
//	//  - Once you start applying different shaders to different objects,
//	//    you'll need to swap the current shaders before each draw
//	material->GetVertexShader()->CopyAllBufferData();
//	material->GetVertexShader()->SetShader();
//	// ----assignment 6----
//	// pass the texture and sampler state to the material's simple vertex shader
//	material->GetPixelShader()->SetShaderResourceView("Texture", this->GetMaterial()->GetSRV());
//	material->GetPixelShader()->SetShaderResourceView("Shadow", this->GetMaterial()->GetSRV()); // final project -shadow mapping
//	material->GetPixelShader()->SetSamplerState("Sampler", this->GetMaterial()->GetSSP());
//	// the methods above take the name of the variable in the shader, and then the actual data to set
//	// --------------------
//
//
//	// Once you've set all of the data you care to change for
//	// the next draw call, you need to actually send it to the GPU
//	//  - If you skip this, the "SetMatrix" calls above won't make it to the GPU!
//
//	material->GetPixelShader()->CopyAllBufferData();
//	material->GetPixelShader()->SetShader();
//
//}