

#ifndef _TRANSPARENTDEPTHSHADERCLASS_H_
#define _TRANSPARENTDEPTHSHADERCLASS_H_

#include <DirectXMath.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <fstream>
using namespace std;
using namespace DirectX;

class TransparentDepthShaderClass {
private:
  struct MatrixBufferType {
    XMMATRIX world;
    XMMATRIX view;
    XMMATRIX projection;
  };

public:
  TransparentDepthShaderClass();
  TransparentDepthShaderClass(const TransparentDepthShaderClass &);
  ~TransparentDepthShaderClass();

  bool Initialize(HWND);
  void Shutdown();
  bool Render(int, const XMMATRIX &, const XMMATRIX &, const XMMATRIX &,
              ID3D11ShaderResourceView *);

private:
  bool InitializeShader(HWND, WCHAR *, WCHAR *);
  void ShutdownShader();
  void OutputShaderErrorMessage(ID3D10Blob *, HWND, WCHAR *);

  bool SetShaderParameters(const XMMATRIX &, const XMMATRIX &, const XMMATRIX &,
                           ID3D11ShaderResourceView *);
  void RenderShader(int);

private:
  ID3D11VertexShader *vertex_shader_;
  ID3D11PixelShader *pixel_shader_;
  ID3D11InputLayout *layout_;
  ID3D11Buffer *matrix_buffer_;
  ID3D11SamplerState *sample_state_;
};

#endif