#include "Game.h"
#include "Vertex.h"
#include "Statemachine.h"

#include "WICTextureLoader.h"  // for loading textures

// for test
#include <iostream>
using namespace std;

// For the DirectX Math library
using namespace DirectX;


// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// DirectX itself, and our window, are not ready yet!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore( 
		hInstance,		   // The application's handle
		"DirectX Game",	   // Text for the window's title bar
		1280,			   // Width of the window's client area
		720,			   // Height of the window's client area
		true)			   // Show extra stats (fps) in title bar?
{
	// Initialize fields
	vertexShader = 0;
	pixelShader = 0;
	
	//sound = 0;

#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.");
#endif
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Release all DirectX objects created here
//  - Delete any objects to prevent memory leaks
// --------------------------------------------------------
Game::~Game()
{
	// Release any (and all!) DirectX objects
	// we've made in the Game class

	// Delete our simple shader objects, which
	// will clean up their own internal DirectX stuff
	
	delete m1;
	delete m2;
	delete m3;
	delete conemesh;
	delete cube;
	delete cylinder;
	delete helix;
	delete sphere;
	delete torus;
	delete plane;
	for (int i = 0; i < 12; i++)
		delete E[i];
	//delete Entity_obj;
	delete c;
	//delete ma_metal;
	//delete ma_concrete;
	delete vertexShader;
	delete pixelShader;
	SRV_Concrete->Release();
	SRV_Metal->Release();
	SampleState->Release(); 
	srv->Release();
	srv1->Release();
	normalMapSRV->Release();
	
	// release the sound object
	/*if (sound)
	{
		sound->Shutdown();
		delete sound;
	}*/

	//clean up the shadow mapping stuffs

	DSV_Shadow->Release();
	SRV_Shadow->Release();
	RS_Shadow->Release();
	Sampler_Shadow->Release();
	delete VS_Shadow;

	delete scoreText[0];
	delete startText[0];
	delete startText[1];

}

// --------------------------------------------------------
// Called once per program, after DirectX and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later


	shadowMapSize = 1024;
	LoadShaders();
	CreateCamera();
	/*CreateSound(hWnd);*/
	//CreateMaterial();
	CreateMeshes();
	CreateEntities(); // the third assignment

	// ----load texture----

	CreateWICTextureFromFile(device, context, L"Assets/Textures/Tile.jpg",0, &SRV_Metal);
	CreateWICTextureFromFile(device, context, L"Assets/Textures/DamageConcrete.jpg", 0, &SRV_Concrete);
	CreateWICTextureFromFile(device, context, L"Assets/Textures/rock.jpg", 0, &srv);
	CreateWICTextureFromFile(device, context, L"Assets/Textures/03.png", 0, &srv1);
	CreateWICTextureFromFile(device, context, L"Assets/Textures/rockNormals.jpg", 0, &normalMapSRV);
	//CreateWICTextureFromFile(device, context, L"Assets/Texture/shadow_cube.png",0,&SRV_Shadow);
	// --------------------
	score = 20;
	swprintf_s(showScore, L"%d", score);

	scoreText[0] = new UI(device, context, showScore, XMFLOAT4(+0.0f, +0.0f, +110.0f, 110.0f));

	startText[0] = new UI(device, context, L"FLYING FIGURES", XMFLOAT4(+200.0f, +100.0f, +110.0f, 110.0f));
	startText[1] = new UI(device, context, L"Press enter to play", XMFLOAT4(+200.0f, +150.0f, +110.0f, 110.0f));
	// ----sampler code----

	// First, create a description
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;  // how to handle addresses outside the 0-1 UV range
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;  // D3D11_TEXTURE_ADDRESS_WRAP is a usual value ( wrapping textures)
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;  // how to handle sampling "between" pixels ;
												//D3D11_FILTER_MIN_MAG_MIP_LINEAR is usual (trilinear filtering)
	sampDesc.MaxAnisotropy = 16;

	// Now, create the sampler from the description
	device->CreateSamplerState(&sampDesc, &SampleState);
	// -------------------------

	
	// shadow mapping stuff

	// Set up the description of the depth stencil texture
	D3D11_TEXTURE2D_DESC shadowMapTexDesc = {};
	shadowMapTexDesc.Width = shadowMapSize; // texture width
	shadowMapTexDesc.Height = shadowMapSize; // texture height
	shadowMapTexDesc.MipLevels = 1;
	shadowMapTexDesc.ArraySize = 1;
	shadowMapTexDesc.Format = DXGI_FORMAT_R32_TYPELESS;  //??? difference with R24G8
	shadowMapTexDesc.SampleDesc.Count = 1;
	shadowMapTexDesc.SampleDesc.Quality = 0;
	shadowMapTexDesc.Usage = D3D11_USAGE_DEFAULT;
	shadowMapTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowMapTexDesc.CPUAccessFlags = 0;
	shadowMapTexDesc.MiscFlags = 0;
	ID3D11Texture2D* shadowTexture;
	device->CreateTexture2D(&shadowMapTexDesc, 0, &shadowTexture);

	// Set up the depth stencil view description --> for using this texture "as a depth buffer"
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
	descDSV.Format = DXGI_FORMAT_D32_FLOAT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	device->CreateDepthStencilView(shadowTexture, &descDSV, &DSV_Shadow);
	
	// Set up the shader resource view description --> for sampling from the texture in a pixel shader
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	device->CreateShaderResourceView(shadowTexture, &srvDesc, &SRV_Shadow);

	// Release the texture reference since we don't need it
	shadowTexture->Release();

	// Create the special "comparison" sampler state for shadows
	D3D11_SAMPLER_DESC shadowSampDesc = {};
	shadowSampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR; // Could be anisotropic
	shadowSampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	shadowSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.BorderColor[0] = 1.0f;
	shadowSampDesc.BorderColor[1] = 1.0f;
	shadowSampDesc.BorderColor[2] = 1.0f;
	shadowSampDesc.BorderColor[3] = 1.0f;
	device->CreateSamplerState(&shadowSampDesc, &Sampler_Shadow);

	// Create a rasterizer state
	D3D11_RASTERIZER_DESC shadowRastDesc = {};
	shadowRastDesc.FillMode = D3D11_FILL_SOLID;
	shadowRastDesc.CullMode = D3D11_CULL_BACK;
	shadowRastDesc.DepthClipEnable = true;
	shadowRastDesc.DepthBias = 1000; // Multiplied by (smallest possible value > 0 in depth buffer)
	shadowRastDesc.DepthBiasClamp = 0.0f;
	shadowRastDesc.SlopeScaledDepthBias = 1.0f;
	device->CreateRasterizerState(&shadowRastDesc, &RS_Shadow);

	// set up the viewport
	//myViewPort.Width = (float)1024;
	//myViewPort.Height = (float)1024;
	//myViewPort.MinDepth = 0.0f;
	//myViewPort.MaxDepth = 1.0f;
	//myViewPort.TopLeftX = 0.0f;
	//myViewPort.TopLeftY = 0.0f;

	// finishing the shadow mapping depth buffer thing??
	
	/*
	// shadow mappign stuff-->setting render states
	// set null render target and clear the depth buffer
	ID3D11RenderTargetView* renderTarget[1] = { 0 };
	context->OMSetRenderTargets(1, renderTarget, DSV_Shadow);
	context->ClearDepthStencilView(DSV_Shadow, D3D11_CLEAR_DEPTH, 1.0f, 0); // clear the depth buffer
	// set the viewport
	context->RSSetViewports(1, &myViewPort);
	// Set up the rasterize state
	D3D11_RASTERIZER_DESC rsDesc = {};
	device->CreateRasterizerState(&rsDesc, &RS_Shadow);
	*/


	
	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	/*
	directionalLight.AmbientColor = XMFLOAT4(0.1, 0.1, 0.1, 0.1);
	directionalLight.DiffuseColor = XMFLOAT4(0, 0, 1, 1);  // blue
	directionalLight.Direction = XMFLOAT3(1, -1, 0);
	*/

	directionalLight2.AmbientColor = XMFLOAT4(0.1, 0.1, 0.1, 0.1);
	directionalLight2.DiffuseColor = XMFLOAT4(1, 1, 0, 1);  // yellow
	directionalLight2.Direction = XMFLOAT3(0, -1, 0);

	/*
	pointLight.Color = XMFLOAT4(1,0.1f,0.1f,1);  
	pointLight.Position = XMFLOAT3(0,2, 0);
	XMFLOAT3 cpos;
	XMStoreFloat3(&cpos, c->GetCameraPosition());
	pointLight.CameraPos = cpos;
	// for test: cout << pointLight.CameraPos.x << pointLight.CameraPos.y << pointLight.CameraPos.z << endl;
	*/
	
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files using
// my SimpleShader wrapper for DirectX shader manipulation.
// - SimpleShader provides helpful methods for sending
//   data to individual variables on the GPU
// --------------------------------------------------------
void Game::LoadShaders()
{

	vertexShader = new SimpleVertexShader(device, context);
	if (!vertexShader->LoadShaderFile(L"Debug/VertexShader.cso"))
		vertexShader->LoadShaderFile(L"VertexShader.cso");		

	pixelShader = new SimplePixelShader(device, context);
	if(!pixelShader->LoadShaderFile(L"Debug/PixelShader.cso"))	
		pixelShader->LoadShaderFile(L"PixelShader.cso");

	// You'll notice that the code above attempts to load each
	// compiled shader file (.cso) from two different relative paths.

	// This is because the "working directory" (where relative paths begin)
	// will be different during the following two scenarios:
	//  - Debugging in VS: The "Project Directory" (where your .cpp files are) 
	//  - Run .exe directly: The "Output Directory" (where the .exe & .cso files are)

	// Checking both paths is the easiest way to ensure both 
	// scenarios work correctly, although others exist

	
	// shadow mapping stuff
	VS_Shadow = new SimpleVertexShader(device, context);
	if (!VS_Shadow->LoadShaderFile(L"Debug/VS_Shadow.cso"))
		VS_Shadow->LoadShaderFile(L"VS_Shadow.cso");

	//context->PSSetShader(0, 0, 0);


	// ??? PS_Shadow = new SimplePixelShader(device, context);  // no pixel shader??
	
}

void Game::CreateCamera()
{
	c = new Camera();
	c->UpdateProjMatrix(width, height);

	XMMATRIX shView = XMMatrixLookAtLH(
		XMVectorSet(0, 20, -20, 0),
		XMVectorSet(0, 0, 0, 0),
		XMVectorSet(0, 1, 0, 0));
	XMStoreFloat4x4(&shadowViewMatrix, XMMatrixTranspose(shView));

	// Orthographic to match the directional light
	XMMATRIX shProj = XMMatrixOrthographicLH(10, 10, 0.1f, 100.0f);
	XMStoreFloat4x4(&shadowProjectionMatrix, XMMatrixTranspose(shProj));
}

//void Game::CreateMaterial()
//{
//	// pass in the texture and sampler state you have made above before drawing it
//	ma_metal = new Material(pixelShader,vertexShader, SRV_Metal, SampleState);
//	ma_concrete = new Material(pixelShader, vertexShader, SRV_Concrete, SampleState);
//
//}


// --------------------------------------------------------
// Initializes the matrices necessary to represent our geometry's 
// transformations and our 3D camera
// --------------------------------------------------------
//void Game::CreateMatrices()
//{
	// Set up world matrix
	// - In an actual game, each object will need one of these and they should
	//   update when/if the object moves (every frame)
	// - You'll notice a "transpose" happening below, which is redundant for
	//   an identity matrix.  This is just to show that HLSL expects a different
	//   matrix (column major vs row major) than the DirectX Math library
	//XMMATRIX W = XMMatrixIdentity();
	//XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(W)); // Transpose for HLSL!
	// Create the View matrix
	// - In an actual game, recreate this matrix every time the camera 
	//    moves (potentially every frame)
	// - We're using the LOOK TO function, which takes the position of the
	//    camera and the direction vector along which to look (as well as "up")
	// - Another option is the LOOK AT function, to look towards a specific
	//    point in 3D space
	//XMVECTOR pos = XMVectorSet(0, 0, -5, 0);
	//XMVECTOR dir = XMVectorSet(0, 0, 1, 0);
	//XMVECTOR up  = XMVectorSet(0, 1, 0, 0);
	//XMMATRIX V   = XMMatrixLookToLH(
	//	pos,     // The position of the "camera"
	//	dir,     // Direction the camera is looking
	//	up);     // "Up" direction in 3D space (prevents roll)
	//XMStoreFloat4x4(&viewMatrix, XMMatrixTranspose(V)); // Transpose for HLSL!
	// Create the Projection matrix
	// - This should match the window's aspect ratio, and also update anytime
	//   the window resizes (which is already happening in OnResize() below)
	//XMMATRIX P = XMMatrixPerspectiveFovLH(
	//	0.25f * 3.1415926535f,		// Field of View Angle
	//	(float)width / height,		// Aspect ratio
	//	0.1f,						// Near clip plane distance
	//	100.0f);					// Far clip plane distance
	//XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(P)); // Transpose for HLSL!
//}



// Creates three Mesh objects, with different geometry, in the Game class
void Game::CreateMeshes()
{
	// create the first mesh
	Vertex v1[]=
{ 
	{ XMFLOAT3(+0.0f, +1.0f, +0.0f), XMFLOAT2(+0.0f,+0.0f), XMFLOAT3(+0.0f,+0.0f,-1.0f) },
	{ XMFLOAT3(+1.5f, -1.0f, +0.0f),  XMFLOAT2(+0.0f,+0.0f), XMFLOAT3(+0.0f,+0.0f,-1.0f) },
	{ XMFLOAT3(-1.5f, -1.0f, +0.0f),  XMFLOAT2(+0.0f,+0.0f), XMFLOAT3(+0.0f,+0.0f,-1.0f) },
};
	unsigned int i1[] = { 0, 1, 2 };
	Vertex * x1 = v1;
	unsigned int * y1 = i1;
	m1 = new Mesh(x1, 3, y1, 3, device);

	// create the second mesh
	Vertex v2[] =
	{
		{ XMFLOAT3(+1.0f, +1.0f, +0.0f),  XMFLOAT2(+0.0f,+0.0f),  XMFLOAT3(+0.0f,+0.0f,-1.0f) },
		{ XMFLOAT3(+2.0f, +1.0f, +0.0f),  XMFLOAT2(+0.0f,+0.0f),  XMFLOAT3(+0.0f,+0.0f,-1.0f) },
		{ XMFLOAT3(+1.0f, +2.0f, +0.0f),  XMFLOAT2(+0.0f,+0.0f),  XMFLOAT3(+0.0f,+0.0f,-1.0f) },
	};
	Vertex * x2 = v2;
	unsigned int i2[] = { 2, 1, 0 };
	unsigned int * y2 = i2;
	m2 = new Mesh(x2, 3, y2, 3, device);

	// create the third mesh
	Vertex v3[] =
	{
		{ XMFLOAT3(-1.0f, +1.0f, +0.0f),  XMFLOAT2(+0.0f,+0.0f),  XMFLOAT3(+0.0f,+0.0f,-1.0f) },
		{ XMFLOAT3(-2.0f, +1.0f, +0.0f),  XMFLOAT2(+0.0f,+0.0f),  XMFLOAT3(+0.0f,+0.0f,-1.0f) },
		{ XMFLOAT3(-1.0f, +2.0f, +0.0f),  XMFLOAT2(+0.0f,+0.0f),  XMFLOAT3(+0.0f,+0.0f,-1.0f) },
	};
	Vertex * x3 = v3;	
	unsigned int i3[] = { 0, 1, 2 };
	unsigned int * y3 = i3;
	m3 = new Mesh(x3, 3, y3, 3, device);

	// create new meshes using third mesh constructor
	conemesh = new Mesh("Assets/Models/cone.obj", device);
	cube = new Mesh("Assets/Models/cube.obj", device);
	cylinder = new Mesh("Assets/Models/cylinder.obj", device);
	helix = new Mesh("Assets/Models/helix.obj", device);
	sphere = new Mesh("Assets/Models/sphere.obj", device);
	torus = new Mesh("Assets/Models/torus.obj", device);
	plane = new Mesh("Assets/Models/plane.obj", device);

}

void Game::CreateEntities()
{

	E[0] = new Entity(cube, "1");
	E[1] = new Entity(cylinder, "2");
	E[2] = new Entity(conemesh, "3");
	E[3] = new Entity(sphere, "4");
	E[4] = new Entity(sphere, "5");
	E[5] = new Entity(conemesh, "6");
	E[6] = new Entity(cube, "7");
	E[7] = new Entity(cylinder, "8");
	E[8] = new Entity(helix, "9");
	E[9] = new Entity(sphere, "10");
	E[10] = new Entity(torus, "11");
	E[11] = new Entity(plane, "12");

}

// create & initialize the sound object
//void Game::CreateSound(HWND hwnd)
//{
//	sound = new Sound;
//	sound->Initialize(hwnd);
//	cout << "sound initialize successfully" << endl;
//}


// --------------------------------------------------------
// Handle resizing DirectX "stuff" to match the new window size.
// For instance, updating our projection matrix's aspect ratio.
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();

	// Update our projection matrix since the window size changed
	c->UpdateProjMatrix(width, height);
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Quit if the escape key is pressed
	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();

	// when getting key input calling camera updating function
	c->Update(deltaTime);

	// ---- assignment 3-----

	float sinTime = (sin(totalTime * 2) + 2.0f) / 10.0f;
	float cosTime = (cos(totalTime * 2)) / 10.0f;

	E[0]->SetRot(XMFLOAT3(totalTime, 0, 0));
	E[0]->SetTrans(XMFLOAT3(-1, 0, 0)); // rotate at x axis

	E[1]->SetTrans(XMFLOAT3(totalTime, 0, 0));  // move off the screen
	E[1]->SetScale (XMFLOAT3(sinTime, sinTime, sinTime)); // scale in a sin wave

	E[2]->SetTrans(XMFLOAT3(3, -3, 0));
	E[2]->SetScale(XMFLOAT3(cosTime, cosTime, cosTime)); // scale in a cos wave

	E[3]->SetTrans(XMFLOAT3(0, 0, -1));
	E[3]->SetRot(XMFLOAT3(0, totalTime, 0)); // rotate at y axis

	E[4]->SetTrans(XMFLOAT3(0, 0, 0));
	E[4]->SetRot(XMFLOAT3(0, 0, totalTime)); // rotate at z axis according
	// ---assignment 3 ------
	

	E[5]->SetTrans(XMFLOAT3(1, 1, 0));
	E[6]->SetRot(XMFLOAT3(0, totalTime, 0));
	E[7]->SetTrans(XMFLOAT3(2, 0, 0));

	E[8]->SetTrans(XMFLOAT3(-2, 1, -2));
	E[8]->SetRot(XMFLOAT3(1, 1, 0));


	E[9]->SetTrans(XMFLOAT3(-2, 0, 0));

	E[10]->SetTrans(XMFLOAT3(0, -1, 0));

	E[11]->SetTrans(XMFLOAT3(0, -2, 0));

}


