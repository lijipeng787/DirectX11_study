#include <new>

#include "../CommonFramework/Camera.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/TypeDefine.h"

#include "Graphics.h"
#include "SimpleMovableSurface.h"
#include "TextureShader.h"

GraphicsClass::GraphicsClass() = default;

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

  std::unique_ptr<Camera> tempCamera{ new (std::nothrow) Camera() };
  if (!tempCamera) {
      MessageBox(hwnd, L"Failed to allocate Camera.", L"Error", MB_OK);
      return false;
  }
  tempCamera->SetPosition(0.0f, 0.0f, -10.0f);

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

  camera_ = std::move(tempCamera);

  return true;
}

void GraphicsClass::Shutdown() {

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

  camera_.reset();
}

void GraphicsClass::Frame(float deltaTime) {
    (void)deltaTime; // Placeholder for future per-frame updates.
    Render();
}

void GraphicsClass::Render() {

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
  }

  result =
      texture_shader_->Render(bitmap_->GetIndexCount(), worldMatrix, viewMatrix,
                              orthoMatrix, bitmap_->GetTexture());
  if (!result) {
  }

  directx11_device_->TurnZBufferOn();

  directx11_device_->EndScene();
}
