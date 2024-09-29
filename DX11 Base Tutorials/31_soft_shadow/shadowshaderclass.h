#ifndef _SHADOWSHADERCLASS_H_
#define _SHADOWSHADERCLASS_H_

#include <DirectXMath.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

#include <fstream>

#include "IShader.h"

class ShadowShaderClass : public IShader {
private:
  struct MatrixBufferType {
    DirectX::XMMATRIX world;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;
    DirectX::XMMATRIX lightView;
    DirectX::XMMATRIX lightProjection;
  };

  struct LightBufferType2 {
    DirectX::XMFLOAT3 lightPosition;
    float padding;
  };

public:
  ShadowShaderClass() = default;

  ShadowShaderClass(const ShadowShaderClass &) = delete;

  ShadowShaderClass &operator=(const ShadowShaderClass &) = delete;

  ~ShadowShaderClass() = default;

public:
  bool Initialize(HWND hwnd) override;

  void Shutdown() override;

  bool Render(int indexCount,
              const ShaderParameterContainer &parameters) const override;

private:
  bool InitializeShader(HWND hwnd, const std::wstring &vsFilename,
                        const std::wstring &psFilename);

  void ShutdownShader();

  void OutputShaderErrorMessage(ID3D10Blob *errorMessage, HWND hwnd,
                                const std::wstring &shaderFilename);

  bool SetShaderParameters(const DirectX::XMMATRIX &worldMatrix,
                           const DirectX::XMMATRIX &viewMatrix,
                           const DirectX::XMMATRIX &projectionMatrix,
                           const DirectX::XMMATRIX &lightViewMatrix,
                           const DirectX::XMMATRIX &lightProjectionMatrix,
                           ID3D11ShaderResourceView *depthMapTexture,
                           const DirectX::XMFLOAT3 &lightPosition) const;

  void RenderShader(int indexCount) const;

private:
  Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader_;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader_;

  Microsoft::WRL::ComPtr<ID3D11InputLayout> layout_;

  Microsoft::WRL::ComPtr<ID3D11Buffer> matrix_buffer_;
  Microsoft::WRL::ComPtr<ID3D11Buffer> light_buffee_2;

  Microsoft::WRL::ComPtr<ID3D11SamplerState> sample_state_;
};

#endif