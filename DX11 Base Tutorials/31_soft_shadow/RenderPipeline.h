#pragma once

#include <d3d11.h>
#include <memory>
#include <vector>

#include "Interfaces.h"
#include "RenderPass.h"
#include "ShaderParameterContainer.h"

class RenderPipeline {
public:
  RenderPipeline() = default;

  void Initialize(ID3D11Device *device, ID3D11DeviceContext *deviceContext) {
    device_ = device;
    device_context_ = deviceContext;
  }

  void AddRenderPass(std::shared_ptr<RenderPass> pass);

  void AddRenderableObject(std::shared_ptr<IRenderable> object);

  void Execute(const ShaderParameterContainer &globalParams);

  void SetGlobalParameters(const ShaderParameterContainer &params);

  ID3D11Device *GetDevice() const { return device_; }
  ID3D11DeviceContext *GetDeviceContext() const { return device_context_; }

private:
  std::vector<std::shared_ptr<RenderPass>> render_passes_;
  std::vector<std::shared_ptr<IRenderable>> renderable_objects_;

  ShaderParameterContainer global_parameters_;

  ID3D11Device *device_ = nullptr;
  ID3D11DeviceContext *device_context_ = nullptr;
};
