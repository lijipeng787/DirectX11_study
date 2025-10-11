#include "Graphics.h"

#include <new>

#include "../CommonFramework/Camera.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/TypeDefine.h"

#include "textclass.h"

using namespace DirectX;

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

  XMMATRIX baseViewMatrix = {};
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
    text_ = (TextClass *)_aligned_malloc(sizeof(TextClass), 16);
    new (text_) TextClass();
    if (!text_) {
      return false;
    }

    result = text_->Initialize(hwnd, screenWidth, screenHeight, baseViewMatrix);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the text object.", L"Error",
                 MB_OK);
      return false;
    }
  }

  return true;
}

void GraphicsClass::Shutdown() {

  if (camera_) {
    camera_->~Camera();
    _aligned_free(camera_);
    camera_ = nullptr;
  }
}

void GraphicsClass::Frame(float deltaTime) { Render(); }

void GraphicsClass::Render() {

  XMMATRIX worldMatrix, viewMatrix, orthoMatrix;

  auto directx_device = DirectX11Device::GetD3d11DeviceInstance();

  directx_device->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

  camera_->Render();

  camera_->GetViewMatrix(viewMatrix);
  directx_device->GetWorldMatrix(worldMatrix);
  directx_device->GetOrthoMatrix(orthoMatrix);

  directx_device->TurnZBufferOff();

  directx_device->TurnOnAlphaBlending();

  auto result = text_->Render(worldMatrix, orthoMatrix);

  directx_device->TurnOffAlphaBlending();

  directx_device->TurnZBufferOn();

  directx_device->EndScene();
}
