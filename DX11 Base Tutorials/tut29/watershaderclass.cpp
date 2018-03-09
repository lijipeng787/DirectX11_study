
// Filename: watershaderclass.cpp

#include "watershaderclass.h"


WaterShaderClass::WaterShaderClass()
{
	vertex_shader_ = nullptr;
	pixel_shader_ = nullptr;
	layout_ = nullptr;
	sample_state_ = nullptr;
	matrix_buffer_ = nullptr;
	m_reflectionBuffer = 0;
	m_waterBuffer = 0;
}


WaterShaderClass::WaterShaderClass(const WaterShaderClass& other)
{
}


WaterShaderClass::~WaterShaderClass()
{
}


bool WaterShaderClass::Initialize(HWND hwnd)
{
	bool result;


	
	result = InitializeShader(device, hwnd, L"../../tut29/water.vs", L"../../tut29/water.ps");
	if(!result)
	{
		return false;
	}

	return true;
}


void WaterShaderClass::Shutdown()
{

	ShutdownShader();

	
}


bool WaterShaderClass::Render(int indexCount, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix,
							  const XMMATRIX& projectionMatrix, const XMMATRIX& reflectionMatrix, 
							  ID3D11ShaderResourceView* reflectionTexture, ID3D11ShaderResourceView* refractionTexture,
							  ID3D11ShaderResourceView* normalTexture, float waterTranslation, float reflectRefractScale)
{
	bool result;



	result = SetShaderParameters(device_context, worldMatrix, viewMatrix, projectionMatrix, reflectionMatrix, reflectionTexture, 
								 refractionTexture, normalTexture, waterTranslation, reflectRefractScale);
	if(!result)
	{
		return false;
	}


	RenderShader(device_context, indexCount);

	return true;
}


bool WaterShaderClass::InitializeShader(HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	unsigned int numElements;
    D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC reflectionBufferDesc;
	D3D11_BUFFER_DESC waterBufferDesc;


	
	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

    
	result = D3DCompileFromFile(vsFilename, NULL, NULL, "WaterVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
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

    
	result = D3DCompileFromFile(psFilename, NULL, NULL, "WaterPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
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

	// Setup the description of the reflection dynamic constant buffer that is in the vertex shader.
	reflectionBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	reflectionBufferDesc.ByteWidth = sizeof(ReflectionBufferType);
	reflectionBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	reflectionBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	reflectionBufferDesc.MiscFlags = 0;
	reflectionBufferDesc.StructureByteStride = 0;

	
	result = device->CreateBuffer(&reflectionBufferDesc, NULL, &m_reflectionBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Setup the description of the water dynamic constant buffer that is in the pixel shader.
	waterBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	waterBufferDesc.ByteWidth = sizeof(WaterBufferType);
	waterBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	waterBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	waterBufferDesc.MiscFlags = 0;
	waterBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the pixel shader constant buffer from within this class.
	result = device->CreateBuffer(&waterBufferDesc, NULL, &m_waterBuffer);
	if(FAILED(result))
	{
		return false;
	}

	return true;
}


void WaterShaderClass::ShutdownShader()
{
	// Release the water constant buffer.
	if(m_waterBuffer)
	{
		m_waterBuffer->Release();
		m_waterBuffer = 0;
	}

	// Release the reflection constant buffer.
	if(m_reflectionBuffer)
	{
		m_reflectionBuffer->Release();
		m_reflectionBuffer = 0;
	}


	if(matrix_buffer_)
	{
		matrix_buffer_->Release();
		matrix_buffer_ = nullptr;
	}


	if(sample_state_)
	{
		sample_state_->Release();
		sample_state_ = nullptr;
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


void WaterShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
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


bool WaterShaderClass::SetShaderParameters(const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix,
										   const XMMATRIX& projectionMatrix, const XMMATRIX& reflectionMatrix, 
										   ID3D11ShaderResourceView* reflectionTexture, 
										   ID3D11ShaderResourceView* refractionTexture, ID3D11ShaderResourceView* normalTexture,
										   float waterTranslation, float reflectRefractScale)
{
	HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int buffer_number;
	MatrixBufferType* dataPtr;
	ReflectionBufferType* dataPtr2;
	WaterBufferType* dataPtr3;

	XMMATRIX worldMatrixCopy = worldMatrix;
	XMMATRIX viewMatrixCopy = viewMatrix;
	XMMATRIX projectionMatrixCopy = projectionMatrix;
	XMMATRIX reflectionMatrixCopy = reflectionMatrix;

	// Transpose all the input matrices to prepare them for the shader.
	worldMatrixCopy = XMMatrixTranspose( worldMatrix );
	viewMatrixCopy = XMMatrixTranspose( viewMatrix );
	projectionMatrixCopy = XMMatrixTranspose( projectionMatrix );
	reflectionMatrixCopy = XMMatrixTranspose( reflectionMatrix );


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

	// Lock the reflection constant buffer so it can be written to.
	result = device_context->Map(m_reflectionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	
	dataPtr2 = (ReflectionBufferType*)mappedResource.pData;

	// Copy the reflection matrix into the constant buffer.
	dataPtr2->reflection = reflectionMatrixCopy;

	
	device_context->Unmap(m_reflectionBuffer, 0);

	// Set the position of the reflection constant buffer in the vertex shader.
	buffer_number = 1;

	// Finally set the reflection constant buffer in the vertex shader with the updated values.
	device_context->VSSetConstantBuffers(buffer_number, 1, &m_reflectionBuffer);

	// Set the reflection texture resource in the pixel shader.
	device_context->PSSetShaderResources(0, 1, &reflectionTexture);

	// Set the refraction texture resource in the pixel shader.
	device_context->PSSetShaderResources(1, 1, &refractionTexture);

	// Set the normal map texture resource in the pixel shader.
	device_context->PSSetShaderResources(2, 1, &normalTexture);

	// Lock the water constant buffer so it can be written to.
	result = device_context->Map(m_waterBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	
	dataPtr3 = (WaterBufferType*)mappedResource.pData;

	// Copy the water data into the constant buffer.
	dataPtr3->waterTranslation = waterTranslation;
	dataPtr3->reflectRefractScale = reflectRefractScale;
	dataPtr3->padding = XMFLOAT2(0.0f, 0.0f);

	
	device_context->Unmap(m_waterBuffer, 0);

	// Set the position of the water constant buffer in the pixel shader.
	buffer_number = 0;

	// Finally set the water constant buffer in the pixel shader with the updated values.
	device_context->PSSetConstantBuffers(buffer_number, 1, &m_waterBuffer);

	return true;
}


void WaterShaderClass::RenderShader(int indexCount)
{

	device_context->IASetInputLayout(layout_);

 
    device_context->VSSetShader(vertex_shader_, NULL, 0);
    device_context->PSSetShader(pixel_shader_, NULL, 0);

	
	device_context->PSSetSamplers(0, 1, &sample_state_);

	
	device_context->DrawIndexed(indexCount, 0, 0);

	
}