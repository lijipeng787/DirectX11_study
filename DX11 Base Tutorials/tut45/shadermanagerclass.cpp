
// Filename: shadermanagerclass.cpp

#include "shadermanagerclass.h"
#include <new>


ShaderManagerClass::ShaderManagerClass()
{
	texture_shader_ = 0;
	light_shader_ = nullptr;;
	bumpmap_shader_ = 0;
}


ShaderManagerClass::ShaderManagerClass(const ShaderManagerClass& other)
{
}


ShaderManagerClass::~ShaderManagerClass()
{
}


bool ShaderManagerClass::Initialize(HWND hwnd)
{
	bool result;


	// Create the texture shader object.
	texture_shader_ = ( TextureShaderClass* )_aligned_malloc( sizeof( TextureShaderClass ), 16 );
	new ( texture_shader_ )TextureShaderClass();
	if(!texture_shader_)
	{
		return false;
	}

	// Initialize the texture shader object.
	result = texture_shader_->Initialize(device, hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the texture shader object.", L"Error", MB_OK);
		return false;
	}

	// Create the light shader object.
	light_shader_ = ( LightShaderClass* )_aligned_malloc( sizeof( LightShaderClass ), 16 );
	new ( light_shader_ )LightShaderClass();
	if(!light_shader_)
	{
		return false;
	}

	// Initialize the light shader object.
	result = light_shader_->Initialize(device, hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the light shader object.", L"Error", MB_OK);
		return false;
	}

	// Create the bump map shader object.
	bumpmap_shader_ = ( BumpMapShaderClass* )_aligned_malloc( sizeof( BumpMapShaderClass ), 16 );
	new ( bumpmap_shader_ )BumpMapShaderClass();
	if(!bumpmap_shader_)
	{
		return false;
	}

	// Initialize the bump map shader object.
	result = bumpmap_shader_->Initialize(device, hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the bump map shader object.", L"Error", MB_OK);
		return false;
	}

	return true;
}


void ShaderManagerClass::Shutdown()
{
	// Release the bump map shader object.
	if(bumpmap_shader_)
	{
		bumpmap_shader_->Shutdown();
		bumpmap_shader_->~BumpMapShaderClass();
		_aligned_free( bumpmap_shader_ );
		bumpmap_shader_ = 0;
	}


	if(light_shader_)
	{
		light_shader_->Shutdown();
		light_shader_->~LightShaderClass();
		_aligned_free( light_shader_ );
		light_shader_ = nullptr;;
	}

	
	if(texture_shader_)
	{
		texture_shader_->Shutdown();
		texture_shader_->~TextureShaderClass();
		_aligned_free( texture_shader_ );
		texture_shader_ = 0;
	}

	
}


bool ShaderManagerClass::RenderTextureShader(ID3D11DeviceContext* device, int indexCount, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix,const XMMATRIX& projectionMatrix, 
											 ID3D11ShaderResourceView* texture)
{
	bool result;


	// Render the model using the texture shader.
	result = texture_shader_->Render(device, indexCount, worldMatrix, viewMatrix, projectionMatrix, texture);
	if(!result)
	{
		return false;
	}

	return true;
}


bool ShaderManagerClass::RenderLightShader(int indexCount, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix,const XMMATRIX& projectionMatrix, 
										   ID3D11ShaderResourceView* texture, const XMFLOAT3& lightDirection, const XMFLOAT4& ambient, const XMFLOAT4& diffuse, 
										   const XMFLOAT3& cameraPosition, const XMFLOAT4& specular, float specularPower)
{
	bool result;


	// Render the model using the light shader.
	result = light_shader_->Render(indexCount, worldMatrix, viewMatrix, projectionMatrix, texture, lightDirection, ambient, diffuse, cameraPosition, 
								   specular, specularPower);
	if(!result)
	{
		return false;
	}

	return true;
}


bool ShaderManagerClass::RenderBumpMapShader(int indexCount, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix,const XMMATRIX& projectionMatrix, 
											 ID3D11ShaderResourceView* colorTexture, ID3D11ShaderResourceView* normalTexture, const XMFLOAT3& lightDirection, 
											 const XMFLOAT4& diffuse )
{
	bool result;


	// Render the model using the bump map shader.
	result = bumpmap_shader_->Render(indexCount, worldMatrix, viewMatrix, projectionMatrix, colorTexture, normalTexture, lightDirection, diffuse);
	if(!result)
	{
		return false;
	}

	return true;
}