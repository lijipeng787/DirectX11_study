#include "Graphics.h"

#include <new>

#include "../CommonFramework/Camera.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/TypeDefine.h"

#include "alphamapshaderclass.h"
#include "modelclass.h"

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

    result = model_->Initialize("data/square.txt", L"data/stone01.dds",
                                L"data/dirt01.dds", L"data/alpha01.dds");
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the model object.", L"Error",
                 MB_OK);
      return false;
    }
  }

  {
    alphamap_shader_ =
        (AlphaMapShaderClass *)_aligned_malloc(sizeof(AlphaMapShaderClass), 16);
    new (alphamap_shader_) AlphaMapShaderClass();
    if (!alphamap_shader_) {
      return false;
    }

    result = alphamap_shader_->Initialize(hwnd);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the alpha map shader object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  return true;
}

void GraphicsClass::Shutdown() {

  if (alphamap_shader_) {
    alphamap_shader_->Shutdown();
    alphamap_shader_->~AlphaMapShaderClass();
    _aligned_free(alphamap_shader_);
    alphamap_shader_ = 0;
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

void GraphicsClass::Frame(float deltaTime) { Render(); }

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

  alphamap_shader_->Render(model_->GetIndexCount(), worldMatrix, viewMatrix,
                           projectionMatrix, model_->GetTextureArray());

  directx_device->EndScene();
}