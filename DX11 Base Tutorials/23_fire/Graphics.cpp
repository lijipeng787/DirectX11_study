#include "Graphics.h"

#include <new>

#include "../CommonFramework/Camera.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/TypeDefine.h"

#include "fireshaderclass.h"
#include "modelclass.h"

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
    result = model_->Initialize("data/square.txt", L"data/fire01.dds",
                                L"data/noise01.dds", L"data/alpha01.dds");
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the model object.", L"Error",
                 MB_OK);
      return false;
    }
  }

  {
    fire_shader_ =
        (FireShaderClass *)_aligned_malloc(sizeof(FireShaderClass), 16);
    new (fire_shader_) FireShaderClass();
    if (!fire_shader_) {
      return false;
    }
    result = fire_shader_->Initialize(hwnd);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the fire shader object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  return true;
}

void GraphicsClass::Shutdown() {

  if (fire_shader_) {
    fire_shader_->Shutdown();
    fire_shader_->~FireShaderClass();
    fire_shader_ = 0;
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

  camera_->SetPosition(0.0f, 0.0f, -10.0f);

  auto result = Render();
  if (!result) {
    return false;
  }

  return true;
}

bool GraphicsClass::Render() {

  static float frameTime = 0.0f;

  frameTime += 0.01f;
  if (frameTime > 1000.0f) {
    frameTime = 0.0f;
  }

  XMFLOAT3 scrollSpeeds = XMFLOAT3(1.3f, 2.1f, 2.3f);

  XMFLOAT3 scales = XMFLOAT3(1.0f, 2.0f, 3.0f);

  XMFLOAT2 distortion1 = XMFLOAT2(0.1f, 0.2f);
  XMFLOAT2 distortion2 = XMFLOAT2(0.1f, 0.3f);
  XMFLOAT2 distortion3 = XMFLOAT2(0.1f, 0.1f);

  float distortionScale = 0.8f;
  float distortionBias = 0.5f;

  auto directx_device_ = DirectX11Device::GetD3d11DeviceInstance();

  directx_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

  camera_->Render();

  XMMATRIX worldMatrix, viewMatrix, projectionMatrix;

  directx_device_->GetWorldMatrix(worldMatrix);
  camera_->GetViewMatrix(viewMatrix);
  directx_device_->GetProjectionMatrix(projectionMatrix);

  directx_device_->TurnOnAlphaBlending();

  model_->Render();

  auto result = fire_shader_->Render(
      model_->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
      model_->GetTexture1(), model_->GetTexture2(), model_->GetTexture3(),
      frameTime, scrollSpeeds, scales, distortion1, distortion2, distortion3,
      distortionScale, distortionBias);
  if (!result) {
    return false;
  }

  directx_device_->TurnOffAlphaBlending();

  directx_device_->EndScene();

  return true;
}