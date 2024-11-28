#pragma once

#include <DirectXMath.h>
#include <d3d11.h>

class AlphaMapShaderClass {
public:
  AlphaMapShaderClass() {}

  AlphaMapShaderClass(const AlphaMapShaderClass &) = delete;

  ~AlphaMapShaderClass() {}

public:
  bool Initialize(HWND);

  void Shutdown();

  bool Render(int, const DirectX::XMMATRIX &, const DirectX::XMMATRIX &,
              const DirectX::XMMATRIX &, ID3D11ShaderResourceView **);

private:
  bool InitializeShader(HWND, WCHAR *, WCHAR *);

  void ShutdownShader();

  void OutputShaderErrorMessage(ID3D10Blob *, HWND, WCHAR *);

  bool SetShaderParameters(const DirectX::XMMATRIX &, const DirectX::XMMATRIX &,
                           const DirectX::XMMATRIX &,
                           ID3D11ShaderResourceView **);

  void RenderShader(int);

private:
  ID3D11VertexShader *vertex_shader_;

  ID3D11PixelShader *pixel_shader_;

  ID3D11InputLayout *layout_;

  ID3D11Buffer *matrix_buffer_;

  ID3D11SamplerState *sample_state_;
};