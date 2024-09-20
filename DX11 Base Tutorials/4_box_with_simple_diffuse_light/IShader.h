#pragma once

#include <DirectXMath.h>
#include <d3d11.h>

class IShader {
public:
  virtual ~IShader() = default;

  virtual bool Initialize(HWND hwnd) = 0;

  virtual void Shutdown() = 0;

  virtual bool Render(int indexCount, const DirectX::XMMATRIX &worldMatrix,
                      const DirectX::XMMATRIX &viewMatrix,
                      const DirectX::XMMATRIX &projectionMatrix,
                      ID3D11ShaderResourceView *texture,
                      const DirectX::XMFLOAT3 &lightDirection,
                      const DirectX::XMFLOAT4 &diffuseColor) const = 0;
};