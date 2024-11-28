#ifndef _GLOWSHADERCLASS_H_
#define _GLOWSHADERCLASS_H_

#include <DirectXMath.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <fstream>

using namespace std;
using namespace DirectX;

class GlowShaderClass {
private:
  struct MatrixBufferType {
    XMMATRIX world;
    XMMATRIX view;
    XMMATRIX projection;
  };

  struct GlowBufferType {
    float glowStrength;
    XMFLOAT3 padding;
  };

public:
  GlowShaderClass();
  GlowShaderClass(const GlowShaderClass &);
  ~GlowShaderClass();

  bool Initialize(HWND);
  void Shutdown();
  bool Render(int, const XMMATRIX &, const XMMATRIX &, const XMMATRIX &,
              ID3D11ShaderResourceView *, ID3D11ShaderResourceView *, float);

private:
  bool InitializeShader(HWND, WCHAR *, WCHAR *);
  void ShutdownShader();
  void OutputShaderErrorMessage(ID3D10Blob *, HWND, WCHAR *);

  bool SetShaderParameters(const XMMATRIX &, const XMMATRIX &, const XMMATRIX &,
                           ID3D11ShaderResourceView *,
                           ID3D11ShaderResourceView *, float);
  void RenderShader(int);

private:
  ID3D11VertexShader *vertex_shader_;
  ID3D11PixelShader *pixel_shader_;
  ID3D11InputLayout *layout_;
  ID3D11Buffer *matrix_buffer_;
  ID3D11Buffer *m_glowBuffer;
  ID3D11SamplerState *sample_state_;
};

#endif