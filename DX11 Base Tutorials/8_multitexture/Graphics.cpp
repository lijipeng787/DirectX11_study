#include "Graphics.h"

#include <new>

#include "../CommonFramework/Camera.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/TypeDefine.h"

#include "modelclass.h"
#include "multitextureshaderclass.h"

using namespace DirectX;

bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd) {

  bool result;

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
  }

  {
    model_ = new ModelClass();
    if (!model_) {
      return false;
    }
    result = model_->Initialize("data/square.txt", L"data/stone01.dds",
                                L"data/dirt01.dds");

    if (!result) {
      MessageBox(hwnd, L"Could not initialize the model object.", L"Error",
                 MB_OK);
      return false;
    }
  }

  {
    multitexture_shader_ = (MultiTextureShaderClass *)_aligned_malloc(
        sizeof(MultiTextureShaderClass), 16);
    new (multitexture_shader_) MultiTextureShaderClass();
    if (!multitexture_shader_) {
      return false;
    }

    result = multitexture_shader_->Initialize(hwnd);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the multitexture shader object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  return true;
}

void GraphicsClass::Shutdown() {

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

  XMMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix;

  camera_->SetPosition(0.0f, 0.0f, -5.0f);

  auto directx_device = DirectX11Device::GetD3d11DeviceInstance();

  directx_device->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

  camera_->Render();

  directx_device->GetWorldMatrix(worldMatrix);
  camera_->GetViewMatrix(viewMatrix);
  directx_device->GetProjectionMatrix(projectionMatrix);
  directx_device->GetOrthoMatrix(orthoMatrix);

  model_->Render();

  multitexture_shader_->Render(model_->GetIndexCount(), worldMatrix, viewMatrix,
                               projectionMatrix, model_->GetTextureArray());

  directx_device->EndScene();
}
