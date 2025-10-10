#pragma once

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>

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
  bool InitializeShader(HWND, const wchar_t *, const wchar_t *);

  void ShutdownShader();

  void OutputShaderErrorMessage(ID3D10Blob *, HWND, const wchar_t *);

  bool SetShaderParameters(const DirectX::XMMATRIX &, const DirectX::XMMATRIX &,
                           const DirectX::XMMATRIX &);

  void RenderShader(int);

private:
  Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader_;

  Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader_;

  Microsoft::WRL::ComPtr<ID3D11InputLayout> layout_;

  Microsoft::WRL::ComPtr<ID3D11Buffer> matrix_buffer_;
};
