

#ifndef _SOFTSHADOWSHADERCLASS_H_
#define _SOFTSHADOWSHADERCLASS_H_

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>

#include "IShader.h"

class SoftShadowShaderClass : public IShader {
private:
  struct MatrixBufferType {
    DirectX::XMMATRIX world;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;
  };

  struct LightBufferType {
    DirectX::XMFLOAT4 ambientColor;
    DirectX::XMFLOAT4 diffuseColor;
  };

  struct LightBufferType2 {
    DirectX::XMFLOAT3 lightPosition;
    float padding;
  };

public:
  SoftShadowShaderClass() = default;

  SoftShadowShaderClass(const SoftShadowShaderClass &) = delete;

  ~SoftShadowShaderClass() = default;

public:
  bool Initialize(HWND) override;

  void Shutdown() override;

  bool
  Render(int indexCount,
         const ShaderParameterContainer &parameterContainer) const override;

private:
  bool InitializeShader(HWND, WCHAR *, WCHAR *);

  void ShutdownShader();

  void OutputShaderErrorMessage(ID3D10Blob *, HWND, WCHAR *);

  bool SetShaderParameters(const DirectX::XMMATRIX &, const DirectX::XMMATRIX &,
                           const DirectX::XMMATRIX &,
                           ID3D11ShaderResourceView *,
                           ID3D11ShaderResourceView *,
                           const DirectX::XMFLOAT3 &, const DirectX::XMFLOAT4 &,
                           const DirectX::XMFLOAT4 &) const;
  void RenderShader(int) const;

private:
  Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader_;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader_;

  Microsoft::WRL::ComPtr<ID3D11InputLayout> layout_;

  Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampleStateWrap;
  Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampleStateClamp;

  Microsoft::WRL::ComPtr<ID3D11Buffer> matrix_buffer_;
  Microsoft::WRL::ComPtr<ID3D11Buffer> light_buffer_;
  Microsoft::WRL::ComPtr<ID3D11Buffer> m_lightBuffer2;
};

#endif