#include "RenderTexture.h"

#include "../CommonFramework2/DirectX11Device.h"

using namespace DirectX;

bool RenderTexture::Initialize(int textureWidth, int textureHeight,
                               float screenDepth, float screenNear) {
  D3D11_TEXTURE2D_DESC textureDesc;
  D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
  D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
  D3D11_TEXTURE2D_DESC depthBufferDesc;
  D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;

  ZeroMemory(&textureDesc, sizeof(textureDesc));

  textureDesc.Width = textureWidth;
  textureDesc.Height = textureHeight;
  textureDesc.MipLevels = 1;
  textureDesc.ArraySize = 1;
  textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
  textureDesc.SampleDesc.Count = 1;
  textureDesc.Usage = D3D11_USAGE_DEFAULT;
  textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
  textureDesc.CPUAccessFlags = 0;
  textureDesc.MiscFlags = 0;

  auto device = DirectX11Device::GetD3d11DeviceInstance()->GetDevice();

  auto result = device->CreateTexture2D(&textureDesc, NULL,
                                        render_target_texture_.GetAddressOf());
  if (FAILED(result)) {
    return false;
  }

  renderTargetViewDesc.Format = textureDesc.Format;
  renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
  renderTargetViewDesc.Texture2D.MipSlice = 0;

  result = device->CreateRenderTargetView(render_target_texture_.Get(),
                                          &renderTargetViewDesc,
                                          render_target_view_.GetAddressOf());
  if (FAILED(result)) {
    return false;
  }

  shaderResourceViewDesc.Format = textureDesc.Format;
  shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
  shaderResourceViewDesc.Texture2D.MipLevels = 1;

  result = device->CreateShaderResourceView(
      render_target_texture_.Get(), &shaderResourceViewDesc,
      shader_resource_view_.GetAddressOf());
  if (FAILED(result)) {
    return false;
  }

  ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

  depthBufferDesc.Width = textureWidth;
  depthBufferDesc.Height = textureHeight;
  depthBufferDesc.MipLevels = 1;
  depthBufferDesc.ArraySize = 1;
  depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depthBufferDesc.SampleDesc.Count = 1;
  depthBufferDesc.SampleDesc.Quality = 0;
  depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
  depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
  depthBufferDesc.CPUAccessFlags = 0;
  depthBufferDesc.MiscFlags = 0;

  result = device->CreateTexture2D(&depthBufferDesc, NULL,
                                   depth_stencil_buffer_.GetAddressOf());
  if (FAILED(result)) {
    return false;
  }

  ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

  depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
  depthStencilViewDesc.Texture2D.MipSlice = 0;

  result = device->CreateDepthStencilView(depth_stencil_buffer_.Get(),
                                          &depthStencilViewDesc,
                                          depth_stencil_view_.GetAddressOf());
  if (FAILED(result)) {
    return false;
  }

  viewport_.Width = (float)textureWidth;
  viewport_.Height = (float)textureHeight;
  viewport_.MinDepth = 0.0f;
  viewport_.MaxDepth = 1.0f;
  viewport_.TopLeftX = 0.0f;
  viewport_.TopLeftY = 0.0f;

  projection_matrix_ = XMMatrixPerspectiveFovLH(
      ((float)XM_PI / 4.0f), ((float)textureWidth / (float)textureHeight),
      screenNear, screenDepth);

  ortho_matrix_ = XMMatrixOrthographicLH(
      (float)textureWidth, (float)textureHeight, screenNear, screenDepth);

  return true;
}

void RenderTexture::SetRenderTarget() {

  auto device_context =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();
  device_context->OMSetRenderTargets(1, render_target_view_.GetAddressOf(),
                                     depth_stencil_view_.Get());

  device_context->RSSetViewports(1, &viewport_);
}

void RenderTexture::ClearRenderTarget(float red, float green, float blue,
                                      float alpha) {
  float color[4];

  color[0] = red;
  color[1] = green;
  color[2] = blue;
  color[3] = alpha;

  auto device_context =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  device_context->ClearRenderTargetView(render_target_view_.Get(), color);

  device_context->ClearDepthStencilView(depth_stencil_view_.Get(),
                                        D3D11_CLEAR_DEPTH, 1.0f, 0);
}

ID3D11ShaderResourceView *RenderTexture::GetShaderResourceView() const {
  return shader_resource_view_.Get();
}

void RenderTexture::GetProjectionMatrix(XMMATRIX &projectionMatrix) const {
  projectionMatrix = projection_matrix_;
}

void RenderTexture::GetOrthoMatrix(XMMATRIX &orthoMatrix) const {
  orthoMatrix = ortho_matrix_;
}