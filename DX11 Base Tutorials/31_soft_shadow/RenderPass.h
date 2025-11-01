#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Interfaces.h"
#include "RenderTexture.h"

class RenderPass {
public:
  explicit RenderPass(const std::string &name, std::shared_ptr<IShader> shader);

public:
  void AddInputTexture(const std::string &name,
                       std::shared_ptr<RenderTexture> texture);

  void SetOutputTexture(std::shared_ptr<RenderTexture> texture);

  std::shared_ptr<RenderTexture> GetOutputTexture() const;

  void SetPassParameters(const ShaderParameterContainer &params);

  void Execute(const std::vector<std::shared_ptr<IRenderable>> &renderables,
               const ShaderParameterContainer &globalFrameParams,
               ID3D11DeviceContext *deviceContext);

  void DisableZBuffer(bool disable = true) {
    need_turn_z_buffer_off_ = disable;
  }

  const std::string &GetName() const { return pass_name_; }

  void AddRenderTag(const std::string &tag) { render_tags_.insert(tag); }

  void RemoveRenderTag(const std::string &tag) { render_tags_.erase(tag); }

  bool ShouldRenderObject(const IRenderable &object) const;

private:
  std::string pass_name_;

  std::shared_ptr<IShader> shader_;

  std::map<std::string, std::shared_ptr<RenderTexture>> input_textures_;

  std::shared_ptr<RenderTexture> output_texture_;

  ShaderParameterContainer pass_parameters_;

  std::unordered_set<std::string> render_tags_;

  bool need_turn_z_buffer_off_ = false;
};