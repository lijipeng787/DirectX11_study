#include "simplelightshader.h"

#include <d3dcompiler.h>

#include "ShaderParameterContainer.h"

using namespace DirectX;

bool SimpleLightShader::Initialize(HWND hwnd, ID3D11Device *device) {

  // Define vertex input layout
  D3D11_INPUT_ELEMENT_DESC polygonLayout[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
       D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}};

  // Initialize base shader components
  if (!InitializeShaderFromFile(hwnd, L"./simplelight.vs",
                                "SimpleLightVertexShader", L"./simplelight.ps",
                                "SimpleLightPixelShader", polygonLayout,
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

  return true;
}

bool SimpleLightShader::Render(int indexCount,
                                const ShaderParameterContainer &parameters,
                                ID3D11DeviceContext *deviceContext) const {

  auto worldMatrix = parameters.GetMatrix("worldMatrix");
  auto viewMatrix = parameters.GetMatrix("viewMatrix");
  auto projectionMatrix = parameters.GetMatrix("projectionMatrix");

  auto texture = parameters.GetTexture("texture");
  
  auto ambientColor = parameters.GetVector4("ambientColor");
  auto diffuseColor = parameters.GetVector4("diffuseColor");
  auto lightDirection = parameters.GetVector3("lightDirection");

  if (!SetShaderParameters(worldMatrix, viewMatrix, projectionMatrix, texture,
                           lightDirection, ambientColor, diffuseColor,
                           deviceContext)) {
    return false;
  }

  // Set vertex shader and input layout
  deviceContext->IASetInputLayout(layout_.Get());

  deviceContext->VSSetShader(vertex_shader_.Get(), NULL, 0);

  deviceContext->PSSetShader(pixel_shader_.Get(), NULL, 0);

  deviceContext->PSSetSamplers(0, 1, sampler_state_.GetAddressOf());

  deviceContext->DrawIndexed(indexCount, 0, 0);

  return true;
}

bool SimpleLightShader::SetShaderParameters(
    const DirectX::XMMATRIX &worldMatrix, const DirectX::XMMATRIX &viewMatrix,
    const DirectX::XMMATRIX &projectionMatrix,
    ID3D11ShaderResourceView *texture, const DirectX::XMFLOAT3 &lightDirection,
    const DirectX::XMFLOAT4 &ambientColor, const DirectX::XMFLOAT4 &diffuseColor,
    ID3D11DeviceContext *deviceContext) const {

  D3D11_MAPPED_SUBRESOURCE mappedResource;

  // Lock the matrix constant buffer
  auto result = deviceContext->Map(matrix_buffer_.Get(), 0,
                                    D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  MatrixBufferType *dataPtr;
  dataPtr = (MatrixBufferType *)mappedResource.pData;

  XMMATRIX worldMatrixCopy = XMMatrixTranspose(worldMatrix);
  XMMATRIX viewMatrixCopy = XMMatrixTranspose(viewMatrix);
  XMMATRIX projectionMatrixCopy = XMMatrixTranspose(projectionMatrix);

  dataPtr->world = worldMatrixCopy;
  dataPtr->view = viewMatrixCopy;
  dataPtr->projection = projectionMatrixCopy;

  deviceContext->Unmap(matrix_buffer_.Get(), 0);

  unsigned int buffer_number = 0;

  deviceContext->VSSetConstantBuffers(buffer_number, 1,
                                       matrix_buffer_.GetAddressOf());

  // Lock the light constant buffer
  result = deviceContext->Map(light_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD,
                              0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  LightBufferType *lightDataPtr;
  lightDataPtr = (LightBufferType *)mappedResource.pData;

  lightDataPtr->ambientColor = ambientColor;
  lightDataPtr->diffuseColor = diffuseColor;
  lightDataPtr->lightDirection = lightDirection;
  lightDataPtr->padding = 0.0f;

  deviceContext->Unmap(light_buffer_.Get(), 0);

  buffer_number = 0;

  deviceContext->PSSetConstantBuffers(buffer_number, 1,
                                       light_buffer_.GetAddressOf());

  // Set shader texture resource
  deviceContext->PSSetShaderResources(0, 1, &texture);

  return true;
}

