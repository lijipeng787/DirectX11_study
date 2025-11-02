#include "Graphics.h"

#include <new>

#include "../CommonFramework/Camera.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/TypeDefine.h"

#include "lightclass.h"
#include "lightshaderclass.h"
#include "modelclass.h"

using namespace DirectX;

float GraphicsClass::rotation_ = 0.0f;

bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd) {

  bool result;

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
    camera_->SetPosition(0.0f, 0.0f, -10.0f);
  }

  {
    model_ = new ModelClass();
    if (!model_) {
      return false;
    }
    result = model_->Initialize("data/cube.txt", L"data/seafloor.dds");
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
    light_->SetAmbientColor(0.15f, 0.15f, 0.15f, 1.0f);
    light_->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
    light_->SetDirection(0.0f, 0.0f, 1.0f);
  }

  return true;
}

void GraphicsClass::Shutdown() {

  if (light_) {
    delete light_;
    light_ = nullptr;
  }

  if (light_shader_) {
    light_shader_->Shutdown();
    light_shader_->~LightShaderClass();
    _aligned_free(light_shader_);
    light_shader_ = nullptr;
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

void GraphicsClass::Frame(float f) {

  rotation_ += XM_PI / 2.0f * f;
  
  const float TWO_PI = 2.0f * XM_PI;
  if (rotation_ > TWO_PI) {
    rotation_ -= TWO_PI;
  }

  Render();
}

void GraphicsClass::Render() {

  XMMATRIX worldMatrix, viewMatrix, projectionMatrix;

  auto directx_device = DirectX11Device::GetD3d11DeviceInstance();

  directx_device->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

  camera_->Render();

  camera_->GetViewMatrix(viewMatrix);
  directx_device->GetWorldMatrix(worldMatrix);
  directx_device->GetProjectionMatrix(projectionMatrix);

  worldMatrix = XMMatrixRotationY(rotation_);

  model_->Render();

  auto result = light_shader_->Render(
      model_->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
      model_->GetTexture(), light_->GetDirection(), light_->GetAmbientColor(),
      light_->GetDiffuseColor());

  if (!result) {
    return;
  }

  directx_device->EndScene();
}