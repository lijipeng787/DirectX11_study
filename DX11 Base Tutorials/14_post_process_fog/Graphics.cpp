#include "Graphics.h"

#include <new>

#include "../CommonFramework/Camera.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/TypeDefine.h"

#include "fogshaderclass.h"
#include "modelclass.h"

using namespace DirectX;

GraphicsClass::GraphicsClass() {}

GraphicsClass::~GraphicsClass() {}

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
    fog_shader_ = (FogShaderClass *)_aligned_malloc(sizeof(FogShaderClass), 16);
    new (fog_shader_) FogShaderClass();
    if (!fog_shader_) {
      return false;
    }

    result = fog_shader_->Initialize(hwnd);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the fog shader object.", L"Error",
                 MB_OK);
      return false;
    }
  }

  return true;
}

void GraphicsClass::Shutdown() {

  if (fog_shader_) {
    fog_shader_->Shutdown();
    fog_shader_->~FogShaderClass();
    _aligned_free(fog_shader_);
    fog_shader_ = 0;
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

void GraphicsClass::Frame(float deltaTime) {

  Render();
}

void GraphicsClass::Render() {

  float fogColor, fogStart, fogEnd;
  XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
  static float rotation_ = 0.0f;

  camera_->SetPosition(0.0f, 0.0f, -10.0f);

  fogColor = 0.5f;

  fogStart = 0.0f;
  fogEnd = 10.0f;

  auto directx_device = DirectX11Device::GetD3d11DeviceInstance();

  directx_device->BeginScene(fogColor, fogColor, fogColor, 1.0f);

  camera_->Render();
  directx_device->GetWorldMatrix(worldMatrix);
  camera_->GetViewMatrix(viewMatrix);
  directx_device->GetProjectionMatrix(projectionMatrix);
  rotation_ += (float)XM_PI * 0.005f;
  if (rotation_ > 360.0f) {
    rotation_ -= 360.0f;
  }

  worldMatrix = XMMatrixRotationY(rotation_);

  model_->Render();

  auto result = fog_shader_->Render(model_->GetIndexCount(), worldMatrix,
                                    viewMatrix, projectionMatrix,
                                    model_->GetTexture(), fogStart, fogEnd);

  directx_device->EndScene();
}