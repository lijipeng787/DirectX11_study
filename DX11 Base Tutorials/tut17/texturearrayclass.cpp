


#include "texturearrayclass.h"


TextureArrayClass::TextureArrayClass()
{
	textures_[0] = nullptr;
	textures_[1] = nullptr;
}


TextureArrayClass::TextureArrayClass(const TextureArrayClass& other)
{
}


TextureArrayClass::~TextureArrayClass()
{
}


bool TextureArrayClass::Initialize(WCHAR* filename1, WCHAR* filename2)
{
	HRESULT result;


	
	result = CreateDDSTextureFromFile( device, filename1, NULL, &textures_[ 0 ] );
	if(FAILED(result))
	{
		return false;
	}

	
	result = CreateDDSTextureFromFile( device, filename2, NULL, &textures_[ 1 ] );
	if(FAILED(result))
	{
		return false;
	}

	return true;
}


void TextureArrayClass::Shutdown()
{
	
	if(textures_[0])
	{
		textures_[0]->Release();
		textures_[0] = nullptr;
	}

	if(textures_[1])
	{
		textures_[1]->Release();
		textures_[1] = nullptr;
	}

	
}


ID3D11ShaderResourceView** TextureArrayClass::GetTextureArray()
{
	return textures_;
}