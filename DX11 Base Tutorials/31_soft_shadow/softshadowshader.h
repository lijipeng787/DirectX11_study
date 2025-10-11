#ifndef _SOFTSHADOWSHADERCLASS_H_
#define _SOFTSHADOWSHADERCLASS_H_

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>

#include "ShaderBase.h"

class SoftShadowShader : public ShaderBase {
protected:
  struct MatrixBufferType {
    DirectX::XMMATRIX world;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;
  };

  struct LightBufferType {
    DirectX::XMFLOAT4 ambientColor;
    DirectX::XMFLOAT4 diffuseColor;
  };

  struct LightPositionBufferType {
    DirectX::XMFLOAT3 lightPosition;
    float padding;
  };

public:
  SoftShadowShader() = default;

  ~SoftShadowShader() = default;

public:
  bool Initialize(HWND hwnd, ID3D11Device *device) override;

  bool Render(int indexCount, const ShaderParameterContainer &parameters,
              ID3D11DeviceContext *deviceContext) const override;

protected:
  bool SetShaderParameters(const DirectX::XMMATRIX &worldMatrix,
                           const DirectX::XMMATRIX &viewMatrix,
                           const DirectX::XMMATRIX &projectionMatrix,
                           ID3D11ShaderResourceView *texture,
                           ID3D11ShaderResourceView *shadowTexture,
                           const DirectX::XMFLOAT3 &lightPosition,
                           const DirectX::XMFLOAT4 &ambientColor,
                           const DirectX::XMFLOAT4 &diffuseColor,
                           ID3D11DeviceContext *deviceContext) const;

private:
  Microsoft::WRL::ComPtr<ID3D11Buffer> matrix_buffer_;

  Microsoft::WRL::ComPtr<ID3D11Buffer> light_buffer_;

  Microsoft::WRL::ComPtr<ID3D11Buffer> light_position_buffer_;

  Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler_state_wrap_;

  Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler_state_clamp_;
};

#endif