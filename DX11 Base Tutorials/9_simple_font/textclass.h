#pragma once

#include <DirectXMath.h>
#include <d3d11.h>

struct SentenceType;
struct VertexType;

class FontClass;
class FontShaderClass;

class TextClass {
public:
  TextClass() {}

  TextClass(const TextClass &);

  ~TextClass() {}

public:
  bool Initialize(HWND, int, int, const DirectX::XMMATRIX &);

  void Shutdown();

  bool Render(const DirectX::XMMATRIX &, const DirectX::XMMATRIX &);

private:
  bool InitializeSentence(SentenceType **, int);

  bool UpdateSentence(SentenceType *, char *, int, int, float, float, float);

  void ReleaseSentence(SentenceType **);

  bool RenderSentence(SentenceType *, const DirectX::XMMATRIX &,
                      const DirectX::XMMATRIX &);

private:
  FontClass *font_ = nullptr;

  FontShaderClass *font_shader_ = nullptr;

  int screen_width_ = 0, screen_height_ = 0;

  DirectX::XMMATRIX base_view_matrix_{};

  SentenceType *sentence_1_ = nullptr, *sentence_2_ = nullptr;
};