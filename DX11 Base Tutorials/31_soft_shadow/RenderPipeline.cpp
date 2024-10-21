#include "RenderPipeline.h"

#include "../CommonFramework/DirectX11Device.h"

void RenderPipeline::AddRenderPass(std::shared_ptr<RenderPass> pass) {
  render_passes_.push_back(pass);
}

void RenderPipeline::AddRenderableObject(std::shared_ptr<IRenderable> object) {
  renderable_objects_.push_back(object);
}

void RenderPipeline::Execute(const ShaderParameterContainer &frameParams) {

  ShaderParameterContainer globalFrameParams = global_parameters_;
  globalFrameParams.Merge(frameParams);

  DirectX11Device::GetD3d11DeviceInstance()->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

  for (const auto &pass : render_passes_) {
    auto pass_name = pass->GetPassName();
    pass->Execute(renderable_objects_, globalFrameParams);
  }

  DirectX11Device::GetD3d11DeviceInstance()->EndScene();
}

void RenderPipeline::SetGlobalParameters(
    const ShaderParameterContainer &params) {
  global_parameters_ = params;
}