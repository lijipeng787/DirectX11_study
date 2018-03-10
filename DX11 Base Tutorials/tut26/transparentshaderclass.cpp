#include <d3dcompiler.h>
#include <fstream>

#include "../CommonFramework/DirectX11Device.h"
#include "transparentshaderclass.h"

using namespace std;
using namespace DirectX;

struct MatrixBufferType {
	XMMATRIX world;
	XMMATRIX view;
	XMMATRIX projection;
};

struct TransparentBufferType {
	float blendAmount;
	XMFLOAT3 padding;
};

bool TransparentShaderClass::Initialize(HWND hwnd) {

	auto result = InitializeShader(hwnd, L"../../tut26/transparent.hlsl", L"../../tut26/transparent.hlsl");
	if (!result) {
		return false;
	}

	return true;
}

void TransparentShaderClass::Shutdown() {
	ShutdownShader();
}

bool TransparentShaderClass::Render(int indexCount, const XMMATRIX& worldMatrix,
									const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* texture,
									float blend) {

	auto result = SetShaderParameters(worldMatrix, viewMatrix, projectionMatrix, texture, blend);
	if (!result) {
		return false;
	}

	RenderShader(indexCount);

	return true;
}

bool TransparentShaderClass::InitializeShader(HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename) {

	ID3D10Blob* errorMessage = nullptr;
	ID3D10Blob* vertexShaderBuffer = nullptr;

	auto result = D3DCompileFromFile(vsFilename, NULL, NULL, "TransparentVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS,
									 0, &vertexShaderBuffer, &errorMessage);
	if (FAILED(result)) {
		if (errorMessage) {
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
		}
		else {
			MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	ID3D10Blob* pixelShaderBuffer = nullptr;

	result = D3DCompileFromFile(psFilename, NULL, NULL, "TransparentPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS,
								0, &pixelShaderBuffer, &errorMessage);
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

	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL,
										&vertex_shader_);
	if (FAILED(result)) {
		return false;
	}

	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL,
									   &pixel_shader_);
	if (FAILED(result)) {
		return false;
	}

	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];

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

	unsigned int numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
									   vertexShaderBuffer->GetBufferSize(), &layout_);
	if (FAILED(result)) {
		return false;
	}

	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

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

	D3D11_SAMPLER_DESC samplerDesc;

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
	if (FAILED(result)) {
		return false;
	}

	D3D11_BUFFER_DESC transparentBufferDesc;

	transparentBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	transparentBufferDesc.ByteWidth = sizeof(TransparentBufferType);
	transparentBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	transparentBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	transparentBufferDesc.MiscFlags = 0;
	transparentBufferDesc.StructureByteStride = 0;

	result = device->CreateBuffer(&transparentBufferDesc, NULL, &transparent_buffer_);
	if (FAILED(result)) {
		return false;
	}

	return true;
}

void TransparentShaderClass::ShutdownShader() {

	if (transparent_buffer_) {
		transparent_buffer_->Release();
		transparent_buffer_ = nullptr;
	}

	if (sample_state_) {
		sample_state_->Release();
		sample_state_ = nullptr;
	}

	if (matrix_buffer_) {
		matrix_buffer_->Release();
		matrix_buffer_ = nullptr;
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

void TransparentShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename) {

	auto compileErrors = (char*)(errorMessage->GetBufferPointer());

	auto bufferSize = errorMessage->GetBufferSize();

	ofstream fout;
	fout.open("shader-error.txt");

	int i = 0;
	for (i = 0; i < bufferSize; i++) {
		fout << compileErrors[i];
	}

	fout.close();

	errorMessage->Release();
	errorMessage = 0;

	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);
}

bool TransparentShaderClass::SetShaderParameters(const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix,
												 const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* texture, float blend) {

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	auto device_context = DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

	auto result = device_context->Map(matrix_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) {
		return false;
	}

	MatrixBufferType* dataPtr;
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	XMMATRIX worldMatrixCopy = XMMatrixTranspose(worldMatrix);
	XMMATRIX viewMatrixCopy = XMMatrixTranspose(viewMatrix);
	XMMATRIX projectionMatrixCopy = XMMatrixTranspose(projectionMatrix);

	dataPtr->world = worldMatrixCopy;
	dataPtr->view = viewMatrixCopy;
	dataPtr->projection = projectionMatrixCopy;

	device_context->Unmap(matrix_buffer_, 0);

	unsigned int buffer_number = 0;

	device_context->VSSetConstantBuffers(buffer_number, 1, &matrix_buffer_);

	device_context->PSSetShaderResources(0, 1, &texture);

	result = device_context->Map(transparent_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) {
		return false;
	}

	TransparentBufferType* dataPtr2;

	dataPtr2 = (TransparentBufferType*)mappedResource.pData;

	dataPtr2->blendAmount = blend;

	device_context->Unmap(transparent_buffer_, 0);

	buffer_number = 0;

	device_context->PSSetConstantBuffers(buffer_number, 1, &transparent_buffer_);

	return true;
}

void TransparentShaderClass::RenderShader(int indexCount) {

	auto device_context = DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

	device_context->IASetInputLayout(layout_);

	device_context->VSSetShader(vertex_shader_, NULL, 0);

	device_context->PSSetShader(pixel_shader_, NULL, 0);

	device_context->PSSetSamplers(0, 1, &sample_state_);

	device_context->DrawIndexed(indexCount, 0, 0);
}