// -------------------------------------------------------
// Shadow Map stuff
void Game::RenderShadowMap()
{
	// Set up targets
	context->OMSetRenderTargets(0, 0, DSV_Shadow);
	context->ClearDepthStencilView(DSV_Shadow, D3D11_CLEAR_DEPTH, 1.0f, 0);
	context->RSSetState(RS_Shadow);

	// Make a viewport to match the render target size
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)shadowMapSize;
	viewport.Height = (float)shadowMapSize;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &viewport);

	// Set up our shadow VS shader
	VS_Shadow->SetShader();
	VS_Shadow->SetMatrix4x4("view", shadowViewMatrix);
	VS_Shadow->SetMatrix4x4("projection", shadowProjectionMatrix);

	// Turn off pixel shader
	context->PSSetShader(0, 0, 0);

	// Loop through entities and draw them
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	for (unsigned int i = 0; i < 12; i++)
	{
		// Grab the data from the first entity's mesh
		
		ID3D11Buffer* vb = E[i]->GetMesh()->GetVertexBuffer();
		ID3D11Buffer* ib = E[i]->GetMesh()->GetIndexBuffer();

		// Set buffers in the input assembler
		context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
		context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);

		VS_Shadow->SetMatrix4x4("world", E[i]->GetMatrix());
		VS_Shadow->CopyAllBufferData();

		// Finally do the actual drawing
		context->DrawIndexed(E[i]->GetMesh()->GetIndexCount(), 0, 0);
	}

	// Change everything back
	context->OMSetRenderTargets(1, &backBufferRTV, depthStencilView);
	viewport.Width = (float)width;
	viewport.Height = (float)height;
	context->RSSetViewports(1, &viewport);
	context->RSSetState(0);
}




// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	if (level == 0) {
		startText[0]->getSpriteBatch()->Begin();
		//scoreText->getSpriteBatch()->Draw(fontSRV, scoreText->getRECT());
		startText[0]->getSpriteFont()->DrawString(
			startText[0]->getSpriteBatch(),
			L"FLYING FIGURES",
			XMFLOAT2(10, 120));

		startText[0]->getSpriteBatch()->End();

		startText[1]->getSpriteBatch()->Begin();
		//scoreText->getSpriteBatch()->Draw(fontSRV, scoreText->getRECT());
		startText[1]->getSpriteFont()->DrawString(
			startText[1]->getSpriteBatch(),
			L"Press enter to play",
			XMFLOAT2(10, 180));

		startText[1]->getSpriteBatch()->End();


		context->RSSetState(0);
		context->OMSetDepthStencilState(0, 0);
		float factors[4] = { 1,1,1,1 };
		context->OMSetBlendState(0, factors, 0xFFFFFFFF);

		swapChain->Present(0, 0);
	}

	if (GetAsyncKeyState(VK_RETURN) & 0x8000) level = 1;

	if (level == 1) {
		RenderShadowMap();
		// Background color (Cornflower Blue in this case) for clearing
		const float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };

		// Clear the render target and depth buffer (erases what's on the screen)
		//  - Do this ONCE PER FRAME
		//  - At the beginning of Draw (before drawing *anything*)
		context->ClearRenderTargetView(backBufferRTV, color);
		context->ClearDepthStencilView(
			depthStencilView,
			D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
			1.0f,
			0);

		// Send data to shader variables
		//  - Do this ONCE PER OBJECT you're drawing
		//  - This is actually a complex process of copying data to a local buffer
		//    and then copying that entire buffer to the GPU.  
		//  - The "SimpleShader" class handles all of that for you.

		// load material once

		UINT stride = sizeof(Vertex);
		UINT offset = 0;

		// ---Third Assignment---
		// draw the entity
		for (int i = 0; i < 12; i++)
		{


			ID3D11Buffer *  vb = E[i]->GetMesh()->GetVertexBuffer();
			ID3D11Buffer *  ib = E[i]->GetMesh()->GetIndexBuffer();



			context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
			context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);

			// for shadow render commented below
			// E[i]->PrepareMaterial(c->GetViewMatrix(), c->GetProjectionMatrix()); // draw each entity



			// for shadow mapping stuffs
			//-----------render all entities in your scene without using materials-----------------

			// trying to pass the light's view matrix
			//XMVECTOR lightdirection = XMLoadFloat3(&directionalLight2.Direction);

			//XMMATRIX lightview =
			//	XMMatrixLookToLH(XMVectorSet(0, 0, 0, 0),	// light's position /center of your world
			//		lightdirection,								// light's direction
			//		XMVectorSet(0, 1, 0, 0));					// Updirection

			//XMFLOAT4X4 LightView;
			//XMStoreFloat4x4(&LightView, lightview);

			//// trying to pass the light's projection matrix
			//XMFLOAT4X4 LightProjection;
			//XMMATRIX lightprojection = XMMatrixOrthographicLH(100, 100, 0, 100);
			//// big enough to encompass entire game world
			//XMStoreFloat4x4(&LightProjection, lightprojection);

			vertexShader->SetMatrix4x4("world", E[i]->GetMatrix());
			vertexShader->SetMatrix4x4("view", c->GetViewMatrix());
			vertexShader->SetMatrix4x4("projection", c->GetProjectionMatrix());

			vertexShader->SetMatrix4x4("shadowView", shadowViewMatrix);
			vertexShader->SetMatrix4x4("shadowProjection", shadowProjectionMatrix);

			vertexShader->CopyAllBufferData();
			vertexShader->SetShader();
			//VS_Shadow->deviceContext->PSSetShader(0, 0, 0);

			pixelShader->SetData(
				"directionalLight2",
				&directionalLight2,
				sizeof(DirectionalLight)
			);
			pixelShader->SetSamplerState("Sampler", SampleState);
			pixelShader->SetShaderResourceView("Texture", srv);
			pixelShader->SetShaderResourceView("NormalMap", normalMapSRV);
			pixelShader->SetShaderResourceView("ShadowMap", SRV_Shadow);
			pixelShader->SetSamplerState("ShadowSampler", Sampler_Shadow);

			vertexShader->SetMatrix4x4("view2", viewMatrix2);
			vertexShader->SetMatrix4x4("projection2", projectionMatrix2);
			
			//pixelShader->SetData("pl", &pl, sizeof(PointLight));
			pixelShader->SetShaderResourceView("diffuseTexture", srv);
			pixelShader->SetShaderResourceView("projectionTexture", srv1);



			pixelShader->CopyAllBufferData();
			pixelShader->SetShader();


			// ---------------reset for "normal" rendering----------------------
			// reset the depth buffer
			//context->OMSetDepthStencilState(0, 0);

			// reset the viewport
			//D3D11_VIEWPORT viewport = {};
			//viewport.TopLeftX = 0;
			//viewport.TopLeftY = 0;
			//viewport.Width = (float)width;
			//viewport.Height = (float)height;
			//viewport.MinDepth = 0.0f;
			//viewport.MaxDepth = 1.0f;
			//context->RSSetViewports(1, &viewport);

			// reset the rasterizer state
			// context->RSSetState(0);
			// ---------------reset for "normal" rendering----------------------


				/*

			// --------------------------------------------------------------------------
			pixelShader->SetData(
				"directionalLight", // the name of the (eventual) variable in the shader
				&directionalLight,  // the address of the data to copy
				sizeof(DirectionalLight)  // the size of the data to copy
			);

			pixelShader->SetData(
				"directionalLight2",
				&directionalLight2,
				sizeof(DirectionalLight)
			);

			XMFLOAT3 camerapos;
			XMStoreFloat3(&camerapos, c->GetCameraPosition());
			//pixelShader->SetFloat3("cameraPosition", camerapos);

			pixelShader->SetFloat3("PointLightPosition", XMFLOAT3(0, 2, 0));
			pixelShader->SetFloat4("PointLightColor", XMFLOAT4(1, 0.1f, 0.1f, 1));  // red
			pixelShader->SetFloat3("CameraPosition", camerapos);
			*/
			/*
			pixelShader->SetData(
				"pointLight",
				&pointLight,
				sizeof(PointLight));
			*/





			context->DrawIndexed(
				E[i]->GetMesh()->GetIndexCount(),     // The number of indices to use (we could draw a subset if we wanted)
				0,     // Offset to the first index we want to use
				0);    // Offset to add to each index when looking up vertices
		}


		// Reset the states!
		context->RSSetState(0);
		context->OMSetDepthStencilState(0, 0);
		pixelShader->SetShaderResourceView("ShadowMap", 0);


		// Present the back buffer to the user
		//  - Puts the final frame we're drawing into the window so the user can see it
		//  - Do this exactly ONCE PER FRAME (always at the very end of the frame)
		swapChain->Present(0, 0);
	}
}


