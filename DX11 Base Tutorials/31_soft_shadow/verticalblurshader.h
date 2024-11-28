#ifndef _VERTICALBLURSHADERCLASS_H_
#define _VERTICALBLURSHADERCLASS_H_

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>

#include "ShaderBase.h"

class VerticalBlurShader : public BlurShaderBase {
public:
  VerticalBlurShader() = default;

  ~VerticalBlurShader() = default;

public:
  bool Initialize(HWND hwnd) override;

  bool Render(int indexCount,
              const ShaderParameterContainer &parameters) const override;
};

#endif