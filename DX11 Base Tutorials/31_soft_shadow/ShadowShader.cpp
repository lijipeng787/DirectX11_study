#include "shadowshader.h"

#include "ShaderParameterContainer.h"

#include <fstream>

using namespace DirectX;

bool ShadowShader::Initialize(HWND hwnd, ID3D11Device *device) {

  // Define the shader input layout
  D3D11_INPUT_ELEMENT_DESC polygonLayout[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
       D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}};

  // Initialize shader files
  if (!InitializeShaderFromFile(hwnd, L"./shadow.vs", "ShadowVertexShader",
                                L"./shadow.ps", "ShadowPixelShader",
                                polygonLayout, _countof(polygonLayout),
                                device)) {
    return false;
  }

  // Create matrix constant buffer
  if (!CreateConstantBuffer(sizeof(MatrixBufferType),
                            matrix_buffer_.GetAddressOf(), device)) {
    return false;
  }

  // Create light constant buffer
  if (!CreateConstantBuffer(sizeof(LightBufferType),
                            light_buffer_.GetAddressOf(), device)) {
    return false;
  }

  // Create sampler state with clamp addressing
  if (!CreateSamplerState(sampler_state_clamp_.GetAddressOf(), device,
                          D3D11_TEXTURE_ADDRESS_CLAMP)) {
    return false;
  }

  return true;
}

bool ShadowShader::Render(int indexCount,
                          const ShaderParameterContainer &parameters,
                          ID3D11DeviceContext *deviceContext) const {

  auto worldMatrix = parameters.GetMatrix("worldMatrix");
  auto viewMatrix = parameters.GetMatrix("viewMatrix");
  auto projectionMatrix = parameters.GetMatrix("projectionMatrix");

  auto lightViewMatrix = parameters.GetMatrix("lightViewMatrix");
  auto lightProjectionMatrix = parameters.GetMatrix("lightProjectionMatrix");
  auto lightPosition = parameters.GetVector3("lightPosition");

  auto depthMapTexture = parameters.GetTexture("depthMapTexture");

  if (!SetShaderParameters(worldMatrix, viewMatrix, projectionMatrix,
                           lightViewMatrix, lightProjectionMatrix,
                           depthMapTexture, lightPosition, deviceContext)) {
    return false;
  }

  // Set the vertex input layout
  deviceContext->IASetInputLayout(layout_.Get());

  // Set the vertex and pixel shaders
  deviceContext->VSSetShader(vertex_shader_.Get(), nullptr, 0);
  deviceContext->PSSetShader(pixel_shader_.Get(), nullptr, 0);

  // Set the sampler state
  deviceContext->PSSetSamplers(0, 1, sampler_state_clamp_.GetAddressOf());

  // Render the geometry
  deviceContext->DrawIndexed(indexCount, 0, 0);

  return true;
}

bool ShadowShader::SetShaderParameters(
    const DirectX::XMMATRIX &worldMatrix, const DirectX::XMMATRIX &viewMatrix,
    const DirectX::XMMATRIX &projectionMatrix,
    const DirectX::XMMATRIX &lightViewMatrix,
    const DirectX::XMMATRIX &lightProjectionMatrix,
    ID3D11ShaderResourceView *depthMapTexture,
    const DirectX::XMFLOAT3 &lightPosition,
    ID3D11DeviceContext *deviceContext) const {

  D3D11_MAPPED_SUBRESOURCE mappedResource;

  // Transpose matrices for shader
  auto worldMatrixT = XMMatrixTranspose(worldMatrix);
  auto viewMatrixT = XMMatrixTranspose(viewMatrix);
  auto projectionMatrixT = XMMatrixTranspose(projectionMatrix);
  auto lightViewMatrixT = XMMatrixTranspose(lightViewMatrix);
  auto lightProjectionMatrixT = XMMatrixTranspose(lightProjectionMatrix);

  // Update matrix buffer
  auto result = deviceContext->Map(matrix_buffer_.Get(), 0,
                                   D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  auto matrixDataPtr = static_cast<MatrixBufferType *>(mappedResource.pData);
  matrixDataPtr->world = worldMatrixT;
  matrixDataPtr->view = viewMatrixT;
  matrixDataPtr->projection = projectionMatrixT;
  matrixDataPtr->lightView = lightViewMatrixT;
  matrixDataPtr->lightProjection = lightProjectionMatrixT;

  deviceContext->Unmap(matrix_buffer_.Get(), 0);
  deviceContext->VSSetConstantBuffers(0, 1, matrix_buffer_.GetAddressOf());

  // Update light buffer
  result = deviceContext->Map(light_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD,
                              0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  auto lightDataPtr = static_cast<LightBufferType *>(mappedResource.pData);
  lightDataPtr->lightPosition = lightPosition;
  lightDataPtr->padding = 0.0f;

  deviceContext->Unmap(light_buffer_.Get(), 0);
  deviceContext->VSSetConstantBuffers(1, 1, light_buffer_.GetAddressOf());

  // Set shader resources
  deviceContext->PSSetShaderResources(0, 1, &depthMapTexture);

  return true;
}
