#include "WaterShader.h"

#include <d3d11.h>

using namespace DirectX;

bool WaterShader::Initialize(HWND hwnd, ID3D11Device *device) {

  D3D11_INPUT_ELEMENT_DESC polygonLayout[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
       D3D11_INPUT_PER_VERTEX_DATA, 0}};

  if (!InitializeShaderFromFile(
          hwnd, L"./water.vs", "WaterVertexShader", L"./water.ps",
          "WaterPixelShader", polygonLayout, _countof(polygonLayout), device)) {
    return false;
  }

  if (!CreateConstantBuffer(sizeof(MatrixBufferType),
                            matrix_buffer_.GetAddressOf(), device)) {
    return false;
  }

  if (!CreateConstantBuffer(sizeof(ReflectionBufferType),
                            reflection_buffer_.GetAddressOf(), device)) {
    return false;
  }

  if (!CreateConstantBuffer(sizeof(WaterBufferType),
                            water_buffer_.GetAddressOf(), device)) {
    return false;
  }

  if (!CreateSamplerState(sampler_state_.GetAddressOf(), device)) {
    return false;
  }

  return true;
}

void WaterShader::Shutdown() { ShaderBase::Shutdown(); }

bool WaterShader::Render(int indexCount,
                         const ShaderParameterContainer &parameters,
                         ID3D11DeviceContext *deviceContext) const {

  auto worldMatrix = parameters.GetMatrix("worldMatrix");
  auto viewMatrix = parameters.GetMatrix("viewMatrix");
  auto projectionMatrix = parameters.GetMatrix("projectionMatrix");
  auto reflectionMatrix = parameters.GetMatrix("waterReflectionMatrix");

  auto reflectionTexture = parameters.GetTexture("reflectionTexture");
  auto refractionTexture = parameters.GetTexture("refractionTexture");
  auto normalTexture = parameters.GetTexture("normalTexture");

  auto waterTranslation = parameters.GetFloat("waterTranslation");
  auto reflectRefractScale = parameters.GetFloat("reflectRefractScale");

  if (!SetShaderParameters(worldMatrix, viewMatrix, projectionMatrix,
                           reflectionMatrix, reflectionTexture,
                           refractionTexture, normalTexture, waterTranslation,
                           reflectRefractScale, deviceContext)) {
    return false;
  }

  deviceContext->IASetInputLayout(layout_.Get());
  deviceContext->VSSetShader(vertex_shader_.Get(), nullptr, 0);
  deviceContext->PSSetShader(pixel_shader_.Get(), nullptr, 0);
  deviceContext->PSSetSamplers(0, 1, sampler_state_.GetAddressOf());
  deviceContext->DrawIndexed(indexCount, 0, 0);

  return true;
}

bool WaterShader::SetShaderParameters(
    const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix,
    const XMMATRIX &projectionMatrix, const XMMATRIX &reflectionMatrix,
    ID3D11ShaderResourceView *reflectionTexture,
    ID3D11ShaderResourceView *refractionTexture,
    ID3D11ShaderResourceView *normalTexture, float waterTranslation,
    float reflectRefractScale, ID3D11DeviceContext *deviceContext) const {

  D3D11_MAPPED_SUBRESOURCE mappedResource;

  auto worldT = XMMatrixTranspose(worldMatrix);
  auto viewT = XMMatrixTranspose(viewMatrix);
  auto projectionT = XMMatrixTranspose(projectionMatrix);

  auto result = deviceContext->Map(matrix_buffer_.Get(), 0,
                                   D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  auto matrixPtr = static_cast<MatrixBufferType *>(mappedResource.pData);
  matrixPtr->world = worldT;
  matrixPtr->view = viewT;
  matrixPtr->projection = projectionT;

  deviceContext->Unmap(matrix_buffer_.Get(), 0);
  deviceContext->VSSetConstantBuffers(0, 1, matrix_buffer_.GetAddressOf());

  auto reflectionT = XMMatrixTranspose(reflectionMatrix);

  result = deviceContext->Map(reflection_buffer_.Get(), 0,
                              D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  auto reflectionPtr =
      static_cast<ReflectionBufferType *>(mappedResource.pData);
  reflectionPtr->reflectionMatrix = reflectionT;

  deviceContext->Unmap(reflection_buffer_.Get(), 0);
  deviceContext->VSSetConstantBuffers(1, 1, reflection_buffer_.GetAddressOf());

  result = deviceContext->Map(water_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD,
                              0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  auto waterPtr = static_cast<WaterBufferType *>(mappedResource.pData);
  waterPtr->waterTranslation = waterTranslation;
  waterPtr->reflectRefractScale = reflectRefractScale;
  waterPtr->padding = XMFLOAT2(0.0f, 0.0f);

  deviceContext->Unmap(water_buffer_.Get(), 0);
  deviceContext->PSSetConstantBuffers(0, 1, water_buffer_.GetAddressOf());

  ID3D11ShaderResourceView *textures[] = {reflectionTexture, refractionTexture,
                                          normalTexture};
  deviceContext->PSSetShaderResources(0, _countof(textures), textures);

  return true;
}
