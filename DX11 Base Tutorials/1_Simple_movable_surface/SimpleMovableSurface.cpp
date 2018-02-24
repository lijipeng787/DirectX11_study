#include <DirectXMath.h>
#include "../CommonFramework/DirectX11Device.h"
#include "SimpleMovableSurface.h"
#include "SimpleTexture.h"

using namespace DirectX;

struct VertexType {
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT2 texture;
};

void SimpleMoveableSurface::MoveTowardTop2D(int units) {
}

void SimpleMoveableSurface::MoveTowardBottom2D(int units) {
}

void SimpleMoveableSurface::SetPosition(float, float, float) {
}

void SimpleMoveableSurface::SetRotation(float, float, float) {
}

void SimpleMoveableSurface::SetPosition2D(float x, float y) {
	position_x_ = x;
	position_y_ = y;
}

void SimpleMoveableSurface::SetRotation2D(float x, float y) {
}

void SimpleMoveableSurface::InitializeWithSurfaceSize(int bitmapWidth, int bitmapHeight) {

	screen_width_ = DirectX11Device::GetD3d11DeviceInstance()->GetScreenWidth();
	screen_height_ = ::DirectX11Device::GetD3d11DeviceInstance()->GetScreenHeight();

	bitmap_width_ = bitmapWidth;
	bitmap_height_ = bitmapHeight;

	previous_posititon_x_ = -1;
	previous_posititon_y_ = -1;
}

void SimpleMoveableSurface::Release() {
	
	ReleaseTexture();

	ReleaseBuffers();
}

bool SimpleMoveableSurface::Render() {

	auto result = UpdateBuffers();
	if (!result) {
		return false;
	}

	SubmitBuffers();

	return true;
}

int SimpleMoveableSurface::GetIndexCount() {
	return index_count_;
}

ID3D11ShaderResourceView* SimpleMoveableSurface::GetTexture() {
	return texture_->GetTexture();
}

bool SimpleMoveableSurface::InitializeVertexAndIndexBuffers() {

	index_count_ = vertex_count_ = 6;

	auto vertices = new VertexType[vertex_count_];
	if (nullptr == vertices) {
		return false;
	}

	auto indices = new unsigned long[index_count_];
	if (nullptr == indices) {
		return false;
	}

	memset(vertices, 0, (sizeof(VertexType) * vertex_count_));

	for (int i = 0; i < index_count_; i++) {
		indices[i] = i;
	}

	D3D11_BUFFER_DESC vertex_buffer_desc;
	ZeroMemory(&vertex_buffer_desc, sizeof(D3D11_BUFFER_DESC));

	vertex_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	vertex_buffer_desc.ByteWidth = sizeof(VertexType) * vertex_count_;
	vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertex_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertex_buffer_desc.MiscFlags = 0;
	vertex_buffer_desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertex_data;
	ZeroMemory(&vertex_data, sizeof(D3D11_SUBRESOURCE_DATA));

	vertex_data.pSysMem = vertices;
	vertex_data.SysMemPitch = 0;
	vertex_data.SysMemSlicePitch = 0;

	auto device = DirectX11Device::GetD3d11DeviceInstance()->GetDevice();

	auto result = device->CreateBuffer(&vertex_buffer_desc, &vertex_data, &vertex_buffer_);
	if (FAILED(result)) {
		return false;
	}

	D3D11_BUFFER_DESC index_buffer_desc;
	ZeroMemory(&index_buffer_desc, sizeof(D3D11_BUFFER_DESC));

	index_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	index_buffer_desc.ByteWidth = sizeof(unsigned long) * index_count_;
	index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	index_buffer_desc.CPUAccessFlags = 0;
	index_buffer_desc.MiscFlags = 0;
	index_buffer_desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA index_data;
	ZeroMemory(&index_data, sizeof(D3D11_SUBRESOURCE_DATA));

	index_data.pSysMem = indices;
	index_data.SysMemPitch = 0;
	index_data.SysMemSlicePitch = 0;

	result = device->CreateBuffer(&index_buffer_desc, &index_data, &index_buffer_);
	if (FAILED(result)) {
		return false;
	}

	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;

	return true;
}

