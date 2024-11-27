#pragma once

#include <DirectXMath.h>
#include <d3d11.h>
#include <d3dcommon.h>
#include <dxgi.h>
#include <wrl\client.h>

class DirectX11Device {
public:
  DirectX11Device() {}

  DirectX11Device(const DirectX11Device &rhs) = delete;

  DirectX11Device &operator=(const DirectX11Device &rhs) = delete;

  ~DirectX11Device() {}

public:
  static DirectX11Device *GetD3d11DeviceInstance() {

    if (nullptr == device_instance_) {
      device_instance_ = new DirectX11Device;
    }

    return device_instance_;
  }

public:
  inline unsigned int GetScreenWidth() const { return screen_width_; }

  inline unsigned int GetScreenHeight() const { return screen_height_; }

public:
  bool Initialize(unsigned int screenWidth, unsigned int screenHeight,
                  bool vsync, HWND hwnd, bool fullscreen, float screenDepth,
                  float screenNear);

  void Shutdown();

  void BeginScene(float red, float green, float blue, float alpha);

  void EndScene();

  void TurnZBufferOn();

  void TurnZBufferOff();

  void TurnOnAlphaBlending();

  void TurnOffAlphaBlending();

  void TurnOnCulling();

  void TurnOffCulling();

  void SetBackBufferRenderTarget();

  void ResetViewport();

  ID3D11Device *GetDevice();

  ID3D11DeviceContext *GetDeviceContext();

  void GetProjectionMatrix(DirectX::XMMATRIX &projection_matrix);

  void GetWorldMatrix(DirectX::XMMATRIX &world_matrix);

  void GetOrthoMatrix(DirectX::XMMATRIX &orthonality_matrix);

  void GetVideoCardInfo(char *, int &);

  ID3D11DepthStencilView *GetDepthStencilView() const;

private:
  bool vsync_enabled_ = false;

  unsigned int videocard_Memory_ = 0;

  char videocard_description_[128] = {};

  Microsoft::WRL::ComPtr<IDXGISwapChain> swap_chain_;

  Microsoft::WRL::ComPtr<ID3D11Device> device_;

  static DirectX11Device *device_instance_;

  Microsoft::WRL::ComPtr<ID3D11DeviceContext> device_context_;

  Microsoft::WRL::ComPtr<ID3D11RenderTargetView> render_target_view_;

  Microsoft::WRL::ComPtr<ID3D11Texture2D> depth_stencil_buffer_;

  Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depth_stencil_state_;

  Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depth_stencil_view_;

  Microsoft::WRL::ComPtr<ID3D11RasterizerState> raster_state_;

  Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterStateNoCulling;

  DirectX::XMMATRIX projection_matrix_ = {};

  DirectX::XMMATRIX world_matrix_ = {};

  DirectX::XMMATRIX orthonality_matrix_ = {};

  Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depth_disabled_stencil_state_;

  Microsoft::WRL::ComPtr<ID3D11BlendState> alpha_enable_blending_state_;

  Microsoft::WRL::ComPtr<ID3D11BlendState> alpha_disable_blending_state_;

  D3D11_VIEWPORT viewport_ = {};

  unsigned int screen_width_ = 0, screen_height_ = 0;
};