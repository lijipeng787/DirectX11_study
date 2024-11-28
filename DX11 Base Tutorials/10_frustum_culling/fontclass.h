#pragma once

#include <d3d11.h>

struct FontType;
struct VertexType;

class TextureClass;

class FontClass {
public:
  FontClass() {}

  FontClass(const FontClass &rhs) = delete;

  ~FontClass() {}

public:
  bool Initialize(char *, WCHAR *);

  void Shutdown();

  ID3D11ShaderResourceView *GetTexture();

  void BuildVertexArray(void *, char *, float, float);

private:
  bool LoadFontData(char *);

  void ReleaseFontData();

  bool LoadTexture(WCHAR *);

  void ReleaseTexture();

private:
  FontType *font_ = nullptr;

  TextureClass *texture_ = nullptr;
};