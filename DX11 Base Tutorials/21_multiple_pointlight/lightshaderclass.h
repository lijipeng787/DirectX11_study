#pragma once

const int NUM_LIGHTS = 4;

#include <DirectXMath.h>
#include <d3d11.h>

using namespace DirectX;

class LightShaderClass {
public:
  LightShaderClass();

  LightShaderClass(const LightShaderClass &);

  ~LightShaderClass();

public:
  bool Initialize(HWND);

  void Shutdown();

  bool Render(int, const XMMATRIX &, const XMMATRIX &, const XMMATRIX &,
              ID3D11ShaderResourceView *, const XMFLOAT4[], const XMFLOAT4[]);

private:
  bool InitializeShader(HWND, WCHAR *, WCHAR *);

  void ShutdownShader();

  void OutputShaderErrorMessage(ID3D10Blob *, HWND, WCHAR *);

  bool SetShaderParameters(const XMMATRIX &, const XMMATRIX &, const XMMATRIX &,
                           ID3D11ShaderResourceView *, const XMFLOAT4[],
                           const XMFLOAT4[]);

  void RenderShader(int);

private:
  ID3D11VertexShader *vertex_shader_;

  ID3D11PixelShader *pixel_shader_;

  ID3D11InputLayout *layout_;

  ID3D11SamplerState *sample_state_;

  ID3D11Buffer *matrix_buffer_;

  ID3D11Buffer *m_lightColorBuffer;

  ID3D11Buffer *m_lightPositionBuffer;
};
