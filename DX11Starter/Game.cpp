#include "Game.h"
#include "Vertex.h"
#include "Statemachine.h"      
#include <stdlib.h>    
#include <time.h>  
#include "DDSTextureLoader.h"
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

	//Particles
	tempEmitterFlag = 1;
	TimeSinceBeginningForGame = 0;
	tempResetEmitterFlag = 0;
	//Particles end
	
	entity_vanish = false;
	vanish_location = XMFLOAT3(0,0,0);

	//sound = 0;

#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here. \n");
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

	for (int i = 0; i < mesh_list.size();i++)
	{
		delete mesh_list[i];
	}
	for (int i = 0; i < pp.size(); i++) 
	{
		delete pp[i];
	}
	for (int i = 0; i < E.size(); i++)
	{
		delete E[i];
	}
	delete Ground;
	delete Ground_Mesh;
	delete LightSource;
	delete LightSource_Mesh;
	//delete Entity_obj;
	delete c;
	//delete ma_metal;
	//delete ma_concrete;
	delete vertexShader;
	delete pixelShader;
	delete skyVS;
	delete skyPS;
	delete ppVS;
	delete ppPS;
	delete ebVS;
	delete ebPS;

	//Particles
	delete particleVS;
	delete particlePS;

	particleTexture->Release();
	particleTexture1->Release();
	fogParticleTexture->Release();

	particleBlendState->Release();
	particleDepthState->Release();

	delete emitter_yellow_explosion;
	delete emitter_red_explosion;
	delete emitter_red_explosion2;
	delete emitter_red_explosion3;
	delete emitter_red_explosion4;
	delete emitter_fog;
	//Particles end

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

	for(int i = 0;i<4;i++)
	delete scoreText[i];
	
	delete writable;

	delete startText[0];
	delete startText[1];

	delete endText[0];

	skySRV->Release();
	rsSky->Release();
	dsSky->Release();

	// clear the outline Rasterizer View
	RS_Outline->Release();
	delete VS_Outline;
	delete PS_Outline;

	delete spriteBatch;

	
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
	mouseMove = false;
	curr_time = 0;
	prev_time = 0;
	shadowMapSize = 1024;
	LoadShaders();
	CreateCamera();
	/*CreateSound(hWnd);*/
	//CreateMaterial();
	CreateMeshes();
	CreateEntities(); // the third assignment
					  
	/* initialize random seed: */
	srand(time(NULL));

	/* generate secret number between 0 and 5: */
	int rand_num = rand() % 6;

	Random_Mesh = mesh_names[rand_num];
	printf("%s", Random_Mesh.c_str());

	// ----load texture----

	CreateWICTextureFromFile(device, context, L"Assets/Textures/Tile.jpg",0, &SRV_Metal);
	CreateWICTextureFromFile(device, context, L"Assets/Textures/DamageConcrete.jpg", 0, &SRV_Concrete);
	CreateWICTextureFromFile(device, context, L"Assets/Textures/rock.jpg", 0, &srv);
	CreateWICTextureFromFile(device, context, L"Assets/Textures/03.jpg", 0, &srv1);
	CreateWICTextureFromFile(device, context, L"Assets/Textures/rockNormals.jpg", 0, &normalMapSRV);
	//CreateWICTextureFromFile(device, context, L"Assets/Texture/shadow_cube.png",0,&SRV_Shadow);
	CreateDDSTextureFromFile(device, L"Assets/Textures//UffiziGallery.dds", 0, &skySRV);
	//CreateDDSTextureFromFile(device, L"Assets/Textures//0.dds", 0, &skySRV);
	CreateWICTextureFromFile(device, context, L"Assets/Textures/light.jpg", 0, &srvlight);

	spriteBatch = new SpriteBatch(context); 

	// Create post process resources -----------------------------------------
	D3D11_TEXTURE2D_DESC pptextureDesc = {};
	pptextureDesc.Width = width;
	pptextureDesc.Height = height;
	pptextureDesc.ArraySize = 1;
	pptextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	pptextureDesc.CPUAccessFlags = 0;
	pptextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	pptextureDesc.MipLevels = 1;
	pptextureDesc.MiscFlags = 0;
	pptextureDesc.SampleDesc.Count = 1;
	pptextureDesc.SampleDesc.Quality = 0;
	pptextureDesc.Usage = D3D11_USAGE_DEFAULT;

	ID3D11Texture2D* ppTexture;
	device->CreateTexture2D(&pptextureDesc, 0, &ppTexture);

	// Create the Render Target View
	D3D11_RENDER_TARGET_VIEW_DESC pprtvDesc = {};
	pprtvDesc.Format = pptextureDesc.Format;
	pprtvDesc.Texture2D.MipSlice = 0;
	pprtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	device->CreateRenderTargetView(ppTexture, &pprtvDesc, &ppRTV);

	// Create the Shader Resource View
	D3D11_SHADER_RESOURCE_VIEW_DESC ppsrvDesc = {};
	ppsrvDesc.Format = pptextureDesc.Format;
	ppsrvDesc.Texture2D.MipLevels = 1;
	ppsrvDesc.Texture2D.MostDetailedMip = 0;
	ppsrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

	device->CreateShaderResourceView(ppTexture, &ppsrvDesc, &ppSRV);

	// We don't need the texture reference itself no mo'
	ppTexture->Release();




	ID3D11Texture2D* ppTexture0;
	device->CreateTexture2D(&pptextureDesc, 0, &ppTexture0);


	// Create the Render Target View
	D3D11_RENDER_TARGET_VIEW_DESC pprtvDesc0 = {};
	pprtvDesc0.Format = pptextureDesc.Format;
	pprtvDesc0.Texture2D.MipSlice = 0;
	pprtvDesc0.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	device->CreateRenderTargetView(ppTexture0, &pprtvDesc0, &ppbRTV);

	// Create the Shader Resource View
	D3D11_SHADER_RESOURCE_VIEW_DESC ppsrvDesc0 = {};
	ppsrvDesc0.Format = pptextureDesc.Format;
	ppsrvDesc0.Texture2D.MipLevels = 1;
	ppsrvDesc0.Texture2D.MostDetailedMip = 0;
	ppsrvDesc0.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

	device->CreateShaderResourceView(ppTexture0, &ppsrvDesc0, &ppbSRV);

	// We don't need the texture reference itself no mo'
	ppTexture0->Release();


	ID3D11Texture2D* ppTexture1;
	device->CreateTexture2D(&pptextureDesc, 0, &ppTexture1);


	// Create the Render Target View
	D3D11_RENDER_TARGET_VIEW_DESC pprtvDesc1 = {};
	pprtvDesc1.Format = pptextureDesc.Format;
	pprtvDesc1.Texture2D.MipSlice = 0;
	pprtvDesc1.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	device->CreateRenderTargetView(ppTexture1, &pprtvDesc1, &ppbloomRTV);

	// Create the Shader Resource View
	D3D11_SHADER_RESOURCE_VIEW_DESC ppsrvDesc1 = {};
	ppsrvDesc1.Format = pptextureDesc.Format;
	ppsrvDesc1.Texture2D.MipLevels = 1;
	ppsrvDesc1.Texture2D.MostDetailedMip = 0;
	ppsrvDesc1.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

	device->CreateShaderResourceView(ppTexture1, &ppsrvDesc1, &ppbloomSRV);

	// We don't need the texture reference itself no mo'
	ppTexture1->Release();





	//Particles
	DirectX::CreateWICTextureFromFile(device, context, L"Assets/Textures/Particles/circleParticle.jpg", 0, &particleTexture);
	DirectX::CreateWICTextureFromFile(device, context, L"Assets/Textures/Particles/particle.jpg", 0, &particleTexture1);
	DirectX::CreateWICTextureFromFile(device, context, L"Assets/Textures/Particles/fogParticle.jpg", 0, &fogParticleTexture);
	
	// A depth state for the particles
	D3D11_DEPTH_STENCIL_DESC dsDesc2 = {};
	dsDesc2.DepthEnable = true;
	dsDesc2.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // Turns off depth writing
	dsDesc2.DepthFunc = D3D11_COMPARISON_LESS;
	device->CreateDepthStencilState(&dsDesc2, &particleDepthState);


	// Blend for particles (additive)
	D3D11_BLEND_DESC blend = {};
	blend.AlphaToCoverageEnable = false;
	blend.IndependentBlendEnable = false;
	blend.RenderTarget[0].BlendEnable = true;
	blend.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blend.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	device->CreateBlendState(&blend, &particleBlendState);


	// Set up particles
	emitter_red_explosion = new Emitter(
		50,								// Max particles
		50,								// Particles per second
		1,								// Particle lifetime
		1.0f,							// Start size	
		5.0f,							// End size
		XMFLOAT4(1, 0.1f, 0.1f, 0.1f),	// Start color
		XMFLOAT4(1, 0.6f, 0.1f, 0),		// End color
		XMFLOAT3(0, 0, 0),				// Start velocity
		XMFLOAT3(2, 2, 0),				// Start position
		XMFLOAT3(10, 10, 0),			// Start acceleration
		device,
		particleVS,
		particlePS,
		particleTexture1,
		1,								// Emitter Type e.g. 1 = simple explosion, 2 Particle stream going upwards
		1.0);							// Emitter Age in seconds

	emitter_red_explosion2 = new Emitter(
		50,								// Max particles
		50,								// Particles per second
		1,								// Particle lifetime
		1.0f,							// Start size	
		5.0f,							// End size
		XMFLOAT4(1, 0.1f, 0.1f, 0.1f),	// Start color
		XMFLOAT4(1, 0.6f, 0.1f, 0),		// End color
		XMFLOAT3(0, 0, 0),				// Start velocity
		XMFLOAT3(2, 2, 0),				// Start position
		XMFLOAT3(10, 10, 0),			// Start acceleration
		device,
		particleVS,
		particlePS,
		particleTexture1,
		1,								// Emitter Type e.g. 1 = simple explosion, 2 Particle stream going upwards
		1.0);							// Emitter Age in seconds

	emitter_red_explosion3 = new Emitter(
		50,								// Max particles
		50,								// Particles per second
		1,								// Particle lifetime
		1.0f,							// Start size	
		5.0f,							// End size
		XMFLOAT4(1, 0.1f, 0.1f, 0.1f),	// Start color
		XMFLOAT4(1, 0.6f, 0.1f, 0),		// End color
		XMFLOAT3(0, 0, 0),				// Start velocity
		XMFLOAT3(2, 2, 0),				// Start position
		XMFLOAT3(10, 10, 0),			// Start acceleration
		device,
		particleVS,
		particlePS,
		particleTexture1,
		1,								// Emitter Type e.g. 1 = simple explosion, 2 Particle stream going upwards
		1.0);							// Emitter Age in seconds


	emitter_red_explosion4 = new Emitter(
		50,								// Max particles
		50,								// Particles per second
		1,								// Particle lifetime
		1.0f,							// Start size	
		5.0f,							// End size
		XMFLOAT4(1, 0.1f, 0.1f, 0.1f),	// Start color
		XMFLOAT4(1, 0.6f, 0.1f, 0),		// End color
		XMFLOAT3(0, 0, 0),				// Start velocity
		XMFLOAT3(2, 2, 0),				// Start position
		XMFLOAT3(10, 10, 0),			// Start acceleration
		device,
		particleVS,
		particlePS,
		particleTexture1,
		1,								// Emitter Type e.g. 1 = simple explosion, 2 Particle stream going upwards
		1.0);							// Emitter Age in seconds



										//////////////////////////////////////////////Perodic Stream of particles ////////////////////////////////////////////////

										// Set up particles
	emitter_yellow_explosion = new Emitter(
		50,								// Max particles
		50,								// Particles per second
		3.0,							// Particle lifetime
		0.5f,							// Start size
		1.5f,							// End size
		XMFLOAT4(1, 0.1f, 0.1f, 0.2f),	// Start color
		XMFLOAT4(1, 0.6f, 0.1f, 0),		// End color
		XMFLOAT3(-10, 20, 0),			// Start velocity
		XMFLOAT3(2, 0, 0),				// Start position
		XMFLOAT3(50, -5, 0),			// Start acceleration
		device,
		particleVS,
		particlePS,
		particleTexture,
		1,								// Emitter Type e.g. 1 = simple explosion
		1.0);							// Emitter Age in seconds	



	emitter_fog = new Emitter(
		1,								// Max particles
		1,								// Particles per second
		10.0,							// Particle lifetime
		40.0f,							// Start size
		50.0f,							// End size
		XMFLOAT4(1, 0.3, 0.5, 0.5),		// Start color
		XMFLOAT4(1, 0.5, 0.7, 0.5),		// End color
		XMFLOAT3(-10, 20, 0),			// Start velocity
		XMFLOAT3(-10, -10, 0),			// Start position
		XMFLOAT3(50, -5, 0),			// Start acceleration
		device,
		particleVS,
		particlePS,
		fogParticleTexture,
		3,								// Emitter Type e.g. 1 = simple red explosion, 2 = simple yellow explosion, 3 = fog
		1.0);							// Emitter Age in seconds	
	//Particles end


	score = 0;
	swprintf_s(showScore, L"%d", score);

	
	wstring widestr = wstring(Random_Mesh.begin(), Random_Mesh.end());


	writable = new wchar_t[widestr.size() + 1];
	//wchar_t * writable;
	copy(widestr.begin(), widestr.end(), writable);
	writable[widestr.size()] = '\0';
	//copy(widestr.begin(), widestr.end(), instruction);

	instruction = writable;

	//swprintf_s(instruction, 256, L"%s", L"fff");
	//wcout << instruction << endl;
    scoreText[2] = new UI(device, context, L"Score: ", XMFLOAT4(+0.0f, +0.0f, +110.0f, 110.0f));
	scoreText[0] = new UI(device, context, showScore, XMFLOAT4(+0.0f, +0.0f, +110.0f, 110.0f));

	scoreText[3] = new UI(device, context, L"Please choose ", XMFLOAT4(+200.0f, +0.0f, +110.0f, 110.0f));
	scoreText[1] = new UI(device, context, instruction, XMFLOAT4(+200.0f, +0.0f, +110.0f, 110.0f));
	
	endText[0] = new UI(device, context, L"Game Over", XMFLOAT4(+200.0f, +0.0f, +110.0f, 110.0f));
	

	startText[0] = new UI(device, context, L"FLYING FIGURES", XMFLOAT4(+200.0f, +100.0f, +110.0f, 110.0f));
	startText[1] = new UI(device, context, L"Press enter to play", XMFLOAT4(+200.0f, +150.0f, +110.0f, 110.0f));
	// ----sampler code----

	// First, create a description
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;  // how to handle addresses outside the 0-1 UV range
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;  // D3D11_TEXTURE_ADDRESS_WRAP is a usual value ( wrapping textures)
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;  // how to handle sampling "between" pixels ;
	
												//D3D11_FILTER_MIN_MAG_MIP_LINEAR is usual (trilinear filtering)
	sampDesc.MaxAnisotropy = 16;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	// Now, create the sampler from the description
	device->CreateSamplerState(&sampDesc, &SampleState);
	// -------------------------

	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
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

	//set up sky

	//set up rasterize state
	D3D11_RASTERIZER_DESC rsDesc = {};
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_FRONT;
	rsDesc.DepthClipEnable = true;
	device->CreateRasterizerState(&rsDesc, &rsSky);

	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	device->CreateDepthStencilState(&dsDesc, &dsSky);

	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	//light from front
	directionalLight.AmbientColor = XMFLOAT4(0.1, 0.1, 0.1, 0.1);
	directionalLight.DiffuseColor = XMFLOAT4(1, 1, 0, 1);  
	directionalLight.Direction = XMFLOAT3(0,0,1);
	

	//light from top
	directionalLight2.AmbientColor = XMFLOAT4(0.1, 0.1, 0.1, 0.1);
	directionalLight2.DiffuseColor = XMFLOAT4(1, 1, 1, 1);  
	//directionalLight2.DiffuseColor = XMFLOAT4(0.8f, 0.8f, 0.8f, 1);
	directionalLight2.Direction = XMFLOAT3(0, -1, 0);
	//directionalLight2.Direction = XMFLOAT3(1, 0, 0);



	pointLight.Color = XMFLOAT4(0, 1, 0, 1);
	pointLight.Position = XMFLOAT3(0, 2, 0);
	XMStoreFloat3(&pointLight.CameraPos, c->GetCameraPosition());

	//set up sky

	// set up rasterizer state for outline
	D3D11_RASTERIZER_DESC RS_Outline_Desc = {};
	RS_Outline_Desc.FillMode = D3D11_FILL_SOLID;
	RS_Outline_Desc.CullMode = D3D11_CULL_FRONT;
	RS_Outline_Desc.DepthClipEnable = true;
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	device->CreateRasterizerState(&RS_Outline_Desc, &RS_Outline);



	level = 0;

	XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.05f * 3.1415926535f,		// Field of View Angle
		(float)width / height,		// Aspect ratio
		0.1f,						// Near clip plane distance
		100.0f);					// Far clip plane distance

	XMStoreFloat4x4(&projectionMatrix2, XMMatrixTranspose(P));

	//delete writable;
	
	//decals = false;
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

	//Particles shaders
	particleVS = new SimpleVertexShader(device, context);
	if (!particleVS->LoadShaderFile(L"Debug/ParticleVS.cso"))
		particleVS->LoadShaderFile(L"ParticleVS.cso");

	particlePS = new SimplePixelShader(device, context);
	if (!particlePS->LoadShaderFile(L"Debug/ParticlePS.cso"))
		particlePS->LoadShaderFile(L"ParticlePS.cso");
	//Particles shaders end

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
	skyVS = new SimpleVertexShader(device, context);
	if (!skyVS->LoadShaderFile(L"Debug/SkyVS.cso"))
		skyVS->LoadShaderFile(L"SkyVS.cso");

	skyPS = new SimplePixelShader(device, context);
	if (!skyPS->LoadShaderFile(L"Debug/SkyPS.cso"))
		skyPS->LoadShaderFile(L"SkyPS.cso");

	// load outline shaders
	VS_Outline = new SimpleVertexShader(device, context);
	if (!VS_Outline->LoadShaderFile(L"Debug/VS_Outline.cso"))
		VS_Outline->LoadShaderFile(L"VS_Outline.cso");

	PS_Outline = new SimplePixelShader(device, context);
	if (!PS_Outline->LoadShaderFile(L"Debug/PS_Outline.cso"))
		PS_Outline->LoadShaderFile(L"PS_Outline.cso");


	ebVS = new SimpleVertexShader(device, context);
	if (!ebVS->LoadShaderFile(L"Debug/ExtractBrightVS.cso"))
		ebVS->LoadShaderFile(L"ExtractBrightVS.cso");

	ebPS = new SimplePixelShader(device, context);
	if (!ebPS->LoadShaderFile(L"Debug/ExtractBrightPS.cso"))
		ebPS->LoadShaderFile(L"ExtractBrightPS.cso");

	ppVS = new SimpleVertexShader(device, context);
	if (!ppVS->LoadShaderFile(L"Debug/PostProcessVS.cso"))
		ppVS->LoadShaderFile(L"PostProcessVS.cso");

	ppPS = new SimplePixelShader(device, context);
	if (!ppPS->LoadShaderFile(L"Debug/PostProcessPS.cso"))
		ppPS->LoadShaderFile(L"PostProcessPS.cso");
	
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
	m1 = new Mesh(x1, 3, y1, 3, device, "m1");

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
	m2 = new Mesh(x2, 3, y2, 3, device, "m2");

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
	m3 = new Mesh(x3, 3, y3, 3, device, "m3");

	// create new meshes using third mesh constructor
	mesh_list.push_back(new Mesh("Assets/Models/cone.obj", device, "cone"));
	mesh_names.push_back("cone");
	mesh_list.push_back(new Mesh("Assets/Models/cube.obj", device, "cube"));
	mesh_names.push_back("cube");
	mesh_list.push_back(new Mesh("Assets/Models/cylinder.obj", device, "cylinder"));
	mesh_names.push_back("cylinder");
	mesh_list.push_back(new Mesh("Assets/Models/helix.obj", device, "helix"));
	mesh_names.push_back("helix");
	mesh_list.push_back(new Mesh("Assets/Models/sphere.obj", device, "sphere"));
	mesh_names.push_back("sphere");
	mesh_list.push_back(new Mesh("Assets/Models/torus.obj", device, "torus"));
	mesh_names.push_back("torus");
	Ground_Mesh = new Mesh("Assets/Models/plane.obj", device, "plane");
	mesh_names.push_back("plane");
	LightSource_Mesh = new Mesh("Assets/Models/sphere.obj", device, "sphere");

}

