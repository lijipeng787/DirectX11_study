#include "Graphics.h"

#include <new>

#include "../CommonFramework/Camera.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/TypeDefine.h"

#include "modelclass.h"
#include "textureshaderclass.h"

using namespace DirectX;

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
    floor_model_ = new ModelClass();
    if (!floor_model_) {
      return false;
    }
    result = floor_model_->Initialize("data/floor.txt", L"data/grid01.dds");
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the floor model object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  {
    billboard_model_ = new ModelClass();
    if (!billboard_model_) {
      return false;
    }
    result =
        billboard_model_->Initialize("data/square.txt", L"data/seafloor.dds");
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the billboard model object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  return true;
}

void GraphicsClass::Shutdown() {

  if (billboard_model_) {
    billboard_model_->Shutdown();
    delete billboard_model_;
    billboard_model_ = 0;
  }

  if (floor_model_) {
    floor_model_->Shutdown();
    delete floor_model_;
    floor_model_ = 0;
  }

  if (texture_shader_) {
    texture_shader_->Shutdown();
    texture_shader_->~TextureShaderClass();
    _aligned_free(texture_shader_);
    texture_shader_ = 0;
  }

  if (camera_) {
    camera_->~Camera();
    _aligned_free(camera_);
    camera_ = nullptr;
  }
}

bool GraphicsClass::Frame() {

  bool result;

  // Update the position of the camera.
  camera_->SetPosition(x_, y_, z_);

  // Render the graphics scene.
  result = Render();
  if (!result) {
    return false;
  }

  return true;
}

bool GraphicsClass::Render() {

  auto directx_device = DirectX11Device::GetD3d11DeviceInstance();

  directx_device->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

  camera_->Render();

  XMMATRIX worldMatrix, viewMatrix, projectionMatrix, translateMatrix;

  camera_->GetViewMatrix(viewMatrix);
  directx_device->GetWorldMatrix(worldMatrix);
  directx_device->GetProjectionMatrix(projectionMatrix);

  floor_model_->Render();

  auto result = texture_shader_->Render(
      floor_model_->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
      floor_model_->GetTexture());
  if (!result) {
    return false;
  }

  XMFLOAT3 cameraPosition = camera_->GetPosition();

  XMFLOAT3 modelPosition;
  modelPosition.x = 0.0f;
  modelPosition.y = 1.5f;
  modelPosition.z = 0.0f;

  float angle = atan2(modelPosition.x - cameraPosition.x,
                      modelPosition.z - cameraPosition.z) *
                (180.0 / XM_PI);

  float rotation_ = (float)angle * 0.0174532925f;

  worldMatrix = XMMatrixRotationY(rotation_);

  translateMatrix =
      XMMatrixTranslation(modelPosition.x, modelPosition.y, modelPosition.z);

  worldMatrix = XMMatrixMultiply(worldMatrix, translateMatrix);

  billboard_model_->Render();

  result = texture_shader_->Render(billboard_model_->GetIndexCount(),
                                   worldMatrix, viewMatrix, projectionMatrix,
                                   billboard_model_->GetTexture());
  if (!result) {
    return false;
  }

  directx_device->EndScene();

  return true;
}

void GraphicsClass::SetPosition(float x, float y, float z) {
  x_ = x;
  y_ = y;
  z_ = z;
}
