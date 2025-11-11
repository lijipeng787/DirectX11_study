#include "VerticalBlurShader.h"

#include <d3dcompiler.h>
#include <fstream>

using namespace std;
using namespace DirectX;

bool VerticalBlurShader::Initialize(HWND hwnd, ID3D11Device *device) {
  shader_name_ = "VerticalBlurShader";
  return InitializeBlurShader(hwnd, L"./shaders/verticalblur.vs",
                              "VerticalBlurVertexShader", L"./shaders/verticalblur.ps",
                              "VerticalBlurPixelShader", device);
}

bool VerticalBlurShader::Render(int indexCount,
                                const ShaderParameterContainer &parameters,
                                ID3D11DeviceContext *deviceContext) const {

  auto worldMatrix = parameters.GetMatrix("worldMatrix");
  auto viewMatrix = parameters.GetMatrix("baseViewMatrix");
  auto projectionMatrix = parameters.GetMatrix("orthoMatrix");

  auto screenHeight = parameters.GetFloat("screenHeight");
  auto texture = parameters.GetTexture("texture");

  if (!SetBaseShaderParameters(worldMatrix, viewMatrix, projectionMatrix,
                               texture, screenHeight, deviceContext)) {
    return false;
  }

  RenderShader(indexCount, deviceContext);
  return true;
}