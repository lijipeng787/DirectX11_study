#include "modelclass.h"
#include "textureclass.h"
#include "../CommonFramework/DirectX11Device.h"

#include <DirectXMath.h>

using namespace DirectX;

struct VertexType {
	XMFLOAT3 position;
	XMFLOAT2 texture;
};

struct InstanceType {
	XMFLOAT3 position;
};

bool ModelClass::Initialize(WCHAR* textureFilename) {

	auto result = InitializeBuffers();
	if (!result) {
		return false;
	}

	result = LoadTexture(textureFilename);
	if (!result) {
		return false;
	}

	return true;
}

void ModelClass::Shutdown() {

	ReleaseTexture();

	ShutdownBuffers();
}

void ModelClass::Render() {
	RenderBuffers();
}

int ModelClass::GetVertexCount() {
	return vertex_count_;
}

int ModelClass::GetInstanceCount() {
	return instance_count_;
}

ID3D11ShaderResourceView* ModelClass::GetTexture() {
	return texture_->GetTexture();
}

bool ModelClass::InitializeBuffers() {

	vertex_count_ = 3;

	auto vertices = new VertexType[vertex_count_];
	if (!vertices) {
		return false;
	}

	// Load the vertex array with data.
	vertices[0].position = XMFLOAT3(-1.0f, -1.0f, 0.0f);  // Bottom left.
	vertices[0].texture = XMFLOAT2(0.0f, 1.0f);

	vertices[1].position = XMFLOAT3(0.0f, 1.0f, 0.0f);  // Top middle.
	vertices[1].texture = XMFLOAT2(0.5f, 0.0f);

	vertices[2].position = XMFLOAT3(1.0f, -1.0f, 0.0f);  // Bottom right.
	vertices[2].texture = XMFLOAT2(1.0f, 1.0f);

	D3D11_BUFFER_DESC vertex_buffer_desc;

	vertex_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	vertex_buffer_desc.ByteWidth = sizeof(VertexType) * vertex_count_;
	vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertex_buffer_desc.CPUAccessFlags = 0;
	vertex_buffer_desc.MiscFlags = 0;
	vertex_buffer_desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertex_data;

	vertex_data.pSysMem = vertices;
	vertex_data.SysMemPitch = 0;
	vertex_data.SysMemSlicePitch = 0;

	auto device = DirectX11Device::GetD3d11DeviceInstance()->GetDevice();

	auto result = device->CreateBuffer(&vertex_buffer_desc, &vertex_data, &vertex_buffer_);
	if (FAILED(result)) {
		return false;
	}

	delete[] vertices;
	vertices = 0;

	instance_count_ = 4;

	auto instances = new InstanceType[instance_count_];
	if (!instances) {
		return false;
	}

	instances[0].position = XMFLOAT3(-1.5f, -1.5f, 5.0f);
	instances[1].position = XMFLOAT3(-1.5f, 1.5f, 5.0f);
	instances[2].position = XMFLOAT3(1.5f, -1.5f, 5.0f);
	instances[3].position = XMFLOAT3(1.5f, 1.5f, 5.0f);

	D3D11_BUFFER_DESC instanceBufferDesc;

	instanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	instanceBufferDesc.ByteWidth = sizeof(InstanceType) * instance_count_;
	instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	instanceBufferDesc.CPUAccessFlags = 0;
	instanceBufferDesc.MiscFlags = 0;
	instanceBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA instanceData;

	instanceData.pSysMem = instances;
	instanceData.SysMemPitch = 0;
	instanceData.SysMemSlicePitch = 0;

	result = device->CreateBuffer(&instanceBufferDesc, &instanceData, &instance_buffer_);
	if (FAILED(result)) {
		return false;
	}

	delete[] instances;
	instances = 0;

	return true;
}

void ModelClass::ShutdownBuffers() {

	if (instance_buffer_) {
		instance_buffer_->Release();
		instance_buffer_ = 0;
	}

	if (vertex_buffer_) {
		vertex_buffer_->Release();
		vertex_buffer_ = nullptr;
	}
}

void ModelClass::RenderBuffers() {

	unsigned int strides[2];
	strides[0] = sizeof(VertexType);
	strides[1] = sizeof(InstanceType);

	unsigned int offsets[2];
	offsets[0] = 0;
	offsets[1] = 0;

	ID3D11Buffer* bufferPointers[2];
	bufferPointers[0] = vertex_buffer_;
	bufferPointers[1] = instance_buffer_;

	auto device_context = DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

	device_context->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);

	device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

bool ModelClass::LoadTexture(WCHAR* filename) {

	texture_ = new TextureClass();
	if (!texture_) {
		return false;
	}

	auto result = texture_->Initialize(filename);
	if (!result) {
		return false;
	}

	return true;
}

void ModelClass::ReleaseTexture() {

	if (texture_) {
		texture_->Shutdown();
		delete texture_;
		texture_ = nullptr;
	}
}