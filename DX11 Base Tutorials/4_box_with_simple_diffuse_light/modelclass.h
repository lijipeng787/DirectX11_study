#pragma once

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>

#include <memory>
#include <string>
#include <vector>

#include "IRenderable.h"
#include "IShader.h"
#include "textureclass.h"

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

  // Implement the IRenderable interface
  void Render(const IShader &shader, const DirectX::XMMATRIX &viewMatrix,
              const DirectX::XMMATRIX &projectionMatrix) const override;

  int GetIndexCount() const;

  ID3D11ShaderResourceView *GetTexture() const;

  DirectX::XMMATRIX GetWorldMatrix() const;

  void SetWorldMatrix(DirectX::XMMATRIX &workdMatrix);

private:
  bool InitializeBuffers();

  void ShutdownBuffers();

  void RenderBuffers() const;

  bool LoadTexture(const std::wstring &filename);

  void ReleaseTexture();

  bool LoadModel(const std::string &filename);

  void ReleaseModel();

private:
  Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer_;
  Microsoft::WRL::ComPtr<ID3D11Buffer> index_buffer_;

  int vertex_count_ = 0;
  int index_count_ = 0;

  std::unique_ptr<TextureClass> texture_;

  struct ModelType {
    float x, y, z;
    float tu, tv;
    float nx, ny, nz;
  };

  std::unique_ptr<ModelType[]> model_;

  DirectX::XMMATRIX world_matrix_ = DirectX::XMMatrixIdentity();

  Microsoft::WRL::ComPtr<ID3D11Buffer> instance_buffer_;
  int instance_count_ = 0;
};