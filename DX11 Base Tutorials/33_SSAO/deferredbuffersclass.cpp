////////////////////////////////////////////////////////////////////////////////
// Filename: deferredbuffersclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "deferredbuffersclass.h"

DeferredBuffersClass::DeferredBuffersClass() {
  int i;

  for (i = 0; i < BUFFER_COUNT; i++) {
    m_renderTargetTextureArray[i] = 0;
    m_renderTargetViewArray[i] = 0;
    m_shaderResourceViewArray[i] = 0;
  }

  m_depthStencilBuffer = 0;
  m_depthStencilView = 0;
}

DeferredBuffersClass::DeferredBuffersClass(const DeferredBuffersClass &other) {}

DeferredBuffersClass::~DeferredBuffersClass() {}

bool DeferredBuffersClass::Initialize(ID3D11Device *device, int textureWidth,
                                      int textureHeight, float screenDepth,
                                      float screenNear) {
  HRESULT result;
  D3D11_TEXTURE2D_DESC depthBufferDesc;
  D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
  bool result2;

  // Store the width and height of the render texture.
  m_textureWidth = textureWidth;
  m_textureHeight = textureHeight;

  // Create the positions render texture.
  result2 = BuildRenderTexture(DXGI_FORMAT_R32G32B32A32_FLOAT, 0, device);
  if (!result2) {
    return false;
  }

  // Create the normals render texture.
  result2 = BuildRenderTexture(DXGI_FORMAT_R32G32B32A32_FLOAT, 1, device);
  if (!result2) {
    return false;
  }

  // Create the colors render texture.
  result2 = BuildRenderTexture(DXGI_FORMAT_R8G8B8A8_UNORM, 2, device);
  if (!result2) {
    return false;
  }

  // Initialize the description of the depth buffer.
  ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

  // Set up the description of the depth buffer.
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

  // Create the texture for the depth buffer using the filled out description.
  result =
      device->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer);
  if (FAILED(result)) {
    return false;
  }

  // Initailze the depth stencil view description.
  ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

  // Set up the depth stencil view description.
  depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
  depthStencilViewDesc.Texture2D.MipSlice = 0;

  // Create the depth stencil view.
  result = device->CreateDepthStencilView(
      m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView);
  if (FAILED(result)) {
    return false;
  }

  // Setup the viewport for rendering.
  m_viewport.Width = (float)textureWidth;
  m_viewport.Height = (float)textureHeight;
  m_viewport.MinDepth = 0.0f;
  m_viewport.MaxDepth = 1.0f;
  m_viewport.TopLeftX = 0.0f;
  m_viewport.TopLeftY = 0.0f;

  // Setup the projection matrix.
  m_projectionMatrix = XMMatrixPerspectiveFovLH(
      (3.141592654f / 4.0f), ((float)textureWidth / (float)textureHeight),
      screenNear, screenDepth);

  // Create an orthographic projection matrix for 2D rendering.
  m_orthoMatrix = XMMatrixOrthographicLH(
      (float)textureWidth, (float)textureHeight, screenNear, screenDepth);

  return true;
}

void DeferredBuffersClass::Shutdown() {
  int i;

  if (m_depthStencilView) {
    m_depthStencilView->Release();
    m_depthStencilView = 0;
  }

  if (m_depthStencilBuffer) {
    m_depthStencilBuffer->Release();
    m_depthStencilBuffer = 0;
  }

  for (i = 0; i < BUFFER_COUNT; i++) {
    if (m_shaderResourceViewArray[i]) {
      m_shaderResourceViewArray[i]->Release();
      m_shaderResourceViewArray[i] = 0;
    }

    if (m_renderTargetViewArray[i]) {
      m_renderTargetViewArray[i]->Release();
      m_renderTargetViewArray[i] = 0;
    }

    if (m_renderTargetTextureArray[i]) {
      m_renderTargetTextureArray[i]->Release();
      m_renderTargetTextureArray[i] = 0;
    }
  }

  return;
}

