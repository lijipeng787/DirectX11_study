
// Filename: glassshaderclass.cpp

#include "glassshaderclass.h"


GlassShaderClass::GlassShaderClass()
{
	vertex_shader_ = 0;
	pixel_shader_ = 0;
	layout_ = 0;
	sample_state_ = 0;
	matrix_buffer_ = 0;
	m_glassBuffer = 0;
}


GlassShaderClass::GlassShaderClass(const GlassShaderClass& other)
{
}


GlassShaderClass::~GlassShaderClass()
{
}


bool GlassShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
	bool result;


	
	result = InitializeShader(device, hwnd, L"../../tut32/glass.vs", L"../../tut32/glass.ps");
	if(!result)
	{
		return false;
	}

	return true;
}


void GlassShaderClass::Shutdown()
{

	ShutdownShader();

	
}


bool GlassShaderClass::Render(ID3D11DeviceContext* device_context, int indexCount, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix,
							  const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* colorTexture, 
							  ID3D11ShaderResourceView* normalTexture, ID3D11ShaderResourceView* refractionTexture, 
							  float refractionScale)
{
	bool result;



	result = SetShaderParameters(device_context, worldMatrix, viewMatrix, projectionMatrix, colorTexture, 
								 normalTexture, refractionTexture, refractionScale);
	if(!result)
	{
		return false;
	}


	RenderShader(device_context, indexCount);

	return true;
}


bool GlassShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	unsigned int numElements;
    D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC glassBufferDesc;


	
	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

    
	result = D3DCompileFromFile(vsFilename, NULL, NULL, "GlassVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
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

    
	result = D3DCompileFromFile(psFilename, NULL, NULL, "GlassPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
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

    // Setup the description of the matrix dynamic constant buffer that is in the vertex shader.
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

	// Setup the description of the glass dynamic constant buffer that is in the pixel shader.
    glassBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	glassBufferDesc.ByteWidth = sizeof(GlassBufferType);
    glassBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    glassBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    glassBufferDesc.MiscFlags = 0;
	glassBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the pixel shader constant buffer from within this class.
	result = device->CreateBuffer(&glassBufferDesc, NULL, &m_glassBuffer);
	if(FAILED(result))
	{
		return false;
	}

	return true;
}


void GlassShaderClass::ShutdownShader()
{
	// Release the glass constant buffer.
	if(m_glassBuffer)
	{
		m_glassBuffer->Release();
		m_glassBuffer = 0;
	}


	if(matrix_buffer_)
	{
		matrix_buffer_->Release();
		matrix_buffer_ = 0;
	}


	if(sample_state_)
	{
		sample_state_->Release();
		sample_state_ = 0;
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


void GlassShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
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


bool GlassShaderClass::SetShaderParameters(ID3D11DeviceContext* device_context, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix,
										   const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* colorTexture, 
										   ID3D11ShaderResourceView* normalTexture, ID3D11ShaderResourceView* refractionTexture,
										   float refractionScale)
{
	HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	GlassBufferType* dataPtr2;
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

	// Get a pointer to the data in the matrix constant buffer.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// Copy the matrices into the matrix constant buffer.
	dataPtr->world = worldMatrixCopy;
	dataPtr->view = viewMatrixCopy;
	dataPtr->projection = projectionMatrixCopy;

	// Unlock the matrix constant buffer.
    device_context->Unmap(matrix_buffer_, 0);

	// Set the position of the matrix constant buffer in the vertex shader.
	buffer_number = 0;

	// Now set the matrix constant buffer in the vertex shader with the updated values.
    device_context->VSSetConstantBuffers(buffer_number, 1, &matrix_buffer_);

	// Set the three shader texture resources in the pixel shader.
	device_context->PSSetShaderResources(0, 1, &colorTexture);
	device_context->PSSetShaderResources(1, 1, &normalTexture);
	device_context->PSSetShaderResources(2, 1, &refractionTexture);

	// Lock the glass constant buffer so it can be written to.
	result = device_context->Map(m_glassBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the glass constant buffer.
	dataPtr2 = (GlassBufferType*)mappedResource.pData;

	// Copy the variables into the glass constant buffer.
	dataPtr2->refractionScale = refractionScale;
	dataPtr2->padding = XMFLOAT3(0.0f, 0.0f, 0.0f);

	// Unlock the glass constant buffer.
    device_context->Unmap(m_glassBuffer, 0);

	// Set the position of the glass constant buffer in the pixel shader.
	buffer_number = 0;

	// Now set the glass constant buffer in the pixel shader with the updated values.
    device_context->PSSetConstantBuffers(buffer_number, 1, &m_glassBuffer);

	return true;
}


void GlassShaderClass::RenderShader(ID3D11DeviceContext* device_context, int indexCount)
{

	device_context->IASetInputLayout(layout_);

 
    device_context->VSSetShader(vertex_shader_, NULL, 0);
    device_context->PSSetShader(pixel_shader_, NULL, 0);

	
	device_context->PSSetSamplers(0, 1, &sample_state_);

	
	device_context->DrawIndexed(indexCount, 0, 0);

	
}