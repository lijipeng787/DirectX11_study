#include "Graphics.h"

#include <new>

#include "../CommonFramework/Camera.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/TypeDefine.h"

#include "clipplaneshaderclass.h"
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

    result = model_->Initialize(L"data/seafloor.dds", "data/triangle.txt");
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the model object.", L"Error",
                 MB_OK);
      return false;
    }
  }

  {
    clipplane_shader_ = (ClipPlaneShaderClass *)_aligned_malloc(
        sizeof(ClipPlaneShaderClass), 16);
    new (clipplane_shader_) ClipPlaneShaderClass();
    if (!clipplane_shader_) {
      return false;
    }

    result = clipplane_shader_->Initialize(hwnd);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the clip plane shader object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  return true;
}

void GraphicsClass::Shutdown() {

  if (clipplane_shader_) {
    clipplane_shader_->Shutdown();
    clipplane_shader_->~ClipPlaneShaderClass();
    _aligned_free(clipplane_shader_);
    clipplane_shader_ = 0;
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

  XMFLOAT4 clipPlane;
  XMMATRIX worldMatrix, viewMatrix, projectionMatrix;

  camera_->SetPosition(0.0f, 0.0f, -10.0f);

  clipPlane = XMFLOAT4(0.0f, -1.0f, 0.0f, 0.0f);

  auto directx_device = DirectX11Device::GetD3d11DeviceInstance();

  directx_device->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

  camera_->Render();

  directx_device->GetWorldMatrix(worldMatrix);
  camera_->GetViewMatrix(viewMatrix);
  directx_device->GetProjectionMatrix(projectionMatrix);

  model_->Render();

  auto result = clipplane_shader_->Render(model_->GetIndexCount(), worldMatrix,
                                          viewMatrix, projectionMatrix,
                                          model_->GetTexture(), clipPlane);
  if (!result) {
    return false;
  }

  directx_device->EndScene();

  return true;
}