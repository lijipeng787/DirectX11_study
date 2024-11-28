#include "GraphicsModule.h"

#include <new>

#include "../CommonFramework/Camera.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/TypeDefine.h"

#include "SimpleMovableSurface.h"
#include "TextureShader.h"

GraphicsModule::GraphicsModule() {}

GraphicsModule::~GraphicsModule() {}

bool GraphicsModule::Initialize(int screenWidth, int screenHeight, HWND hwnd) {

  auto directx11_device_ = DirectX11Device::GetD3d11DeviceInstance();

  auto result = directx11_device_->Initialize(screenWidth, screenHeight,
                                              VSYNC_ENABLED, hwnd, FULL_SCREEN,
                                              SCREEN_DEPTH, SCREEN_NEAR);
  if (!result) {
    MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
    return false;
  }

  camera_ = new Camera;
  new (camera_) Camera();
  if (!camera_) {
    return false;
  }
  camera_->SetPosition(0.0f, 0.0f, -10.0f);

  texture_shader_ = new TextureShader;
  new (texture_shader_) TextureShader();
  if (!texture_shader_) {
    return false;
  }

  result = texture_shader_->Initialize(hwnd);
  if (!result) {
    MessageBox(hwnd, L"Could not initialize the texture shader object.",
               L"Error", MB_OK);
    return false;
  }

  bitmap_ = new SimpleMoveableSurface();
  if (!bitmap_) {
    return false;
  }

  bitmap_->InitializeWithSurfaceSize(256, 256);
  result = bitmap_->InitializeVertexAndIndexBuffers();
  if (!result) {
    MessageBox(hwnd, L"Could not initialize the bitmap buffers.", L"Error",
               MB_OK);
    return false;
  }

  result = bitmap_->LoadTextureFromFile(L"data/seafloor.dds");
  if (!result) {
    MessageBox(hwnd, L"Could not initialize the bitmap texture.", L"Error",
               MB_OK);
    return false;
  }

  return true;
}

void GraphicsModule::Shutdown() {

  if (bitmap_) {
    bitmap_->Release();
    delete bitmap_;
    bitmap_ = nullptr;
  }

  if (texture_shader_) {
    texture_shader_->Shutdown();
    delete texture_shader_;
    texture_shader_ = nullptr;
  }

  if (camera_) {
    delete camera_;
    camera_ = nullptr;
  }
}

bool GraphicsModule::Frame() {

  bool result = Render();
  if (!result) {
    return false;
  }

  return true;
}

bool GraphicsModule::Render() {

  auto directx11_device_ = DirectX11Device::GetD3d11DeviceInstance();

  directx11_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

  camera_->Render();

  XMMATRIX worldMatrix, viewMatrix, orthoMatrix;

  camera_->GetViewMatrix(viewMatrix);
  directx11_device_->GetWorldMatrix(worldMatrix);
  directx11_device_->GetOrthoMatrix(orthoMatrix);

  directx11_device_->TurnZBufferOff();

  static float x_step = -0.5, y_step = -0.5;

  bitmap_->SetPosition2D(x_step, y_step);

  ++x_step;
  ++y_step;

  auto result = bitmap_->Render();
  if (!result) {
    return false;
  }

  result =
      texture_shader_->Render(bitmap_->GetIndexCount(), worldMatrix, viewMatrix,
                              orthoMatrix, bitmap_->GetTexture());
  if (!result) {
    return false;
  }

  directx11_device_->TurnZBufferOn();

  directx11_device_->EndScene();

  return true;
}
