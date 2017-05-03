#include "Game.h"
#include "Vertex.h"
#include "WICTextureLoader.h" // From DirectX Tool Kit

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
	vertexBuffer = 0;
	indexBuffer = 0;
	vertexShader = 0;
	pixelShader = 0;
	camera = 0; 
	tempEmitterFlag = 1;
	TimeSinceBeginningForGame = 0;
	tempResetEmitterFlag = 0;

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
	if (vertexBuffer) { vertexBuffer->Release(); }
	if (indexBuffer) { indexBuffer->Release(); }
	sampler->Release();
	textureSRV->Release();
	normalMapSRV->Release();

	particleTexture->Release();
	particleTexture1->Release();
	fogParticleTexture->Release();

	particleBlendState->Release();
	particleDepthState->Release();

	// Delete our simple shader objects, which
	// will clean up their own internal DirectX stuff
	delete vertexShader;
	delete pixelShader;
	delete particleVS;
	delete particlePS;

	// Clean up resources
	for(auto& e : entities) delete e;
	for(auto& m : meshes) delete m;
	delete camera;
	delete emitter_yellow_explosion;
	delete emitter_red_explosion;
	delete emitter_fog;
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
	LoadShaders();
	CreateMatrices();
	CreateBasicGeometry();

	

	// Load texture stuff
	HRESULT result = CreateWICTextureFromFile(
		device,
		context, // If I provide the context, it will auto-generate Mipmaps
		L"Debug/Textures/rock.jpg",
		0, // We don't actually need the texture reference
		&textureSRV);
	CreateWICTextureFromFile(device, context, L"Debug/Textures/rockNormals.jpg", 0, &normalMapSRV);
	DirectX::CreateWICTextureFromFile(device, context, L"Debug/Textures/circleParticle.jpg", 0, &particleTexture);
	DirectX::CreateWICTextureFromFile(device, context, L"Debug/Textures/particle.jpg", 0, &particleTexture1);
	DirectX::CreateWICTextureFromFile(device, context, L"Debug/Textures/fogParticle.jpg", 0, &fogParticleTexture);




	// Create a sampler state for texture sampling
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	
	// Ask the device to create a state
	device->CreateSamplerState(&samplerDesc, &sampler);


	// A depth state for the particles
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // Turns off depth writing
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	device->CreateDepthStencilState(&dsDesc, &particleDepthState);


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












	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
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

	particleVS = new SimpleVertexShader(device, context);
	if (!particleVS->LoadShaderFile(L"Debug/ParticleVS.cso"))
		particleVS->LoadShaderFile(L"ParticleVS.cso");

	particlePS = new SimplePixelShader(device, context);
	if(!particlePS->LoadShaderFile(L"Debug/ParticlePS.cso"))	
		particlePS->LoadShaderFile(L"ParticlePS.cso");

	// You'll notice that the code above attempts to load each
	// compiled shader file (.cso) from two different relative paths.

	// This is because the "working directory" (where relative paths begin)
	// will be different during the following two scenarios:
	//  - Debugging in VS: The "Project Directory" (where your .cpp files are) 
	//  - Run .exe directly: The "Output Directory" (where the .exe & .cso files are)

	// Checking both paths is the easiest way to ensure both 
	// scenarios work correctly, although others exist
}



// --------------------------------------------------------
// Initializes the matrices necessary to represent our geometry's 
// transformations and our 3D camera
// --------------------------------------------------------
void Game::CreateMatrices()
{
	camera = new Camera(0, 0, -5);
	camera->UpdateProjectionMatrix((float)width / height);
}


// --------------------------------------------------------
// Loads geometry and creates entities (for this demo)
// --------------------------------------------------------
void Game::CreateBasicGeometry()
{
	Mesh* sphereMesh = new Mesh("Models/sphere.obj", device);
	Mesh* helixMesh = new Mesh("Models/helix.obj", device);
	Mesh* cubeMesh = new Mesh("Models/cube.obj", device);

	meshes.push_back(sphereMesh);
	meshes.push_back(helixMesh);
	meshes.push_back(cubeMesh);

	// Make some entities
	GameEntity* sphere = new GameEntity(sphereMesh);
	GameEntity* helix = new GameEntity(helixMesh);
	GameEntity* cube = new GameEntity(cubeMesh);
	entities.push_back(sphere);
	entities.push_back(helix);
	entities.push_back(cube);

	sphere->SetScale(2.5f, 2.5f, 2.5f);
	helix->SetScale(1.5f, 1.5f, 1.5f);
	cube->SetScale(10, 10, 10);

	currentEntity = 0;
}


