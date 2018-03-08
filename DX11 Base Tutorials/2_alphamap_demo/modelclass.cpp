#include <DirectXMath.h>
#include <fstream>

#include "../CommonFramework/DirectX11Device.h"
#include "modelclass.h"
#include "texturearrayclass.h"

using namespace std;
using namespace DirectX;

struct VertexType {
	XMFLOAT3 position;
	XMFLOAT2 texture;
};

struct ModelType {
	float x, y, z;
	float tu, tv;
	float nx, ny, nz;
};

bool ModelClass::Initialize(char* modelFilename,
							WCHAR* textureFilename1,
							WCHAR* textureFilename2,
							WCHAR* textureFilename3) {

	// Load in the model data,
	auto result = LoadModel(modelFilename);
	if (!result) {
		return false;
	}

	// Initialize the vertex and index buffers.
	result = InitializeBuffers();
	if (!result) {
		return false;
	}

	// Load the textures for this model.
	result = LoadTextures(textureFilename1, textureFilename2, textureFilename3);
	if (!result) {
		return false;
	}

	return true;
}

void ModelClass::Shutdown() {
	// Release the model textures.
	ReleaseTextures();

	// Shutdown the vertex and index buffers.
	ShutdownBuffers();

	// Release the model data.
	ReleaseModel();
}

void ModelClass::Render() {
	// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
	RenderBuffers();
}

int ModelClass::GetIndexCount() {
	return index_count_;
}

ID3D11ShaderResourceView** ModelClass::GetTextureArray() {
	return texture_array_->GetTextureArray();
}

bool ModelClass::InitializeBuffers() {
	// Create the vertex array.
	auto vertices = new VertexType[vertex_count_];
	if (!vertices) {
		return false;
	}

	// Create the index array.
	auto indices = new unsigned long[index_count_];
	if (!indices) {
		return false;
	}

	int i = 0;
	// Load the vertex array and index array with data.
	for (i = 0; i < vertex_count_; i++) {
		vertices[i].position = XMFLOAT3(model_[i].x, model_[i].y, model_[i].z);
		vertices[i].texture = XMFLOAT2(model_[i].tu, model_[i].tv);

		indices[i] = i;
	}

	D3D11_BUFFER_DESC vertexBufferDesc;
	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * vertex_count_;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertexData;
	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;
	
	auto device = DirectX11Device::GetD3d11DeviceInstance()->GetDevice();
	// Now create the vertex buffer.
	auto result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertex_buffer_);
	if (FAILED(result)) {
		return false;
	}

	D3D11_BUFFER_DESC indexBufferDesc;
	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * index_count_;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexData;
	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &index_buffer_);
	if (FAILED(result)) {
		return false;
	}

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;

	return true;
}

void ModelClass::ShutdownBuffers() {
	// Release the index buffer.
	if (index_buffer_) {
		index_buffer_->Release();
		index_buffer_ = 0;
	}

	// Release the vertex buffer.
	if (vertex_buffer_) {
		vertex_buffer_->Release();
		vertex_buffer_ = 0;
	}
}

void ModelClass::RenderBuffers() {
	unsigned int stride = sizeof(VertexType);
	unsigned int offset = 0;

	auto deviceContext = DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();
	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, &vertex_buffer_, &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(index_buffer_, DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

bool ModelClass::LoadTextures(WCHAR* filename1, WCHAR* filename2, WCHAR* filename3) {
	// Create the texture array object.
	texture_array_ = new TextureArrayClass();
	if (!texture_array_) {
		return false;
	}

	// Initialize the texture array object.
	auto result = texture_array_->Initialize(filename1, filename2, filename3);
	if (!result) {
		return false;
	}

	return true;
}

void ModelClass::ReleaseTextures() {
	// Release the texture array object.
	if (texture_array_) {
		texture_array_->Shutdown();
		delete texture_array_;
		texture_array_ = 0;
	}
}

bool ModelClass::LoadModel(char* filename) {
	ifstream fin;

	// Open the model file.  If it could not open the file then exit.
	fin.open(filename);
	if (fin.fail()) {
		return false;
	}

	char input = 0;
	// Read up to the value of vertex count.
	fin.get(input);
	while (input != ':') {
		fin.get(input);
	}

	// Read in the vertex count.
	fin >> vertex_count_;

	// Set the number of indices to be the same as the vertex count.
	index_count_ = vertex_count_;

	// Create the model using the vertex count that was read in.
	model_ = new ModelType[vertex_count_];
	if (!model_) {
		return false;
	}

	// Read up to the beginning of the data.
	fin.get(input);
	while (input != ':') {
		fin.get(input);
	}
	fin.get(input);
	fin.get(input);

	int i = 0;
	// Read in the vertex data.
	for (i = 0; i < vertex_count_; i++) {
		fin >> model_[i].x >> model_[i].y >> model_[i].z;
		fin >> model_[i].tu >> model_[i].tv;
		fin >> model_[i].nx >> model_[i].ny >> model_[i].nz;
	}

	// Close the model file.
	fin.close();

	return true;
}

void ModelClass::ReleaseModel() {
	if (model_) {
		delete[] model_;
		model_ = 0;
	}
}