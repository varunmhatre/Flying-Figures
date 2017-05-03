#pragma once

#include "DXCore.h"
#include "SimpleShader.h"
#include <DirectXMath.h>
#include "Mesh.h"
#include<d3d11.h>
#include "Entity.h"
#include "Camera.h"
//#include "Material.h"
#include "Lights.h"
#include "Sound.h"
#include "Physics.h"
#include "UI.h"
#include "Scenes.h"


class Game 
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);

	// Overridden mouse input helper methods
	void OnMouseDown (WPARAM buttonState, int x, int y);
	void OnMouseUp	 (WPARAM buttonState, int x, int y);
	void OnMouseMove (WPARAM buttonState, int x, int y);
	void OnMouseWheel(float wheelDelta,   int x, int y);

	
private:

	// Initialization helper methods - feel free to customize, combine, etc.
	void LoadShaders(); 
	//void CreateMatrices();
	void CreateMeshes();
	void CreateEntities();
	void CreateCamera();
	//void CreateMaterial();
	void RenderShadowMap(); // render the shadow map
	//void CreateSound(HWND hwnd);
	//void CreateBasicGeometry();
	
	// Wrappers for DirectX shaders to provide simplified functionality
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;

	// The matrices to go from model space to screen space
	//DirectX::XMFLOAT4X4 worldMatrix;
	//DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;

	DirectX::XMFLOAT4X4 viewMatrix2;
	DirectX::XMFLOAT4X4 projectionMatrix2;

	// Keeps track of the old mouse position.  Useful for 
	// determining how far the mouse moved in a single frame.
	POINT prevMousePos;

	// Create 3 private Mesh variables
	Mesh *m1;
	Mesh *m2;
	Mesh *m3;
	// Create a new mesh using constructor overloading
	Mesh * conemesh;
	Mesh * cube;
	Mesh * cylinder;
	Mesh * helix;
	Mesh * sphere;
	Mesh * torus;
	Mesh * plane;

	// Create 5 entities
	std::vector<Entity*> E;
	std::vector<int> xpos;
	Entity * Ground;
	//Entity * E[12]; // an array of 12 entities
	//Entity * Entity_obj;

	// Create a camera
	Camera *c;

	// Create materials for different textures
	//Material * ma_metal;
	//Material * ma_concrete;

	// Create a field of DirectionalLight (no pointer)
	DirectionalLight directionalLight;
	DirectionalLight directionalLight2;
	//PointLight pointLight;

	// ----assignment 6----
	ID3D11ShaderResourceView* SRV_Metal;
	ID3D11ShaderResourceView* SRV_Concrete;
	ID3D11SamplerState* SampleState;

	ID3D11ShaderResourceView* srv;
	ID3D11ShaderResourceView* srv1;

	ID3D11ShaderResourceView* normalMapSRV;
	// --------------------

	// ---for shadow mapping---
	//ID3D11Texture2D* shadowMapTexture;		// a depth stencil texture
	int shadowMapSize;
	ID3D11DepthStencilView* DSV_Shadow;		// depth stecil view
	ID3D11ShaderResourceView* SRV_Shadow;	// shader resource view
	ID3D11SamplerState* Sampler_Shadow;		// shader sampler
	ID3D11RasterizerState* RS_Shadow;		// rasterizer state
	SimpleVertexShader* VS_Shadow;			// shadow vertex shader
	DirectX::XMFLOAT4X4 shadowViewMatrix;	// shadow view matrix
	DirectX::XMFLOAT4X4 shadowProjectionMatrix;	// shadow projection matrix
	//SimplePixelShader* PS_Shadow;
	// D3D11_VIEWPORT myViewPort;


	// adding sound!
	// Sound* sound;

	UI *scoreText[1];
	//UI *instruction;
	int score;
	wchar_t showScore[256];

	//	Scenes *scene[2];
	UI * startText[2];

	int level;

	Physics *pp[12];

	XMVECTOR dir;
};

