
// Filename: colorshaderclass.cpp

#include "colorshaderclass.h"


ColorShaderClass::ColorShaderClass()
{
	vertex_shader_ = 0;
	m_hullShader = 0;
	m_domainShader = 0;
	pixel_shader_ = 0;
	layout_ = 0;
	matrix_buffer_ = 0;
	m_tessellationBuffer = 0;
}


ColorShaderClass::ColorShaderClass(const ColorShaderClass& other)
{
}


ColorShaderClass::~ColorShaderClass()
{
}


bool ColorShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
	bool result;


	// Initialize the shaders.
	result = InitializeShader(device, hwnd, L"../../tut38/color.vs", L"../../tut38/color.hs", L"../../tut38/color.ds", L"../../tut38/color.ps");
	if(!result)
	{
		return false;
	}

	return true;
}


void ColorShaderClass::Shutdown()
{
	// Shutdown the shaders as well as the related objects.
	ShutdownShader();

	
}


bool ColorShaderClass::Render(ID3D11DeviceContext* device_context, int indexCount, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix,
							  const XMMATRIX& projectionMatrix, float tessellationAmount)
{
	bool result;



	result = SetShaderParameters(device_context, worldMatrix, viewMatrix, projectionMatrix, tessellationAmount);
	if(!result)
	{
		return false;
	}


	RenderShader(device_context, indexCount);

	return true;
}


bool ColorShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* hsFilename, WCHAR* dsFilename, WCHAR* psFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* hullShaderBuffer;
	ID3D10Blob* domainShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	unsigned int numElements;
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC tessellationBufferDesc;


	
	errorMessage = 0;
	vertexShaderBuffer = 0;
	hullShaderBuffer = 0;
	domainShaderBuffer = 0;
	pixelShaderBuffer = 0;

    
	result = D3DCompileFromFile(vsFilename, NULL, NULL, "ColorVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
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

    // Compile the hull shader code.
	result = D3DCompileFromFile(hsFilename, NULL, NULL, "ColorHullShader", "hs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
								   &hullShaderBuffer, &errorMessage);
	if(FAILED(result))
	{
		
		if(errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, hsFilename);
		}
		
		else
		{
			MessageBox(hwnd, hsFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

    // Compile the domain shader code.
	result = D3DCompileFromFile(dsFilename, NULL, NULL, "ColorDomainShader", "ds_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
								   &domainShaderBuffer, &errorMessage);
	if(FAILED(result))
	{
		
		if(errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, dsFilename);
		}
		
		else
		{
			MessageBox(hwnd, dsFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

    
	result = D3DCompileFromFile(psFilename, NULL, NULL, "ColorPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
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

	// Create the vertex shader from the buffer.
    result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &vertex_shader_);
	if(FAILED(result))
	{
		return false;
	}

    // Create the hull shader from the buffer.
    result = device->CreateHullShader(hullShaderBuffer->GetBufferPointer(), hullShaderBuffer->GetBufferSize(), NULL, &m_hullShader);
	if(FAILED(result))
	{
		return false;
	}

    // Create the domain shader from the buffer.
    result = device->CreateDomainShader(domainShaderBuffer->GetBufferPointer(), domainShaderBuffer->GetBufferSize(), NULL, &m_domainShader);
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

	polygonLayout[1].SemanticName = "COLOR";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	
    numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	
	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &layout_);
	if(FAILED(result))
	{
		return false;
	}

	// Release the shader buffers since they are no longer needed.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	hullShaderBuffer->Release();
	hullShaderBuffer = 0;

	domainShaderBuffer->Release();
	domainShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

    // Setup the description of the dynamic matrix constant buffer that is in the domain shader.
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the domain shader constant buffer from within this class.
	result = device->CreateBuffer(&matrixBufferDesc, NULL, &matrix_buffer_);
	if(FAILED(result))
	{
		return false;
	}

    // Setup the description of the dynamic tessellation constant buffer that is in the hull shader.
    tessellationBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	tessellationBufferDesc.ByteWidth = sizeof(TessellationBufferType);
    tessellationBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    tessellationBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    tessellationBufferDesc.MiscFlags = 0;
	tessellationBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the hull shader constant buffer from within this class.
	result = device->CreateBuffer(&tessellationBufferDesc, NULL, &m_tessellationBuffer);
	if(FAILED(result))
	{
		return false;
	}

	return true;
}


void ColorShaderClass::ShutdownShader()
{
	// Release the tessellation constant buffer.
	if(m_tessellationBuffer)
	{
		m_tessellationBuffer->Release();
		m_tessellationBuffer = 0;
	}


	if(matrix_buffer_)
	{
		matrix_buffer_->Release();
		matrix_buffer_ = 0;
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

	// Release the domain shader.
	if(m_domainShader)
	{
		m_domainShader->Release();
		m_domainShader = 0;
	}

	// Release the hull shader.
	if(m_hullShader)
	{
		m_hullShader->Release();
		m_hullShader = 0;
	}

	
	if(vertex_shader_)
	{
		vertex_shader_->Release();
		vertex_shader_ = 0;
	}

	
}


void ColorShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
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


bool ColorShaderClass::SetShaderParameters(ID3D11DeviceContext* device_context, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix,
										   const XMMATRIX& projectionMatrix, float tessellationAmount)
{
	HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int buffer_number;
	MatrixBufferType* dataPtr;
	TessellationBufferType* dataPtr2;

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

	
	dataPtr->world = worldMatrixCopy;
	dataPtr->view = viewMatrixCopy;
	dataPtr->projection = projectionMatrixCopy;

	// Unlock the matrix constant buffer.
    device_context->Unmap(matrix_buffer_, 0);

	// Set the position of the matrix constant buffer in the domain shader.
	buffer_number = 0;

	// Finally set the matrix constant buffer in the domain shader with the updated values.
    device_context->DSSetConstantBuffers(buffer_number, 1, &matrix_buffer_);

	// Lock the tessellation constant buffer so it can be written to.
	result = device_context->Map(m_tessellationBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the tessellation constant buffer.
	dataPtr2 = (TessellationBufferType*)mappedResource.pData;

	// Copy the tessellation data into the constant buffer.
	dataPtr2->tessellationAmount = tessellationAmount;
	dataPtr2->padding = XMFLOAT3(0.0f, 0.0f, 0.0f);

	// Unlock the tessellation constant buffer.
    device_context->Unmap(m_tessellationBuffer, 0);

	// Set the position of the tessellation constant buffer in the hull shader.
	buffer_number = 0;

	// Now set the tessellation constant buffer in the hull shader with the updated values.
    device_context->HSSetConstantBuffers(buffer_number, 1, &m_tessellationBuffer);

	return true;
}


void ColorShaderClass::RenderShader(ID3D11DeviceContext* device_context, int indexCount)
{

	device_context->IASetInputLayout(layout_);

    // Set the vertex, hull, domain, and pixel shaders that will be used to render this triangle.
    device_context->VSSetShader(vertex_shader_, NULL, 0);
    device_context->HSSetShader(m_hullShader, NULL, 0);
    device_context->DSSetShader(m_domainShader, NULL, 0);
    device_context->PSSetShader(pixel_shader_, NULL, 0);

	
	device_context->DrawIndexed(indexCount, 0, 0);

	
}