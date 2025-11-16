#pragma once

#include <d3d11.h>
#include <directxmath.h>
#include <wrl/client.h>

#include "ShaderBase.h"

class PbrShader : public ShaderBase {
protected:
  struct MatrixBufferType {
    DirectX::XMMATRIX world;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;
  };

  struct CameraBufferType {
    DirectX::XMFLOAT3 cameraPosition;
    float padding;
  };

  struct LightBufferType {
    DirectX::XMFLOAT3 lightDirection;
    float padding;
  };

public:
  PbrShader() = default;

  ~PbrShader() = default;

public:
  bool Initialize(HWND hwnd, ID3D11Device *device) override;

  bool Render(int indexCount, const ShaderParameterContainer &parameters,
              ID3D11DeviceContext *deviceContext) const override;

protected:
  bool SetShaderParameters(const DirectX::XMMATRIX &worldMatrix,
                           const DirectX::XMMATRIX &viewMatrix,
                           const DirectX::XMMATRIX &projectionMatrix,
                           ID3D11ShaderResourceView *albedoTexture,
                           ID3D11ShaderResourceView *normalMap,
                           ID3D11ShaderResourceView *roughnessMetallicTexture,
                           const DirectX::XMFLOAT3 &lightDirection,
                           const DirectX::XMFLOAT3 &cameraPosition,
                           ID3D11DeviceContext *deviceContext) const;

private:
  Microsoft::WRL::ComPtr<ID3D11Buffer> matrix_buffer_;
  Microsoft::WRL::ComPtr<ID3D11Buffer> camera_buffer_;
  Microsoft::WRL::ComPtr<ID3D11Buffer> light_buffer_;
  Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler_state_;
};