#pragma once
#include <DirectXMath.h>

class IShader;

class IRenderable {
public:
  virtual ~IRenderable() = default;

  virtual void Render(const IShader &shader,
                      const DirectX::XMMATRIX &viewMatrix,
                      const DirectX::XMMATRIX &projectionMatrix) const = 0;
};