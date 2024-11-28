#include "RenderPipeline.h"

#include "../CommonFramework/Camera.h"
#include "../CommonFramework/DirectX11Device.h"
#include "LightShaderClass.h"
#include "StandardRenderGroup.h"

#include <iostream>
#include <stdexcept>

bool RenderPipeline::Initialize(HWND hwnd) {
  // Initialize default shader
  shader_ = std::make_unique<LightShaderClass>();
  return shader_->Initialize(hwnd);
}

void RenderPipeline::Shutdown() {
  if (shader_) {
    shader_->Shutdown();
    shader_.reset();
  }
}

bool RenderPipeline::Render(const Scene &scene) const {
  if (!shader_) {
    return false;
  }

  auto directx_device = DirectX11Device::GetD3d11DeviceInstance();
  directx_device->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

  const auto camera = scene.GetCamera();
  if (!camera) {
    return false;
  }

  camera->Render();

  DirectX::XMMATRIX viewMatrix, projectionMatrix;
  camera->GetViewMatrix(viewMatrix);
  directx_device->GetProjectionMatrix(projectionMatrix);

  for (const auto &renderGroup : scene.GetRenderGroups()) {
    renderGroup->PreRender();

    for (const auto &gameObject :
         std::dynamic_pointer_cast<StandardRenderGroup>(renderGroup)
             ->GetGameObjects()) {
      const auto &model = gameObject->GetModel();
      DirectX::XMMATRIX worldMatrix = gameObject->GetWorldMatrix();
      model->SetWorldMatrix(worldMatrix);
      model->Render(*shader_, viewMatrix, projectionMatrix);
    }

    renderGroup->PostRender();
  }

  directx_device->EndScene();
  return true;
}

void RenderPipeline::SetShader(std::unique_ptr<IShader> shader) {
  shader_ = std::move(shader);
}