#include <memory>
#include <new>
#include <utility>
#include <windows.h>

#include "../CommonFramework/Camera.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/TypeDefine.h"

#include "Graphics.h"
#include "colorshaderclass.h"
#include "modelclass.h"

GraphicsClass::GraphicsClass() = default;

GraphicsClass::~GraphicsClass() { Shutdown(); }

bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd) {

  directx_device_ = DirectX11Device::GetD3d11DeviceInstance();
  if (!directx_device_) {
    MessageBox(hwnd, L"DirectX device instance unavailable.", L"Error", MB_OK);
    return false;
  }

  if (!directx_device_->Initialize(screenWidth, screenHeight, VSYNC_ENABLED,
                                   hwnd, FULL_SCREEN, SCREEN_DEPTH,
                                   SCREEN_NEAR)) {
    MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
    return false;
  }

  // Stage allocations in temporaries for strong exception / failure safety.
  std::unique_ptr<Camera> tempCamera{new (std::nothrow) Camera()};
  if (!tempCamera) {
    MessageBox(hwnd, L"Failed to allocate Camera.", L"Error", MB_OK);
    return false;
  }
  tempCamera->SetPosition(0.0f, 0.0f, -10.0f);

  std::unique_ptr<ModelClass> tempModel{new (std::nothrow) ModelClass()};
  if (!tempModel) {
    MessageBox(hwnd, L"Failed to allocate ModelClass.", L"Error", MB_OK);
    return false;
  }
  if (!tempModel->Initialize()) {
    MessageBox(hwnd, L"Could not initialize the model object.", L"Error",
               MB_OK);
    return false;
  }

  std::unique_ptr<ColorShaderClass> tempShader{new (std::nothrow)
                                                   ColorShaderClass()};
  if (!tempShader) {
    MessageBox(hwnd, L"Failed to allocate ColorShaderClass.", L"Error", MB_OK);
    return false;
  }
  if (!tempShader->Initialize(hwnd)) {
    MessageBox(hwnd, L"Could not initialize the color shader object.", L"Error",
               MB_OK);
    return false;
  }

  // Commit after success.
  camera_ = std::move(tempCamera);
  model_ = std::move(tempModel);
  color_shader_ = std::move(tempShader);

  return true;
}

void GraphicsClass::Shutdown() {
  if (color_shader_) {
    color_shader_->Shutdown();
    color_shader_.reset();
  }

  if (model_) {
    model_->Shutdown();
    model_.reset();
  }

  camera_.reset();
}

void GraphicsClass::Frame(float deltaTime) {
  (void)deltaTime; // Placeholder for future per-frame updates.
  Render();
}

void GraphicsClass::Render() {
  if (!directx_device_ || !camera_ || !model_ || !color_shader_) {
    return;
  }

  DirectX::XMMATRIX worldMatrix, viewMatrix, projectionMatrix;

  directx_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

  camera_->Render();
  camera_->GetViewMatrix(viewMatrix);
  directx_device_->GetWorldMatrix(worldMatrix);
  directx_device_->GetProjectionMatrix(projectionMatrix);

  model_->Render();

  // If shader render fails, we can log; for now we proceed silently.
  color_shader_->Render(model_->GetIndexCount(), worldMatrix, viewMatrix,
                        projectionMatrix);

  directx_device_->EndScene();
}