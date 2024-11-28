#ifndef _DEPTHSHADERCLASS_H_
#define _DEPTHSHADERCLASS_H_

#include <DirectXMath.h>
#include <d3d11.h>

using namespace DirectX;

class DepthShaderClass {
public:
  DepthShaderClass();

  DepthShaderClass(const DepthShaderClass &);

  ~DepthShaderClass();

public:
  bool Initialize(HWND);

  void Shutdown();

  bool Render(int, const XMMATRIX &, const XMMATRIX &, const XMMATRIX &);

private:
  bool InitializeShader(HWND, WCHAR *, WCHAR *);

  void ShutdownShader();

  void OutputShaderErrorMessage(ID3D10Blob *, HWND, WCHAR *);

  bool SetShaderParameters(const XMMATRIX &, const XMMATRIX &,
                           const XMMATRIX &);

  void RenderShader(int);

private:
  ID3D11VertexShader *vertex_shader_;

  ID3D11PixelShader *pixel_shader_;

  ID3D11InputLayout *layout_;

  ID3D11Buffer *matrix_buffer_;
};

#endif