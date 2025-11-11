#pragma once

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>

#include "ShaderBase.h"

class FontShader : public ShaderBase {
private:
  struct MatrixBufferType {
    DirectX::XMMATRIX world;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;
  };

  struct PixelBufferType {
    DirectX::XMFLOAT4 pixelColor;
  };

public:
  FontShader() = default;

  ~FontShader() = default;

  bool Initialize(HWND hwnd, ID3D11Device *device) override;

  bool Render(int indexCount, const ShaderParameterContainer &parameters,
              ID3D11DeviceContext *deviceContext) const override;

private:
  bool SetShaderParameters(const DirectX::XMMATRIX &worldMatrix,
                           const DirectX::XMMATRIX &viewMatrix,
                           const DirectX::XMMATRIX &projectionMatrix,
                           ID3D11ShaderResourceView *texture,
                           const DirectX::XMFLOAT4 &pixelColor,
                           ID3D11DeviceContext *deviceContext) const;

  Microsoft::WRL::ComPtr<ID3D11Buffer> matrix_buffer_;
  
  Microsoft::WRL::ComPtr<ID3D11Buffer> pixel_buffer_;
};
