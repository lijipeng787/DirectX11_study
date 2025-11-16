#pragma once

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>

#include "ShaderBase.h"

class SimpleLightShader : public ShaderBase {
protected:
  struct MatrixBufferType {
    DirectX::XMMATRIX world;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;
  };

  struct LightBufferType {
    DirectX::XMFLOAT4 ambientColor;
    DirectX::XMFLOAT4 diffuseColor;
    DirectX::XMFLOAT3 lightDirection;
    float padding;
  };

public:
  SimpleLightShader() = default;

  ~SimpleLightShader() = default;

public:
  bool Initialize(HWND hwnd, ID3D11Device *device) override;

  bool Render(int indexCount, const ShaderParameterContainer &parameters,
              ID3D11DeviceContext *deviceContext) const override;

protected:
  bool SetShaderParameters(const DirectX::XMMATRIX &worldMatrix,
                           const DirectX::XMMATRIX &viewMatrix,
                           const DirectX::XMMATRIX &projectionMatrix,
                           ID3D11ShaderResourceView *texture,
                           const DirectX::XMFLOAT3 &lightDirection,
                           const DirectX::XMFLOAT4 &ambientColor,
                           const DirectX::XMFLOAT4 &diffuseColor,
                           ID3D11DeviceContext *deviceContext) const;

private:
  Microsoft::WRL::ComPtr<ID3D11Buffer> matrix_buffer_;
  Microsoft::WRL::ComPtr<ID3D11Buffer> light_buffer_;
};
