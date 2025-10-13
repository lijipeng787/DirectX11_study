#include "softshadowshader.h"

#include <d3dcompiler.h>
#include <fstream>

using namespace std;
using namespace DirectX;

bool SoftShadowShader::Initialize(HWND hwnd, ID3D11Device *device) {

  // Define vertex input layout
  D3D11_INPUT_ELEMENT_DESC polygonLayout[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
       D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}};

  // Initialize base shader components
  if (!InitializeShaderFromFile(hwnd, L"./softshadow.vs",
                                "SoftShadowVertexShader", L"./softshadow.ps",
                                "SoftShadowPixelShader", polygonLayout,
                                _countof(polygonLayout), device)) {
    return false;
  }

  // Create constant buffers
  if (!CreateConstantBuffer(sizeof(MatrixBufferType),
                            matrix_buffer_.GetAddressOf(), device)) {
    return false;
  }

  if (!CreateConstantBuffer(sizeof(LightBufferType),
                            light_buffer_.GetAddressOf(), device)) {
    return false;
  }

  if (!CreateConstantBuffer(sizeof(LightPositionBufferType),
                            light_position_buffer_.GetAddressOf(), device)) {
    return false;
  }

  // Create wrap sampler state for regular textures
  if (!CreateSamplerState(sampler_state_wrap_.GetAddressOf(), device,
                          D3D11_TEXTURE_ADDRESS_WRAP)) {
    return false;
  }

  // Create clamp sampler state for shadow map
  if (!CreateSamplerState(sampler_state_clamp_.GetAddressOf(), device,
                          D3D11_TEXTURE_ADDRESS_CLAMP)) {
    return false;
  }

  return true;
}

bool SoftShadowShader::Render(int indexCount,
                              const ShaderParameterContainer &parameters,
                              ID3D11DeviceContext *deviceContext) const {

  auto worldMatrix = parameters.GetMatrix("worldMatrix");
  auto viewMatrix = parameters.GetMatrix("viewMatrix");
  auto projectionMatrix = parameters.GetMatrix("projectionMatrix");

  auto texture = parameters.GetTexture("texture");
  auto shadowTexture = parameters.GetTexture("shadowTexture");

  auto ambientColor = parameters.GetVector4("ambientColor");
  auto diffuseColor = parameters.GetVector4("diffuseColor");

  auto lightPosition = parameters.GetVector3("lightPosition");

  if (!SetShaderParameters(worldMatrix, viewMatrix, projectionMatrix, texture,
                           shadowTexture, lightPosition, ambientColor,
                           diffuseColor, deviceContext)) {
    return false;
  }

  // Set the vertex input layout
  deviceContext->IASetInputLayout(layout_.Get());

  // Set the vertex and pixel shaders
  deviceContext->VSSetShader(vertex_shader_.Get(), nullptr, 0);
  deviceContext->PSSetShader(pixel_shader_.Get(), nullptr, 0);

  // Set the sampler states
  deviceContext->PSSetSamplers(0, 1, sampler_state_clamp_.GetAddressOf());
  deviceContext->PSSetSamplers(1, 1, sampler_state_wrap_.GetAddressOf());

  // Draw the geometry
  deviceContext->DrawIndexed(indexCount, 0, 0);

  return true;
}

bool SoftShadowShader::SetShaderParameters(
    const DirectX::XMMATRIX &worldMatrix, const DirectX::XMMATRIX &viewMatrix,
    const DirectX::XMMATRIX &projectionMatrix,
    ID3D11ShaderResourceView *texture, ID3D11ShaderResourceView *shadowTexture,
    const DirectX::XMFLOAT3 &lightPosition,
    const DirectX::XMFLOAT4 &ambientColor,
    const DirectX::XMFLOAT4 &diffuseColor,
    ID3D11DeviceContext *deviceContext) const {

  D3D11_MAPPED_SUBRESOURCE mappedResource;

  // Update matrix buffer
  DirectX::XMMATRIX worldMatrixT = XMMatrixTranspose(worldMatrix);
  DirectX::XMMATRIX viewMatrixT = XMMatrixTranspose(viewMatrix);
  DirectX::XMMATRIX projectionMatrixT = XMMatrixTranspose(projectionMatrix);

  auto result = deviceContext->Map(matrix_buffer_.Get(), 0,
                                   D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  auto matrixDataPtr = static_cast<MatrixBufferType *>(mappedResource.pData);
  matrixDataPtr->world = worldMatrixT;
  matrixDataPtr->view = viewMatrixT;
  matrixDataPtr->projection = projectionMatrixT;

  deviceContext->Unmap(matrix_buffer_.Get(), 0);
  deviceContext->VSSetConstantBuffers(0, 1, matrix_buffer_.GetAddressOf());

  // Update light buffer
  result = deviceContext->Map(light_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD,
                              0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  auto lightDataPtr = static_cast<LightBufferType *>(mappedResource.pData);
  lightDataPtr->ambientColor = ambientColor;
  lightDataPtr->diffuseColor = diffuseColor;

  deviceContext->Unmap(light_buffer_.Get(), 0);
  deviceContext->PSSetConstantBuffers(0, 1, light_buffer_.GetAddressOf());

  // Update light position buffer
  result = deviceContext->Map(light_position_buffer_.Get(), 0,
                              D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  auto lightPosDataPtr =
      static_cast<LightPositionBufferType *>(mappedResource.pData);
  lightPosDataPtr->lightPosition = lightPosition;
  lightPosDataPtr->padding = 0.0f;

  deviceContext->Unmap(light_position_buffer_.Get(), 0);
  deviceContext->VSSetConstantBuffers(1, 1,
                                      light_position_buffer_.GetAddressOf());

  // Set shader textures and samplers
  deviceContext->PSSetShaderResources(0, 1, &texture);
  deviceContext->PSSetShaderResources(1, 1, &shadowTexture);

  return true;
}
