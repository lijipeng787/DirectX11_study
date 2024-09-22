

#ifndef _VERTICALBLURSHADERCLASS_H_
#define _VERTICALBLURSHADERCLASS_H_

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>

#include "IShader.h"

class VerticalBlurShaderClass : public IShader {
private:
  struct MatrixBufferType {
    DirectX::XMMATRIX world;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;
  };

  struct ScreenSizeBufferType {
    float screenHeight;
    DirectX::XMFLOAT3 padding;
  };

public:
  VerticalBlurShaderClass() = default;

  VerticalBlurShaderClass(const VerticalBlurShaderClass &) = delete;

  ~VerticalBlurShaderClass() = default;

public:
  bool Initialize(HWND) override;

  void Shutdown() override;

  bool
  Render(int,
         const ShaderParameterContainer &parameterContainer) const override;

private:
  bool InitializeShader(HWND, WCHAR *, WCHAR *);

  void ShutdownShader();

  void OutputShaderErrorMessage(ID3D10Blob *, HWND, WCHAR *);

  bool SetShaderParameters(const DirectX::XMMATRIX &, const DirectX::XMMATRIX &,
                           const DirectX::XMMATRIX &,
                           ID3D11ShaderResourceView *, float) const;

  void RenderShader(int) const;

private:
  Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader_;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader_;

  Microsoft::WRL::ComPtr<ID3D11InputLayout> layout_;

  Microsoft::WRL::ComPtr<ID3D11SamplerState> sample_state_;

  Microsoft::WRL::ComPtr<ID3D11Buffer> matrix_buffer_;
  Microsoft::WRL::ComPtr<ID3D11Buffer> screen_size_buffer_;
};

#endif