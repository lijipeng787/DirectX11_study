#pragma once

#include <d3d11.h>
#include <wrl/client.h>

class Texture {
public:
  explicit Texture() = default;

  Texture(const Texture &) = delete;

  Texture &operator=(const Texture &) = delete;

  Texture(Texture &&) noexcept = default;

  Texture &operator=(Texture &&) noexcept = default;

  virtual ~Texture() = default;

public:
  bool Initialize(const WCHAR *filename);

  ID3D11ShaderResourceView *GetTexture() const { return texture_.Get(); };

private:
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture_;
};

class TGATexture {
private:
  struct TargaHeader {
    unsigned char data1[12];
    unsigned short width;
    unsigned short height;
    unsigned char bpp;
    unsigned char data2;
  };

public:
  explicit TGATexture() = default;

  TGATexture(const Texture &) = delete;

  ~TGATexture() = default;

public:
  bool Initialize(const char *);

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
