#ifndef _SHADOWSHADERCLASS_H_
#define _SHADOWSHADERCLASS_H_

#include <DirectXMath.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

#include "ShaderBase.h"

class ShadowShader : public ShaderBase {
protected:
  struct MatrixBufferType {
    DirectX::XMMATRIX world;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;
    DirectX::XMMATRIX lightView;
    DirectX::XMMATRIX lightProjection;
  };

  struct LightBufferType {
    DirectX::XMFLOAT3 lightPosition;
    float padding;
  };

public:
  ShadowShader() = default;

  ~ShadowShader() = default;

public:
  bool Initialize(HWND hwnd) override;

  bool Render(int indexCount,
              const ShaderParameterContainer &parameters) const override;

protected:
  bool InitializeShader(HWND hwnd);

  bool SetShaderParameters(const DirectX::XMMATRIX &worldMatrix,
                           const DirectX::XMMATRIX &viewMatrix,
                           const DirectX::XMMATRIX &projectionMatrix,
                           const DirectX::XMMATRIX &lightViewMatrix,
                           const DirectX::XMMATRIX &lightProjectionMatrix,
                           ID3D11ShaderResourceView *depthMapTexture,
                           const DirectX::XMFLOAT3 &lightPosition) const;

  void RenderShader(int indexCount) const;

private:
  Microsoft::WRL::ComPtr<ID3D11Buffer> matrix_buffer_;
  Microsoft::WRL::ComPtr<ID3D11Buffer> light_buffer_;
  Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler_state_clamp_;
};

#endif