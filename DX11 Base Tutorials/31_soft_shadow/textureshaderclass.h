#pragma once

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>

#include "IShader.h"

class TextureShaderClass : public IShader {
private:
  struct MatrixBufferType {
    DirectX::XMMATRIX world;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;
  };

public:
  TextureShaderClass() = default;

  TextureShaderClass(const TextureShaderClass &) = delete;

  ~TextureShaderClass() = default;

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
                           ID3D11ShaderResourceView *) const;

  void RenderShader(int) const;

private:
  Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader_;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader_;

  Microsoft::WRL::ComPtr<ID3D11InputLayout> layout_;

  Microsoft::WRL::ComPtr<ID3D11Buffer> matrix_buffer_;

  Microsoft::WRL::ComPtr<ID3D11SamplerState> sample_state_;
};
