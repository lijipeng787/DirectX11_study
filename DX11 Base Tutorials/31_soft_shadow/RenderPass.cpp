#include "RenderPass.h"
#include "../CommonFramework/DirectX11Device.h"

RenderPass::RenderPass(const std::string &name, std::shared_ptr<IShader> shader)
    : pass_name_(name), shader_(shader) {}

void RenderPass::AddInputTexture(const std::string &name,
                                 std::shared_ptr<RenderTextureClass> texture) {
  input_textures_[name] = texture;
}

void RenderPass::SetOutputTexture(std::shared_ptr<RenderTextureClass> texture) {
  output_texture_ = texture;
}

std::shared_ptr<RenderTextureClass> RenderPass::GetOutputTexture() const {
  return output_texture_;
}

void RenderPass::SetPassParameters(const ShaderParameterContainer &params) {
  pass_parameters_ = params;
}

void RenderPass::Execute(
    const std::vector<std::shared_ptr<IRenderable>> &renderables,
    const ShaderParameterContainer &globalFrameParams) {
  if (output_texture_) {
    output_texture_->SetRenderTarget();
    output_texture_->ClearRenderTarget(0.0f, 0.0f, 0.0f, 1.0f);
  }

  ShaderParameterContainer globalFramePassParams = pass_parameters_;
  globalFramePassParams.Merge(globalFrameParams);

  for (const auto &[name, texture] : input_textures_) {
    globalFramePassParams.Set(name, texture->GetShaderResourceView());
  }

  if (need_turn_z_buffer_off_)
    DirectX11Device::GetD3d11DeviceInstance()->TurnZBufferOff();

  for (const auto &renderable : renderables) {
    if (ShouldRenderObject(*renderable)) {
      ShaderParameterContainer objectParams = globalFramePassParams;
      objectParams.Set("worldMatrix", renderable->GetWorldMatrix());

      auto callback = renderable->GetParameterCallback();
      if (callback) {
        callback(objectParams);
      }

      renderable->Render(*shader_, objectParams);
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
