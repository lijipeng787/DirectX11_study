#include "RenderPipeline.h"

#include "../CommonFramework2/DirectX11Device.h"

void RenderPipeline::AddRenderPass(std::shared_ptr<RenderPass> pass) {
  render_passes_.push_back(pass);
}

void RenderPipeline::AddRenderableObject(std::shared_ptr<IRenderable> object) {
  renderable_objects_.push_back(object);
}

void RenderPipeline::Execute(const ShaderParameterContainer &frameParams) {
  Execute(renderable_objects_, frameParams);
}

void RenderPipeline::Execute(
    const std::vector<std::shared_ptr<IRenderable>> &objects,
    const ShaderParameterContainer &frameParams) {

  ShaderParameterContainer globalFrameParams = global_parameters_;
  globalFrameParams.Merge(frameParams);

  DirectX11Device::GetD3d11DeviceInstance()->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

  for (const auto &pass : render_passes_) {
    pass->Execute(objects, globalFrameParams, device_context_);
  }

  DirectX11Device::GetD3d11DeviceInstance()->EndScene();
}

void RenderPipeline::SetGlobalParameters(
    const ShaderParameterContainer &params) {
  global_parameters_ = params;
}