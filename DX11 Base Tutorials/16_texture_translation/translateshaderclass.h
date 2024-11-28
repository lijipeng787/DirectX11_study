#pragma once

#include <DirectXMath.h>
#include <d3d11.h>

struct MatrixBufferType;
struct TranslateBufferType;

class TranslateShaderClass {
public:
  TranslateShaderClass() {}

  TranslateShaderClass(const TranslateShaderClass &) = delete;

  ~TranslateShaderClass() {}

public:
  bool Initialize(HWND);

  void Shutdown();

  bool Render(int, const DirectX::XMMATRIX &, const DirectX::XMMATRIX &,
              const DirectX::XMMATRIX &, ID3D11ShaderResourceView *, float);

private:
  bool InitializeShader(HWND, WCHAR *, WCHAR *);

  void ShutdownShader();

  void OutputShaderErrorMessage(ID3D10Blob *, HWND, WCHAR *);

  bool SetShaderParameters(const DirectX::XMMATRIX &, const DirectX::XMMATRIX &,
                           const DirectX::XMMATRIX &,
                           ID3D11ShaderResourceView *, float);

  void RenderShader(int);

private:
  ID3D11VertexShader *vertex_shader_ = nullptr;

  ID3D11PixelShader *pixel_shader_ = nullptr;

  ID3D11InputLayout *layout_ = nullptr;

  ID3D11Buffer *matrix_buffer_ = nullptr, *m_translateBuffer = nullptr;

  ID3D11SamplerState *sample_state_ = nullptr;
};