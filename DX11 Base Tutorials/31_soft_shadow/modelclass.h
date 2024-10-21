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

  void SetParameterCallback(ShaderParameterCallback callback) override;

  ShaderParameterCallback GetParameterCallback() const override;

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

class PBRModelClass : IRenderable {
private:
  struct VertexType {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT2 texture;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT3 tangent;
    DirectX::XMFLOAT3 binormal;
  };

  struct ModelType {
    float x, y, z;
    float tu, tv;
    float nx, ny, nz;
    float tx, ty, tz;
    float bx, by, bz;
  };

  struct TempVertexType {
    float x, y, z;
    float tu, tv;
    float nx, ny, nz;
  };

  struct VectorType {
    float x, y, z;
  };

public:
  PBRModelClass() = default;

  PBRModelClass(const ModelClass &) = delete;

  ~PBRModelClass() = default;

public:
  bool Initialize(const char *, const std::string &, const std::string &,
                  const std::string &);

  void Shutdown();

  void
  Render(const IShader &shader,
         const ShaderParameterContainer &parameterContainer) const override;

  void SetParameterCallback(ShaderParameterCallback callback) override;

  ShaderParameterCallback GetParameterCallback() const override;

  int GetIndexCount() const { return m_indexCount; }

  ID3D11ShaderResourceView *GetTexture(int);

  void SetWorldMatrix(const DirectX::XMMATRIX &worldMatrix) {
    world_matrix_ = worldMatrix;
  }

  DirectX::XMMATRIX GetWorldMatrix() const { return world_matrix_; }

private:
  bool InitializeBuffers();

  void RenderBuffers() const;

  bool LoadTextures(const std::string &, const std::string &,
                    const std::string &);

  void ReleaseTextures();

  bool LoadModel(const char *);

  void CalculateModelVectors();

  void CalculateTangentBinormal(TempVertexType, TempVertexType, TempVertexType,
                                VectorType &, VectorType &);

private:
  Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer, m_indexBuffer;

  int m_vertexCount, m_indexCount;

  TGATextureClass *m_Textures;

  std::vector<ModelType> m_model;

  DirectX::XMMATRIX world_matrix_ = DirectX::XMMatrixIdentity();
};