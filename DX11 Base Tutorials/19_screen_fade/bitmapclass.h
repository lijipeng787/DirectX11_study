#pragma once

#include <d3d11.h>

struct VertexType;

class SimpleMoveableSurface {
public:
  SimpleMoveableSurface() {}

  SimpleMoveableSurface(const SimpleMoveableSurface &rhs) = delete;

  ~SimpleMoveableSurface() {}

public:
  bool Initialize(int, int, int, int);

  void Shutdown();

  bool Render(int, int);

  int GetIndexCount();

private:
  bool InitializeBuffers();

  void ShutdownBuffers();

  bool UpdateBuffers(int, int);

  void RenderBuffers();

  bool LoadTexture(WCHAR *);

  void ReleaseTexture();

private:
  ID3D11Buffer *vertex_buffer_ = nullptr, *index_buffer_ = nullptr;

  int vertex_count_ = 0, index_count_ = 0;

  int screen_width_ = 0, screen_height_ = 0;

  int bitmap_width_ = 0, bitmap_height_ = 0;

  int previous_pos_x_ = 0, previous_pos_y_ = 0;
};