void DeferredBuffersClass::SetRenderTargets(
    ID3D11DeviceContext *deviceContext) {
  // Bind the render target view array and depth stencil buffer to the output
  // render pipeline.
  deviceContext->OMSetRenderTargets(BUFFER_COUNT, m_renderTargetViewArray,
                                    m_depthStencilView);

  // Set the viewport.
  deviceContext->RSSetViewports(1, &m_viewport);

  return;
}

void DeferredBuffersClass::ClearRenderTargets(
    ID3D11DeviceContext *deviceContext, float red, float green, float blue,
    float alpha) {
  float color[4];
  int i;

  // Setup the color to clear the buffer to.
  color[0] = red;
  color[1] = green;
  color[2] = blue;
  color[3] = alpha;

  // Clear the render target buffers.
  for (i = 0; i < BUFFER_COUNT; i++) {
    deviceContext->ClearRenderTargetView(m_renderTargetViewArray[i], color);
  }

  // Clear the depth buffer.
  deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH,
                                       1.0f, 0);

  return;
}

bool DeferredBuffersClass::BuildRenderTexture(DXGI_FORMAT rtFormat,
                                              int arraySlot,
                                              ID3D11Device *device) {
  HRESULT result;
  D3D11_TEXTURE2D_DESC textureDesc;
  D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
  D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

  // Initialize the render target texture description.
  ZeroMemory(&textureDesc, sizeof(textureDesc));

  // Setup the render target texture description.
  textureDesc.Width = m_textureWidth;
  textureDesc.Height = m_textureHeight;
  textureDesc.MipLevels = 1;
  textureDesc.ArraySize = 1;
  textureDesc.Format = rtFormat;
  textureDesc.SampleDesc.Count = 1;
  textureDesc.Usage = D3D11_USAGE_DEFAULT;
  textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
  textureDesc.CPUAccessFlags = 0;
  textureDesc.MiscFlags = 0;

  // Create the render target texture.
  result = device->CreateTexture2D(&textureDesc, NULL,
                                   &m_renderTargetTextureArray[arraySlot]);
  if (FAILED(result)) {
    return false;
  }

  // Setup the description of the render target view.
  renderTargetViewDesc.Format = textureDesc.Format;
  renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
  renderTargetViewDesc.Texture2D.MipSlice = 0;

  // Create the render target views.
  result = device->CreateRenderTargetView(m_renderTargetTextureArray[arraySlot],
                                          &renderTargetViewDesc,
                                          &m_renderTargetViewArray[arraySlot]);
  if (FAILED(result)) {
    return false;
  }

  // Setup the description of the shader resource view.
  shaderResourceViewDesc.Format = textureDesc.Format;
  shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
  shaderResourceViewDesc.Texture2D.MipLevels = 1;

  // Create the shader resource views.
  result = device->CreateShaderResourceView(
      m_renderTargetTextureArray[arraySlot], &shaderResourceViewDesc,
      &m_shaderResourceViewArray[arraySlot]);
  if (FAILED(result)) {
    return false;
  }

  return true;
}

void DeferredBuffersClass::GetProjectionMatrix(XMMATRIX &projectionMatrix) {
  projectionMatrix = m_projectionMatrix;
  return;
}

void DeferredBuffersClass::GetOrthoMatrix(XMMATRIX &orthoMatrix) {
  orthoMatrix = m_orthoMatrix;
  return;
}

int DeferredBuffersClass::GetTextureWidth() { return m_textureWidth; }

int DeferredBuffersClass::GetTextureHeight() { return m_textureHeight; }

ID3D11ShaderResourceView *DeferredBuffersClass::GetShaderResourcePositions() {
  return m_shaderResourceViewArray[0];
}

ID3D11ShaderResourceView *DeferredBuffersClass::GetShaderResourceNormals() {
  return m_shaderResourceViewArray[1];
}

ID3D11ShaderResourceView *DeferredBuffersClass::GetShaderResourceColors() {
  return m_shaderResourceViewArray[2];
}