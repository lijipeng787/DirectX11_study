#pragma once

#include <DirectXMath.h>
#include <d3d11.h>

struct MatrixBufferType;
struct ClipPlaneBufferType;

class ClipPlaneShaderClass {
public:
  ClipPlaneShaderClass() {}

  ClipPlaneShaderClass(const ClipPlaneShaderClass &rhs) = delete;

  ~ClipPlaneShaderClass() {}

public:
  bool Initialize(HWND);

  void Shutdown();

  bool Render(int, const DirectX::XMMATRIX &, const DirectX::XMMATRIX &,
              const DirectX::XMMATRIX &, ID3D11ShaderResourceView *,
              const DirectX::XMFLOAT4 &);

private:
  bool InitializeShader(HWND, WCHAR *, WCHAR *);

  void ShutdownShader();

  void OutputShaderErrorMessage(ID3D10Blob *, HWND, WCHAR *);

  bool SetShaderParameters(const DirectX::XMMATRIX &, const DirectX::XMMATRIX &,
                           const DirectX::XMMATRIX &,
                           ID3D11ShaderResourceView *,
                           const DirectX::XMFLOAT4 &);

  void RenderShader(int);

private:
  ID3D11VertexShader *vertex_shader_ = nullptr;

  ID3D11PixelShader *pixel_shader_ = nullptr;

  ID3D11InputLayout *layout_ = nullptr;

  ID3D11Buffer *matrix_buffer_ = nullptr, *clipplane_buffer_ = nullptr;

  ID3D11SamplerState *sample_state_ = nullptr;
};