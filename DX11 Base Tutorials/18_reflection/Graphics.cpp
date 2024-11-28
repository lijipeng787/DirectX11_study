#include "Graphics.h"

#include <new>

#include "../CommonFramework/Camera.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/TypeDefine.h"

#include "modelclass.h"
#include "reflectionshaderclass.h"
#include "rendertextureclass.h"
#include "textureshaderclass.h"

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
    camera_->SetPosition(0.0f, 0.0f, -10.0f);
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
    texture_shader_ =
        (TextureShaderClass *)_aligned_malloc(sizeof(TextureShaderClass), 16);
    new (texture_shader_) TextureShaderClass();
    if (!texture_shader_) {
      return false;
    }

    result = texture_shader_->Initialize(hwnd);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the texture shader object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  {
    render_texture_ = new RenderTextureClass();
    if (!render_texture_) {
      return false;
    }
    result = render_texture_->Initialize(screenWidth, screenHeight);
    if (!result) {
      return false;
    }
  }

  {
    floor_model_ = new ModelClass();
    if (!floor_model_) {
      return false;
    }
    result = floor_model_->Initialize(L"data/blue01.dds", "data/floor.txt");
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the floor model object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  {
    reflection_model_ = (ReflectionShaderClass *)_aligned_malloc(
        sizeof(ReflectionShaderClass), 16);
    new (reflection_model_) ReflectionShaderClass();
    if (!reflection_model_) {
      return false;
    }
    result = reflection_model_->Initialize(hwnd);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the reflection shader object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  return true;
}

void GraphicsClass::Shutdown() {

  if (reflection_model_) {
    reflection_model_->Shutdown();
    reflection_model_->~ReflectionShaderClass();
    _aligned_free(reflection_model_);
    reflection_model_ = 0;
  }

  if (floor_model_) {
    floor_model_->Shutdown();
    delete floor_model_;
    floor_model_ = 0;
  }

  if (render_texture_) {
    render_texture_->Shutdown();
    delete render_texture_;
    render_texture_ = 0;
  }

  if (texture_shader_) {
    texture_shader_->Shutdown();
    texture_shader_->~TextureShaderClass();
    _aligned_free(texture_shader_);
    texture_shader_ = 0;
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
  bool result;

  result = RenderToTexture();
  if (!result) {
    return false;
  }

  result = RenderScene();
  if (!result) {
    return false;
  }

  return true;
}

bool GraphicsClass::RenderToTexture() {

  XMMATRIX worldMatrix, reflectionViewMatrix, projectionMatrix;
  static float rotation_ = 0.0f;

  auto directx_device_ = DirectX11Device::GetD3d11DeviceInstance();

  render_texture_->SetRenderTarget(directx_device_->GetDepthStencilView());

  render_texture_->ClearRenderTarget(directx_device_->GetDepthStencilView(),
                                     0.0f, 0.0f, 0.0f, 1.0f);

  camera_->RenderReflection(-1.5f);

  reflectionViewMatrix = camera_->GetReflectionViewMatrix();

  directx_device_->GetWorldMatrix(worldMatrix);
  directx_device_->GetProjectionMatrix(projectionMatrix);

  rotation_ += (float)XM_PI * 0.005f;
  if (rotation_ > 360.0f) {
    rotation_ -= 360.0f;
  }
  worldMatrix = XMMatrixRotationY(rotation_);

  model_->Render();

  texture_shader_->Render(model_->GetIndexCount(), worldMatrix,
                          reflectionViewMatrix, projectionMatrix,
                          model_->GetTexture());

  directx_device_->SetBackBufferRenderTarget();

  return true;
}

bool GraphicsClass::RenderScene() {

  static float rotation_ = 0.0f;

  auto directx_device_ = DirectX11Device::GetD3d11DeviceInstance();

  directx_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

  camera_->Render();

  XMMATRIX worldMatrix, viewMatrix, projectionMatrix, reflectionMatrix;

  directx_device_->GetWorldMatrix(worldMatrix);
  camera_->GetViewMatrix(viewMatrix);
  directx_device_->GetProjectionMatrix(projectionMatrix);

  rotation_ += (float)XM_PI * 0.005f;
  if (rotation_ > 360.0f) {
    rotation_ -= 360.0f;
  }

  worldMatrix = XMMatrixRotationY(rotation_);

  model_->Render();

  auto result =
      texture_shader_->Render(model_->GetIndexCount(), worldMatrix, viewMatrix,
                              projectionMatrix, model_->GetTexture());
  if (!result) {
    return false;
  }

  directx_device_->GetWorldMatrix(worldMatrix);
  worldMatrix = XMMatrixTranslation(0.0f, -1.5f, 0.0f);

  reflectionMatrix = camera_->GetReflectionViewMatrix();

  floor_model_->Render();

  result = reflection_model_->Render(
      floor_model_->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
      floor_model_->GetTexture(), render_texture_->GetShaderResourceView(),
      reflectionMatrix);

  directx_device_->EndScene();

  return true;
}