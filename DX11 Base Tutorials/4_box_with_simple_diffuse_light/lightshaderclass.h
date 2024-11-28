#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>

#include <memory>
#include <string>

#include "IShader.h"

class LightShaderClass : public IShader {
public:
  LightShaderClass() = default;

  LightShaderClass(const LightShaderClass &) = delete;

  LightShaderClass &operator=(const LightShaderClass &) = delete;

  ~LightShaderClass() = default;

public:
  bool Initialize(HWND hwnd) override;

  void Shutdown() override;

  bool Render(int indexCount, const DirectX::XMMATRIX &worldMatrix,
              const DirectX::XMMATRIX &viewMatrix,
              const DirectX::XMMATRIX &projectionMatrix,
              ID3D11ShaderResourceView *texture,
              const DirectX::XMFLOAT3 &lightDirection,
              const DirectX::XMFLOAT4 &diffuseColor) const override;

private:
  bool InitializeShader(HWND hwnd, const std::wstring &vsFilename,
                        const std::wstring &psFilename);

  void ShutdownShader();

  void OutputShaderErrorMessage(ID3D10Blob *errorMessage, HWND hwnd,
                                const std::wstring &shaderFilename);

  bool SetShaderParameters(const DirectX::XMMATRIX &worldMatrix,
                           const DirectX::XMMATRIX &viewMatrix,
                           const DirectX::XMMATRIX &projectionMatrix,
                           ID3D11ShaderResourceView *texture,
                           const DirectX::XMFLOAT3 &lightDirection,
                           const DirectX::XMFLOAT4 &diffuseColor) const;

  void RenderShader(int indexCount) const;

private:
  struct ShaderResources {
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader_;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader_;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> layout_;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler_state_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> matrix_buffer_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> light_buffer_;
  };

  ShaderResources resources_;
};