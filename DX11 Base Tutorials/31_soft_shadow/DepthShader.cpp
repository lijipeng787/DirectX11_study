#include "DepthShader.h"

#include <d3dcompiler.h>
#include <fstream>

using namespace std;
using namespace DirectX;

bool DepthShader::Initialize(HWND hwnd, ID3D11Device *device) {

  shader_name_ = "DepthShader";
  // Define the depth shader's input layout - only needs position
  D3D11_INPUT_ELEMENT_DESC polygonLayout[1] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
       D3D11_INPUT_PER_VERTEX_DATA, 0}};

  // Initialize shader files and create base shader components
  if (!InitializeShaderFromFile(
          hwnd, L"./depth.vs", "DepthVertexShader", L"./depth.ps",
          "DepthPixelShader", polygonLayout, _countof(polygonLayout), device)) {
    return false;
  }

  // Create the matrix constant buffer
  if (!CreateConstantBuffer(sizeof(MatrixBufferType),
                            matrix_buffer_.GetAddressOf(), device)) {
    return false;
  }

  return true;
}

bool DepthShader::Render(int indexCount,
                         const ShaderParameterContainer &parameters,
                         ID3D11DeviceContext *deviceContext) const {
  // Get all required matrices from parameters
  auto worldMatrix = parameters.GetMatrix("worldMatrix");
  auto viewMatrix = parameters.GetMatrix("lightViewMatrix");
  auto projectionMatrix = parameters.GetMatrix("lightProjectionMatrix");

  // Set the shader parameters
  if (!SetShaderParameters(worldMatrix, viewMatrix, projectionMatrix,
                           deviceContext)) {
    return false;
  }

  // Set the input layout
  deviceContext->IASetInputLayout(layout_.Get());

  // Set the vertex and pixel shaders
  deviceContext->VSSetShader(vertex_shader_.Get(), nullptr, 0);
  deviceContext->PSSetShader(pixel_shader_.Get(), nullptr, 0);

  // Draw the geometry
  deviceContext->DrawIndexed(indexCount, 0, 0);

  return true;
}

bool DepthShader::SetShaderParameters(
    const DirectX::XMMATRIX &worldMatrix, const DirectX::XMMATRIX &viewMatrix,
    const DirectX::XMMATRIX &projectionMatrix,
    ID3D11DeviceContext *deviceContext) const {
  D3D11_MAPPED_SUBRESOURCE mappedResource;

  // Transpose the matrices for shader
  DirectX::XMMATRIX worldMatrixCopy = DirectX::XMMatrixTranspose(worldMatrix);
  DirectX::XMMATRIX viewMatrixCopy = DirectX::XMMatrixTranspose(viewMatrix);
  DirectX::XMMATRIX projectionMatrixCopy =
      DirectX::XMMatrixTranspose(projectionMatrix);

  // Lock and update the constant buffer
  auto result = deviceContext->Map(matrix_buffer_.Get(), 0,
                                   D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

  if (FAILED(result)) {
    return false;
  }

  // Get a pointer to the buffer data and copy the matrices
  auto dataPtr = static_cast<MatrixBufferType *>(mappedResource.pData);
  dataPtr->world = worldMatrixCopy;
  dataPtr->view = viewMatrixCopy;
  dataPtr->projection = projectionMatrixCopy;

  deviceContext->Unmap(matrix_buffer_.Get(), 0);

  // Set the constant buffer in the vertex shader
  deviceContext->VSSetConstantBuffers(0, 1, matrix_buffer_.GetAddressOf());

  return true;
}
