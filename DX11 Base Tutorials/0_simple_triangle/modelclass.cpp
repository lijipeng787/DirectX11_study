#include <DirectXMath.h>

#include "../CommonFramework/DirectX11Device.h"
#include "modelclass.h"

using namespace DirectX;

struct VertexType {
  XMFLOAT3 position;
  XMFLOAT4 color;
};

bool ModelClass::Initialize() {

  auto result = InitializeBuffers();
  if (!result) {
    return false;
  }

  return true;
}

void ModelClass::Shutdown() { ShutdownBuffers(); }

void ModelClass::Render() { RenderBuffers(); }

bool ModelClass::InitializeBuffers() {

  vertex_count_ = 3;
  index_count_ = 3;

  VertexType vertices[3];
  unsigned long indices[3];

  // Load the vertex array with data.
  vertices[0].position = XMFLOAT3(-1.0f, -1.0f, 0.0f); // Bottom left.
  vertices[0].color = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

  vertices[1].position = XMFLOAT3(0.0f, 1.0f, 0.0f); // Top middle.
  vertices[1].color = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

  vertices[2].position = XMFLOAT3(1.0f, -1.0f, 0.0f); // Bottom right.
  vertices[2].color = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

  indices[0] = 0; // Bottom left.
  indices[1] = 1; // Top middle.
  indices[2] = 2; // Bottom right.

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

  auto result =
      device->CreateBuffer(&vertex_buffer_desc, &vertex_data, &vertex_buffer_);
  if (FAILED(result)) {
    return false;
  }

  D3D11_BUFFER_DESC index_buffer_desc;

  index_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
  index_buffer_desc.ByteWidth = sizeof(unsigned long) * index_count_;
  index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
  index_buffer_desc.CPUAccessFlags = 0;
  index_buffer_desc.MiscFlags = 0;
  index_buffer_desc.StructureByteStride = 0;

  D3D11_SUBRESOURCE_DATA indexData;

  indexData.pSysMem = indices;
  indexData.SysMemPitch = 0;
  indexData.SysMemSlicePitch = 0;

  result = device->CreateBuffer(&index_buffer_desc, &indexData, &index_buffer_);
  if (FAILED(result)) {
    return false;
  }

  return true;
}

void ModelClass::ShutdownBuffers() {

  if (index_buffer_) {
    index_buffer_->Release();
    index_buffer_ = nullptr;
  }

  if (vertex_buffer_) {
    vertex_buffer_->Release();
    vertex_buffer_ = nullptr;
  }
}

void ModelClass::RenderBuffers() {

  unsigned int stride = sizeof(VertexType);
  unsigned int offset = 0;

  auto device_context =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  device_context->IASetVertexBuffers(0, 1, &vertex_buffer_, &stride, &offset);

  device_context->IASetIndexBuffer(index_buffer_, DXGI_FORMAT_R32_UINT, 0);

  device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}