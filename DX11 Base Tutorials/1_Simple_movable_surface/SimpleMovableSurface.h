#pragma once

#include <d3d11.h>

class SimpleTexture;

class SimpleMoveableSurface {
public:
  SimpleMoveableSurface() {}

  // for simplicity, this type of object is nocopyable
  SimpleMoveableSurface(const SimpleMoveableSurface &rhs) = delete;

  SimpleMoveableSurface &operator=(const SimpleMoveableSurface &rhs) = delete;

  ~SimpleMoveableSurface() {}

public:
  // this type of obejct just same like camera object that has own coordinate
  // system itself it should be move and can submit their coordinate data so
  // that can be drawn by rendering system
  void SetMoveStep(int step);

  void MoveTowardLeft2D(int units);

  void MoveTowardRight2D(int units);

  void MoveTowardTop2D(int units);

  void MoveTowardBottom2D(int units);

  void SetPosition(float x, float y, float z);

  void SetRotation(float x, float y, float z);

  void SetPosition2D(float x, float y);

  void SetRotation2D(float x, float y);

public:
  void InitializeWithSurfaceSize(int bitmapHeight, int bitmapWidth);

  // for now, using WCHAR type of character/string only
  bool LoadTextureFromFile(WCHAR *fileName);

  bool InitializeVertexAndIndexBuffers();

  void Release();

  bool Render();

  int GetIndexCount();

  ID3D11ShaderResourceView *GetTexture();

private:
  void ReleaseBuffers();

  bool UpdateBuffers();

  void SubmitBuffers();

  void ReleaseTexture();

private:
  float position_x_ = 0.0f, position_y_ = 0.0f, position_z_ = 0.0f;

  float rotation_x_ = 0.0f, rotation_y_ = 0.0f, rotation_z_ = 0.0f;

  int move_step_ = 0;

  ID3D11Buffer *vertex_buffer_ = nullptr, *index_buffer_ = nullptr;

  int vertex_count_ = 0, index_count_ = 0;

  SimpleTexture *texture_ = nullptr;

  int screen_width_ = 0, screen_height_ = 0;

  int bitmap_width_ = 0, bitmap_height_ = 0;

  float previous_posititon_x_ = 0, previous_posititon_y_ = 0;
};
