
// Filename: colorshaderclass.cpp

#include "colorshaderclass.h"


ColorShaderClass::ColorShaderClass()
{
	vertex_shader_ = 0;
	pixel_shader_ = 0;
	layout_ = 0;
	matrix_buffer_ = 0;
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


	
	result = InitializeShader(device, hwnd, L"../../tut04/color_vs.hlsl", L"../../tut04/color_ps.hlsl");
	if(!result)
	{
		return false;
	}

	return true;
}


void ColorShaderClass::Shutdown()
{

	ShutdownShader();

	
}


bool ColorShaderClass::Render( ID3D11DeviceContext* device_context, int indexCount, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix,
							  const XMMATRIX& projectionMatrix)
{
	bool result;



	result = SetShaderParameters(device_context, worldMatrix, viewMatrix, projectionMatrix);
	if(!result)
	{
		return false;
	}


	RenderShader(device_context, indexCount);

	return true;
}


bool ColorShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	unsigned int numElements;
	D3D11_BUFFER_DESC matrixBufferDesc;


	
	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

    
	result = D3DCompileFromFile(vsFilename, NULL, NULL, "ColorVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
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

    
	result = D3DCompileFromFile(psFilename, NULL, NULL, "ColorPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
								   &pixelShaderBuffer, &errorMessage );
	if(FAILED(result))
	{
		
		if(errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
		}
		// If there was nothing in the error message then it simply could not find the file itself.
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

	polygonLayout[1].SemanticName = "COLOR";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
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

	return true;
}


void ColorShaderClass::ShutdownShader()
{

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


bool ColorShaderClass::SetShaderParameters( ID3D11DeviceContext* device_context, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix,
										   const XMMATRIX& projectionMatrix)
{
	HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
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

	// Finanly set the constant buffer in the vertex shader with the updated values.
    device_context->VSSetConstantBuffers(buffer_number, 1, &matrix_buffer_);

	return true;
}


void ColorShaderClass::RenderShader(ID3D11DeviceContext* device_context, int indexCount)
{

	device_context->IASetInputLayout(layout_);

 
    device_context->VSSetShader(vertex_shader_, NULL, 0);
    device_context->PSSetShader(pixel_shader_, NULL, 0);

	
	device_context->DrawIndexed(indexCount, 0, 0);

	
}