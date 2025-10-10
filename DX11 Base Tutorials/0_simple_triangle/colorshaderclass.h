#pragma once

#include <DirectXMath.h>
#include <d3d11.h>

struct MatrixBufferType;

class ColorShaderClass {
public:
  ColorShaderClass() {}

  ColorShaderClass(const ColorShaderClass &) = delete;

  ~ColorShaderClass() {}

public:
  bool Initialize(HWND);

  void Shutdown();

  bool Render(int indexCount, const DirectX::XMMATRIX &,
              const DirectX::XMMATRIX &, const DirectX::XMMATRIX &) noexcept;

private:
  bool InitializeShader(HWND, WCHAR *, WCHAR *);

  void ShutdownShader();

  void OutputShaderErrorMessage(ID3D10Blob *, HWND, WCHAR *);

  bool SetShaderParameters(const DirectX::XMMATRIX &, const DirectX::XMMATRIX &,
                           const DirectX::XMMATRIX &);

  void RenderShader(int);

private:
  ID3D11VertexShader *vertex_shader_ = nullptr;

  ID3D11PixelShader *pixel_shader_ = nullptr;

  ID3D11InputLayout *layout_ = nullptr;

  ID3D11Buffer *matrix_buffer_ = nullptr;
};
