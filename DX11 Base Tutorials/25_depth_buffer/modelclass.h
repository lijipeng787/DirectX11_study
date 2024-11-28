#pragma once

#include <d3d11.h>

struct ModelType;

class ModelClass {
public:
  ModelClass() {}

  ModelClass(const ModelClass &rhs) = delete;

  ~ModelClass() {}

public:
  bool Initialize(char *);

  void Shutdown();

  void Render();

  int GetIndexCount();

private:
  bool InitializeBuffers();

  void ShutdownBuffers();

  void RenderBuffers();

  bool LoadModel(char *);

  void ReleaseModel();

private:
  ID3D11Buffer *vertex_buffer_ = nullptr, *index_buffer_ = nullptr;

  int vertex_count_ = 0, index_count_ = 0;

  ModelType *model_ = nullptr;
};