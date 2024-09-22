
#pragma once

#include "IRenderable.h"
#include "ShaderParameterLayout.h"
#include "textureclass.h"

#include <d3d11.h>
#include <wrl/client.h>

#include <memory>
#include <string>
#include <vector>

class ModelClass : public IRenderable {
public:
  ModelClass() = default;

  ModelClass(const ModelClass &) = delete;

  ModelClass &operator=(const ModelClass &) = delete;

  ~ModelClass();

public:
  bool Initialize(const std::string &modelFilename,
                  const std::wstring &textureFilename);

  void Shutdown();

  void
  Render(const IShader &shader,
         const ShaderParameterContainer &parameterContainer) const override;

  int GetIndexCount() const { return index_count_; }

  ID3D11ShaderResourceView *GetTexture() const;

  DirectX::XMMATRIX GetWorldMatrix() const { return world_matrix_; }

  void SetWorldMatrix(const DirectX::XMMATRIX &worldMatrix) {
    world_matrix_ = worldMatrix;
  }

private:
  bool InitializeBuffers();

  void ShutdownBuffers();

  void RenderBuffers() const;

  bool LoadTexture(const std::wstring &filename);

  void ReleaseTexture();

  bool LoadModel(const std::string &filename);

  void ReleaseModel();

private:
  struct Vertex {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT2 texture;
    DirectX::XMFLOAT3 normal;
  };

  struct ModelType {
    float x, y, z;
    float tu, tv;
    float nx, ny, nz;
  };

  Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer_;
  Microsoft::WRL::ComPtr<ID3D11Buffer> index_buffer_;

  int vertex_count_ = 0;
  int index_count_ = 0;

  std::unique_ptr<TextureClass> texture_;

  std::vector<ModelType> model_;

  DirectX::XMMATRIX world_matrix_ = DirectX::XMMatrixIdentity();
};