#pragma once
#include "SpriteBatch.h"
#include "SimpleMath.h"
#include <SpriteFont.h>
#include <wrl.h>

using namespace std;
using namespace DirectX;
using Microsoft::WRL::ComPtr;
using namespace SimpleMath;

class UI
{
	//unique_ptr<SpriteBatch> m_spriteBatch;
	//
	//ComPtr<ID3D11ShaderResourceView> m_texture;

	SpriteBatch* spriteBatch;
	SpriteFont* spriteFont;
	RECT rect;
	ID3D11ShaderResourceView* batchTextureSrv;
	wchar_t * fontContent;

public:
	UI(ID3D11Device*, ID3D11DeviceContext*, ID3D11ShaderResourceView*, XMFLOAT4);
	UI(ID3D11Device*, ID3D11DeviceContext*, wchar_t *, XMFLOAT4);
	~UI();
	SpriteBatch* getSpriteBatch();
	SpriteFont* getSpriteFont();
	RECT getRECT();
};

