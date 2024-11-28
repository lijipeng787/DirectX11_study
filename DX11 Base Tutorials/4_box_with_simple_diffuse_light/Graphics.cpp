#include "Graphics.h"

#include <new>

#include "lightclass.h"
#include "ModelClass.h"
#include "RenderSystem.h"
#include "StandardRenderGroup.h"

#include <DirectXMath.h>

#include "../CommonFramework/Camera.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/TypeDefine.h"

#include "lightshaderclass.h"

using namespace DirectX;

bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd) {

  auto directx11_device_ = DirectX11Device::GetD3d11DeviceInstance();

  if (!(directx11_device_->Initialize(screenWidth, screenHeight, VSYNC_ENABLED,
                                      hwnd, FULL_SCREEN, SCREEN_DEPTH,
                                      SCREEN_NEAR))) {
    MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
    return false;
  }

  render_system_ = std::make_unique<RenderSystem>();
  if (!(render_system_->Initialize(hwnd))) {
    MessageBox(hwnd, L"Could not initialize RenderSystem.", L"Error", MB_OK);
    return false;
  }

  scene_ = std::make_unique<Scene>();

  // Set up camera
  auto camera = std::make_shared<Camera>();
  camera->SetPosition(0.0f, 0.0f, -10.0f);
  scene_->SetCamera(camera);

  // Set up light
  auto light = std::make_shared<LightClass>();
  light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
  light->SetDirection(0.0f, 0.0f, 1.0f);
  scene_->AddLight(light);

  // Set up render group with model
  auto renderGroup = std::make_shared<StandardRenderGroup>();
  auto model = std::make_shared<ModelClass>();
  if (!model->Initialize("data/cube.txt", L"data/seafloor.dds")) {
    MessageBox(hwnd, L"Could not initialize the model object.", L"Error",
               MB_OK);
    return false;
  }
  // Create multiple cube GameObjects
  for (int i = 0; i < 5; ++i) {
    auto cube = std::make_shared<GameObject>(model);
    cube->SetPosition(DirectX::XMFLOAT3(i * 2.0f + -4.0f, 0.0f, 0.0f));
    renderGroup->AddGameObject(cube);
  }

  scene_->AddRenderGroup(renderGroup);

  return true;
}

void GraphicsClass::Shutdown() {
  if (render_system_) {
    render_system_->Shutdown();
  }
  scene_.reset();
}

void GraphicsClass::Frame(float deltatime) {
  // Update model's world matrix
  if (!scene_->GetRenderGroups().empty()) {

    auto renderGroup = std::dynamic_pointer_cast<StandardRenderGroup>(
        scene_->GetRenderGroups()[0]);
    for (const auto &gameObject : renderGroup->GetGameObjects()) {
      gameObject->Update(deltatime); // Assume 60 FPS for simplicity
      gameObject->GetModel()->SetWorldMatrix(gameObject->GetWorldMatrix());
      Render();
    }
  }
}

void GraphicsClass::Render() {
  if (render_system_) {
    render_system_->Render(*scene_);
  }
}