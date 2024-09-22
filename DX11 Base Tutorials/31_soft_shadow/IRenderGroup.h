#pragma once

#include "IRenderable.h"
#include <memory>
#include <vector>

class IRenderGroup {
public:
  virtual ~IRenderGroup() = default;

public:
  virtual void PreRender() = 0;

  virtual void PostRender() = 0;

  virtual const std::vector<std::shared_ptr<IRenderable>> &
  GetRenderables() const = 0;
};