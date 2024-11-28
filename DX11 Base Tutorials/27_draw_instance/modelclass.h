#pragma once

#include <d3d11.h>

struct VertexType;
struct InstanceType;

class TextureClass;

class ModelClass {
public:
  ModelClass() {}

  ModelClass(const ModelClass &rhs) = delete;

  ~ModelClass() {}

public:
  bool Initialize(WCHAR *);

  void Shutdown();

  void Render();

  int GetVertexCount();

  int GetInstanceCount();

  ID3D11ShaderResourceView *GetTexture();

private:
  bool InitializeBuffers();

  void ShutdownBuffers();

  void RenderBuffers();

  bool LoadTexture(WCHAR *);

  void ReleaseTexture();

private:
  ID3D11Buffer *vertex_buffer_ = nullptr, *instance_buffer_ = nullptr;

  int vertex_count_ = 0, instance_count_ = 0;

  TextureClass *texture_ = nullptr;
};
