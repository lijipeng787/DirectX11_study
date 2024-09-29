#pragma once

#include "IRenderGroup.h"

class StandardRenderGroup : public IRenderGroup {
public:
  void PreRender() override {}

  void PostRender() override {}

  const std::vector<std::shared_ptr<IRenderable>> &
  GetRenderables() const override {
    return renderables_;
  }

  void AddRenderable(std::shared_ptr<IRenderable> renderable);

private:
  std::vector<std::shared_ptr<IRenderable>> renderables_;
};