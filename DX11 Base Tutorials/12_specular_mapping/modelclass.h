#pragma once

#include <d3d11.h>

struct VertexType;
struct ModelType;
struct TempVertexType;
struct VectorType;
class TextureArrayClass;

class ModelClass {
public:
  ModelClass() {}

  ModelClass(const ModelClass &) = delete;

  ~ModelClass() {}

  bool Initialize(char *, WCHAR *, WCHAR *, WCHAR *);

  void Shutdown();

  void Render();

  int GetIndexCount();

  ID3D11ShaderResourceView **GetTextureArray();

private:
  bool InitializeBuffers();

  void ShutdownBuffers();

  void RenderBuffers();

  bool LoadTextures(WCHAR *, WCHAR *, WCHAR *);

  void ReleaseTextures();

  bool LoadModel(char *);

  void ReleaseModel();

  void CalculateModelVectors();

  void CalculateTangentBinormal(TempVertexType *, TempVertexType *,
                                TempVertexType *, VectorType &, VectorType &);

  void CalculateNormal(VectorType *, VectorType *, VectorType &);

private:
  ID3D11Buffer *vertex_buffer_ = nullptr, *index_buffer_ = nullptr;

  int vertex_count_ = 0, index_count_ = 0;

  ModelType *model_ = nullptr;

  TextureArrayClass *texture_array_ = nullptr;
};
