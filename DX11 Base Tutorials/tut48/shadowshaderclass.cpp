#include "shadowshaderclass.h"

#include <fstream>

#include "../CommonFramework/DirectX11Device.h"

using namespace std;

struct MatrixBufferType {
	XMMATRIX world;
	XMMATRIX view;
	XMMATRIX projection;
	XMMATRIX lightView;
	XMMATRIX lightProjection;
};

struct LightBufferType {
	XMFLOAT4 ambientColor;
	XMFLOAT4 diffuseColor;
	XMFLOAT3 lightDirection;
	float padding;
};

ShadowShaderClass::ShadowShaderClass() {
	vertex_shader_ = nullptr;
	pixel_shader_ = nullptr;
	layout_ = nullptr;
	m_sampleStateWrap = 0;
	m_sampleStateClamp = 0;
	matrix_buffer_ = nullptr;
	light_buffer_ = nullptr;
}

ShadowShaderClass::ShadowShaderClass(const ShadowShaderClass& other) {}

ShadowShaderClass::~ShadowShaderClass() {}

bool ShadowShaderClass::Initialize(HWND hwnd) {

	auto result = InitializeShader(hwnd, L"shadow.vs", L"shadow.ps");
	if (!result) {
		return false;
	}

	return true;
}

void ShadowShaderClass::Shutdown() {
	ShutdownShader();
}

bool ShadowShaderClass::Render(int indexCount, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix,
	const XMMATRIX& projectionMatrix, const XMMATRIX& lightViewMatrix, const XMMATRIX& lightProjectionMatrix,
	ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* depthMapTexture, const XMFLOAT3& lightDirection,
	const XMFLOAT4& ambientColor, const XMFLOAT4& diffuseColor) {

	auto result = SetShaderParameters(worldMatrix, viewMatrix, projectionMatrix, lightViewMatrix, lightProjectionMatrix, texture,
		depthMapTexture, lightDirection, ambientColor, diffuseColor);
	if (!result) {
		return false;
	}

	RenderShader(indexCount);

	return true;
}

bool ShadowShaderClass::InitializeShader(HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename) {

	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;

	errorMessage = 0;
	vertexShaderBuffer = 0;

	auto result = D3DCompileFromFile(
		vsFilename, NULL, NULL, "ShadowVertexShader",
		"vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
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

	ID3D10Blob* pixelShaderBuffer;

	result = D3DCompileFromFile(
		psFilename, NULL, NULL, "ShadowPixelShader",
		"ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
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

	D3D11_INPUT_ELEMENT_DESC polygonLayout[3];

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

	unsigned int numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(),
		&layout_);
	if (FAILED(result)) {
		return false;
	}

	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	D3D11_SAMPLER_DESC samplerDesc;

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

	// Create a clamp texture sampler state description.
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

	result = device->CreateSamplerState(&samplerDesc, &m_sampleStateClamp);
	if (FAILED(result)) {
		return false;
	}

	D3D11_BUFFER_DESC matrixBufferDesc;

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

	D3D11_BUFFER_DESC lightBufferDesc;

	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;


	result = device->CreateBuffer(&lightBufferDesc, NULL, &light_buffer_);
	if (FAILED(result)) {
		return false;
	}

	return true;
}

void ShadowShaderClass::ShutdownShader() {

	if (light_buffer_) {
		light_buffer_->Release();
		light_buffer_ = nullptr;
	}

	if (matrix_buffer_) {
		matrix_buffer_->Release();
		matrix_buffer_ = nullptr;
	}

	if (m_sampleStateWrap) {
		m_sampleStateWrap->Release();
		m_sampleStateWrap = 0;
	}

	if (m_sampleStateClamp) {
		m_sampleStateClamp->Release();
		m_sampleStateClamp = 0;
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

void ShadowShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename) {
	
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


bool ShadowShaderClass::SetShaderParameters(const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix,
	const XMMATRIX& projectionMatrix, const XMMATRIX& lightViewMatrix, const XMMATRIX& lightProjectionMatrix,
	ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* depthMapTexture, const XMFLOAT3& lightDirection,
	const XMFLOAT4& ambientColor, const XMFLOAT4& diffuseColor) {

	XMMATRIX worldMatrixCopy = worldMatrix;
	XMMATRIX viewMatrixCopy = viewMatrix;
	XMMATRIX projectionMatrixCopy = projectionMatrix;
	XMMATRIX lightViewMatrixCopy = lightViewMatrix;
	XMMATRIX lightProjectionMatrixCopy = lightProjectionMatrix;

	worldMatrixCopy = XMMatrixTranspose(worldMatrix);
	viewMatrixCopy = XMMatrixTranspose(viewMatrix);
	projectionMatrixCopy = XMMatrixTranspose(projectionMatrix);
	lightViewMatrixCopy = XMMatrixTranspose(lightViewMatrix);
	lightProjectionMatrixCopy = XMMatrixTranspose(lightProjectionMatrix);

	auto device_context = DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

	D3D11_MAPPED_SUBRESOURCE mappedResource;

	auto result = device_context->Map(matrix_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) {
		return false;
	}

	MatrixBufferType* dataPtr;

	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = worldMatrixCopy;
	dataPtr->view = viewMatrixCopy;
	dataPtr->projection = projectionMatrixCopy;
	dataPtr->lightView = lightViewMatrixCopy;
	dataPtr->lightProjection = lightProjectionMatrixCopy;

	device_context->Unmap(matrix_buffer_, 0);

	unsigned int buffer_number = 0;

	device_context->VSSetConstantBuffers(buffer_number, 1, &matrix_buffer_);

	device_context->PSSetShaderResources(0, 1, &texture);
	device_context->PSSetShaderResources(1, 1, &depthMapTexture);

	result = device_context->Map(light_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) {
		return false;
	}

	LightBufferType* dataPtr2;

	dataPtr2 = (LightBufferType*)mappedResource.pData;

	dataPtr2->ambientColor = ambientColor;
	dataPtr2->diffuseColor = diffuseColor;
	dataPtr2->lightDirection = lightDirection;
	dataPtr2->padding = 0.0f;

	device_context->Unmap(light_buffer_, 0);

	buffer_number = 0;

	device_context->PSSetConstantBuffers(buffer_number, 1, &light_buffer_);

	return true;
}

void ShadowShaderClass::RenderShader(int indexCount) {

	auto device_context = DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

	device_context->IASetInputLayout(layout_);
	device_context->VSSetShader(vertex_shader_, NULL, 0);
	device_context->PSSetShader(pixel_shader_, NULL, 0);
	device_context->PSSetSamplers(0, 1, &m_sampleStateClamp);
	device_context->PSSetSamplers(1, 1, &m_sampleStateWrap);

	device_context->DrawIndexed(indexCount, 0, 0);
}