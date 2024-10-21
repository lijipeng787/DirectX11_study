#ifndef _ORTHOWINDOWCLASS_H_
#define _ORTHOWINDOWCLASS_H_

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl\client.h>

#include "IRenderable.h"

class OrthoWindowClass : public IRenderable {
private:
  struct VertexType {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT2 texture;
  };

public:
  OrthoWindowClass() = default;

  OrthoWindowClass(const OrthoWindowClass &) = delete;

  ~OrthoWindowClass() = default;

public:
  bool Initialize(int, int);

  void Shutdown();

  void
  Render(const IShader &shader,
         const ShaderParameterContainer &parameterContainer) const override;

  void SetParameterCallback(ShaderParameterCallback callback) override;

  ShaderParameterCallback GetParameterCallback() const override;

  int GetIndexCount() const;

  DirectX::XMMATRIX GetWorldMatrix() const override {
    return device_world_matrix_;
  }

private:
  bool InitializeBuffers(int, int);

  void ShutdownBuffers();

  void RenderBuffers() const;

private:
  Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer_, index_buffer_;
  int vertex_count_, index_count_;
  DirectX::XMMATRIX device_world_matrix_;
};

#endif