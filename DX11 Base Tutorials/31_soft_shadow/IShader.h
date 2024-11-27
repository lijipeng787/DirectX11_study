#pragma once

#include <Windows.h>

#include "ShaderParameterContainer.h"

class IShader {
public:
  virtual ~IShader() = default;

  virtual bool Initialize(HWND hwnd) = 0;

  virtual void Shutdown() = 0;

  virtual bool Render(int indexCount,
                      const ShaderParameterContainer &parameters) const = 0;
};
