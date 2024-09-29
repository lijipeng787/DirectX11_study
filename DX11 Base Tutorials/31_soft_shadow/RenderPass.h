#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "IRenderable.h"
#include "IShader.h"
#include "RenderTextureClass.h"

class RenderPass {
public:
  RenderPass(const std::string &name, std::shared_ptr<IShader> shader);

public:
  void AddInputTexture(const std::string &name,
                       std::shared_ptr<RenderTextureClass> texture);

  void SetOutputTexture(std::shared_ptr<RenderTextureClass> texture);

  std::shared_ptr<RenderTextureClass> GetOutputTexture() const;

  void SetPassParameters(const ShaderParameterContainer &params);

  void Execute(const std::vector<std::shared_ptr<IRenderable>> &renderables,
               const ShaderParameterContainer &globalFrameParams);

  void NeedTurnOffZBuffer() { need_turn_z_buffer_off_ = true; }

  std::string GetPassName() const { return pass_name_; }

  void AddRenderTag(const std::string &tag) { render_tags_.insert(tag); }

  void RemoveRenderTag(const std::string &tag) { render_tags_.erase(tag); }

  bool ShouldRenderObject(const IRenderable &object) const;

private:
  std::string pass_name_;

  std::shared_ptr<IShader> shader_;

  std::map<std::string, std::shared_ptr<RenderTextureClass>> input_textures_;

  std::shared_ptr<RenderTextureClass> output_texture_;

  ShaderParameterContainer pass_parameters_;

  std::unordered_set<std::string> render_tags_;

  bool need_turn_z_buffer_off_ = false;
};