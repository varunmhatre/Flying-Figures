#include "UI.h"

UI::UI(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11ShaderResourceView* textureSrv, XMFLOAT4 textureRect) {
	spriteBatch = new SpriteBatch(context);
	batchTextureSrv = textureSrv;
	rect = { (int)textureRect.x,(int)textureRect.y,(int)textureRect.z,(int)textureRect.w };
}

UI::UI(ID3D11Device* device , ID3D11DeviceContext* context , wchar_t * fontc, XMFLOAT4 fontRect) {
	spriteBatch = new SpriteBatch(context);
	spriteFont=new SpriteFont(device, L"Assets/Fonts/myfile.spritefont");
	fontContent = fontc;
	rect = { (int)fontRect.x,(int)fontRect.y,(int)fontRect.z,(int)fontRect.w };
}

UI::~UI()
{
	if(spriteBatch)
		delete spriteBatch;
	
	if(spriteFont)
		delete spriteFont;
}

SpriteBatch* UI::getSpriteBatch() {
	return spriteBatch;
}

SpriteFont* UI::getSpriteFont() {
	return spriteFont;
}

RECT UI::getRECT() {
	return rect;
}