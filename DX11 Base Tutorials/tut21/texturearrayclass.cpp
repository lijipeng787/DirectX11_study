////////////////////////////////////////////////////////////////////////////////
// Filename: texturearrayclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "texturearrayclass.h"


TextureArrayClass::TextureArrayClass()
{
	textures_[0] = 0;
	textures_[1] = 0;
	textures_[2] = 0;
}


TextureArrayClass::TextureArrayClass(const TextureArrayClass& other)
{
}


TextureArrayClass::~TextureArrayClass()
{
}


bool TextureArrayClass::Initialize(ID3D11Device* device, WCHAR* filename1, WCHAR* filename2, WCHAR* filename3)
{
	HRESULT result;


	// Load the first texture in.
	result = CreateDDSTextureFromFile( device, filename1, NULL, &textures_[ 0 ] );
	if(FAILED(result))
	{
		return false;
	}

	// Load the second texture in.
	result = CreateDDSTextureFromFile( device, filename2, NULL, &textures_[ 1 ] );
	if(FAILED(result))
	{
		return false;
	}

	// Load the third texture in.
	result = CreateDDSTextureFromFile( device, filename3, NULL, &textures_[ 2 ] );
	if(FAILED(result))
	{
		return false;
	}

	return true;
}


void TextureArrayClass::Shutdown()
{
	// Release the texture resources.
	if(textures_[0])
	{
		textures_[0]->Release();
		textures_[0] = 0;
	}

	if(textures_[1])
	{
		textures_[1]->Release();
		textures_[1] = 0;
	}

	if(textures_[2])
	{
		textures_[2]->Release();
		textures_[2] = 0;
	}

	
}


ID3D11ShaderResourceView** TextureArrayClass::GetTextureArray()
{
	return textures_;
}