#pragma once

#include <d3d11.h>
#include <wrl/client.h>

class TextureClass {
public:
  explicit TextureClass() = default;

  TextureClass(const TextureClass &) = delete;

  TextureClass &operator=(const TextureClass &) = delete;

  TextureClass(TextureClass &&) noexcept = default;

  TextureClass &operator=(TextureClass &&) noexcept = default;

  virtual ~TextureClass() = default;

public:
  bool Initialize(const WCHAR *filename);

  void Shutdown();

  ID3D11ShaderResourceView *GetTexture() const { return texture_.Get(); };

private:
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture_;
};

class TGATextureClass {
private:
  struct TargaHeader {
    unsigned char data1[12];
    unsigned short width;
    unsigned short height;
    unsigned char bpp;
    unsigned char data2;
  };

public:
  explicit TGATextureClass() = default;

  TGATextureClass(const TextureClass &) = delete;

  ~TGATextureClass() = default;

public:
  bool Initialize(const char *);

  void Shutdown() {}

  ID3D11ShaderResourceView *GetTexture() const { return m_textureView.Get(); }

  int GetWidth() const { return m_width; }

  int GetHeight() const { return m_height; }

private:
  bool LoadTarga32Bit(const char *);

private:
  unsigned char *m_targaData;

  Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;

  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureView;

  int m_width, m_height;
};
