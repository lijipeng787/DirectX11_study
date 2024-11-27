#pragma once

#include "IShader.h"

#include <DirectXMath.h>
#include <d3d11.h>
#include <string>
#include <wrl/client.h>

class ShaderBase : public IShader {
public:
  ShaderBase() = default;

  virtual ~ShaderBase() = default;

public:
  // Interface implementation
  bool Initialize(HWND hwnd) override;

  void Shutdown() override;

  virtual bool
  Render(int indexCount,
         const ShaderParameterContainer &parameters) const override = 0;

protected:
  // Protected utility methods for shader compilation and setup
  bool InitializeShaderFromFile(HWND hwnd, const std::wstring &vsFilename,
                                const std::string &vsEntryName,
                                const std::wstring &psFilename,
                                const std::string &psEntryName,
                                const D3D11_INPUT_ELEMENT_DESC *layoutDesc,
                                UINT numElements);

  bool CreateConstantBuffer(UINT byteWidth, ID3D11Buffer **buffer);

  bool CreateSamplerState(
      ID3D11SamplerState **samplerState,
      D3D11_TEXTURE_ADDRESS_MODE addressMode = D3D11_TEXTURE_ADDRESS_WRAP);

  // Shader resources
  Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader_;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader_;
  Microsoft::WRL::ComPtr<ID3D11InputLayout> layout_;
  Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler_state_;

private:
  void OutputShaderErrorMessage(ID3D10Blob *errorMessage, HWND hwnd,
                                const std::wstring &shaderFilename);
};

class BlurShaderBase : public ShaderBase {
protected:
  struct MatrixBufferType {
    DirectX::XMMATRIX world;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;
  };

  struct ScreenSizeBufferType {
    float screenSize; // width or height depending on blur direction
    DirectX::XMFLOAT3 padding;
  };

public:
  BlurShaderBase() = default;

  virtual ~BlurShaderBase() = default;

protected:
  bool InitializeBlurShader(HWND hwnd, const std::wstring &vsFilename,
                            const std::string &vsEntryName,
                            const std::wstring &psFilename,
                            const std::string &psEntryName);

  bool SetBaseShaderParameters(const DirectX::XMMATRIX &worldMatrix,
                               const DirectX::XMMATRIX &viewMatrix,
                               const DirectX::XMMATRIX &projectionMatrix,
                               ID3D11ShaderResourceView *texture,
                               float screenSize) const;

  void RenderShader(int indexCount) const;

protected:
  Microsoft::WRL::ComPtr<ID3D11Buffer> matrix_buffer_;
  Microsoft::WRL::ComPtr<ID3D11Buffer> screen_size_buffer_;
};