#pragma once

#include <DDSTextureLoader.h>
#include <d3d11.h>
using namespace DirectX;

class TextureClass {
public:
  TextureClass();
  TextureClass(const TextureClass &);
  ~TextureClass();

  bool Initialize(WCHAR *);
  void Shutdown();

  ID3D11ShaderResourceView *GetTexture();

private:
  ID3D11ShaderResourceView *texture_;
};
