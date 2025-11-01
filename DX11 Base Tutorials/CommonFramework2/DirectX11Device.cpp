#include "stdafx.h"

#include "DirectX11Device.h"

using namespace DirectX;

DirectX11Device *DirectX11Device::device_instance_ = nullptr;

bool DirectX11Device::Initialize(unsigned int screenWidth,
                                 unsigned int screenHeight, bool vsync,
                                 HWND hwnd, bool fullscreen, float screenDepth,
                                 float screenNear) {
  screen_width_ = screenWidth;
  screen_height_ = screenHeight;
  vsync_enabled_ = vsync;

  IDXGIFactory *factory = nullptr;
  auto result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)&factory);
  if (FAILED(result)) {
    return false;
  }

  IDXGIAdapter *adapter = nullptr;
  result = factory->EnumAdapters(0, &adapter);
  if (FAILED(result)) {
    return false;
  }

  IDXGIOutput *adapterOutput = nullptr;
  result = adapter->EnumOutputs(0, &adapterOutput);
  if (FAILED(result)) {
    return false;
  }

  unsigned int numModes = 0, i = 0, numerator = 0, denominator = 0;
  result = adapterOutput->GetDisplayModeList(
      DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
  if (FAILED(result)) {
    return false;
  }

  DXGI_MODE_DESC *displayModeList = nullptr;
  displayModeList = new DXGI_MODE_DESC[numModes];
  if (!displayModeList) {
    return false;
  }

  result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM,
                                             DXGI_ENUM_MODES_INTERLACED,
                                             &numModes, displayModeList);
  if (FAILED(result)) {
    return false;
  }

  for (i = 0; i < numModes; i++) {

    if (displayModeList[i].Width == (unsigned int)screenWidth) {

      if (displayModeList[i].Height == (unsigned int)screenHeight) {
        numerator = displayModeList[i].RefreshRate.Numerator;
        denominator = displayModeList[i].RefreshRate.Denominator;
      }
    }
  }

  DXGI_ADAPTER_DESC adapterDesc;
  ZeroMemory(&adapterDesc, sizeof(DXGI_ADAPTER_DESC));
  result = adapter->GetDesc(&adapterDesc);
  if (FAILED(result)) {
    return false;
  }

  videocard_Memory_ = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

  size_t stringLength = 0;
  auto error = wcstombs_s(&stringLength, videocard_description_, 128,
                          adapterDesc.Description, 128);
  if (error != 0) {
    return false;
  }

  delete[] displayModeList;
  displayModeList = nullptr;

  adapterOutput->Release();
  adapterOutput = nullptr;

  adapter->Release();
  adapter = nullptr;

  factory->Release();
  factory = nullptr;

  DXGI_SWAP_CHAIN_DESC swapChainDesc;
  ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

  swapChainDesc.BufferCount = 1;
  swapChainDesc.BufferDesc.Width = screenWidth;
  swapChainDesc.BufferDesc.Height = screenHeight;
  swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

  if (vsync_enabled_) {
    swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
  } else {
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
  }

  swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swapChainDesc.OutputWindow = hwnd;
  swapChainDesc.SampleDesc.Count = 1;
  swapChainDesc.SampleDesc.Quality = 0;

  if (fullscreen) {
    swapChainDesc.Windowed = false;
  } else {
    swapChainDesc.Windowed = true;
  }

  swapChainDesc.BufferDesc.ScanlineOrdering =
      DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
  swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
  swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
  swapChainDesc.Flags = 0;

  D3D_FEATURE_LEVEL featureLevel;
  ZeroMemory(&featureLevel, sizeof(featureLevel));
  featureLevel = D3D_FEATURE_LEVEL_11_0;

  result = D3D11CreateDeviceAndSwapChain(
      NULL, D3D_DRIVER_TYPE_WARP, NULL, 0, &featureLevel, 1, D3D11_SDK_VERSION,
      &swapChainDesc, &swap_chain_, &device_, NULL, &device_context_);
  if (FAILED(result)) {
    return false;
  }

  ID3D11Texture2D *backBufferPtr = nullptr;
  result = swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D),
                                  (LPVOID *)&backBufferPtr);
  if (FAILED(result)) {
    return false;
  }

  result = device_->CreateRenderTargetView(backBufferPtr, NULL,
                                           render_target_view_.GetAddressOf());
  if (FAILED(result)) {
    return false;
  }

  backBufferPtr->Release();
  backBufferPtr = 0;

  D3D11_TEXTURE2D_DESC depthBufferDesc;
  ZeroMemory(&depthBufferDesc, sizeof(D3D11_TEXTURE2D_DESC));

  depthBufferDesc.Width = screenWidth;
  depthBufferDesc.Height = screenHeight;
  depthBufferDesc.MipLevels = 1;
  depthBufferDesc.ArraySize = 1;
  depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depthBufferDesc.SampleDesc.Count = 1;
  depthBufferDesc.SampleDesc.Quality = 0;
  depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
  depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
  depthBufferDesc.CPUAccessFlags = 0;
  depthBufferDesc.MiscFlags = 0;

  result =
      device_->CreateTexture2D(&depthBufferDesc, NULL, &depth_stencil_buffer_);
  if (FAILED(result)) {
    return false;
  }

  D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
  ZeroMemory(&depthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

  depthStencilDesc.DepthEnable = true;
  depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
  depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

  depthStencilDesc.StencilEnable = true;
  depthStencilDesc.StencilReadMask = 0xFF;
  depthStencilDesc.StencilWriteMask = 0xFF;

  depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
  depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
  depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
  depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

  depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
  depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
  depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
  depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

  result = device_->CreateDepthStencilState(&depthStencilDesc,
                                            &depth_stencil_state_);
  if (FAILED(result)) {
    return false;
  }

  device_context_->OMSetDepthStencilState(depth_stencil_state_.Get(), 1);

  D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
  ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));

  depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
  depthStencilViewDesc.Texture2D.MipSlice = 0;

  result = device_->CreateDepthStencilView(
      depth_stencil_buffer_.Get(), &depthStencilViewDesc, &depth_stencil_view_);
  if (FAILED(result)) {
    return false;
  }

  device_context_->OMSetRenderTargets(1, render_target_view_.GetAddressOf(),
                                      depth_stencil_view_.Get());

  D3D11_RASTERIZER_DESC rasterDesc;
  ZeroMemory(&rasterDesc, sizeof(D3D11_RASTERIZER_DESC));

  rasterDesc.AntialiasedLineEnable = false;
  rasterDesc.CullMode = D3D11_CULL_BACK;
  rasterDesc.DepthBias = 0;
  rasterDesc.DepthBiasClamp = 0.0f;
  rasterDesc.DepthClipEnable = true;
  // For tessellation
  // rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
  rasterDesc.FillMode = D3D11_FILL_SOLID;
  rasterDesc.FrontCounterClockwise = false;
  rasterDesc.MultisampleEnable = false;
  rasterDesc.ScissorEnable = false;
  rasterDesc.SlopeScaledDepthBias = 0.0f;

  result = device_->CreateRasterizerState(&rasterDesc, &raster_state_);
  if (FAILED(result)) {
    return false;
  }

  // Setup a raster description which turns off back face culling.
  rasterDesc.AntialiasedLineEnable = false;
  rasterDesc.CullMode = D3D11_CULL_NONE;
  rasterDesc.DepthBias = 0;
  rasterDesc.DepthBiasClamp = 0.0f;
  rasterDesc.DepthClipEnable = true;
  rasterDesc.FillMode = D3D11_FILL_SOLID;
  rasterDesc.FrontCounterClockwise = false;
  rasterDesc.MultisampleEnable = false;
  rasterDesc.ScissorEnable = false;
  rasterDesc.SlopeScaledDepthBias = 0.0f;

  // Create the no culling rasterizer state.
  result = device_->CreateRasterizerState(&rasterDesc, &m_rasterStateNoCulling);
  if (FAILED(result)) {
    return false;
  }

  device_context_->RSSetState(raster_state_.Get());

  viewport_.Width = (float)screenWidth;
  viewport_.Height = (float)screenHeight;
  viewport_.MinDepth = 0.0f;
  viewport_.MaxDepth = 1.0f;
  viewport_.TopLeftX = 0.0f;
  viewport_.TopLeftY = 0.0f;

  device_context_->RSSetViewports(1, &viewport_);

  float fieldOfView = 0.0f, screenAspect = 0.0f;
  {
    fieldOfView = (float)XM_PI / 4.0f;
    screenAspect = (float)screenWidth / (float)screenHeight;

    projection_matrix_ = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect,
                                                  screenNear, screenDepth);

    world_matrix_ = XMMatrixIdentity();

    orthonality_matrix_ = XMMatrixOrthographicLH(
        (float)screenWidth, (float)screenHeight, screenNear, screenDepth);
  }

  D3D11_DEPTH_STENCIL_DESC depthDisabledStencilDesc;
  ZeroMemory(&depthDisabledStencilDesc, sizeof(depth_disabled_stencil_state_));
  depthDisabledStencilDesc.DepthEnable = false;
  depthDisabledStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
  depthDisabledStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
  depthDisabledStencilDesc.StencilEnable = true;
  depthDisabledStencilDesc.StencilReadMask = 0xFF;
  depthDisabledStencilDesc.StencilWriteMask = 0xFF;
  depthDisabledStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
  depthDisabledStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
  depthDisabledStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
  depthDisabledStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
  depthDisabledStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
  depthDisabledStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
  depthDisabledStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
  depthDisabledStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

  result = device_->CreateDepthStencilState(&depthDisabledStencilDesc,
                                            &depth_disabled_stencil_state_);
  if (FAILED(result)) {
    return false;
  }

  D3D11_BLEND_DESC blendStateDescription = {};
  blendStateDescription.AlphaToCoverageEnable = FALSE;
  blendStateDescription.IndependentBlendEnable = FALSE;
  blendStateDescription.RenderTarget[0].BlendEnable = TRUE;
  blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
  blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
  blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
  blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
  blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
  blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
  blendStateDescription.RenderTarget[0].RenderTargetWriteMask =
      D3D11_COLOR_WRITE_ENABLE_ALL;

  result = device_->CreateBlendState(&blendStateDescription,
                                     &alpha_enable_blending_state_);
  if (FAILED(result)) {
    return false;
  }

  // Disabled blending state (no alpha-to-coverage, blending off)
  D3D11_BLEND_DESC blendDisableDesc = {};
  blendDisableDesc.AlphaToCoverageEnable = FALSE;
  blendDisableDesc.IndependentBlendEnable = FALSE;
  blendDisableDesc.RenderTarget[0].BlendEnable = FALSE;
  blendDisableDesc.RenderTarget[0].RenderTargetWriteMask =
      D3D11_COLOR_WRITE_ENABLE_ALL;

  result = device_->CreateBlendState(&blendDisableDesc,
                                     &alpha_disable_blending_state_);
  if (FAILED(result)) {
    return false;
  }

  // Preserve particle alpha-to-coverage configuration for future use
  D3D11_BLEND_DESC particleBlendDesc = {};
  particleBlendDesc.AlphaToCoverageEnable = TRUE;
  particleBlendDesc.IndependentBlendEnable = FALSE;
  particleBlendDesc.RenderTarget[0].BlendEnable = TRUE;
  particleBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
  particleBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
  particleBlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
  particleBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
  particleBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
  particleBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
  particleBlendDesc.RenderTarget[0].RenderTargetWriteMask =
      D3D11_COLOR_WRITE_ENABLE_ALL;

  result = device_->CreateBlendState(&particleBlendDesc,
                                     &alpha_particle_blending_state_);
  if (FAILED(result)) {
    return false;
  }

  return true;
}

