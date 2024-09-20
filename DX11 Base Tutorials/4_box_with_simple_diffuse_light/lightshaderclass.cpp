#include "LightShaderClass.h"

#include <d3dcompiler.h>
#include <fstream>
#include <stdexcept>

#include "../CommonFramework/DirectX11Device.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace {
struct MatrixBufferType {
  XMMATRIX world;
  XMMATRIX view;
  XMMATRIX projection;
};

struct LightBufferType {
  XMFLOAT4 diffuseColor;
  XMFLOAT3 lightDirection;
  float padding;
};
} // namespace

bool LightShaderClass::Initialize(HWND hwnd) {
  return InitializeShader(hwnd, L"simple_light.hlsl", L"simple_light.hlsl");
}

void LightShaderClass::Shutdown() { ShutdownShader(); }

bool LightShaderClass::Render(int indexCount,
                              const DirectX::XMMATRIX &worldMatrix,
                              const DirectX::XMMATRIX &viewMatrix,
                              const DirectX::XMMATRIX &projectionMatrix,
                              ID3D11ShaderResourceView *texture,
                              const DirectX::XMFLOAT3 &lightDirection,
                              const DirectX::XMFLOAT4 &diffuseColor) const {
  if (!SetShaderParameters(worldMatrix, viewMatrix, projectionMatrix, texture,
                           lightDirection, diffuseColor)) {
    return false;
  }

  RenderShader(indexCount);
  return true;
}

