#include "horizontalblurshaderclass.h"
#include "../CommonFramework2/DirectX11Device.h"

#include <d3dcompiler.h>
#include <fstream>

using namespace std;
using namespace DirectX;

bool HorizontalBlurShader::Initialize(HWND hwnd) {
  return InitializeBlurShader(
      hwnd, L"./horizontalblur.vs", "HorizontalBlurVertexShader",
      L"./horizontalblur.ps", "HorizontalBlurPixelShader");
}

bool HorizontalBlurShader::Render(
    int indexCount, const ShaderParameterContainer &parameters) const {
  auto worldMatrix = parameters.GetMatrix("worldMatrix");
  auto viewMatrix = parameters.GetMatrix("baseViewMatrix");
  auto projectionMatrix = parameters.GetMatrix("orthoMatrix");
  auto screenWidth = parameters.GetFloat("screenWidth");
  auto texture = parameters.GetTexture("texture");

  if (!SetBaseShaderParameters(worldMatrix, viewMatrix, projectionMatrix,
                               texture, screenWidth)) {
    return false;
  }

  RenderShader(indexCount);
  return true;
}