// --------------------------------------------------------
// Handle resizing DirectX "stuff" to match the new window size.
// For instance, updating our projection matrix's aspect ratio.
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();

	// Update the projection matrix assuming the
	// camera exists
	if( camera ) 
		camera->UpdateProjectionMatrix((float)width / height);
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Quit if the escape key is pressed
	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();
	//printf("%f",deltaTime);

	TimeSinceBeginningForGame += deltaTime;								// Time passed since beginning of this particle system spawn OR Beginning of game?
	printf("\n%f = Game", TimeSinceBeginningForGame);




//	if (TimeSinceBeginningForGame > 5)									// Execute the code in if block after 5 seconds game has started
//	{
	emitter_yellow_explosion->Update(deltaTime);			// Simple Yellow Red explosion Emitter

		emitter_red_explosion->Update(deltaTime);		// Nomal Explosion Emitter

		emitter_fog->Update(deltaTime);
//}

		if (TimeSinceBeginningForGame > 5 && tempResetEmitterFlag==0)			// Reposition emitter and for how long to execute the particle system
		{
			emitter_red_explosion->ResetEmitter(XMFLOAT3(2, 2, 0), 1.0);
			tempResetEmitterFlag = 1;
		}

	this->deltaTimeNew = deltaTime;


	// Update the camera
	camera->Update(deltaTime);

	// Check for entity swap
	bool currentTab = (GetAsyncKeyState(VK_TAB) & 0x8000) != 0;
	if (currentTab && !prevTab)
		currentEntity = (currentEntity + 1) % entities.size();
	prevTab = currentTab;

	// Spin current entity
	entities[currentEntity]->Rotate(0, deltaTime * 0.2f, 0);

	// Sin wave
	//entities[currentEntity]->SetPosition(sin(totalTime) * 3, 0, 0);

	// Always update current entity's world matrix
	entities[currentEntity]->UpdateWorldMatrix();
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Background color for clearing
	const float color[4] = {0,0,0,0};

	// Clear the render target and depth buffer (erases what's on the screen)
	//  - Do this ONCE PER FRAME
	//  - At the beginning of Draw (before drawing *anything*)
	context->ClearRenderTargetView(backBufferRTV, color);
	context->ClearDepthStencilView(
		depthStencilView, 
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);

	// Grab the data from the first entity's mesh
	GameEntity* ge = entities[currentEntity];
	ID3D11Buffer* vb = ge->GetMesh()->GetVertexBuffer();
	ID3D11Buffer* ib = ge->GetMesh()->GetIndexBuffer();

	// Set buffers in the input assembler
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);

	vertexShader->SetMatrix4x4("world", *ge->GetWorldMatrix());
	vertexShader->SetMatrix4x4("view", camera->GetView());
	vertexShader->SetMatrix4x4("projection", camera->GetProjection());

	vertexShader->CopyAllBufferData();
	vertexShader->SetShader();

	pixelShader->SetFloat3("DirLightDirection", XMFLOAT3(1, 0, 0));
	pixelShader->SetFloat4("DirLightColor", XMFLOAT4(0.8f, 0.8f, 0.8f, 1));
	pixelShader->SetFloat3("PointLightPosition", XMFLOAT3(3, 0, 0));
	pixelShader->SetFloat4("PointLightColor", XMFLOAT4(1, 0.3f, 0.3f, 1));
	pixelShader->SetFloat3("CameraPosition", camera->GetPosition());

	pixelShader->SetSamplerState("Sampler", sampler);
	pixelShader->SetShaderResourceView("Texture", textureSRV);
	pixelShader->SetShaderResourceView("NormalMap", normalMapSRV);

	pixelShader->CopyAllBufferData();
	pixelShader->SetShader();

	// Finally do the actual drawing
//	context->DrawIndexed(ge->GetMesh()->GetIndexCount(), 0, 0);								////////// Drawing circular Mesh frgure


	// Particle states
	float blend[4] = { 1,1,1,1 };
	context->OMSetBlendState(particleBlendState, blend, 0xffffffff);  // Additive blending
	context->OMSetDepthStencilState(particleDepthState, 0);			// No depth WRITING

	// Draw the emitter
	emitter_yellow_explosion->Draw(context, camera);

		emitter_red_explosion->Draw(context, camera);

		emitter_fog->Draw(context, camera);

	// Reset to default states for next frame
	context->OMSetBlendState(0, blend, 0xffffffff);
	context->OMSetDepthStencilState(0, 0);


	// Present the back buffer to the user
	//  - Puts the final frame we're drawing into the window so the user can see it
	//  - Do this exactly ONCE PER FRAME (always at the very end of the frame)
	swapChain->Present(0, 0);
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
//	emitter_yellow_explosion->Update(0.1);
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
	// Check left mouse button
	if (buttonState & 0x0001)
	{
		float xDiff = (x - prevMousePos.x) * 0.005f;
		float yDiff = (y - prevMousePos.y) * 0.005f;
		camera->Rotate(yDiff, xDiff);
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