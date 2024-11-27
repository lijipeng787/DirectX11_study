#pragma once

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>

#include "ShaderBase.h"

class TextureShader : public ShaderBase {
private:
  struct MatrixBufferType {
    DirectX::XMMATRIX world;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;
  };

public:
  TextureShader() = default;

  ~TextureShader() = default;

public:
  bool Initialize(HWND hwnd) override;

  bool Render(int indexCount,
              const ShaderParameterContainer &parameters) const override;

private:
  bool InitializeShader(HWND hwnd);

  bool SetShaderParameters(const DirectX::XMMATRIX &worldMatrix,
                           const DirectX::XMMATRIX &viewMatrix,
                           const DirectX::XMMATRIX &projectionMatrix,
                           ID3D11ShaderResourceView *texture) const;

  void RenderShader(int indexCount) const;

private:
  Microsoft::WRL::ComPtr<ID3D11Buffer> matrix_buffer_;
};
