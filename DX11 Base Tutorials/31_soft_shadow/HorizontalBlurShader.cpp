#include "HorizontalBlurShader.h"

#include <d3dcompiler.h>
#include <fstream>

using namespace std;
using namespace DirectX;

bool HorizontalBlurShader::Initialize(HWND hwnd, ID3D11Device *device) {
  shader_name_ = "HorizontalBlurShader";
  return InitializeBlurShader(
    hwnd, L"./shaders/horizontalblur.vs", "HorizontalBlurVertexShader",
    L"./shaders/horizontalblur.ps", "HorizontalBlurPixelShader", device);
}

bool HorizontalBlurShader::Render(int indexCount,
                                  const ShaderParameterContainer &parameters,
                                  ID3D11DeviceContext *deviceContext) const {

  auto worldMatrix = parameters.GetMatrix("worldMatrix");
  auto viewMatrix = parameters.GetMatrix("baseViewMatrix");
  auto projectionMatrix = parameters.GetMatrix("orthoMatrix");

  auto screenWidth = parameters.GetFloat("screenWidth");
  auto texture = parameters.GetTexture("texture");

  if (!SetBaseShaderParameters(worldMatrix, viewMatrix, projectionMatrix,
                               texture, screenWidth, deviceContext)) {
    return false;
  }

  RenderShader(indexCount, deviceContext);
  return true;
}