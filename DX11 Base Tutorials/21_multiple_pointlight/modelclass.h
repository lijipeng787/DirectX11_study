#pragma once

#include <DirectXMath.h>
#include <d3d11.h>

using namespace DirectX;

struct ModelType;
class TextureClass;

class ModelClass {
public:
  ModelClass();

  ModelClass(const ModelClass &);

  ~ModelClass();

public:
  bool Initialize(WCHAR *, char *);

  void Shutdown();

  void Render();

  int GetIndexCount();

  ID3D11ShaderResourceView *GetTexture();

private:
  bool InitializeBuffers();

  void ShutdownBuffers();

  void RenderBuffers();

  bool LoadTexture(WCHAR *);

  void ReleaseTexture();

  bool LoadModel(char *);

  void ReleaseModel();

private:
  ID3D11Buffer *vertex_buffer_, *index_buffer_;

  int vertex_count_, index_count_;

  TextureClass *texture_;

  ModelType *model_;
};
