#include <memory>
#include <utility>
#include <windows.h>

#include "../CommonFramework/Camera.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/TypeDefine.h"

#include "Graphics.h"
#include "colorshaderclass.h"
#include "modelclass.h"

GraphicsClass::GraphicsClass() = default;

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

  try {
    auto tempCamera = std::make_unique<Camera>();
    tempCamera->SetPosition(0.0f, 0.0f, -10.0f);

    auto tempModel = std::make_unique<ModelClass>();
    if (!tempModel->Initialize()) {
      MessageBox(hwnd, L"Could not initialize the model object.", L"Error",
                 MB_OK);
      return false;
    }

    auto tempShader = std::make_unique<ColorShaderClass>();
    if (!tempShader->Initialize(hwnd)) {
      MessageBox(hwnd, L"Could not initialize the color shader object.",
                 L"Error", MB_OK);
      return false;
    }

    camera_ = std::move(tempCamera);
    model_ = std::move(tempModel);
    color_shader_ = std::move(tempShader);
  } catch (const std::bad_alloc &) {
    MessageBox(hwnd, L"Memory allocation failed.", L"Error", MB_OK);
    return false;
  }

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