#pragma region Mouse Input

// --------------------------------------------------------
// Helper method for mouse clicking.  We get this information
// from the OS-level messages anyway, so these helpers have
// been created to provide basic mouse input if you want it.
// --------------------------------------------------------
void Game::OnMouseDown(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...
	BoundingFrustum camFrustum;
	c->GetFrustum().Transform(camFrustum, XMMatrixInverse(NULL, XMMatrixTranspose(XMLoadFloat4x4(&c->GetViewMatrix()))));

	XMFLOAT4X4 proj;
	XMStoreFloat4x4(&proj, XMMatrixTranspose(XMLoadFloat4x4(&c->GetProjectionMatrix())));

	XMFLOAT3 tempRayDir;
	tempRayDir.x = (((2 * (float)x) / width) - 1) / proj._11;
	tempRayDir.y = -(((2 * (float)y) / height) - 1) / proj._22;
	tempRayDir.z = 1.0f;

	XMMATRIX view = XMMatrixInverse(NULL, XMMatrixTranspose(XMLoadFloat4x4(&c->GetViewMatrix())));
	XMVECTOR rayDir = XMVector3TransformNormal(XMVector3Normalize(XMLoadFloat3(&tempRayDir)), view);
	XMVECTOR rayPos = c->GetCameraPosition(); //Was Load Float 3. If it breaks everything BLAME DARREN

	BoundingOrientedBox entBox;
	float distance;

	std::vector<Entity*> entityQueue;

	for (size_t i = 0; i < 12; i++)
	{
		E[i]->GetMesh()->getOBB().Transform(entBox, XMMatrixTranspose(XMLoadFloat4x4(&E[i]->GetMatrix())));
		if (camFrustum.Contains(entBox) != ContainmentType::DISJOINT)
		{
			if (entBox.Intersects(rayPos, rayDir, distance))
			{
				entityQueue.push_back(E[i]);
			}
		}
	}

	bool ent_clicked = false;
	Entity* nearestEntity;
	float filterDistance = FLT_MAX;

	for (size_t i = 0; i < entityQueue.size(); ++i)
	{
		Entity* entity = entityQueue[i];

		XMMATRIX entMatrix = XMMatrixInverse(NULL, XMMatrixTranspose(XMLoadFloat4x4(&entity->GetMatrix())));
		XMVECTOR rayPosLocal = XMVector3TransformCoord(rayPos, entMatrix);
		XMVECTOR rayDirLocal = XMVector3Normalize(XMVector3TransformNormal(rayDir, entMatrix));

		UINT* faces = entity->GetMesh()->GetFaces(); //indices
		Vertex* verts = entity->GetMesh()->GetVertices();

		for (size_t j = 0; j < entity->GetMesh()->GetIndexCount(); j += 3)
		{
			if (TriangleTests::Intersects(rayPosLocal, rayDirLocal, XMLoadFloat3(&verts[faces[j]].Position), XMLoadFloat3(&verts[faces[j + 1]].Position), XMLoadFloat3(&verts[faces[j + 2]].Position), distance))
				if (distance < filterDistance)
				{
					filterDistance = distance;
					nearestEntity = entity;
					ent_clicked = true;
				}
		}
	}

	if (ent_clicked)
	{
		//nearestEntity->SetTrans(XMFLOAT3(20.0f,0.0f,0.0f));
		printf("\n%s",nearestEntity->GetName().c_str());
	}
	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;

	// Caputure the mouse so we keep getting mouse move
	// events even if the mouse leaves the window.  we'll be
	// releasing the capture once a mouse button is released
	SetCapture(hWnd);
}

// --------------------------------------------------------
// Helper method for mouse release
// --------------------------------------------------------
void Game::OnMouseUp(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...

	// We don't care about the tracking the cursor outside
	// the window anymore (we're not dragging if the mouse is up)
	ReleaseCapture();
}

// --------------------------------------------------------
// Helper method for mouse movement.  We only get this message
// if the mouse is currently over the window, or if we're 
// currently capturing the mouse.
// --------------------------------------------------------
void Game::OnMouseMove(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...
	// calling camera movement methods here

	if (buttonState & 0x0001)
	{
		float xDiff = (x - prevMousePos.x) * 0.005f;
		float yDiff = (y - prevMousePos.y) * 0.005f;
		c->Rotate(yDiff, xDiff);
	}



	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;
}

// --------------------------------------------------------
// Helper method for mouse wheel scrolling.  
// WheelDelta may be positive or negative, depending 
// on the direction of the scroll
// --------------------------------------------------------
void Game::OnMouseWheel(float wheelDelta, int x, int y)
{
	// Add any custom code here...
}
#pragma endregion