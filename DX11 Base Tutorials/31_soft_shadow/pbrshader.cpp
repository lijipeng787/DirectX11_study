#include "pbrshader.h"

#include <d3dcompiler.h>
#include <fstream>

using namespace DirectX;
using namespace std;

bool PbrShader::Initialize(HWND hwnd, ID3D11Device *device) {

  // Define vertex input layout for PBR rendering
  D3D11_INPUT_ELEMENT_DESC polygonLayout[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
       D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
       D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
       D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}};

  // Initialize base shader components
  if (!InitializeShaderFromFile(hwnd, L"./pbr.vs", "PbrVertexShader",
                                L"./pbr.ps", "PbrPixelShader", polygonLayout,
                                _countof(polygonLayout), device)) {
    return false;
  }

  // Create constant buffers
  if (!CreateConstantBuffer(sizeof(MatrixBufferType),
                            matrix_buffer_.GetAddressOf(), device)) {
    return false;
  }

  if (!CreateConstantBuffer(sizeof(CameraBufferType),
                            camera_buffer_.GetAddressOf(), device)) {
    return false;
  }

  if (!CreateConstantBuffer(sizeof(LightBufferType),
                            light_buffer_.GetAddressOf(), device)) {
    return false;
  }

  // Create sampler state for PBR textures
  if (!CreateSamplerState(sampler_state_.GetAddressOf(), device)) {
    return false;
  }

  return true;
}

bool PbrShader::Render(int indexCount,
                       const ShaderParameterContainer &parameters,
                       ID3D11DeviceContext *deviceContext) const {

  // Get required parameters from container
  auto worldMatrix = parameters.GetMatrix("worldMatrix");
  auto viewMatrix = parameters.GetMatrix("viewMatrix");
  auto projectionMatrix = parameters.GetMatrix("projectionMatrix");

  auto diffuseTexture = parameters.GetTexture("diffuseTexture");
  auto normalMap = parameters.GetTexture("normalMap");
  auto rmTexture = parameters.GetTexture("rmTexture");

  auto lightDirection = parameters.GetVector3("lightDirection");
  auto cameraPosition = parameters.GetVector3("cameraPosition");

  // Set shader parameters
  if (!SetShaderParameters(worldMatrix, viewMatrix, projectionMatrix,
                           diffuseTexture, normalMap, rmTexture, lightDirection,
                           cameraPosition, deviceContext)) {
    return false;
  }

  // Set the vertex input layout
  deviceContext->IASetInputLayout(layout_.Get());

  // Set the vertex and pixel shaders
  deviceContext->VSSetShader(vertex_shader_.Get(), nullptr, 0);
  deviceContext->PSSetShader(pixel_shader_.Get(), nullptr, 0);

  // Draw the geometry
  deviceContext->DrawIndexed(indexCount, 0, 0);

  return true;
}

bool PbrShader::SetShaderParameters(
    const DirectX::XMMATRIX &worldMatrix, const DirectX::XMMATRIX &viewMatrix,
    const DirectX::XMMATRIX &projectionMatrix,
    ID3D11ShaderResourceView *albedoTexture,
    ID3D11ShaderResourceView *normalMap,
    ID3D11ShaderResourceView *roughnessMetallicTexture,
    const DirectX::XMFLOAT3 &lightDirection,
    const DirectX::XMFLOAT3 &cameraPosition,
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

  // Update camera buffer
  result = deviceContext->Map(camera_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD,
                              0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  auto cameraDataPtr = static_cast<CameraBufferType *>(mappedResource.pData);
  cameraDataPtr->cameraPosition = cameraPosition;
  cameraDataPtr->padding = 0.0f;

  deviceContext->Unmap(camera_buffer_.Get(), 0);
  deviceContext->VSSetConstantBuffers(1, 1, camera_buffer_.GetAddressOf());

  // Update light buffer
  result = deviceContext->Map(light_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD,
                              0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  auto lightDataPtr = static_cast<LightBufferType *>(mappedResource.pData);
  lightDataPtr->lightDirection = lightDirection;
  lightDataPtr->padding = 0.0f;

  deviceContext->Unmap(light_buffer_.Get(), 0);
  deviceContext->PSSetConstantBuffers(0, 1, light_buffer_.GetAddressOf());

  // Set PBR material textures
  ID3D11ShaderResourceView *textures[3] = {albedoTexture, normalMap,
                                           roughnessMetallicTexture};
  deviceContext->PSSetShaderResources(0, 3, textures);

  // Set sampler state
  deviceContext->PSSetSamplers(0, 1, sampler_state_.GetAddressOf());

  return true;
}
