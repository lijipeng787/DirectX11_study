
// Filename: glowshaderclass.cpp

#include "glowshaderclass.h"


GlowShaderClass::GlowShaderClass()
{
	vertex_shader_ = nullptr;
	pixel_shader_ = nullptr;
	layout_ = nullptr;
	matrix_buffer_ = nullptr;
	m_glowBuffer = 0;
	sample_state_ = nullptr;
}


GlowShaderClass::GlowShaderClass(const GlowShaderClass& other)
{
}


GlowShaderClass::~GlowShaderClass()
{
}


bool GlowShaderClass::Initialize(HWND hwnd)
{
	bool result;


	
	result = InitializeShader(device, hwnd, L"../../tut46/glow.vs", L"../../tut46/glow.ps");
	if(!result)
	{
		return false;
	}

	return true;
}


void GlowShaderClass::Shutdown()
{

	ShutdownShader();

	
}


bool GlowShaderClass::Render(int indexCount, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix,
							 const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* colorTexture, ID3D11ShaderResourceView* glowTexture, 
							 float glowStrength)
{
	bool result;



	result = SetShaderParameters(worldMatrix, viewMatrix, projectionMatrix, colorTexture, glowTexture, glowStrength);
	if(!result)
	{
		return false;
	}


	RenderShader(indexCount);

	return true;
}


bool GlowShaderClass::InitializeShader(HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	unsigned int numElements;
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC glowBufferDesc;
    D3D11_SAMPLER_DESC samplerDesc;


	
	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

    
	result = D3DCompileFromFile(vsFilename, NULL, NULL, "GlowVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
								   &vertexShaderBuffer, &errorMessage );
	if(FAILED(result))
	{
		
		if(errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
		}
		
		else
		{
			MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

    
	result = D3DCompileFromFile(psFilename, NULL, NULL, "GlowPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
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


    result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &vertex_shader_);
	if(FAILED(result))
	{
		return false;
	}


    result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &pixel_shader_);
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

	
	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), 
		                               &layout_);
	if(FAILED(result))
	{
		return false;
	}

	
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	
	result = device->CreateBuffer(&matrixBufferDesc, NULL, &matrix_buffer_);
	if(FAILED(result))
	{
		return false;
	}
	
	// Setup the description of the dynamic glow constant buffer that is in the pixel shader.
    glowBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	glowBufferDesc.ByteWidth = sizeof(GlowBufferType);
    glowBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    glowBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    glowBufferDesc.MiscFlags = 0;
	glowBufferDesc.StructureByteStride = 0;


	result = device->CreateBuffer(&glowBufferDesc, NULL, &m_glowBuffer);
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


void GlowShaderClass::ShutdownShader()
{

	if(sample_state_)
	{
		sample_state_->Release();
		sample_state_ = nullptr;
	}


	if(m_glowBuffer)
	{
		m_glowBuffer->Release();
		m_glowBuffer = 0;
	}


	if(matrix_buffer_)
	{
		matrix_buffer_->Release();
		matrix_buffer_ = nullptr;
	}

	
	if(layout_)
	{
		layout_->Release();
		layout_ = nullptr;
	}

	
	if(pixel_shader_)
	{
		pixel_shader_->Release();
		pixel_shader_ = nullptr;
	}

	
	if(vertex_shader_)
	{
		vertex_shader_->Release();
		vertex_shader_ = nullptr;
	}

	
}


void GlowShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
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


bool GlowShaderClass::SetShaderParameters(const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix,
										  const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* colorTexture, 
										  ID3D11ShaderResourceView* glowTexture, float glowStrength)
{
	HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	GlowBufferType* dataPtr2;
	unsigned int buffer_number;

	XMMATRIX worldMatrixCopy = worldMatrix;
	XMMATRIX viewMatrixCopy = viewMatrix;
	XMMATRIX projectionMatrixCopy = projectionMatrix;


	worldMatrixCopy = XMMatrixTranspose( worldMatrix );
	viewMatrixCopy = XMMatrixTranspose( viewMatrix );
	projectionMatrixCopy = XMMatrixTranspose( projectionMatrix );


	result = device_context->Map(matrix_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	
	dataPtr->world = worldMatrixCopy;
	dataPtr->view = viewMatrixCopy;
	dataPtr->projection = projectionMatrixCopy;

	
    device_context->Unmap(matrix_buffer_, 0);

	
	buffer_number = 0;

	
    device_context->VSSetConstantBuffers(buffer_number, 1, &matrix_buffer_);


	result = device_context->Map(m_glowBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	
	dataPtr2 = (GlowBufferType*)mappedResource.pData;

	
	dataPtr2->glowStrength = glowStrength;
	dataPtr2->padding = XMFLOAT3(0.0f, 0.0f, 0.0f);

	
    device_context->Unmap(m_glowBuffer, 0);

	// Set the position of the constant buffer in the pixel shader.
	buffer_number = 0;

	// Now set the constant buffer in the pixel shader with the updated values.
    device_context->PSSetConstantBuffers(buffer_number, 1, &m_glowBuffer);

	// Set shader texture resources in the pixel shader.
	device_context->PSSetShaderResources(0, 1, &colorTexture);
	device_context->PSSetShaderResources(1, 1, &glowTexture);

	return true;
}


void GlowShaderClass::RenderShader(int indexCount)
{

	device_context->IASetInputLayout(layout_);

 
    device_context->VSSetShader(vertex_shader_, NULL, 0);
    device_context->PSSetShader(pixel_shader_, NULL, 0);

	
	device_context->PSSetSamplers(0, 1, &sample_state_);

	
	device_context->DrawIndexed(indexCount, 0, 0);

	
}