#include "Graphics.h"

#include <new>

#include "../CommonFramework/Camera.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/TypeDefine.h"

#include "particleshaderclass.h"
#include "particlesystemclass.h"

using namespace DirectX;

bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd) {

  bool result;

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
    camera_->SetPosition(0.0f, -1.0f, -10.0f);
    camera_->Render();
  }

  particle_shader_ =
      (ParticleShaderClass *)_aligned_malloc(sizeof(ParticleShaderClass), 16);
  new (particle_shader_) ParticleShaderClass();
  if (!particle_shader_) {
    return false;
  }

  result = particle_shader_->Initialize(hwnd);
  if (!result) {
    MessageBox(hwnd, L"Could not initialize the particle shader object.",
               L"Error", MB_OK);
    return false;
  }

  particle_system_ =
      (ParticleSystemClass *)_aligned_malloc(sizeof(ParticleSystemClass), 16);
  new (particle_system_) ParticleSystemClass();
  if (!particle_system_) {
    return false;
  }

  result = particle_system_->Initialize(L"data/star.dds");
  if (!result) {
    return false;
  }

  return true;
}

void GraphicsClass::Shutdown() {

  if (particle_system_) {
    particle_system_->Shutdown();
    particle_system_->~ParticleSystemClass();
    _aligned_free(particle_system_);
    particle_system_ = 0;
  }

  if (particle_shader_) {
    particle_shader_->Shutdown();
    particle_shader_->~ParticleShaderClass();
    _aligned_free(particle_shader_);
    particle_shader_ = 0;
  }

  if (camera_) {
    camera_->~Camera();
    _aligned_free(camera_);
    camera_ = nullptr;
  }
}

void GraphicsClass::Frame(float deltaTime) {

  particle_system_->Frame(frame_time_);

  Render();
}

void GraphicsClass::Render() {

  auto directx_device = DirectX11Device::GetD3d11DeviceInstance();

  directx_device->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

  camera_->Render();

  XMMATRIX worldMatrix, viewMatrix, projectionMatrix;

  camera_->GetViewMatrix(viewMatrix);
  directx_device->GetWorldMatrix(worldMatrix);
  directx_device->GetProjectionMatrix(projectionMatrix);

  directx_device->TurnOnAlphaBlending();

  particle_system_->Render();

  auto result = particle_shader_->Render(
      particle_system_->GetIndexCount(), worldMatrix, viewMatrix,
      projectionMatrix, particle_system_->GetTexture());

  directx_device->TurnOffAlphaBlending();

  directx_device->EndScene();
}