#pragma once

#include <DirectXMath.h>
#include <d3d11.h>
#include <memory>
#include <string>

struct SentenceType;

class Font;
class FontShader;

class Text {
public:
  Text() = default;

  Text(const Text &) = delete;

  Text &operator=(const Text &) = delete;

  ~Text();

public:
  bool Initialize(int screenWidth, int screenHeight,
                  const DirectX::XMMATRIX &baseViewMatrix,
                  std::shared_ptr<Font> font,
                  std::shared_ptr<FontShader> fontShader, ID3D11Device *device);

  void Shutdown();

  bool Render(const DirectX::XMMATRIX &worldMatrix,
              const DirectX::XMMATRIX &orthoMatrix,
              ID3D11DeviceContext *deviceContext);

  bool SetRenderCount(int count);

private:
  bool InitializeSentence(SentenceType **sentence, int maxLength);

  bool UpdateSentence(SentenceType *sentence, const char *text, int positionX,
                      int positionY, float red, float green, float blue);

  void ReleaseSentence(SentenceType **sentence);

  bool RenderSentence(SentenceType *sentence,
                      const DirectX::XMMATRIX &worldMatrix,
                      const DirectX::XMMATRIX &orthoMatrix,
                      ID3D11DeviceContext *deviceContext);

private:
  std::shared_ptr<Font> font_;
  std::shared_ptr<FontShader> font_shader_;

  int screen_width_ = 0, screen_height_ = 0;

  DirectX::XMFLOAT4X4 base_view_matrix_{};

  SentenceType *sentence_1_ = nullptr;
};
