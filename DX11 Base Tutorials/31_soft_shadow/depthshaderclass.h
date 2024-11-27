#ifndef _DEPTHSHADERCLASS_H_
#define _DEPTHSHADERCLASS_H_

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>

#include "ShaderBase.h"

class DepthShader : public ShaderBase {
private:
  struct MatrixBufferType {
    DirectX::XMMATRIX world;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;
  };

public:
  DepthShader() = default;

  ~DepthShader() = default;

public:
  bool Initialize(HWND hwnd) override;

  bool Render(int indexCount,
              const ShaderParameterContainer &parameters) const override;

private:
  bool InitializeShader(HWND hwnd);

  bool SetShaderParameters(const DirectX::XMMATRIX &worldMatrix,
                           const DirectX::XMMATRIX &viewMatrix,
                           const DirectX::XMMATRIX &projectionMatrix) const;

  void RenderShader(int indexCount) const;

private:
  Microsoft::WRL::ComPtr<ID3D11Buffer> matrix_buffer_;
};

#endif