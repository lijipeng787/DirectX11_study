#pragma once

#include <Windows.h>
#include <d3d11.h>

#include "ShaderParameterContainer.h"

class IShader {
public:
  IShader() = default;

  virtual ~IShader() = default;

  virtual bool Initialize(HWND hwnd, ID3D11Device *device) = 0;

  virtual void Shutdown() = 0;

  virtual bool Render(int indexCount,
                      const ShaderParameterContainer &parameters,
                      ID3D11DeviceContext *deviceContext) const = 0;
};
