#include "RenderPass.h"

#include "../CommonFramework2/DirectX11Device.h"

RenderPass::RenderPass(const std::string &name, std::shared_ptr<IShader> shader)
    : pass_name_(name), shader_(shader) {}

void RenderPass::AddInputTexture(const std::string &name,
                                 std::shared_ptr<RenderTexture> texture) {
  input_textures_[name] = texture;
}

void RenderPass::SetOutputTexture(std::shared_ptr<RenderTexture> texture) {
  output_texture_ = texture;
}

std::shared_ptr<RenderTexture> RenderPass::GetOutputTexture() const {
  return output_texture_;
}

void RenderPass::SetPassParameters(const ShaderParameterContainer &params) {
  pass_parameters_ = params;
}

void RenderPass::Execute(
    const std::vector<std::shared_ptr<IRenderable>> &renderables,
    const ShaderParameterContainer &globalFrameParams,
    ID3D11DeviceContext *deviceContext) {
  if (output_texture_) {
    output_texture_->SetRenderTarget();
    output_texture_->ClearRenderTarget(0.0f, 0.0f, 0.0f, 1.0f);
  }

  ShaderParameterContainer globalFramePassParams =
      ShaderParameterContainer::ChainMerge(globalFrameParams, pass_parameters_);

  for (const auto &[name, texture] : input_textures_) {
    globalFramePassParams.SetTexture(
        name, texture->GetShaderResourceView(),
        ShaderParameterContainer::ParameterOrigin::Pass);
  }

  if (need_turn_z_buffer_off_)
    DirectX11Device::GetD3d11DeviceInstance()->TurnZBufferOff();

  for (const auto &renderable : renderables) {
    if (ShouldRenderObject(*renderable)) {
      ShaderParameterContainer::BuildParametersInput inputs;
      inputs.base_params = &globalFramePassParams;

      const auto &object_params = renderable->GetObjectParameters();
      inputs.object_params = &object_params;

      DirectX::XMMATRIX world_matrix = renderable->GetWorldMatrix();
      inputs.world_matrix = &world_matrix;

      inputs.callback = renderable->GetParameterCallback();

      ShaderParameterContainer final_params =
          ShaderParameterContainer::BuildFinalParameters(inputs);

      renderable->Render(*shader_, final_params, deviceContext);
    }
  }

  if (need_turn_z_buffer_off_)
    DirectX11Device::GetD3d11DeviceInstance()->TurnZBufferOn();

  if (output_texture_) {
    DirectX11Device::GetD3d11DeviceInstance()->SetBackBufferRenderTarget();
    DirectX11Device::GetD3d11DeviceInstance()->ResetViewport();
  }
}

bool RenderPass::ShouldRenderObject(const IRenderable &object) const {
  if (render_tags_.empty()) {
    return true; // If no tags specified, render all objects
  }
  for (const auto &tag : object.GetTags()) {
    if (render_tags_.find(tag) != render_tags_.end()) {
      return true;
    }
  }
  return false;
}
