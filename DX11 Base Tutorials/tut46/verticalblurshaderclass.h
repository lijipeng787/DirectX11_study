

#ifndef _VERTICALBLURSHADERCLASS_H_
#define _VERTICALBLURSHADERCLASS_H_

#include <DirectXMath.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <fstream>
using namespace std;
using namespace DirectX;

class VerticalBlurShaderClass {
private:
  struct MatrixBufferType {
    XMMATRIX world;
    XMMATRIX view;
    XMMATRIX projection;
  };

  struct ScreenSizeBufferType {
    float screenHeight;
    XMFLOAT3 padding;
  };

public:
  VerticalBlurShaderClass();
  VerticalBlurShaderClass(const VerticalBlurShaderClass &);
  ~VerticalBlurShaderClass();

  bool Initialize(HWND);
  void Shutdown();
  bool Render(int, const XMMATRIX &, const XMMATRIX &, const XMMATRIX &,
              ID3D11ShaderResourceView *, float);

private:
  bool InitializeShader(HWND, WCHAR *, WCHAR *);
  void ShutdownShader();
  void OutputShaderErrorMessage(ID3D10Blob *, HWND, WCHAR *);

  bool SetShaderParameters(const XMMATRIX &, const XMMATRIX &, const XMMATRIX &,
                           ID3D11ShaderResourceView *, float);
  void RenderShader(int);

private:
  ID3D11VertexShader *vertex_shader_;
  ID3D11PixelShader *pixel_shader_;
  ID3D11InputLayout *layout_;
  ID3D11SamplerState *sample_state_;
  ID3D11Buffer *matrix_buffer_;
  ID3D11Buffer *screen_size_buffer_;
};

#endif