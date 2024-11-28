#include <new>

#include "../CommonFramework/Camera.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/TypeDefine.h"

#include "Graphics.h"
#include "colorshaderclass.h"
#include "modelclass.h"

GraphicsClass::GraphicsClass() {}

GraphicsClass::~GraphicsClass() {}

bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd) {

  auto directx11_device_ = DirectX11Device::GetD3d11DeviceInstance();

  auto result = directx11_device_->Initialize(screenWidth, screenHeight,
                                              VSYNC_ENABLED, hwnd, FULL_SCREEN,
                                              SCREEN_DEPTH, SCREEN_NEAR);
  if (!result) {
    MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
    return false;
  }

  camera_ = (Camera *)_aligned_malloc(sizeof(Camera), 16);
  new (camera_) Camera();
  if (!camera_) {
    return false;
  }

  camera_->SetPosition(0.0f, 0.0f, -10.0f);

  model_ = new ModelClass();
  if (!model_) {
    return false;
  }

  result = model_->Initialize();
  if (!result) {
    MessageBox(hwnd, L"Could not initialize the model object.", L"Error",
               MB_OK);
    return false;
  }

  color_shader_ =
      (ColorShaderClass *)_aligned_malloc(sizeof(ColorShaderClass), 16);
  new (color_shader_) ColorShaderClass();
  if (!color_shader_) {
    return false;
  }

  result = color_shader_->Initialize(hwnd);
  if (!result) {
    MessageBox(hwnd, L"Could not initialize the color shader object.", L"Error",
               MB_OK);
    return false;
  }

  return true;
}

void GraphicsClass::Shutdown() {

  if (color_shader_) {
    color_shader_->Shutdown();
    color_shader_->~ColorShaderClass();
    _aligned_free(color_shader_);
    color_shader_ = nullptr;
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

void GraphicsClass::Frame(float deltatime) { Render(); }

void GraphicsClass::Render() {

  XMMATRIX worldMatrix, viewMatrix, projectionMatrix;

  auto directx_device_ = DirectX11Device::GetD3d11DeviceInstance();

  directx_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

  camera_->Render();

  camera_->GetViewMatrix(viewMatrix);
  directx_device_->GetWorldMatrix(worldMatrix);
  directx_device_->GetProjectionMatrix(projectionMatrix);

  model_->Render();

  color_shader_->Render(model_->GetIndexCount(), worldMatrix, viewMatrix,
                        projectionMatrix);

  directx_device_->EndScene();
}