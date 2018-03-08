


#include "textureshaderclass.h"


TextureShaderClass::TextureShaderClass()
{
	vertex_shader_ = 0;
	pixel_shader_ = 0;
	layout_ = 0;
	m_constantBuffer = 0;
	sample_state_ = 0;
}


TextureShaderClass::TextureShaderClass(const TextureShaderClass& other)
{
}


TextureShaderClass::~TextureShaderClass()
{
}


bool TextureShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
	bool result;


	
	result = InitializeShader(device, hwnd, L"../../tut26/texture.vs", L"../../tut26/texture.ps");
	if(!result)
	{
		return false;
	}

	return true;
}


void TextureShaderClass::Shutdown()
{

	ShutdownShader();

	
}


bool TextureShaderClass::Render(ID3D11DeviceContext* device_context, int indexCount, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix,
								const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* texture)
{
	bool result;



	result = SetShaderParameters(device_context, worldMatrix, viewMatrix, projectionMatrix, texture);
	if(!result)
	{
		return false;
	}


	RenderShader(device_context, indexCount);

	return true;
}


bool TextureShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	unsigned int numElements;
	D3D11_BUFFER_DESC constantBufferDesc;
    D3D11_SAMPLER_DESC samplerDesc;


	
	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

    
	result = D3DCompileFromFile(vsFilename, NULL, NULL, "TextureVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
								   &vertexShaderBuffer, &errorMessage );
	if(FAILED(result))
	{
		
		if(errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
		}
		// If there was  nothing in the error message then it simply could not find the shader file itself.
		else
		{
			MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

    
	result = D3DCompileFromFile(psFilename, NULL, NULL, "TexturePixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
								   &pixelShaderBuffer, &errorMessage );
	if(FAILED(result))
	{
		
		if(errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
		}
		
		else
		{
			MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}


    result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, 
										&vertex_shader_);
	if(FAILED(result))
	{
		return false;
	}


    result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, 
									   &pixel_shader_);
	if(FAILED(result))
	{
		return false;
	}

	
	
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	
    numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	
	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), 
									   vertexShaderBuffer->GetBufferSize(), &layout_);
	if(FAILED(result))
	{
		return false;
	}

	
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

    // Setup the description of the dynamic constant buffer that is in the vertex shader.
    constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constantBufferDesc.ByteWidth = sizeof(ConstantBufferType);
    constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    constantBufferDesc.MiscFlags = 0;
	constantBufferDesc.StructureByteStride = 0;

	
	result = device->CreateBuffer(&constantBufferDesc, NULL, &m_constantBuffer);
	if(FAILED(result))
	{
		return false;
	}

	
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	
    result = device->CreateSamplerState(&samplerDesc, &sample_state_);
	if(FAILED(result))
	{
		return false;
	}

	return true;
}


void TextureShaderClass::ShutdownShader()
{

	if(sample_state_)
	{
		sample_state_->Release();
		sample_state_ = 0;
	}

	// Release the constant buffer.
	if(m_constantBuffer)
	{
		m_constantBuffer->Release();
		m_constantBuffer = 0;
	}

	
	if(layout_)
	{
		layout_->Release();
		layout_ = 0;
	}

	
	if(pixel_shader_)
	{
		pixel_shader_->Release();
		pixel_shader_ = 0;
	}

	
	if(vertex_shader_)
	{
		vertex_shader_->Release();
		vertex_shader_ = 0;
	}

	
}


void TextureShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
	char* compileErrors;
	SIZE_T bufferSize, i;
	ofstream fout;


	
	compileErrors = (char*)(errorMessage->GetBufferPointer());


	bufferSize = errorMessage->GetBufferSize();

	
	fout.open("shader-error.txt");


	for(i=0; i<bufferSize; i++)
	{
		fout << compileErrors[i];
	}

	
	fout.close();

	
	errorMessage->Release();
	errorMessage = 0;

	
	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);

	
}


bool TextureShaderClass::SetShaderParameters(ID3D11DeviceContext* device_context, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix,
											 const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* texture)
{
	HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
	ConstantBufferType* dataPtr;
	unsigned int buffer_number;

	XMMATRIX worldMatrixCopy = worldMatrix;
	XMMATRIX viewMatrixCopy = viewMatrix;
	XMMATRIX projectionMatrixCopy = projectionMatrix;


	worldMatrixCopy = XMMatrixTranspose( worldMatrix );
	viewMatrixCopy = XMMatrixTranspose( viewMatrix );
	projectionMatrixCopy = XMMatrixTranspose( projectionMatrix );


	result = device_context->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	
	dataPtr = (ConstantBufferType*)mappedResource.pData;

	
	dataPtr->world = worldMatrixCopy;
	dataPtr->view = viewMatrixCopy;
	dataPtr->projection = projectionMatrixCopy;

	
    device_context->Unmap(m_constantBuffer, 0);

	
	buffer_number = 0;

	
    device_context->VSSetConstantBuffers(buffer_number, 1, &m_constantBuffer);

	
	device_context->PSSetShaderResources(0, 1, &texture);

	return true;
}


void TextureShaderClass::RenderShader(ID3D11DeviceContext* device_context, int indexCount)
{

	device_context->IASetInputLayout(layout_);

 
    device_context->VSSetShader(vertex_shader_, NULL, 0);
    device_context->PSSetShader(pixel_shader_, NULL, 0);

	
	device_context->PSSetSamplers(0, 1, &sample_state_);

	
	device_context->DrawIndexed(indexCount, 0, 0);

	
}