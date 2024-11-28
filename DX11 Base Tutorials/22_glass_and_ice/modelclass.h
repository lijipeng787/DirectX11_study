#pragma once

#include <d3d11.h>

struct VertexType;
struct ModelType;

class TextureClass;

class ModelClass {
public:
  ModelClass() {}

  ModelClass(const ModelClass &rhs) = delete;

  ~ModelClass() {}

public:
  bool Initialize(char *, WCHAR *, WCHAR *);

  void Shutdown();

  void Render();

  int GetIndexCount();

  ID3D11ShaderResourceView *GetTexture();

  ID3D11ShaderResourceView *GetNormalMap();

private:
  bool InitializeBuffers();

  void ShutdownBuffers();

  void RenderBuffers();

  bool LoadTextures(WCHAR *, WCHAR *);

  void ReleaseTextures();

  bool LoadModel(char *);

  void ReleaseModel();

private:
  ID3D11Buffer *vertex_buffer_ = nullptr, *index_buffer_ = nullptr;

  int vertex_count_ = 0, index_count_ = 0;

  TextureClass *texture_ = nullptr, *normal_map_ = nullptr;

  ModelType *model_ = nullptr;
};
