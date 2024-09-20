#include "bitmapclass.h"

#include "../CommonFramework/DirectX11Device.h"

SimpleMoveableSurface::SimpleMoveableSurface() {
	vertex_buffer_ = nullptr;
	index_buffer_ = nullptr;
	texture_ = nullptr;
	m_GlowMap = 0;
}

SimpleMoveableSurface::SimpleMoveableSurface(const SimpleMoveableSurface& other) {}

SimpleMoveableSurface::~SimpleMoveableSurface() {}

bool SimpleMoveableSurface::Initialize(
	int screenWidth, int screenHeight, 
	WCHAR* textureFilename, WCHAR* glowMapFilename,
	int bitmapWidth, int bitmapHeight) {

	screen_width_ = screenWidth;
	screen_height_ = screenHeight;

	bitmap_width_ = bitmapWidth;
	bitmap_height_ = bitmapHeight;

	previous_pos_x_ = -1;
	previous_pos_y_ = -1;

	auto result = InitializeBuffers();
	if (!result) {
		return false;
	}

	result = LoadTextures(textureFilename, glowMapFilename);
	if (!result) {
		return false;
	}

	return true;
}

void SimpleMoveableSurface::Shutdown() {
	ReleaseTextures();

	ShutdownBuffers();
}

bool SimpleMoveableSurface::Render(int positionX, int positionY) {

	auto result = UpdateBuffers(positionX, positionY);
	if (!result) {
		return false;
	}

	RenderBuffers();

	return true;
}

int SimpleMoveableSurface::GetIndexCount() {
	return index_count_;
}

ID3D11ShaderResourceView* SimpleMoveableSurface::GetTexture() {
	return texture_->GetTexture();
}

ID3D11ShaderResourceView* SimpleMoveableSurface::GetGlowMap() {
	return m_GlowMap->GetTexture();
}

bool SimpleMoveableSurface::InitializeBuffers() {

	D3D11_BUFFER_DESC vertex_buffer_desc, index_buffer_desc;
	D3D11_SUBRESOURCE_DATA vertex_data, indexData;
	HRESULT result;
	int i;

	vertex_count_ = 6;

	index_count_ = vertex_count_;

	auto vertices = new VertexType[vertex_count_];
	if (!vertices) {
		return false;
	}

	auto indices = new unsigned long[index_count_];
	if (!indices) {
		return false;
	}

	memset(vertices, 0, (sizeof(VertexType) * vertex_count_));

	for (i = 0; i < index_count_; i++) {
		indices[i] = i;
	}

	vertex_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	vertex_buffer_desc.ByteWidth = sizeof(VertexType) * vertex_count_;
	vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertex_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertex_buffer_desc.MiscFlags = 0;
	vertex_buffer_desc.StructureByteStride = 0;

	vertex_data.pSysMem = vertices;
	vertex_data.SysMemPitch = 0;
	vertex_data.SysMemSlicePitch = 0;

	auto device = DirectX11Device::GetD3d11DeviceInstance()->GetDevice();

	result = device->CreateBuffer(&vertex_buffer_desc, &vertex_data, &vertex_buffer_);
	if (FAILED(result)) {
		return false;
	}

	index_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	index_buffer_desc.ByteWidth = sizeof(unsigned long) * index_count_;
	index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	index_buffer_desc.CPUAccessFlags = 0;
	index_buffer_desc.MiscFlags = 0;
	index_buffer_desc.StructureByteStride = 0;

	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	result = device->CreateBuffer(&index_buffer_desc, &indexData, &index_buffer_);
	if (FAILED(result)) {
		return false;
	}

	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;

	return true;
}

void SimpleMoveableSurface::ShutdownBuffers() {

	if (index_buffer_) {
		index_buffer_->Release();
		index_buffer_ = nullptr;
	}

	if (vertex_buffer_) {
		vertex_buffer_->Release();
		vertex_buffer_ = nullptr;
	}
}

bool SimpleMoveableSurface::UpdateBuffers(int positionX, int positionY) {
	
	float left, right, top, bottom;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	VertexType* verticesPtr;
	HRESULT result;

	if ((positionX == previous_pos_x_) && (positionY == previous_pos_y_)) {
		return true;
	}

	previous_pos_x_ = positionX;
	previous_pos_y_ = positionY;

	// Calculate the screen coordinates of the left side of the bitmap.
	left = (float)((screen_width_ / 2) * -1) + (float)positionX;

	// Calculate the screen coordinates of the right side of the bitmap.
	right = left + (float)bitmap_width_;

	// Calculate the screen coordinates of the top of the bitmap.
	top = (float)(screen_height_ / 2) - (float)positionY;

	// Calculate the screen coordinates of the bottom of the bitmap.
	bottom = top - (float)bitmap_height_;

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

	auto device_context = DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

	result = device_context->Map(vertex_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) {
		return false;
	}

	verticesPtr = (VertexType*)mappedResource.pData;

	memcpy(verticesPtr, (void*)vertices, (sizeof(VertexType) * vertex_count_));

	device_context->Unmap(vertex_buffer_, 0);

	delete[] vertices;
	vertices = 0;

	return true;
}

void SimpleMoveableSurface::RenderBuffers() {

	unsigned int stride;
	unsigned int offset;

	stride = sizeof(VertexType);
	offset = 0;

	auto device_context = DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

	device_context->IASetVertexBuffers(0, 1, &vertex_buffer_, &stride, &offset);
	device_context->IASetIndexBuffer(index_buffer_, DXGI_FORMAT_R32_UINT, 0);
	device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

bool SimpleMoveableSurface::LoadTextures(WCHAR* filename, WCHAR* glowMapFilename) {
	
	texture_ = new TextureClass();
	if (!texture_) {
		return false;
	}

	auto result = texture_->Initialize(filename);
	if (!result) {
		return false;
	}

	m_GlowMap = new TextureClass();
	if (!m_GlowMap) {
		return false;
	}

	// Initialize the glow map texture object.
	result = m_GlowMap->Initialize(glowMapFilename);
	if (!result) {
		return false;
	}

	return true;
}

void SimpleMoveableSurface::ReleaseTextures() {
	// Release the texture objects.
	if (texture_) {
		texture_->Shutdown();
		delete texture_;
		texture_ = nullptr;
	}

	if (m_GlowMap) {
		m_GlowMap->Shutdown();
		delete m_GlowMap;
		m_GlowMap = 0;
	}
}