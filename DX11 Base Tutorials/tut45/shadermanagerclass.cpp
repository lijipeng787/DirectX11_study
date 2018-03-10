
// Filename: shadermanagerclass.cpp

#include "shadermanagerclass.h"
#include <new>


ShaderManagerClass::ShaderManagerClass()
{
	m_TextureShader = 0;
	light_shader_ = nullptr;;
	m_BumpMapShader = 0;
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
	m_TextureShader = ( TextureShaderClass* )_aligned_malloc( sizeof( TextureShaderClass ), 16 );
	new ( m_TextureShader )TextureShaderClass();
	if(!m_TextureShader)
	{
		return false;
	}

	// Initialize the texture shader object.
	result = m_TextureShader->Initialize(device, hwnd);
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
	m_BumpMapShader = ( BumpMapShaderClass* )_aligned_malloc( sizeof( BumpMapShaderClass ), 16 );
	new ( m_BumpMapShader )BumpMapShaderClass();
	if(!m_BumpMapShader)
	{
		return false;
	}

	// Initialize the bump map shader object.
	result = m_BumpMapShader->Initialize(device, hwnd);
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
	if(m_BumpMapShader)
	{
		m_BumpMapShader->Shutdown();
		m_BumpMapShader->~BumpMapShaderClass();
		_aligned_free( m_BumpMapShader );
		m_BumpMapShader = 0;
	}

	// Release the light shader object.
	if(light_shader_)
	{
		light_shader_->Shutdown();
		light_shader_->~LightShaderClass();
		_aligned_free( light_shader_ );
		light_shader_ = nullptr;;
	}

	
	if(m_TextureShader)
	{
		m_TextureShader->Shutdown();
		m_TextureShader->~TextureShaderClass();
		_aligned_free( m_TextureShader );
		m_TextureShader = 0;
	}

	
}


bool ShaderManagerClass::RenderTextureShader(ID3D11DeviceContext* device, int indexCount, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix,const XMMATRIX& projectionMatrix, 
											 ID3D11ShaderResourceView* texture)
{
	bool result;


	// Render the model using the texture shader.
	result = m_TextureShader->Render(device, indexCount, worldMatrix, viewMatrix, projectionMatrix, texture);
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
	result = light_shader_->Render(device_context, indexCount, worldMatrix, viewMatrix, projectionMatrix, texture, lightDirection, ambient, diffuse, cameraPosition, 
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
	result = m_BumpMapShader->Render(device_context, indexCount, worldMatrix, viewMatrix, projectionMatrix, colorTexture, normalTexture, lightDirection, diffuse);
	if(!result)
	{
		return false;
	}

	return true;
}