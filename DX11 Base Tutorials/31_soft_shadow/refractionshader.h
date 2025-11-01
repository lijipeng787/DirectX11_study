#pragma once

#include "ShaderBase.h"

class RefractionShader : public ShaderBase {
public:
  RefractionShader() = default;

  ~RefractionShader() override = default;

  bool Initialize(HWND hwnd, ID3D11Device *device) override;

  void Shutdown() override;

  bool Render(int indexCount, const ShaderParameterContainer &parameters,
              ID3D11DeviceContext *deviceContext) const override;

private:
  struct MatrixBufferType {
    DirectX::XMMATRIX world;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;
  };

  struct ClipPlaneBufferType {
    DirectX::XMFLOAT4 clipPlane;
  };

  struct LightBufferType {
    DirectX::XMFLOAT4 ambientColor;
    DirectX::XMFLOAT4 diffuseColor;
    DirectX::XMFLOAT3 lightDirection;
    float padding;
  };

  bool SetShaderParameters(const DirectX::XMMATRIX &worldMatrix,
                           const DirectX::XMMATRIX &viewMatrix,
                           const DirectX::XMMATRIX &projectionMatrix,
                           ID3D11ShaderResourceView *texture,
                           const DirectX::XMFLOAT4 &ambientColor,
                           const DirectX::XMFLOAT4 &diffuseColor,
                           const DirectX::XMFLOAT3 &lightDirection,
                           const DirectX::XMFLOAT4 &clipPlane,
                           ID3D11DeviceContext *deviceContext) const;

private:
  Microsoft::WRL::ComPtr<ID3D11Buffer> matrix_buffer_;
  Microsoft::WRL::ComPtr<ID3D11Buffer> clip_plane_buffer_;
  Microsoft::WRL::ComPtr<ID3D11Buffer> light_buffer_;
};