bool LightShaderClass::SetShaderParameters(
    const DirectX::XMMATRIX &worldMatrix, const DirectX::XMMATRIX &viewMatrix,
    const DirectX::XMMATRIX &projectionMatrix,
    ID3D11ShaderResourceView *texture, const DirectX::XMFLOAT3 &lightDirection,
    const DirectX::XMFLOAT4 &diffuseColor) const {

  auto deviceContext =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  D3D11_MAPPED_SUBRESOURCE mappedResource;
  HRESULT result =
      deviceContext->Map(resources_.matrix_buffer_.Get(), 0,
                         D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  auto dataPtr = static_cast<MatrixBufferType *>(mappedResource.pData);
  dataPtr->world = XMMatrixTranspose(worldMatrix);
  dataPtr->view = XMMatrixTranspose(viewMatrix);
  dataPtr->projection = XMMatrixTranspose(projectionMatrix);

  deviceContext->Unmap(resources_.matrix_buffer_.Get(), 0);
  deviceContext->VSSetConstantBuffers(0, 1,
                                      resources_.matrix_buffer_.GetAddressOf());

  // Set the texture
  deviceContext->PSSetShaderResources(0, 1, &texture);

  // Set the light buffer
  result = deviceContext->Map(resources_.light_buffer_.Get(), 0,
                              D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  auto dataPtr2 = static_cast<LightBufferType *>(mappedResource.pData);
  dataPtr2->diffuseColor = diffuseColor;
  dataPtr2->lightDirection = lightDirection;
  dataPtr2->padding = 0.0f;

  deviceContext->Unmap(resources_.light_buffer_.Get(), 0);
  deviceContext->PSSetConstantBuffers(0, 1,
                                      resources_.light_buffer_.GetAddressOf());

  return true;
}

bool LightShaderClass::InitializeShader(HWND hwnd,
                                        const std::wstring &vsFilename,
                                        const std::wstring &psFilename) {
  ComPtr<ID3D10Blob> errorMessage;
  ComPtr<ID3D10Blob> vertexShaderBuffer;
  ComPtr<ID3D10Blob> pixelShaderBuffer;

  // Compile the vertex shader
  HRESULT result = D3DCompileFromFile(
      vsFilename.c_str(), nullptr, nullptr, "LightVertexShader", "vs_5_0",
      D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage);
  if (FAILED(result)) {
    if (errorMessage) {
      OutputShaderErrorMessage(errorMessage.Get(), hwnd, vsFilename);
    } else {
      MessageBox(hwnd, vsFilename.c_str(), L"Missing Shader File", MB_OK);
    }
    return false;
  }

  // Compile the pixel shader
  result = D3DCompileFromFile(
      psFilename.c_str(), nullptr, nullptr, "LightPixelShader", "ps_5_0",
      D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage);
  if (FAILED(result)) {
    if (errorMessage) {
      OutputShaderErrorMessage(errorMessage.Get(), hwnd, psFilename);
    } else {
      MessageBox(hwnd, psFilename.c_str(), L"Missing Shader File", MB_OK);
    }
    return false;
  }

  // Create the vertex shader
  auto device = DirectX11Device::GetD3d11DeviceInstance()->GetDevice();

  result =
      device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
                                 vertexShaderBuffer->GetBufferSize(), nullptr,
                                 resources_.vertex_shader_.GetAddressOf());
  if (FAILED(result)) {
    return false;
  }

  // Create the pixel shader
  result = device->CreatePixelShader(
      pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(),
      nullptr, resources_.pixel_shader_.GetAddressOf());
  if (FAILED(result)) {
    return false;
  }

  // Create the vertex input layout
  D3D11_INPUT_ELEMENT_DESC polygonLayout[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
       D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
  };

  unsigned int numElements = ARRAYSIZE(polygonLayout);

  result = device->CreateInputLayout(
      polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
      vertexShaderBuffer->GetBufferSize(), resources_.layout_.GetAddressOf());
  if (FAILED(result)) {
    return false;
  }

  // Create the texture sampler state
  D3D11_SAMPLER_DESC samplerDesc = {};
  samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.MipLODBias = 0.0f;
  samplerDesc.MaxAnisotropy = 1;
  samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
  samplerDesc.MinLOD = 0;
  samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

  result = device->CreateSamplerState(&samplerDesc,
                                      resources_.sampler_state_.GetAddressOf());
  if (FAILED(result)) {
    return false;
  }

  // Create the matrix constant buffer
  D3D11_BUFFER_DESC matrixBufferDesc = {};
  matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
  matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
  matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

  result = device->CreateBuffer(&matrixBufferDesc, nullptr,
                                resources_.matrix_buffer_.GetAddressOf());
  if (FAILED(result)) {
    return false;
  }

  // Create the light constant buffer
  D3D11_BUFFER_DESC lightBufferDesc = {};
  lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
  lightBufferDesc.ByteWidth = sizeof(LightBufferType);
  lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

  result = device->CreateBuffer(&lightBufferDesc, nullptr,
                                resources_.light_buffer_.GetAddressOf());
  if (FAILED(result)) {
    return false;
  }

  return true;
}

void LightShaderClass::ShutdownShader() { resources_ = {}; }

void LightShaderClass::OutputShaderErrorMessage(
    ID3D10Blob *errorMessage, HWND hwnd, const std::wstring &shaderFilename) {
  char *compileErrors = static_cast<char *>(errorMessage->GetBufferPointer());
  size_t bufferSize = errorMessage->GetBufferSize();

  std::ofstream fout("shader-error.txt");
  for (size_t i = 0; i < bufferSize; i++) {
    fout << compileErrors[i];
  }
  fout.close();

  MessageBox(hwnd,
             L"Error compiling shader. Check shader-error.txt for message.",
             shaderFilename.c_str(), MB_OK);
}

void LightShaderClass::RenderShader(int indexCount) const {
  auto deviceContext =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  deviceContext->IASetInputLayout(resources_.layout_.Get());
  deviceContext->VSSetShader(resources_.vertex_shader_.Get(), nullptr, 0);
  deviceContext->PSSetShader(resources_.pixel_shader_.Get(), nullptr, 0);
  deviceContext->PSSetSamplers(0, 1, resources_.sampler_state_.GetAddressOf());
  deviceContext->DrawIndexed(indexCount, 0, 0);
}