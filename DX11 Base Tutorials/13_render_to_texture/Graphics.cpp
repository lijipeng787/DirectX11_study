#include "Graphics.h"

#include <new>

#include "../CommonFramework/Camera.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/TypeDefine.h"

#include "debugwindowclass.h"
#include "lightclass.h"
#include "lightshaderclass.h"
#include "modelclass.h"
#include "rendertextureclass.h"
#include "textureshaderclass.h"

GraphicsClass::GraphicsClass() {}

GraphicsClass::~GraphicsClass() {}

using namespace DirectX;

bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd) {

  bool result;
  XMMATRIX baseViewMatrix;

  {
    auto directx11_device_ = DirectX11Device::GetD3d11DeviceInstance();

    result = directx11_device_->Initialize(screenWidth, screenHeight,
                                           VSYNC_ENABLED, hwnd, FULL_SCREEN,
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
    camera_->SetPosition(0.0f, 0.0f, -1.0f);
    camera_->Render();
    camera_->GetViewMatrix(baseViewMatrix);
  }

  {
    model_ = new ModelClass();
    if (!model_) {
      return false;
    }

    result = model_->Initialize(L"data/seafloor.dds", "data/cube.txt");
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the model object.", L"Error",
                 MB_OK);
      return false;
    }
  }

  {
    light_shader_ =
        (LightShaderClass *)_aligned_malloc(sizeof(LightShaderClass), 16);
    new (light_shader_) LightShaderClass();
    if (!light_shader_) {
      return false;
    }

    result = light_shader_->Initialize(hwnd);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the light shader object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  {
    light_ = new LightClass();
    if (!light_) {
      return false;
    }

    light_->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
    light_->SetDirection(0.0f, 0.0f, 1.0f);
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
    debug_window_ = new DebugWindowClass();
    if (!debug_window_) {
      return false;
    }

    result = debug_window_->Initialize(screenWidth, screenHeight, 100, 100);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the debug window object.",
                 L"Error", MB_OK);
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

  return true;
}

void GraphicsClass::Shutdown() {

  if (texture_shader_) {
    texture_shader_->Shutdown();
    texture_shader_->~TextureShaderClass();
    _aligned_free(texture_shader_);
    texture_shader_ = 0;
  }

  if (debug_window_) {
    debug_window_->Shutdown();
    delete debug_window_;
    debug_window_ = 0;
  }

  if (render_texture_) {
    render_texture_->Shutdown();
    delete render_texture_;
    render_texture_ = 0;
  }

  if (light_) {
    delete light_;
    light_ = nullptr;
    ;
  }

  if (light_shader_) {
    light_shader_->Shutdown();
    light_shader_->~LightShaderClass();
    _aligned_free(light_shader_);
    light_shader_ = nullptr;
    ;
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

  bool result;

  result = Render();
  if (!result) {
    return false;
  }

  return true;
}

bool GraphicsClass::Render() {

  XMMATRIX worldMatrix, viewMatrix, orthoMatrix;
  bool result;

  camera_->SetPosition(0.0f, 0.0f, -5.0f);

  result = RenderToTexture();
  if (!result) {
    return false;
  }

  auto directx_device_ = DirectX11Device::GetD3d11DeviceInstance();

  directx_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

  result = RenderScene();
  if (!result) {
    return false;
  }

  directx_device_->TurnZBufferOff();

  directx_device_->GetWorldMatrix(worldMatrix);
  camera_->GetViewMatrix(viewMatrix);
  directx_device_->GetOrthoMatrix(orthoMatrix);

  result = debug_window_->Render(50, 50);
  if (!result) {
    return false;
  }

  result = texture_shader_->Render(debug_window_->GetIndexCount(), worldMatrix,
                                   viewMatrix, orthoMatrix,
                                   render_texture_->GetShaderResourceView());
  if (!result) {
    return false;
  }

  directx_device_->TurnZBufferOn();

  directx_device_->EndScene();

  return true;
}

bool GraphicsClass::RenderToTexture() {

  auto directx_device_ = DirectX11Device::GetD3d11DeviceInstance();

  render_texture_->SetRenderTarget(directx_device_->GetDepthStencilView());

  render_texture_->ClearRenderTarget(directx_device_->GetDepthStencilView(),
                                     0.0f, 0.0f, 1.0f, 1.0f);

  auto result = RenderScene();
  if (!result) {
    return false;
  }

  directx_device_->SetBackBufferRenderTarget();

  return true;
}

bool GraphicsClass::RenderScene() {

  XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
  bool result;
  static float rotation_ = 0.0f;

  camera_->Render();

  auto directx_device_ = DirectX11Device::GetD3d11DeviceInstance();

  directx_device_->GetWorldMatrix(worldMatrix);
  camera_->GetViewMatrix(viewMatrix);
  directx_device_->GetProjectionMatrix(projectionMatrix);

  rotation_ += (float)XM_PI * 0.005f;
  if (rotation_ > 360.0f) {
    rotation_ -= 360.0f;
  }

  worldMatrix = XMMatrixRotationY(rotation_);

  model_->Render();
  result = light_shader_->Render(
      model_->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
      model_->GetTexture(), light_->GetDirection(), light_->GetDiffuseColor());
  if (!result) {
    return false;
  }

  return true;
}