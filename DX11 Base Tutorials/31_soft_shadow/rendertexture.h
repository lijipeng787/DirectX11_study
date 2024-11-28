#pragma once

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl\client.h>

class RenderTexture {
public:
  RenderTexture() = default;

  RenderTexture(const RenderTexture &) = delete;

  ~RenderTexture() = default;

public:
  bool Initialize(int, int, float, float);

  void Shutdown() {}

  void SetRenderTarget();

  void ClearRenderTarget(float, float, float, float);

  ID3D11ShaderResourceView *GetShaderResourceView() const;

  void GetProjectionMatrix(DirectX::XMMATRIX &) const;

  void GetOrthoMatrix(DirectX::XMMATRIX &) const;

private:
  Microsoft::WRL::ComPtr<ID3D11RenderTargetView> render_target_view_;
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_view_;
  Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depth_stencil_view_;

  Microsoft::WRL::ComPtr<ID3D11Texture2D> render_target_texture_;
  Microsoft::WRL::ComPtr<ID3D11Texture2D> depth_stencil_buffer_;
  D3D11_VIEWPORT viewport_;

  DirectX::XMMATRIX projection_matrix_;
  DirectX::XMMATRIX ortho_matrix_;
};
