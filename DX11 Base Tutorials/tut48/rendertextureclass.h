#pragma once

#include <DirectXMath.h>
#include <d3d11.h>

using namespace DirectX;

class RenderTextureClass {
public:
  RenderTextureClass();

  RenderTextureClass(const RenderTextureClass &);

  ~RenderTextureClass();

public:
  bool Initialize(int, int, float, float);

  void Shutdown();

  void SetRenderTarget();

  void ClearRenderTarget(float, float, float, float);

  ID3D11ShaderResourceView *GetShaderResourceView();

  void GetProjectionMatrix(XMMATRIX &);

  void GetOrthoMatrix(XMMATRIX &);

private:
  ID3D11Texture2D *render_target_texture_;

  ID3D11RenderTargetView *render_target_view_;

  ID3D11ShaderResourceView *shader_resource_view_;

  ID3D11Texture2D *depth_stencil_buffer_;

  ID3D11DepthStencilView *depth_stencil_view_;

  D3D11_VIEWPORT viewport_;

  XMMATRIX projection_matrix_;

  XMMATRIX ortho_matrix_;
};
