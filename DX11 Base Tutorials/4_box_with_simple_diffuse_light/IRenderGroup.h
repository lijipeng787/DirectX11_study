#pragma once

#include <memory>
#include <vector>


#include "IRenderable.h"

class IRenderGroup {
public:
  virtual ~IRenderGroup() = default;

  virtual void PreRender() = 0;

  virtual void PostRender() = 0;

  virtual const std::vector<std::shared_ptr<IRenderable>> &
  GetRenderables() const = 0;
};