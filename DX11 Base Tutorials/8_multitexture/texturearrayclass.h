#pragma once

#include <d3d11.h>

class TextureArrayClass {
public:
  TextureArrayClass() {}

  TextureArrayClass(const TextureArrayClass &rhs) = delete;

  ~TextureArrayClass() {}

  bool Initialize(WCHAR *, WCHAR *);

  void Shutdown();

  ID3D11ShaderResourceView **GetTextureArray();

private:
  ID3D11ShaderResourceView *textures_[2];
};