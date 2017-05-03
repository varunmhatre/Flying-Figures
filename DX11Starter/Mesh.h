#pragma once
#include "Vertex.h"
#include <d3d11.h>
#include <fstream>
#include <vector>
#include <DirectXMath.h>
#include <DirectXCollision.h>

using namespace DirectX;
using namespace std;

class Mesh
{

public:
	Mesh(Vertex * v, int num_v, unsigned int * i, int num_i, ID3D11Device* d, std::string s);
	Mesh( const char* objFile, ID3D11Device * d, std::string s);  // a second constructor that accepts the name of a file to load
	~Mesh();
	ID3D11Buffer * const GetVertexBuffer();
	ID3D11Buffer * const GetIndexBuffer();
	int GetIndexCount();

	Vertex * GetVertices();
	UINT * GetFaces();
	BoundingOrientedBox getOBB();
	std::string GetName();

private:
	// create two pointers for vertex buffer and index buffer
	ID3D11Buffer * vbuffer;
	ID3D11Buffer * ibuffer;
	int num_index; // how many indices are in the mesh's index buffer
	void CreateBuffers(Vertex * v, int num_v, unsigned int * i, int num_i, ID3D11Device * d);
	void CalculateTangents(Vertex* verts, int numVerts, unsigned int* indices, int numIndices);


	BoundingOrientedBox obb;
	std::vector<Vertex> verts;
	std::vector<UINT> indices;
	std::string name;
};

