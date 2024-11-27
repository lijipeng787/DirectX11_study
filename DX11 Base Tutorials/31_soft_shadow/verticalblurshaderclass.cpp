#include "verticalblurshaderclass.h"

#include "../CommonFramework2/DirectX11Device.h"

#include <d3dcompiler.h>
#include <fstream>

using namespace std;
using namespace DirectX;

bool VerticalBlurShader::Initialize(HWND hwnd) {
  return InitializeBlurShader(hwnd, L"./verticalblur.vs",
                              "VerticalBlurVertexShader", L"./verticalblur.ps",
                              "VerticalBlurPixelShader");
}

bool VerticalBlurShader::Render(
    int indexCount, const ShaderParameterContainer &parameters) const {
  auto worldMatrix = parameters.GetMatrix("worldMatrix");
  auto viewMatrix = parameters.GetMatrix("baseViewMatrix");
  auto projectionMatrix = parameters.GetMatrix("orthoMatrix");
  auto screenHeight = parameters.GetFloat("screenHeight");
  auto texture = parameters.GetTexture("texture");

  if (!SetBaseShaderParameters(worldMatrix, viewMatrix, projectionMatrix,
                               texture, screenHeight)) {
    return false;
  }

  RenderShader(indexCount);
  return true;
}