void DirectX11Device::Shutdown() {}

void DirectX11Device::BeginScene(float red, float green, float blue,
                                 float alpha) {

  float color[4] = {0};

  color[0] = red;
  color[1] = green;
  color[2] = blue;
  color[3] = alpha;

  device_context_->ClearRenderTargetView(render_target_view_.Get(), color);

  device_context_->ClearDepthStencilView(depth_stencil_view_.Get(),
                                         D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void DirectX11Device::EndScene() {

  if (vsync_enabled_) {
    swap_chain_->Present(1, 0);
  } else {
    swap_chain_->Present(0, 0);
  }
}

void DirectX11Device::TurnZBufferOn() {
  device_context_->OMSetDepthStencilState(depth_stencil_state_.Get(), 1);
}

void DirectX11Device::TurnZBufferOff() {
  device_context_->OMSetDepthStencilState(depth_disabled_stencil_state_.Get(),
                                          1);
}

void DirectX11Device::TurnOnAlphaBlending() {

  float blendFactor[4];

  blendFactor[0] = 0.0f;
  blendFactor[1] = 0.0f;
  blendFactor[2] = 0.0f;
  blendFactor[3] = 0.0f;

  device_context_->OMSetBlendState(alpha_enable_blending_state_.Get(),
                                   blendFactor, 0xffffffff);
}

void DirectX11Device::TurnOffAlphaBlending() {

  float blendFactor[4];

  blendFactor[0] = 0.0f;
  blendFactor[1] = 0.0f;
  blendFactor[2] = 0.0f;
  blendFactor[3] = 0.0f;

  device_context_->OMSetBlendState(alpha_disable_blending_state_.Get(),
                                   blendFactor, 0xffffffff);
}

ID3D11Device *DirectX11Device::GetDevice() { return device_.Get(); }

ID3D11DeviceContext *DirectX11Device::GetDeviceContext() {
  return device_context_.Get();
}

void DirectX11Device::GetProjectionMatrix(XMMATRIX &projectionMatrix) {
  projectionMatrix = projection_matrix_;
}

void DirectX11Device::GetWorldMatrix(XMMATRIX &worldMatrix) {
  worldMatrix = world_matrix_;
}

void DirectX11Device::GetOrthoMatrix(XMMATRIX &orthoMatrix) {
  orthoMatrix = orthonality_matrix_;
}

void DirectX11Device::GetVideoCardInfo(char *cardName, int &memory) {
  strcpy_s(cardName, 128, videocard_description_);
  memory = videocard_Memory_;
}

ID3D11DepthStencilView *DirectX11Device::GetDepthStencilView() const {
  return depth_stencil_view_.Get();
}

void DirectX11Device::SetBackBufferRenderTarget() {
  device_context_->OMSetRenderTargets(1, render_target_view_.GetAddressOf(),
                                      depth_stencil_view_.Get());
}

void DirectX11Device::ResetViewport() {
  device_context_->RSSetViewports(1, &viewport_);
}

void DirectX11Device::TurnOnCulling() {
  device_context_->RSSetState(raster_state_.Get());
}

void DirectX11Device::TurnOffCulling() {
  device_context_->RSSetState(m_rasterStateNoCulling.Get());
}