void Game::CreateEntities()
{



	for (int i = 0; i < 13; i++)
	{
		pp.push_back(new Physics);
	}
	
	
	E.push_back(new Entity(mesh_list[0], "1", pp[0]));
	E.push_back(new Entity(mesh_list[2], "2", pp[1]));
	E.push_back(new Entity(mesh_list[4], "3", pp[2]));
	E.push_back(new Entity(mesh_list[1], "4", pp[3]));
	E.push_back(new Entity(mesh_list[3], "5", pp[4]));
	E.push_back(new Entity(mesh_list[5], "6", pp[5]));
	E.push_back(new Entity(mesh_list[3], "7", pp[6]));
	E.push_back(new Entity(mesh_list[5], "8", pp[7]));
	E.push_back(new Entity(mesh_list[1], "9", pp[8]));
	E.push_back(new Entity(mesh_list[2], "10", pp[9]));
	E.push_back(new Entity(mesh_list[4], "11", pp[10]));
	E.push_back(new Entity(mesh_list[0], "12", pp[11]));
	Ground = new Entity(Ground_Mesh, "Ground", pp[12]);
	LightSource = new Entity(LightSource_Mesh, "Light", pp[12]);

	for (int i = 0; i < E.size(); i++)
	{
		en_pos.push_back(i);
	}

	

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


	TimeSinceBeginningForGame += deltaTime;								// Time passed since beginning of this particle system spawn OR Beginning of game?
	
	emitter_red_explosion->Update(deltaTime);		// Nomal Explosion Emitter
	emitter_red_explosion2->Update(deltaTime);		// Nomal Explosion Emitter
	emitter_red_explosion3->Update(deltaTime);		// Nomal Explosion Emitter
	emitter_red_explosion4->Update(deltaTime);		// Nomal Explosion Emitter

	if (entity_vanish)
	{
		if (emitter_red_explosion->GetTimeSinceBeginning() > 1.0f)
		{
			emitter_red_explosion->ResetEmitter(vanish_location, 1.0f);
		}
		else if (emitter_red_explosion2->GetTimeSinceBeginning() > 1.0f)
		{
			emitter_red_explosion2->ResetEmitter(vanish_location, 1.0f);
		}
		else if(emitter_red_explosion3->GetTimeSinceBeginning() > 1.0f)
		{
			emitter_red_explosion3->ResetEmitter(vanish_location, 1.0f);
		}
		else
		{
			emitter_red_explosion4->ResetEmitter(vanish_location, 1.0f);
		}
		entity_vanish = false;
	}

	this->deltaTimeNew = deltaTime;

	// when getting key input calling camera updating function
	c->Update(deltaTime);

	// ---- assignment 3-----

	float sinTime = (sin(totalTime * 2)) / 10.0f;
	float cosTime = (cos(totalTime * 2)) / 10.0f;

	//respawn clicked items after 6 seconds
	curr_time = totalTime;
	if (curr_time > prev_time)
	{
		prev_time = curr_time;
		for (int i = 0; i < count_down.size(); i++)
		{
			count_down[i]--;
			if (count_down[i] == 0)
			{
				count_down.erase(count_down.begin() + i);
				pp.push_back(new Physics);

				/* initialize random seed: */
				srand(time(NULL));

				/* generate secret number between 0 and 5: */
				int rand_num = rand() % 6;

				E.push_back(new Entity(mesh_list[rand_num], "", pp[E.size()-1]));
				

			}
		}
	}

	for (int i = 0; i < E.size(); i++)
	{
		E[i]->phy->setTranslation(totalTime - en_pos[i], totalTime - en_pos[i], 0);
		//E[0]->SetRot(XMFLOAT3(totalTime, 0, 0));
		E[i]->SetTrans(E[i]->phy->getTranslation()); // rotate at x axis
													 //cout << E[i]->phy->getTranslation().y;
	

		srand(time(NULL));
		int rand_num = rand() % 6;



		if (E[i]->phy->getTranslation().x > 0 && E[i]->phy->getTranslation().y < -9.4f)
		{
			E[i]->SetMesh(mesh_list[rand_num]);
			
			E[i]->isdecal = false;
		
		}
		
		
	}

	Ground->SetTrans(XMFLOAT3(0, -2, 1));
    
	//LightSource->SetRot(XMFLOAT3(0, totalTime, 0));
	LightSource->SetTrans(XMFLOAT3(3.3, 1.9, 1));
	

	

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

	//Draw Flying objects
	for (unsigned int i = 0; i < E.size(); i++)
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

	//Draw Ground
	ID3D11Buffer* vb = Ground->GetMesh()->GetVertexBuffer();
	ID3D11Buffer* ib = Ground->GetMesh()->GetIndexBuffer();

	// Set buffers in the input assembler
	context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);

	VS_Shadow->SetMatrix4x4("world", Ground->GetMatrix());
	VS_Shadow->CopyAllBufferData();

	// Finally do the actual drawing
	context->DrawIndexed(Ground->GetMesh()->GetIndexCount(), 0, 0);

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
		const float color[4] = { 0,0,0, 0.0f };
		context->ClearRenderTargetView(backBufferRTV, color);
		context->ClearDepthStencilView(
			depthStencilView,
			D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
			1.0f,
			0);
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




		// ---Third Assignment---
		// draw the entity
		for (int i = 0; i < E.size(); i++)
		{

			//cout << E[i]->getdecalsPos().z << E[i]->getPos().z << endl;
			if (E[i]->isdecal) {
				//cout << "a";
				XMVECTOR dir = XMVectorSet(0, 0, 1, 0);
				XMVECTOR pos = XMVectorSet(E[i]->getdecalsPos().x, E[i]->getdecalsPos().y, E[i]->getdecalsPos().z, 0);
				XMVECTOR up = XMVectorSet(0, 1, 0, 0);
				XMMATRIX V = XMMatrixLookToLH(
					pos,     // The position of the "camera"
					dir,     // Direction the camera is looking
					up);     // "Up" direction in 3D space (prevents roll)
				XMStoreFloat4x4(&viewMatrix2, XMMatrixTranspose(V));
			
			}

			if (mouseMove)
			{
				Highlight();
			}
			
			ID3D11Buffer *  vb = E[i]->GetMesh()->GetVertexBuffer();
			ID3D11Buffer *  ib = E[i]->GetMesh()->GetIndexBuffer();

			UINT stride = sizeof(Vertex);
			UINT offset = 0;

			context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
			context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);

			vertexShader->SetMatrix4x4("world", E[i]->GetMatrix());
			vertexShader->SetMatrix4x4("view", c->GetViewMatrix());
			vertexShader->SetMatrix4x4("projection", c->GetProjectionMatrix());

			vertexShader->SetMatrix4x4("shadowView", shadowViewMatrix);
			vertexShader->SetMatrix4x4("shadowProjection", shadowProjectionMatrix);

			vertexShader->SetMatrix4x4("view2", viewMatrix2);
			vertexShader->SetMatrix4x4("projection2", projectionMatrix2);
			vertexShader->CopyAllBufferData();
			vertexShader->SetShader();
			//VS_Shadow->deviceContext->PSSetShader(0, 0, 0);

			pixelShader->SetData(
				"directionalLight",
				&directionalLight,
				sizeof(DirectionalLight)
			);
			pixelShader->SetData(
				"directionalLight2",
				&directionalLight2,
				sizeof(DirectionalLight)
			);
			pixelShader->SetSamplerState("Sampler", SampleState);
			pixelShader->SetSamplerState("ShadowSampler", Sampler_Shadow);

			pixelShader->SetShaderResourceView("Texture", srv);
			 pixelShader->SetShaderResourceView("NormalMap", normalMapSRV);
			pixelShader->SetShaderResourceView("ShadowMap", SRV_Shadow);
            //pixelShader->SetShaderResourceView("diffuseTexture", srv);
			pixelShader->SetShaderResourceView("projectionTexture", srv1);
           


			
			//pixelShader->SetData("pl", &pl, sizeof(PointLight));
			


			pixelShader->CopyAllBufferData();
			pixelShader->SetShader();

			context->DrawIndexed(
				E[i]->GetMesh()->GetIndexCount(),     // The number of indices to use (we could draw a subset if we wanted)
				0,     // Offset to the first index we want to use
				0);    // Offset to add to each index when looking up vertices
		}

		ID3D11Buffer *  vb = Ground->GetMesh()->GetVertexBuffer();
		ID3D11Buffer *  ib = Ground->GetMesh()->GetIndexBuffer();

	    UINT stride = sizeof(Vertex);
		UINT offset = 0;

		context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
		context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);

		vertexShader->SetMatrix4x4("world", Ground->GetMatrix());
		vertexShader->SetMatrix4x4("view", c->GetViewMatrix());
		vertexShader->SetMatrix4x4("projection", c->GetProjectionMatrix());

		vertexShader->SetMatrix4x4("shadowView", shadowViewMatrix);
		vertexShader->SetMatrix4x4("shadowProjection", shadowProjectionMatrix);

		vertexShader->SetMatrix4x4("view2", viewMatrix2);
		vertexShader->SetMatrix4x4("projection2", projectionMatrix2);
		vertexShader->CopyAllBufferData();
		vertexShader->SetShader();
		//VS_Shadow->deviceContext->PSSetShader(0, 0, 0);

		pixelShader->SetData(
			"directionalLight",
			&directionalLight,
			sizeof(DirectionalLight)
		);
		pixelShader->SetData(
			"directionalLight2",
			&directionalLight2,
			sizeof(DirectionalLight)
		);

		pixelShader->SetData(
			"pointLight",
			&pointLight,
			sizeof(PointLight)
		);

		pixelShader->SetSamplerState("Sampler", SampleState);
		pixelShader->SetSamplerState("ShadowSampler", Sampler_Shadow);

		pixelShader->SetShaderResourceView("Texture", SRV_Metal);
		pixelShader->SetShaderResourceView("NormalMap", normalMapSRV);
		pixelShader->SetShaderResourceView("ShadowMap", SRV_Shadow);
		//pixelShader->SetShaderResourceView("diffuseTexture", srv);
		pixelShader->SetShaderResourceView("projectionTexture", srv1);

		pixelShader->SetShaderResourceView("Sky", skySRV);


		//pixelShader->SetData("pl", &pl, sizeof(PointLight));



		pixelShader->CopyAllBufferData();
		pixelShader->SetShader();

		context->DrawIndexed(
			Ground->GetMesh()->GetIndexCount(),     // The number of indices to use (we could draw a subset if we wanted)
			0,     // Offset to the first index we want to use
			0);    // Offset to add to each index when looking up vertices

		// Reset the states!
		context->RSSetState(0);
		context->OMSetDepthStencilState(0, 0);
		pixelShader->SetShaderResourceView("ShadowMap", 0);


		// Draw the sky ------------------------

		vb = mesh_list[1]->GetVertexBuffer();
		ib = mesh_list[1]->GetIndexBuffer();

		// Set buffers in the input assembler
		context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
		context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);

		// Set up the sky shaders
		skyVS->SetMatrix4x4("view", c->GetViewMatrix());
		skyVS->SetMatrix4x4("projection", c->GetProjectionMatrix());
		skyVS->CopyAllBufferData();
		skyVS->SetShader();

		skyPS->SetShaderResourceView("Sky", skySRV);
		skyPS->CopyAllBufferData();
		skyPS->SetShader();

		context->RSSetState(rsSky);
		context->OMSetDepthStencilState(dsSky, 0);
		context->DrawIndexed(mesh_list[1]->GetIndexCount(), 0, 0);





		UINT stride0 = sizeof(Vertex);
		UINT offset0 = 0;

		context->RSSetState(0);
		context->OMSetDepthStencilState(0, 0);


		//post process
		context->ClearRenderTargetView(ppRTV, color);
		context->OMSetRenderTargets(1, &ppRTV, depthStencilView);

		RECT crateRect = { 0, 0, width, height };

		spriteBatch->Begin();

		spriteBatch->Draw(srvlight, crateRect);

		spriteBatch->End();

		context->RSSetState(0);
		context->OMSetDepthStencilState(0, 0);

		context->ClearRenderTargetView(ppbRTV, color);
		context->OMSetRenderTargets(1, &ppbRTV, depthStencilView);

		ebVS->SetShader();

		ebPS->SetFloat("BloomTreshold", 0.45f);
		ebPS->SetShaderResourceView("ColorTexture", ppSRV);
		ebPS->SetSamplerState("Sampler", SampleState);
		ebPS->CopyAllBufferData();
		ebPS->SetShader();


		ID3D11Buffer* nothing = 0;
		context->IASetVertexBuffers(0, 1, &nothing, &stride0, &offset0);
		context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

		// Actually draw exactly 3 vertices
		context->Draw(3, 0);
		// Unbind the post process texture from input
		ebPS->SetShaderResourceView("ColorTexture", 0);


		context->RSSetState(0);
		context->OMSetDepthStencilState(0, 0);

		context->ClearRenderTargetView(ppbloomRTV, color);
		context->ClearDepthStencilView(
			depthStencilView,
			D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
			1.0f,
			0);
		context->OMSetRenderTargets(1, &ppbloomRTV, depthStencilView);

		//context->OMSetRenderTargets(1, &backBufferRTV, 0);

		ppVS->SetShader();

		ppPS->SetInt("blurAmount", 5);
		ppPS->SetFloat("pixelWidth", 1.0f / width);
		ppPS->SetFloat("pixelHeight", 1.0f / height);
		ppPS->SetShaderResourceView("Pixels", ppbSRV);
		ppPS->SetShaderResourceView("ColorTexture", srvlight);
		ppPS->SetSamplerState("Sampler", SampleState);
		ppPS->CopyAllBufferData();
		ppPS->SetShader();

		ID3D11Buffer* nothing0 = 0;
		context->IASetVertexBuffers(0, 1, &nothing0, &stride0, &offset0);
		context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

		// Actually draw exactly 3 vertices
		context->Draw(3, 0);

		// Unbind the post process texture from input
		ppPS->SetShaderResourceView("Pixels", 0);
		ppPS->SetShaderResourceView("ColorTexture", 0);

		
		
		
		context->RSSetState(0);
		context->OMSetDepthStencilState(0, 0);

		context->OMSetRenderTargets(1, &backBufferRTV, 0);

		crateRect = { 1000,50 , 1296, 100 };

		spriteBatch->Begin();

		spriteBatch->Draw(ppbloomSRV, crateRect);

		spriteBatch->End();

	/*
		UINT stride1 = sizeof(Vertex);
		UINT offset1 = 0;


		ID3D11Buffer *  vb0 = LightSource->GetMesh()->GetVertexBuffer();
		ID3D11Buffer *  ib0 = LightSource->GetMesh()->GetIndexBuffer();

		context->IASetVertexBuffers(0, 1, &vb0, &stride1, &offset1);
		context->IASetIndexBuffer(ib0, DXGI_FORMAT_R32_UINT, 0);

		vertexShader->SetMatrix4x4("world", LightSource->GetMatrix());
		vertexShader->SetMatrix4x4("view", c->GetViewMatrix());
		vertexShader->SetMatrix4x4("projection", c->GetProjectionMatrix());

		vertexShader->SetMatrix4x4("shadowView", shadowViewMatrix);
		vertexShader->SetMatrix4x4("shadowProjection", shadowProjectionMatrix);

		vertexShader->SetMatrix4x4("view2", viewMatrix2);
		vertexShader->SetMatrix4x4("projection2", projectionMatrix2);
		vertexShader->CopyAllBufferData();
		vertexShader->SetShader();
		//VS_Shadow->deviceContext->PSSetShader(0, 0, 0);

		//pixelShader->SetData("directionalLight",&directionalLight,sizeof(DirectionalLight));
		//pixelShader->SetData("directionalLight2",&directionalLight2,sizeof(DirectionalLight));
		//pixelShader->SetData("pointLight",&pointLight,sizeof(PointLight));

		pixelShader->SetSamplerState("Sampler", SampleState);
		pixelShader->SetSamplerState("ShadowSampler", Sampler_Shadow);

		pixelShader->SetShaderResourceView("Texture", ppbloomSRV);
		pixelShader->SetShaderResourceView("NormalMap", normalMapSRV);
		pixelShader->SetShaderResourceView("ShadowMap", SRV_Shadow);
		//pixelShader->SetShaderResourceView("diffuseTexture", srv);
		pixelShader->SetShaderResourceView("projectionTexture", srv1);

		pixelShader->SetShaderResourceView("Sky", skySRV);


		//pixelShader->SetData("pl", &pl, sizeof(PointLight));



		pixelShader->CopyAllBufferData();
		pixelShader->SetShader();

		context->DrawIndexed(
			LightSource->GetMesh()->GetIndexCount(),     // The number of indices to use (we could draw a subset if we wanted)
			0,     // Offset to the first index we want to use
			0);    // Offset to add to each index when looking up vertices
	
	*/
		context->RSSetState(0);
		context->OMSetDepthStencilState(0, 0);

		
		swprintf_s(showScore, L"%d", score);

		
		scoreText[2]->getSpriteBatch()->Begin();
		scoreText[2]->getSpriteFont()->DrawString(
			scoreText[2]->getSpriteBatch(),
			L"Score: ",
			XMFLOAT2(10, 120));

		scoreText[2]->getSpriteBatch()->End();


		scoreText[0]->getSpriteBatch()->Begin();
		scoreText[0]->getSpriteFont()->DrawString(
			scoreText[0]->getSpriteBatch(),
			showScore,
			XMFLOAT2(200, 120));

		scoreText[0]->getSpriteBatch()->End();

		//cout << instruction[0];

		scoreText[3]->getSpriteBatch()->Begin();
		scoreText[3]->getSpriteFont()->DrawString(
			scoreText[3]->getSpriteBatch(),
			L"Please choose ",
			XMFLOAT2(450, 90));

		scoreText[3]->getSpriteBatch()->End();

		scoreText[1]->getSpriteBatch()->Begin();
		scoreText[1]->getSpriteFont()->DrawString(
			scoreText[1]->getSpriteBatch(),
			instruction,
			XMFLOAT2(800, 90));

		scoreText[1]->getSpriteBatch()->End();
		
		// Particle states
		float blend[4] = { 1,1,1,1 };
		context->OMSetBlendState(particleBlendState, blend, 0xffffffff);  // Additive blending
		context->OMSetDepthStencilState(particleDepthState, 0);			// No depth WRITING

																		// Draw the emitter
		//emitter_yellow_explosion->Draw(context, c);

		emitter_red_explosion->Draw(context, c);
		emitter_red_explosion2->Draw(context, c);
		emitter_red_explosion3->Draw(context, c);
		emitter_red_explosion4->Draw(context, c);

		//emitter_fog->Draw(context, c);
		// Particle states end

		context->RSSetState(0);
		context->OMSetDepthStencilState(0, 0);
		float factors[4] = { 1,1,1,1 };
		context->OMSetBlendState(0, factors, 0xFFFFFFFF);

		// Present the back buffer to the user
		//  - Puts the final frame we're drawing into the window so the user can see it
		//  - Do this exactly ONCE PER FRAME (always at the very end of the frame)
		swapChain->Present(0, 0);
	}

	if (score < 0) level = 2;
	if (level==2) {

		const float color[4] = { 0,0,0, 0.0f };
		context->ClearRenderTargetView(backBufferRTV, color);
		context->ClearDepthStencilView(
			depthStencilView,
			D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
			1.0f,
			0);


		endText[0]->getSpriteBatch()->Begin();
		//scoreText->getSpriteBatch()->Draw(fontSRV, scoreText->getRECT());
		endText[0]->getSpriteFont()->DrawString(
			endText[0]->getSpriteBatch(),
			L"Game Over",
			XMFLOAT2(80, 120));

		endText[0]->getSpriteBatch()->End();
		swapChain->Present(0, 0);
	}
}

