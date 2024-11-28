#include "textureshader.h"

#include "../CommonFramework2/DirectX11Device.h"
#include "ShaderParameterContainer.h"

#include <d3dcompiler.h>
#include <fstream>

using namespace std;
using namespace DirectX;

bool TextureShader::Initialize(HWND hwnd) { return InitializeShader(hwnd); }

bool TextureShader::InitializeShader(HWND hwnd) {
  // Define input layout
  D3D11_INPUT_ELEMENT_DESC polygonLayout[2] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
       D3D11_INPUT_PER_VERTEX_DATA, 0}};

  // Initialize base shader components
  if (!InitializeShaderFromFile(hwnd, L"./texture.vs", "TextureVertexShader",
                                L"./texture.ps", "TexturePixelShader",
                                polygonLayout, _countof(polygonLayout))) {
    return false;
  }

  // Create matrix buffer
  if (!CreateConstantBuffer(sizeof(MatrixBufferType),
                            matrix_buffer_.GetAddressOf())) {
    return false;
  }

  // Create sampler state
  if (!CreateSamplerState(sampler_state_.GetAddressOf())) {
    return false;
  }

  return true;
}

bool TextureShader::Render(int indexCount,
                           const ShaderParameterContainer &parameters) const {
  auto worldMatrix = parameters.GetMatrix("deviceWorldMatrix");
  auto viewMatrix = parameters.GetMatrix("baseViewMatrix");
  auto orthoMatrix = parameters.GetMatrix("orthoMatrix");
  auto texture = parameters.GetTexture("texture");

  if (!SetShaderParameters(worldMatrix, viewMatrix, orthoMatrix, texture)) {
    return false;
  }

  RenderShader(indexCount);
  return true;
}

bool TextureShader::SetShaderParameters(
    const DirectX::XMMATRIX &worldMatrix, const DirectX::XMMATRIX &viewMatrix,
    const DirectX::XMMATRIX &projectionMatrix,
    ID3D11ShaderResourceView *texture) const {
  D3D11_MAPPED_SUBRESOURCE mappedResource;

  auto deviceContext =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  DirectX::XMMATRIX worldMatrixCopy = DirectX::XMMatrixTranspose(worldMatrix);
  DirectX::XMMATRIX viewMatrixCopy = DirectX::XMMatrixTranspose(viewMatrix);
  DirectX::XMMATRIX projectionMatrixCopy =
      DirectX::XMMatrixTranspose(projectionMatrix);

  // Lock and update the matrix constant buffer
  auto result = deviceContext->Map(matrix_buffer_.Get(), 0,
                                   D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  auto dataPtr = static_cast<MatrixBufferType *>(mappedResource.pData);
  dataPtr->world = worldMatrixCopy;
  dataPtr->view = viewMatrixCopy;
  dataPtr->projection = projectionMatrixCopy;

  deviceContext->Unmap(matrix_buffer_.Get(), 0);

  // Set the matrix buffer in the vertex shader
  deviceContext->VSSetConstantBuffers(0, 1, matrix_buffer_.GetAddressOf());

  // Set the texture resource in the pixel shader
  deviceContext->PSSetShaderResources(0, 1, &texture);

  return true;
}

void TextureShader::RenderShader(int indexCount) const {
  auto deviceContext =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  // Set the vertex input layout
  deviceContext->IASetInputLayout(layout_.Get());

  // Set the vertex and pixel shaders
  deviceContext->VSSetShader(vertex_shader_.Get(), nullptr, 0);
  deviceContext->PSSetShader(pixel_shader_.Get(), nullptr, 0);

  // Set the sampler state in the pixel shader
  deviceContext->PSSetSamplers(0, 1, sampler_state_.GetAddressOf());

  // Render the geometry
  deviceContext->DrawIndexed(indexCount, 0, 0);
}