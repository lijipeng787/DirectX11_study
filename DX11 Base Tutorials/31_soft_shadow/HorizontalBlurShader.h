#ifndef _HORIZONTALBLURSHADERCLASS_H_
#define _HORIZONTALBLURSHADERCLASS_H_

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>

#include "ShaderBase.h"

class HorizontalBlurShader : public BlurShaderBase {
public:
  HorizontalBlurShader() = default;

  ~HorizontalBlurShader() = default;

public:
  bool Initialize(HWND hwnd, ID3D11Device *device) override;

  bool Render(int indexCount, const ShaderParameterContainer &parameters,
              ID3D11DeviceContext *deviceContext) const override;
};

#endif