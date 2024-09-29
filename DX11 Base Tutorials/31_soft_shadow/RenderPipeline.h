#pragma once

#include <memory>
#include <vector>

#include "IRenderable.h"
#include "RenderPass.h"
#include "ShaderParameterContainer.h"

class RenderPipeline {
public:
  void AddRenderPass(std::shared_ptr<RenderPass> pass);

  void AddRenderableObject(std::shared_ptr<IRenderable> object);

  void Execute(const ShaderParameterContainer &globalParams);

  void SetGlobalParameters(const ShaderParameterContainer &params);

private:
  std::vector<std::shared_ptr<RenderPass>> render_passes_;
  std::vector<std::shared_ptr<IRenderable>> renderable_objects_;

  ShaderParameterContainer global_parameters_;
};
