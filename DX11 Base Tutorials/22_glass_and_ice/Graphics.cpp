#include "Graphics.h"

#include <new>

#include "../CommonFramework/Camera.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/TypeDefine.h"

#include "glassshaderclass.h"
#include "modelclass.h"
#include "rendertextureclass.h"
#include "textureshaderclass.h"

using namespace DirectX;

bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd) {

  bool result;
  XMMATRIX baseViewMatrix;

  {
    auto directx11_device_ = DirectX11Device::GetD3d11DeviceInstance();

    auto result = directx11_device_->Initialize(
        screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN,
        SCREEN_DEPTH, SCREEN_NEAR);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
      return false;
    }
  }

  {
    camera_ = (Camera *)_aligned_malloc(sizeof(Camera), 16);
    new (camera_) Camera();
    if (!camera_) {
      return false;
    }
    camera_->SetPosition(0.0f, 0.0f, -10.0f);
    camera_->Render();
    camera_->GetViewMatrix(baseViewMatrix);
  }

  {
    model_ = new ModelClass();
    if (!model_) {
      return false;
    }
    result = model_->Initialize("data/cube.txt", L"data/seafloor.dds",
                                L"data/bump03.dds");
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the model object.", L"Error",
                 MB_OK);
      return false;
    }

    window_model_ = new ModelClass();
    if (!window_model_) {
      return false;
    }
    result = window_model_->Initialize("data/square.txt", L"data/glass01.dds",
                                       L"data/bump03.dds");
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the window model object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  {
    render_texture_ = new RenderTextureClass();
    if (!render_texture_) {
      return false;
    }
    result = render_texture_->Initialize(screenWidth, screenHeight);
    if (!result) {
      return false;
    }
  }

  {
    texture_shader_ =
        (TextureShaderClass *)_aligned_malloc(sizeof(TextureShaderClass), 16);
    new (texture_shader_) TextureShaderClass();
    if (!texture_shader_) {
      return false;
    }
    result = texture_shader_->Initialize(hwnd);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the texture shader object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  {
    glass_shader_ =
        (GlassShaderClass *)_aligned_malloc(sizeof(GlassShaderClass), 16);
    new (glass_shader_) GlassShaderClass();
    if (!glass_shader_) {
      return false;
    }
    result = glass_shader_->Initialize(hwnd);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the glass shader object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  return true;
}

void GraphicsClass::Shutdown() {

  if (glass_shader_) {
    glass_shader_->Shutdown();
    glass_shader_->~GlassShaderClass();
    _aligned_free(glass_shader_);
    glass_shader_ = 0;
  }

  if (texture_shader_) {
    texture_shader_->Shutdown();
    texture_shader_->~TextureShaderClass();
    _aligned_free(texture_shader_);
    texture_shader_ = 0;
  }

  if (render_texture_) {
    render_texture_->Shutdown();
    delete render_texture_;
    render_texture_ = 0;
  }

  if (window_model_) {
    window_model_->Shutdown();
    delete window_model_;
    window_model_ = 0;
  }

  if (model_) {
    model_->Shutdown();
    delete model_;
    model_ = nullptr;
  }

  if (camera_) {
    camera_->~Camera();
    _aligned_free(camera_);
    camera_ = nullptr;
  }
}

bool GraphicsClass::Frame() {

  static float rotation_ = 0.0f;

  // Update the rotation_ variable each frame.
  rotation_ += (float)XM_PI * 0.005f;
  if (rotation_ > 360.0f) {
    rotation_ -= 360.0f;
  }
  rotation_ = rotation_;

  camera_->SetPosition(0.0f, 0.0f, -10.0f);

  // Render the scene to texture first.
  auto result = RenderToTexture(rotation_);
  if (!result) {
    return false;
  }

  // Render the scene.
  result = Render();
  if (!result) {
    return false;
  }

  return true;
}

bool GraphicsClass::RenderToTexture(float rotation_) {

  auto directx_device = DirectX11Device::GetD3d11DeviceInstance();

  render_texture_->SetRenderTarget(directx_device->GetDepthStencilView());

  render_texture_->ClearRenderTarget(directx_device->GetDepthStencilView(),
                                     0.0f, 0.0f, 0.0f, 1.0f);

  camera_->Render();

  XMMATRIX worldMatrix, viewMatrix, projectionMatrix;

  directx_device->GetWorldMatrix(worldMatrix);
  camera_->GetViewMatrix(viewMatrix);
  directx_device->GetProjectionMatrix(projectionMatrix);

  worldMatrix = XMMatrixRotationY(rotation_);

  model_->Render();

  auto result =
      texture_shader_->Render(model_->GetIndexCount(), worldMatrix, viewMatrix,
                              projectionMatrix, model_->GetTexture());
  if (!result) {
    return false;
  }

  directx_device->SetBackBufferRenderTarget();

  return true;
}

bool GraphicsClass::Render() {

  float refractionScale = 0.01f;

  auto directx_device_ = DirectX11Device::GetD3d11DeviceInstance();

  directx_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

  camera_->Render();

  XMMATRIX worldMatrix, viewMatrix, projectionMatrix;

  directx_device_->GetWorldMatrix(worldMatrix);
  camera_->GetViewMatrix(viewMatrix);
  directx_device_->GetProjectionMatrix(projectionMatrix);

  worldMatrix = XMMatrixRotationY(rotation_);

  model_->Render();

  auto result =
      texture_shader_->Render(model_->GetIndexCount(), worldMatrix, viewMatrix,
                              projectionMatrix, model_->GetTexture());
  if (!result) {
    return false;
  }

  directx_device_->GetWorldMatrix(worldMatrix);

  worldMatrix = XMMatrixTranslation(0.0f, 0.0f, -1.5f);

  window_model_->Render();

  result = glass_shader_->Render(
      window_model_->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
      window_model_->GetTexture(), window_model_->GetNormalMap(),
      render_texture_->GetShaderResourceView(), refractionScale);
  if (!result) {
    return false;
  }

  directx_device_->EndScene();

  return true;
}