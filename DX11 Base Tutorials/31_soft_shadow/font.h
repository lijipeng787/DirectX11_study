#pragma once

#include <d3d11.h>
#include <memory>
#include <string>

struct FontType;

class Font {
public:
  Font() = default;

  Font(const Font &) = delete;

  Font &operator=(const Font &) = delete;

  ~Font();

public:
  bool Initialize(const std::string &fontDataFilename,
                  const std::wstring &textureFilename, ID3D11Device *device);

  void Shutdown();

  ID3D11ShaderResourceView *GetTexture() const;

  void BuildVertexArray(void *vertices, const char *sentence, float drawX,
                        float drawY);

private:
  bool LoadFontData(const std::string &filename);

  void ReleaseFontData();

  bool LoadTexture(const std::wstring &filename, ID3D11Device *device);

  void ReleaseTexture();

private:
  FontType *font_ = nullptr;

  std::unique_ptr<class DDSTexture> texture_;
};
