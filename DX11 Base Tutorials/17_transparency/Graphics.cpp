#include "Graphics.h"

#include <new>

#include "../CommonFramework/Camera.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/TypeDefine.h"

#include "modelclass.h"
#include "textureshaderclass.h"
#include "transparentshaderclass.h"

using namespace DirectX;

GraphicsClass::GraphicsClass() {}

GraphicsClass::~GraphicsClass() {}

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
    camera_->SetPosition(0.0f, 0.0f, -10.0f);
    camera_->Render();
    camera_->GetViewMatrix(baseViewMatrix);
  }

  {
    model_1_ = new ModelClass();
    if (!model_1_) {
      return false;
    }

    result = model_1_->Initialize(L"data/dirt01.dds", "data/square.txt");
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the first model object.",
                 L"Error", MB_OK);
      return false;
    }

    model_2_ = new ModelClass();
    if (!model_2_) {
      return false;
    }

    result = model_2_->Initialize(L"data/stone01.dds", "data/square.txt");
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the second model object.",
                 L"Error", MB_OK);
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
    transparent_shader_ = (TransparentShaderClass *)_aligned_malloc(
        sizeof(TransparentShaderClass), 16);
    new (transparent_shader_) TransparentShaderClass();
    if (!transparent_shader_) {
      return false;
    }

    result = transparent_shader_->Initialize(hwnd);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the transparent shader object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  return true;
}

void GraphicsClass::Shutdown() {

  if (transparent_shader_) {
    transparent_shader_->Shutdown();
    transparent_shader_->~TransparentShaderClass();
    _aligned_free(transparent_shader_);
    transparent_shader_ = 0;
  }

  if (texture_shader_) {
    texture_shader_->Shutdown();
    texture_shader_->~TextureShaderClass();
    _aligned_free(texture_shader_);
    texture_shader_ = 0;
  }

  if (model_2_) {
    model_2_->Shutdown();
    delete model_2_;
    model_2_ = 0;
  }

  if (model_1_) {
    model_1_->Shutdown();
    delete model_1_;
    model_1_ = 0;
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

  XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
  bool result;
  float blendAmount;

  blendAmount = 0.5f;

  auto directx_device_ = DirectX11Device::GetD3d11DeviceInstance();

  directx_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

  camera_->Render();

  directx_device_->GetWorldMatrix(worldMatrix);
  camera_->GetViewMatrix(viewMatrix);
  directx_device_->GetProjectionMatrix(projectionMatrix);

  model_1_->Render();

  result = texture_shader_->Render(model_1_->GetIndexCount(), worldMatrix,
                                   viewMatrix, projectionMatrix,
                                   model_1_->GetTexture());

  worldMatrix = XMMatrixTranslation(1.0f, 0.0f, -1.0f);

  directx_device_->TurnOnAlphaBlending();

  model_2_->Render();

  result = transparent_shader_->Render(model_2_->GetIndexCount(), worldMatrix,
                                       viewMatrix, projectionMatrix,
                                       model_2_->GetTexture(), blendAmount);

  directx_device_->TurnOffAlphaBlending();

  directx_device_->EndScene();
}