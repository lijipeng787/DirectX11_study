#include "deferredshaderclass.h"

#include "../CommonFramework/DirectX11Device.h"

DeferredShaderClass::DeferredShaderClass() {
	vertex_shader_ = nullptr;
	pixel_shader_ = nullptr;
	layout_ = nullptr;
	m_sampleStateWrap = 0;
	matrix_buffer_ = nullptr;
}


DeferredShaderClass::DeferredShaderClass(const DeferredShaderClass& other) {
}


DeferredShaderClass::~DeferredShaderClass() {
}


bool DeferredShaderClass::Initialize(HWND hwnd) {
	bool result;



	result = InitializeShader(hwnd, L"deferred.vs", L"deferred.ps");
	if (!result) {
		return false;
	}

	return true;
}


void DeferredShaderClass::Shutdown() {

	ShutdownShader();


}


bool DeferredShaderClass::Render(int indexCount, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix,
	const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* texture) {
	bool result;



	result = SetShaderParameters(worldMatrix, viewMatrix, projectionMatrix, texture);
	if (!result) {
		return false;
	}


	RenderShader(indexCount);

	return true;
}


bool DeferredShaderClass::InitializeShader(HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename) {
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[3];
	unsigned int numElements;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC matrixBufferDesc;



	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;


	result = D3DCompileFromFile(vsFilename, NULL, NULL, "DeferredVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		&vertexShaderBuffer, &errorMessage);
	if (FAILED(result)) {

		if (errorMessage) {
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
		}

		else {
			MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}


	result = D3DCompileFromFile(psFilename, NULL, NULL, "DeferredPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		&pixelShaderBuffer, &errorMessage);
	if (FAILED(result)) {

		if (errorMessage) {
			OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
		}

		else {
			MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	auto device = DirectX11Device::GetD3d11DeviceInstance()->GetDevice();

	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &vertex_shader_);
	if (FAILED(result)) {
		return false;
	}


	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &pixel_shader_);
	if (FAILED(result)) {
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

	polygonLayout[2].SemanticName = "NORMAL";
	polygonLayout[2].SemanticIndex = 0;
	polygonLayout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[2].InputSlot = 0;
	polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[2].InstanceDataStepRate = 0;


	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);


	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(),
		&layout_);
	if (FAILED(result)) {
		return false;
	}


	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	// Create a wrap texture sampler state description.
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


	result = device->CreateSamplerState(&samplerDesc, &m_sampleStateWrap);
	if (FAILED(result)) {
		return false;
	}


	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;


	result = device->CreateBuffer(&matrixBufferDesc, NULL, &matrix_buffer_);
	if (FAILED(result)) {
		return false;
	}

	return true;
}


void DeferredShaderClass::ShutdownShader() {

	if (matrix_buffer_) {
		matrix_buffer_->Release();
		matrix_buffer_ = nullptr;
	}


	if (m_sampleStateWrap) {
		m_sampleStateWrap->Release();
		m_sampleStateWrap = 0;
	}


	if (layout_) {
		layout_->Release();
		layout_ = nullptr;
	}


	if (pixel_shader_) {
		pixel_shader_->Release();
		pixel_shader_ = nullptr;
	}


	if (vertex_shader_) {
		vertex_shader_->Release();
		vertex_shader_ = nullptr;
	}


}


void DeferredShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename) {
	char* compileErrors;
	SIZE_T bufferSize, i;
	ofstream fout;



	compileErrors = (char*)(errorMessage->GetBufferPointer());


	bufferSize = errorMessage->GetBufferSize();


	fout.open("shader-error.txt");


	for (i = 0; i < bufferSize; i++) {
		fout << compileErrors[i];
	}


	fout.close();


	errorMessage->Release();
	errorMessage = 0;


	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);


}


bool DeferredShaderClass::SetShaderParameters(const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix,
	const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* texture) {
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int buffer_number;
	MatrixBufferType* dataPtr;

	XMMATRIX worldMatrixCopy = worldMatrix;
	XMMATRIX viewMatrixCopy = viewMatrix;
	XMMATRIX projectionMatrixCopy = projectionMatrix;


	worldMatrixCopy = XMMatrixTranspose(worldMatrix);
	viewMatrixCopy = XMMatrixTranspose(viewMatrix);
	projectionMatrixCopy = XMMatrixTranspose(projectionMatrix);

	auto device_context = DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

	result = device_context->Map(matrix_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) {
		return false;
	}


	dataPtr = (MatrixBufferType*)mappedResource.pData;


	dataPtr->world = worldMatrixCopy;
	dataPtr->view = viewMatrixCopy;
	dataPtr->projection = projectionMatrixCopy;


	device_context->Unmap(matrix_buffer_, 0);


	buffer_number = 0;


	device_context->VSSetConstantBuffers(buffer_number, 1, &matrix_buffer_);


	device_context->PSSetShaderResources(0, 1, &texture);

	return true;
}


void DeferredShaderClass::RenderShader(int indexCount) {

	auto device_context = DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

	device_context->IASetInputLayout(layout_);

	// Set the vertex and pixel shaders that will be used to render.
	device_context->VSSetShader(vertex_shader_, NULL, 0);
	device_context->PSSetShader(pixel_shader_, NULL, 0);

	// Set the sampler states in the pixel shader.
	device_context->PSSetSamplers(0, 1, &m_sampleStateWrap);

	// Render the geometry.
	device_context->DrawIndexed(indexCount, 0, 0);


}