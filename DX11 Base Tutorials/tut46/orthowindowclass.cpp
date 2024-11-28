#include "orthowindowclass.h"

#include "../CommonFramework/DirectX11Device.h"

OrthoWindowClass::OrthoWindowClass() {
  vertex_buffer_ = nullptr;
  index_buffer_ = nullptr;
}

OrthoWindowClass::OrthoWindowClass(const OrthoWindowClass &other) {}

OrthoWindowClass::~OrthoWindowClass() {}

bool OrthoWindowClass::Initialize(int windowWidth, int windowHeight) {

  auto result = InitializeBuffers(windowWidth, windowHeight);
  if (!result) {
    return false;
  }

  return true;
}

void OrthoWindowClass::Shutdown() { ShutdownBuffers(); }

void OrthoWindowClass::Render() { RenderBuffers(); }

int OrthoWindowClass::GetIndexCount() { return index_count_; }

bool OrthoWindowClass::InitializeBuffers(int windowWidth, int windowHeight) {

  float left, right, top, bottom;
  D3D11_BUFFER_DESC vertex_buffer_desc, index_buffer_desc;
  D3D11_SUBRESOURCE_DATA vertex_data, indexData;
  int i;

  // Calculate the screen coordinates of the left side of the window.
  left = (float)((windowWidth / 2) * -1);

  // Calculate the screen coordinates of the right side of the window.
  right = left + (float)windowWidth;

  // Calculate the screen coordinates of the top of the window.
  top = (float)(windowHeight / 2);

  // Calculate the screen coordinates of the bottom of the window.
  bottom = top - (float)windowHeight;

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

  // Load the vertex array with data.
  // First triangle.
  vertices[0].position = XMFLOAT3(left, top, 0.0f); // Top left.
  vertices[0].texture = XMFLOAT2(0.0f, 0.0f);

  vertices[1].position = XMFLOAT3(right, bottom, 0.0f); // Bottom right.
  vertices[1].texture = XMFLOAT2(1.0f, 1.0f);

  vertices[2].position = XMFLOAT3(left, bottom, 0.0f); // Bottom left.
  vertices[2].texture = XMFLOAT2(0.0f, 1.0f);

  // Second triangle.
  vertices[3].position = XMFLOAT3(left, top, 0.0f); // Top left.
  vertices[3].texture = XMFLOAT2(0.0f, 0.0f);

  vertices[4].position = XMFLOAT3(right, top, 0.0f); // Top right.
  vertices[4].texture = XMFLOAT2(1.0f, 0.0f);

  vertices[5].position = XMFLOAT3(right, bottom, 0.0f); // Bottom right.
  vertices[5].texture = XMFLOAT2(1.0f, 1.0f);

  for (i = 0; i < index_count_; i++) {
    indices[i] = i;
  }

  vertex_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
  vertex_buffer_desc.ByteWidth = sizeof(VertexType) * vertex_count_;
  vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  vertex_buffer_desc.CPUAccessFlags = 0;
  vertex_buffer_desc.MiscFlags = 0;
  vertex_buffer_desc.StructureByteStride = 0;

  vertex_data.pSysMem = vertices;
  vertex_data.SysMemPitch = 0;
  vertex_data.SysMemSlicePitch = 0;

  auto device = DirectX11Device::GetD3d11DeviceInstance()->GetDevice();

  auto result =
      device->CreateBuffer(&vertex_buffer_desc, &vertex_data, &vertex_buffer_);
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

void OrthoWindowClass::ShutdownBuffers() {

  if (index_buffer_) {
    index_buffer_->Release();
    index_buffer_ = nullptr;
  }

  if (vertex_buffer_) {
    vertex_buffer_->Release();
    vertex_buffer_ = nullptr;
  }
}

void OrthoWindowClass::RenderBuffers() {

  unsigned int stride;
  unsigned int offset;

  stride = sizeof(VertexType);
  offset = 0;

  auto device_context =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  device_context->IASetVertexBuffers(0, 1, &vertex_buffer_, &stride, &offset);
  device_context->IASetIndexBuffer(index_buffer_, DXGI_FORMAT_R32_UINT, 0);
  device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}