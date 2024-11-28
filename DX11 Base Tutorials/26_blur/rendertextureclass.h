#pragma once

#include <DirectXMath.h>
#include <d3d11.h>

class RenderTextureClass {
public:
  RenderTextureClass() {}

  RenderTextureClass(const RenderTextureClass &) = delete;

  ~RenderTextureClass() {}

public:
  bool Initialize(int, int, float, float);

  void Shutdown();

  void SetRenderTarget();

  void ClearRenderTarget(float, float, float, float);

  ID3D11ShaderResourceView *GetShaderResourceView();

  void GetProjectionMatrix(DirectX::XMMATRIX &);

  void GetOrthoMatrix(DirectX::XMMATRIX &);

  int GetTextureWidth();

  int GetTextureHeight();

private:
  int texture_width_ = 0, texture_height_ = 0;

  ID3D11Texture2D *render_target_texture_ = nullptr,
                  *depth_stencil_buffer_ = nullptr;

  ID3D11RenderTargetView *render_target_view_ = nullptr;

  ID3D11ShaderResourceView *shader_resource_view_ = nullptr;

  ID3D11DepthStencilView *depth_stencil_view_ = nullptr;

  D3D11_VIEWPORT viewport_{};

  DirectX::XMMATRIX projection_matrix_{};

  DirectX::XMMATRIX ortho_matrix_{};
};
