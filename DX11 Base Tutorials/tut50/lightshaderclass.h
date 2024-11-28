#pragma once

#include <DirectXMath.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <fstream>

using namespace std;
using namespace DirectX;

class LightShaderClass {
private:
  struct MatrixBufferType {
    XMMATRIX world;
    XMMATRIX view;
    XMMATRIX projection;
  };

  struct LightBufferType {
    XMFLOAT3 lightDirection;
    float padding;
  };

public:
  LightShaderClass();
  LightShaderClass(const LightShaderClass &);
  ~LightShaderClass();

  bool Initialize(HWND);
  void Shutdown();
  bool Render(int, const XMMATRIX &, const XMMATRIX &, const XMMATRIX &,
              ID3D11ShaderResourceView *, ID3D11ShaderResourceView *,
              const XMFLOAT3 &);

private:
  bool InitializeShader(HWND, WCHAR *, WCHAR *);
  void ShutdownShader();
  void OutputShaderErrorMessage(ID3D10Blob *, HWND, WCHAR *);

  bool SetShaderParameters(const XMMATRIX &, const XMMATRIX &, const XMMATRIX &,
                           ID3D11ShaderResourceView *,
                           ID3D11ShaderResourceView *, const XMFLOAT3 &);
  void RenderShader(int);

private:
  ID3D11VertexShader *vertex_shader_;
  ID3D11PixelShader *pixel_shader_;
  ID3D11InputLayout *layout_;
  ID3D11SamplerState *sample_state_;
  ID3D11Buffer *matrix_buffer_;
  ID3D11Buffer *light_buffer_;
};