void Game::Highlight()
{
	BoundingOrientedBox entBox;
	float distance;
	bool hio = false;

	Entity* nearestEntity;

	std::vector<int> remove_pos_vector;
	for (size_t i = 0; i < E.size(); i++)
	{
		E[i]->GetMesh()->getOBB().Transform(entBox, XMMatrixTranspose(XMLoadFloat4x4(&E[i]->GetMatrix())));
		if (camFrustum.Contains(entBox) != ContainmentType::DISJOINT)
		{
			if (entBox.Intersects(rayPos, rayDir, distance))
			{
				remove_pos_vector.push_back(i);
				nearestEntity = E[i];
				hio = true;
			}
		}
	}
	if (hio)
	{
		// draw the outline
		ID3D11Buffer *  vb = nearestEntity->GetMesh()->GetVertexBuffer();
		ID3D11Buffer *  ib = nearestEntity->GetMesh()->GetIndexBuffer();

		UINT stride = sizeof(Vertex);
		UINT offset = 0;

		context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
		context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);


		// Set up outline VS shader
		VS_Outline->SetShader();
		VS_Outline->SetMatrix4x4("world", nearestEntity->GetMatrix());
		VS_Outline->SetMatrix4x4("view", c->GetViewMatrix());
		VS_Outline->SetMatrix4x4("projection", c->GetProjectionMatrix());
		VS_Outline->CopyAllBufferData();

		// Set up outline PS shader
		PS_Outline->CopyAllBufferData();
		PS_Outline->SetShader();

		context->RSSetState(RS_Outline);

		// Finally do the actual drawing
		context->DrawIndexed(nearestEntity->GetMesh()->GetIndexCount(), 0, 0);

		// reset the rasterizer state
		context->RSSetState(0);
		context->OMSetDepthStencilState(0, 0);


		hio = false;
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

	bool ent_clicked = false;
	Entity* nearestEntity;
	int remove_pos;


	std::vector<int> remove_pos_vector;
	for (size_t i = 0; i < E.size(); i++)
	{
		E[i]->GetMesh()->getOBB().Transform(entBox, XMMatrixTranspose(XMLoadFloat4x4(&E[i]->GetMatrix())));
		if (camFrustum.Contains(entBox) != ContainmentType::DISJOINT)
		{
			if (entBox.Intersects(rayPos, rayDir, distance))
			{
				nearestEntity = E[i];
				vanish_location = nearestEntity->getPos();
				remove_pos = i;
				dir = XMVectorSet(nearestEntity->phy->getTranslation().x, nearestEntity->phy->getTranslation().y - 1, nearestEntity->phy->getTranslation().z + 2, 0);
				ent_clicked = true;
				E[i]->isdecal = true;
			}
		}
	}


	//for (size_t i = 0; i < entityQueue.size(); ++i)
	//{
	//	Entity* entity = entityQueue[i];
	//	int temp_pos = remove_pos_vector[i];

	//	XMMATRIX entMatrix = XMMatrixInverse(NULL, XMMatrixTranspose(XMLoadFloat4x4(&entity->GetMatrix())));
	//	XMVECTOR rayPosLocal = XMVector3TransformCoord(rayPos, entMatrix);
	//	XMVECTOR rayDirLocal = XMVector3Normalize(XMVector3TransformNormal(rayDir, entMatrix));

	//	UINT* faces = entity->GetMesh()->GetFaces(); //indices
	//	Vertex* verts = entity->GetMesh()->GetVertices();

	//	for (size_t j = 0; j < entity->GetMesh()->GetIndexCount(); j += 3)
	//	{
	//		if (TriangleTests::Intersects(rayPosLocal, rayDirLocal, XMLoadFloat3(&verts[faces[j]].Position), XMLoadFloat3(&verts[faces[j + 1]].Position), XMLoadFloat3(&verts[faces[j + 2]].Position), distance))
	//			if (distance < filterDistance)
	//			{
	//				filterDistance = distance;
	//				nearestEntity = entity;
	//				vanish_pos = nearestEntity->getPos();
	//				remove_pos = temp_pos;
	//				//dir = XMVectorSet(entity->phy->getTranslation().x, entity->phy->getTranslation().y - 1, entity->phy->getTranslation().z + 2, 0);
	//				//dir = XMVectorSet(0,0,1, 0);
	//				ent_clicked = true;
	//				entity->isdecal = true;
	//			}
	//	}
	//}

	if (ent_clicked && nearestEntity->GetMesh()->GetName() == Random_Mesh)
	{
		//nearestEntity->SetTrans(XMFLOAT3(20.0f,0.0f,0.0f));
		delete E[remove_pos];
		E.erase(E.begin() + remove_pos);
		en_pos.push_back(en_pos[remove_pos]);
		en_pos.erase(en_pos.begin() + remove_pos);
		pp.push_back(pp[remove_pos]);
		pp.erase(pp.begin() + remove_pos);
		count_down.push_back(6);
		
		entity_vanish = true;

		score++;
		
	}
	
	else if (ent_clicked && nearestEntity->GetMesh()->GetName() != Random_Mesh) {
		//XMVECTOR pos = XMVectorSet(0, 1, -2, 0);
		score--;
		
// Transpose for HLSL!
	}
	else {

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
	camFrustum;
	c->GetFrustum().Transform(camFrustum, XMMatrixInverse(NULL, XMMatrixTranspose(XMLoadFloat4x4(&c->GetViewMatrix()))));
	if (!isnan(camFrustum.Origin.x))
	{
		mouseMove = true;
	}
	proj;
	XMStoreFloat4x4(&proj, XMMatrixTranspose(XMLoadFloat4x4(&c->GetProjectionMatrix())));

	tempRayDir;
	tempRayDir.x = (((2 * (float)x) / width) - 1) / proj._11;
	tempRayDir.y = -(((2 * (float)y) / height) - 1) / proj._22;
	tempRayDir.z = 1.0f;

	view = XMMatrixInverse(NULL, XMMatrixTranspose(XMLoadFloat4x4(&c->GetViewMatrix())));
	rayDir = XMVector3TransformNormal(XMVector3Normalize(XMLoadFloat3(&tempRayDir)), view);
	rayPos = c->GetCameraPosition(); //Was Load Float 3. If it breaks everything BLAME DARREN

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