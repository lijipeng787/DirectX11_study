#pragma once

#include <d3d11.h>

#include <wrl\client.h>

struct VertexType;

class ModelClass {
public:
  ModelClass() {}

  ModelClass(const ModelClass &) = delete;

  ~ModelClass() {}

public:
  bool Initialize();

  void Shutdown();

  void Render();

  int GetIndexCount() const noexcept { return index_count_; }

private:
  bool InitializeBuffers();

  void ShutdownBuffers();

  void RenderBuffers();

private:
  Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer_, index_buffer_;

  int vertex_count_ = 0, index_count_ = 0;
};
