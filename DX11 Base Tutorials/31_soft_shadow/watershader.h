#pragma once

#include "ShaderBase.h"

class WaterShader : public ShaderBase {
public:
  WaterShader() = default;

  ~WaterShader() override = default;

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

  struct ReflectionBufferType {
    DirectX::XMMATRIX reflectionMatrix;
  };

  struct WaterBufferType {
    float waterTranslation;
    float reflectRefractScale;
    DirectX::XMFLOAT2 padding;
  };

  bool SetShaderParameters(const DirectX::XMMATRIX &worldMatrix,
                           const DirectX::XMMATRIX &viewMatrix,
                           const DirectX::XMMATRIX &projectionMatrix,
                           const DirectX::XMMATRIX &reflectionMatrix,
                           ID3D11ShaderResourceView *reflectionTexture,
                           ID3D11ShaderResourceView *refractionTexture,
                           ID3D11ShaderResourceView *normalTexture,
                           float waterTranslation, float reflectRefractScale,
                           ID3D11DeviceContext *deviceContext) const;

private:
  Microsoft::WRL::ComPtr<ID3D11Buffer> matrix_buffer_;
  Microsoft::WRL::ComPtr<ID3D11Buffer> reflection_buffer_;
  Microsoft::WRL::ComPtr<ID3D11Buffer> water_buffer_;
};

