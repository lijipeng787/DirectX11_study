#include "ModelClass.h"

#include "../CommonFramework/DirectX11Device.h"

#include <DirectXMath.h>
#include <d3d11.h>

#include <fstream>
#include <stdexcept>
#include <vector>

struct VertexType {
  DirectX::XMFLOAT3 position;
  DirectX::XMFLOAT2 texture;
  DirectX::XMFLOAT3 normal;
};

ModelClass::~ModelClass() { Shutdown(); }

bool ModelClass::Initialize(const std::string &modelFilename,
                            const std::wstring &textureFilename) {
  if (!LoadModel(modelFilename) || !InitializeBuffers() ||
      !LoadTexture(textureFilename)) {
    return false;
  }
  return true;
}

void ModelClass::Shutdown() {
  ReleaseTexture();
  ShutdownBuffers();
  ReleaseModel();
}

void ModelClass::Render(const IShader &shader,
                        const DirectX::XMMATRIX &viewMatrix,
                        const DirectX::XMMATRIX &projectionMatrix) const {
  RenderBuffers();

  shader.Render(GetIndexCount(), GetWorldMatrix(), viewMatrix, projectionMatrix,
                GetTexture(), DirectX::XMFLOAT3(0, 0, 1),
                DirectX::XMFLOAT4(1, 1, 1, 1));
}

DirectX::XMMATRIX ModelClass::GetWorldMatrix() const { return world_matrix_; }

void ModelClass::SetWorldMatrix(DirectX::XMMATRIX &workdMatrix) {
  world_matrix_ = workdMatrix;
}

int ModelClass::GetIndexCount() const { return index_count_; }

ID3D11ShaderResourceView *ModelClass::GetTexture() const {
  return texture_ ? texture_->GetTexture() : nullptr;
}

bool ModelClass::InitializeBuffers() {
  std::vector<VertexType> vertices(vertex_count_);
  std::vector<unsigned long> indices(index_count_);

  for (int i = 0; i < vertex_count_; i++) {
    vertices[i].position =
        DirectX::XMFLOAT3(model_[i].x, model_[i].y, model_[i].z);
    vertices[i].texture = DirectX::XMFLOAT2(model_[i].tu, model_[i].tv);
    vertices[i].normal =
        DirectX::XMFLOAT3(model_[i].nx, model_[i].ny, model_[i].nz);
    indices[i] = i;
  }

  D3D11_BUFFER_DESC vertexBufferDesc = {};
  vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
  vertexBufferDesc.ByteWidth = sizeof(VertexType) * vertex_count_;
  vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

  D3D11_SUBRESOURCE_DATA vertexData = {};
  vertexData.pSysMem = vertices.data();

  auto device = DirectX11Device::GetD3d11DeviceInstance()->GetDevice();
  if (FAILED(device->CreateBuffer(&vertexBufferDesc, &vertexData,
                                  &vertex_buffer_))) {
    return false;
  }

  D3D11_BUFFER_DESC indexBufferDesc = {};
  indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
  indexBufferDesc.ByteWidth = sizeof(unsigned long) * index_count_;
  indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

  D3D11_SUBRESOURCE_DATA indexData = {};
  indexData.pSysMem = indices.data();

  if (FAILED(
          device->CreateBuffer(&indexBufferDesc, &indexData, &index_buffer_))) {
    return false;
  }

  return true;
}

void ModelClass::ShutdownBuffers() {
  index_buffer_.Reset();
  vertex_buffer_.Reset();
}

void ModelClass::RenderBuffers() const {
  UINT stride = sizeof(VertexType);
  UINT offset = 0;

  auto deviceContext =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();
  deviceContext->IASetVertexBuffers(0, 1, vertex_buffer_.GetAddressOf(),
                                    &stride, &offset);
  deviceContext->IASetIndexBuffer(index_buffer_.Get(), DXGI_FORMAT_R32_UINT, 0);
  deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

bool ModelClass::LoadTexture(const std::wstring &filename) {
  texture_ = std::make_unique<TextureClass>();
  return texture_->Initialize(filename.c_str());
}

void ModelClass::ReleaseTexture() {
  if (texture_) {
    texture_->Shutdown();
    texture_.reset();
  }
}

bool ModelClass::LoadModel(const std::string &filename) {
  std::ifstream fin(filename);
  if (!fin) {
    return false;
  }

  char input = 0;
  fin.get(input);
  while (input != ':') {
    fin.get(input);
  }

  fin >> vertex_count_;
  if (fin.fail()) {
    return false;
  }

  index_count_ = vertex_count_;
  model_ = std::make_unique<ModelType[]>(vertex_count_);

  fin.get(input);
  while (input != ':') {
    fin.get(input);
  }
  fin.get(input);
  fin.get(input);

  for (int i = 0; i < vertex_count_; i++) {
    fin >> model_[i].x >> model_[i].y >> model_[i].z;
    fin >> model_[i].tu >> model_[i].tv;
    fin >> model_[i].nx >> model_[i].ny >> model_[i].nz;

    if (fin.fail()) {
      return false;
    }
  }

  return true;
}

void ModelClass::ReleaseModel() { model_.reset(); }