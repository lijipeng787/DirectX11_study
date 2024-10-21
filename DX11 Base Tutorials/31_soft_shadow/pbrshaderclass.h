#ifndef _PBRSHADERCLASS_H_
#define _PBRSHADERCLASS_H_

#include <d3d11.h>
#include <directxmath.h>
#include <wrl/client.h>

#include "IShader.h"

class PbrShaderClass : public IShader {
private:
  struct MatrixBufferType {
    DirectX::XMMATRIX world;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;
  };

  struct CameraBufferType {
    DirectX::XMFLOAT3 cameraPosition;
    float padding;
  };

  struct LightBufferType {
    DirectX::XMFLOAT3 lightDirection;
    float padding;
  };

public:
  PbrShaderClass() = default;

  PbrShaderClass(const PbrShaderClass &) = delete;

  ~PbrShaderClass() = default;

public:
  bool Initialize(HWND);

  void Shutdown() override {}

  bool
  Render(int indexCount,
         const ShaderParameterContainer &parameterContainer) const override;

private:
  bool InitializeShader(HWND, WCHAR *, WCHAR *);

  void OutputShaderErrorMessage(ID3D10Blob *, HWND, WCHAR *);

  bool SetShaderParameters(DirectX::XMMATRIX &, DirectX::XMMATRIX &,
                           DirectX::XMMATRIX &, ID3D11ShaderResourceView *,
                           ID3D11ShaderResourceView *,
                           ID3D11ShaderResourceView *,
                           const DirectX::XMFLOAT3 &,
                           const DirectX::XMFLOAT3 &) const;

  void RenderShader(int) const;

private:
  Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;

  Microsoft::WRL::ComPtr<ID3D11InputLayout> m_layout;

  Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampleState;

  Microsoft::WRL::ComPtr<ID3D11Buffer> m_matrixBuffer;
  Microsoft::WRL::ComPtr<ID3D11Buffer> m_cameraBuffer;
  Microsoft::WRL::ComPtr<ID3D11Buffer> m_lightBuffer;
};

#endif