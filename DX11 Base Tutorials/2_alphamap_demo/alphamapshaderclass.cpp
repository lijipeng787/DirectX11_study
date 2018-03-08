#include <fstream>
#include <d3dcompiler.h>

#include "../CommonFramework/DirectX11Device.h"
#include "alphamapshaderclass.h"

using namespace std;
using namespace DirectX;

struct MatrixBufferType {
	XMMATRIX world;
	XMMATRIX view;
	XMMATRIX projection;
};

bool AlphaMapShaderClass::Initialize(HWND hwnd) {

	auto result = InitializeShader(hwnd, L"../../2_alphamap_demo/alphamap_shader.hlsl", L"../../2_alphamap_demo/alphamap_shader.hlsl");
	if (!result) {
		return false;
	}

	return true;
}

void AlphaMapShaderClass::Shutdown() {
	ShutdownShader();
}

bool AlphaMapShaderClass::Render(int indexCount,
								 const XMMATRIX& worldMatrix,
								 const XMMATRIX& viewMatrix,
								 const XMMATRIX& projectionMatrix,
								 ID3D11ShaderResourceView** textureArray) {

	auto result = SetShaderParameters(worldMatrix, viewMatrix, projectionMatrix, textureArray);
	if (!result) {
		return false;
	}

	RenderShader(indexCount);

	return true;
}

bool AlphaMapShaderClass::InitializeShader(HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename) {

	// Initialize the pointers this function will use to null.
	ID3D10Blob* errorMessage=nullptr;
	ID3D10Blob* vertexShaderBuffer = nullptr;

	// Compile the vertex shader code.
	auto result = D3DCompileFromFile(vsFilename, NULL, NULL, "AlphaMapVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS,
								0, &vertexShaderBuffer, &errorMessage);
	if (FAILED(result)) {
		// If the shader failed to compile it should have writen something to the error message.
		if (errorMessage) {
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
		}
		// If there was  nothing in the error message then it simply could not find the shader file itself.
		else {
			MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	ID3D10Blob* pixelShaderBuffer = nullptr;
	// Compile the pixel shader code.
	result = D3DCompileFromFile(psFilename, NULL, NULL, "AlphaMapPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS,
								0, &pixelShaderBuffer, &errorMessage);
	if (FAILED(result)) {
		// If the shader failed to compile it should have writen something to the error message.
		if (errorMessage) {
			OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
		}
		// If there was  nothing in the error message then it simply could not find the file itself.
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

	// Create the vertex shader from the buffer.
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL,
									   &pixel_shader_);
	if (FAILED(result)) {
		return false;
	}

	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	unsigned int numElements;

	// Create the vertex input layout description.
	// This setup needs to match the VertexType stucture in the ModelClass and in the shader.
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

	// Get a count of the elements in the layout.
	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
									   vertexShaderBuffer->GetBufferSize(), &layout_);
	if (FAILED(result)) {
		return false;
	}

	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	D3D11_BUFFER_DESC matrixBufferDesc;

	// Setup the description of the matrix dynamic constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the matrix constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&matrixBufferDesc, NULL, &matrix_buffer_);
	if (FAILED(result)) {
		return false;
	}

	D3D11_SAMPLER_DESC samplerDesc;
	// Create a texture sampler state description.
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

	// Create the texture sampler state.
	result = device->CreateSamplerState(&samplerDesc, &sample_state_);
	if (FAILED(result)) {
		return false;
	}

	return true;
}

void AlphaMapShaderClass::ShutdownShader() {
	// Release the sampler state.
	if (sample_state_) {
		sample_state_->Release();
		sample_state_ = 0;
	}

	// Release the matrix constant buffer.
	if (matrix_buffer_) {
		matrix_buffer_->Release();
		matrix_buffer_ = 0;
	}

	// Release the layout.
	if (layout_) {
		layout_->Release();
		layout_ = 0;
	}

	// Release the pixel shader.
	if (pixel_shader_) {
		pixel_shader_->Release();
		pixel_shader_ = 0;
	}

	// Release the vertex shader.
	if (vertex_shader_) {
		vertex_shader_->Release();
		vertex_shader_ = 0;
	}
}

void AlphaMapShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename) {
	char* compileErrors;
	SIZE_T bufferSize, i;
	ofstream fout;


	// Get a pointer to the error message text buffer.
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	// Get the length of the message.
	bufferSize = errorMessage->GetBufferSize();

	// Open a file to write the error message to.
	fout.open("shader-error.txt");

	// Write out the error message.
	for (i = 0; i < bufferSize; i++) {
		fout << compileErrors[i];
	}

	// Close the file.
	fout.close();

	// Release the error message.
	errorMessage->Release();
	errorMessage = 0;

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);
}

bool AlphaMapShaderClass::SetShaderParameters(const XMMATRIX& worldMatrix,
											  const XMMATRIX& viewMatrix,
											  const XMMATRIX& projectionMatrix,
											  ID3D11ShaderResourceView** textureArray) {
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	unsigned int bufferNumber;

	XMMATRIX worldMatrixCopy = worldMatrix;
	XMMATRIX viewMatrixCopy = viewMatrix;
	XMMATRIX projectionMatrixCopy = projectionMatrix;

	// Transpose the matrices to prepare them for the shader.
	worldMatrixCopy = XMMatrixTranspose(worldMatrix);
	viewMatrixCopy = XMMatrixTranspose(viewMatrix);
	projectionMatrixCopy = XMMatrixTranspose(projectionMatrix);

	auto deviceContext = DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

	// Lock the matrix constant buffer so it can be written to.
	result = deviceContext->Map(matrix_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) {
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	dataPtr->world = worldMatrixCopy;
	dataPtr->view = viewMatrixCopy;
	dataPtr->projection = projectionMatrixCopy;

	// Unlock the matrix constant buffer.
	deviceContext->Unmap(matrix_buffer_, 0);

	// Set the position of the matrix constant buffer in the vertex shader.
	bufferNumber = 0;

	// Now set the matrix constant buffer in the vertex shader with the updated values.
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &matrix_buffer_);

	// Set shader texture array resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 3, textureArray);

	return true;
}

void AlphaMapShaderClass::RenderShader(int indexCount) {
	auto deviceContext = DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(layout_);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	deviceContext->VSSetShader(vertex_shader_, NULL, 0);
	deviceContext->PSSetShader(pixel_shader_, NULL, 0);

	// Set the sampler state in the pixel shader.
	deviceContext->PSSetSamplers(0, 1, &sample_state_);

	// Render the triangles.
	deviceContext->DrawIndexed(indexCount, 0, 0);
}