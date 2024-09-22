#pragma once

#include <DirectXMath.h>

#include "ShaderParameterContainer.h"

class IShader;

class IRenderable {
public:
  virtual ~IRenderable() = default;

  virtual void
  Render(const IShader &shader,
         const ShaderParameterContainer &parameterContainer) const = 0;
};