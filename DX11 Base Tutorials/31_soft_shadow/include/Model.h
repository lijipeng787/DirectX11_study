#pragma once

#include "BoundingVolume.h"
#include "Interfaces.h"
#include "ShaderParameter.h"
#include "Texture.h"

#include <d3d11.h>
#include <wrl/client.h>

#include <memory>
#include <string>
#include <vector>

class Model : public IRenderable {
public:
  Model() = default;

  // Delete copy operations (models should not be copied)
  Model(const Model &) = delete;
  Model &operator=(const Model &) = delete;

  // Enable move operations for efficient transfer of ownership
  Model(Model &&) noexcept = default;
  Model &operator=(Model &&) noexcept = default;

  ~Model();

public:
  [[nodiscard]] bool Initialize(const std::string &modelFilename,
                                const std::wstring &textureFilename, ID3D11Device *device);

  void Shutdown();

  void Render(const IShader &shader,
              const ShaderParameterContainer &parameterContainer,
              ID3D11DeviceContext *deviceContext) const override;

  void SetParameterCallback(ShaderParameterCallback callback) override;

  ShaderParameterCallback GetParameterCallback() const override;

  int GetIndexCount() const { return index_count_; }

  ID3D11ShaderResourceView *GetTexture() const;

  DirectX::XMMATRIX GetWorldMatrix() const noexcept { return world_matrix_; }

  void SetWorldMatrix(const DirectX::XMMATRIX &worldMatrix) override {
    world_matrix_ = worldMatrix;
  }

  const BoundingVolume &GetLocalBoundingVolume() const;

  BoundingVolume GetWorldBoundingVolume() const;

private:
  bool InitializeBuffers(ID3D11Device *device);

  void ShutdownBuffers();

  void RenderBuffers(ID3D11DeviceContext *deviceContext) const;

  bool LoadTexture(const std::wstring &filename, ID3D11Device *device);

  bool LoadModel(const std::string &filename);

  void ReleaseModel();

  void CalculateBoundingVolume(); // Calculate bounding volume

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

  std::unique_ptr<DDSTexture> texture_;

  std::vector<ModelType> model_;

  DirectX::XMMATRIX world_matrix_ = DirectX::XMMatrixIdentity();

  BoundingVolume bounding_volume_; // Bounding volume in local space
};

class PBRModel : IRenderable {
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
  PBRModel() = default;

  // Delete copy operations (PBR models should not be copied)
  PBRModel(const PBRModel &) = delete;
  PBRModel &operator=(const PBRModel &) = delete;

  // Enable move operations for efficient transfer of ownership
  PBRModel(PBRModel &&) noexcept = default;
  PBRModel &operator=(PBRModel &&) noexcept = default;

  ~PBRModel() = default;

public:
  [[nodiscard]] bool Initialize(const char *, const std::string &, const std::string &,
                                const std::string &, ID3D11Device *device);

  void Shutdown();

  void Render(const IShader &shader,
              const ShaderParameterContainer &parameterContainer,
              ID3D11DeviceContext *deviceContext) const override;

  void SetParameterCallback(ShaderParameterCallback callback) override;

  ShaderParameterCallback GetParameterCallback() const override;

  int GetIndexCount() const { return index_count_; }

  ID3D11ShaderResourceView *GetTexture(int index);

  void SetWorldMatrix(const DirectX::XMMATRIX &worldMatrix) {
    world_matrix_ = worldMatrix;
  }

  DirectX::XMMATRIX GetWorldMatrix() const noexcept { return world_matrix_; }

private:
  bool InitializeBuffers(ID3D11Device *device);

  void RenderBuffers(ID3D11DeviceContext *deviceContext) const;

  bool LoadTextures(const std::string &, const std::string &,
                    const std::string &, ID3D11Device *device);

  void ReleaseTextures();

  bool LoadModel(const char *);

  void CalculateModelVectors();

  void CalculateTangentBinormal(TempVertexType, TempVertexType, TempVertexType,
                                VectorType &, VectorType &);

private:
  Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer_;
  Microsoft::WRL::ComPtr<ID3D11Buffer> index_buffer_;

  int vertex_count_ = 0;
  int index_count_ = 0;

  std::unique_ptr<TGATexture[]> textures_;

  std::vector<ModelType> model_;

  DirectX::XMMATRIX world_matrix_ = DirectX::XMMatrixIdentity();
};