void SimpleMoveableSurface::ReleaseBuffers() {

	if (index_buffer_) {
		index_buffer_->Release();
		index_buffer_ = nullptr;
	}

	if (vertex_buffer_) {
		vertex_buffer_->Release();
		vertex_buffer_ = nullptr;
	}
}

bool SimpleMoveableSurface::UpdateBuffers() {

	if ((position_x_ == previous_posititon_x_) && (position_y_ == previous_posititon_y_)) {
		return true;
	}

	// If it has changed then update the position it is being rendered to.
	previous_posititon_x_ = position_x_;
	previous_posititon_y_ = position_y_;

	// for example,the screen resolution is 800*600,
	// the screen coordinate just like as follow:
	//	(screen)			+300
	//      ----------------------------------
	//      |    (bitmap)   |                |
	//      |left------right|(0,0)           |
	//  -400|----|----|-------------------- -|+400
	//      |    |    |     |                |
	//      |    ------     |                |
	//      ----------------------------------
	//						-300
	
	// Calculate the screen coordinates of the left side of the bitmap.
	auto left = (float)((screen_width_ / 2) * -1) + (float)position_x_;

	// Calculate the screen coordinates of the right side of the bitmap.
	auto right = left + (float)bitmap_width_;

	// Calculate the screen coordinates of the top of the bitmap.
	auto top = (float)(screen_height_ / 2) - (float)position_y_;

	// Calculate the screen coordinates of the bottom of the bitmap.
	auto bottom = top - (float)bitmap_height_;

	// Create the vertex array.
	auto vertices = new VertexType[vertex_count_];
	if (!vertices) {
		return false;
	}

	// Load the vertex array with data.
	// First triangle.
	vertices[0].position = XMFLOAT3(left, top, 0.0f);  // Top left.
	vertices[0].texture = XMFLOAT2(0.0f, 0.0f);

	vertices[1].position = XMFLOAT3(right, bottom, 0.0f);  // Bottom right.
	vertices[1].texture = XMFLOAT2(1.0f, 1.0f);

	vertices[2].position = XMFLOAT3(left, bottom, 0.0f);  // Bottom left.
	vertices[2].texture = XMFLOAT2(0.0f, 1.0f);

	// Second triangle.
	vertices[3].position = XMFLOAT3(left, top, 0.0f);  // Top left.
	vertices[3].texture = XMFLOAT2(0.0f, 0.0f);

	vertices[4].position = XMFLOAT3(right, top, 0.0f);  // Top right.
	vertices[4].texture = XMFLOAT2(1.0f, 0.0f);

	vertices[5].position = XMFLOAT3(right, bottom, 0.0f);  // Bottom right.
	vertices[5].texture = XMFLOAT2(1.0f, 1.0f);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	auto device_context = DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();
	
	auto result = device_context->Map(vertex_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) {
		return false;
	}

	auto verticesPtr = (VertexType*)mappedResource.pData;

	memcpy(verticesPtr, (void*)vertices, (sizeof(VertexType) * vertex_count_));

	device_context->Unmap(vertex_buffer_, 0);

	delete[] vertices;
	vertices = 0;

	return true;
}

void SimpleMoveableSurface::SubmitBuffers() {

	UINT stride = sizeof(VertexType);
	UINT offset = 0;

	auto device_context = DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

	device_context->IASetVertexBuffers(0, 1, &vertex_buffer_, &stride, &offset);

	device_context->IASetIndexBuffer(index_buffer_, DXGI_FORMAT_R32_UINT, 0);

	device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

bool SimpleMoveableSurface::LoadTextureFromFile(WCHAR* filename) {

	texture_ = new SimpleTexture();
	if (nullptr == texture_) {
		return false;
	}

	auto result = texture_->LoadDDSTextureFromFile(filename);
	if (!result) {
		return false;
	}

	return true;
}

void SimpleMoveableSurface::ReleaseTexture() {

	if (texture_) {
		texture_->Release();
		delete texture_;
		texture_ = nullptr;
	}
}

void SimpleMoveableSurface::SetMoveStep(int step) {
}

void SimpleMoveableSurface::MoveTowardLeft2D(int units) {
}

void SimpleMoveableSurface::MoveTowardRight2D(int units) {
}