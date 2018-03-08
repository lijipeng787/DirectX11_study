
// Filename: textureclass.cpp

#include "textureclass.h"


TextureClass::TextureClass()
{
	texture_ = 0;
}


TextureClass::TextureClass(const TextureClass& other)
{
}


TextureClass::~TextureClass()
{
}


bool TextureClass::Initialize(ID3D11Device* device, WCHAR* filename)
{
	HRESULT result;


	
	result = CreateDDSTextureFromFile( device, filename, NULL, &texture_ );
	if(FAILED(result))
	{
		return false;
	}

	return true;
}


void TextureClass::Shutdown()
{
	
	if(texture_)
	{
		texture_->Release();
		texture_ = 0;
	}

	
}


ID3D11ShaderResourceView* TextureClass::GetTexture()
{
	return texture_;
}