

#ifndef _SHADOWSHADERCLASS_H_
#define _SHADOWSHADERCLASS_H_

#include <DirectXMath.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <fstream>
using namespace std;
using namespace DirectX;

class ShadowShaderClass {
private:
  struct MatrixBufferType {
    XMMATRIX world;
    XMMATRIX view;
    XMMATRIX projection;
    XMMATRIX lightView;
    XMMATRIX lightProjection;
  };

  struct LightBufferType {
    XMFLOAT4 ambientColor;
    XMFLOAT4 diffuseColor;
    XMFLOAT3 lightDirection;
    float padding;
  };

public:
  ShadowShaderClass();
  ShadowShaderClass(const ShadowShaderClass &);
  ~ShadowShaderClass();

  bool Initialize(HWND);
  void Shutdown();
  bool Render(int, const XMMATRIX &, const XMMATRIX &, const XMMATRIX &,
              const XMMATRIX &, const XMMATRIX &, ID3D11ShaderResourceView *,
              ID3D11ShaderResourceView *, const XMFLOAT3 &, const XMFLOAT4 &,
              const XMFLOAT4 &);

private:
  bool InitializeShader(HWND, WCHAR *, WCHAR *);
  void ShutdownShader();
  void OutputShaderErrorMessage(ID3D10Blob *, HWND, WCHAR *);

  bool SetShaderParameters(const XMMATRIX &, const XMMATRIX &, const XMMATRIX &,
                           const XMMATRIX &, const XMMATRIX &,
                           ID3D11ShaderResourceView *,
                           ID3D11ShaderResourceView *, const XMFLOAT3 &,
                           const XMFLOAT4 &, const XMFLOAT4 &);
  void RenderShader(int);

private:
  ID3D11VertexShader *vertex_shader_;
  ID3D11PixelShader *pixel_shader_;
  ID3D11InputLayout *layout_;
  ID3D11SamplerState *m_sampleStateWrap;
  ID3D11SamplerState *m_sampleStateClamp;
  ID3D11Buffer *matrix_buffer_;
  ID3D11Buffer *light_buffer_;
};

#endif