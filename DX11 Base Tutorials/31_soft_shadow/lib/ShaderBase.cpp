#include "ShaderBase.h"

#include "../../CommonFramework2/DirectX11Device.h"
#include "Logger.h"
#include <d3dcompiler.h>
#include <fstream>
#include <iostream>

bool ShaderBase::Initialize(HWND hwnd, ID3D11Device *device) {
  // To be implemented by derived classes
  return true;
}

void ShaderBase::Shutdown() {
  vertex_shader_.Reset();
  pixel_shader_.Reset();
  layout_.Reset();
  sampler_state_.Reset();
}

bool ShaderBase::InitializeShaderFromFile(
    HWND hwnd, const std::wstring &vsFilename, const std::string &vsEntryName,
    const std::wstring &psFilename, const std::string &psEntryName,
    const D3D11_INPUT_ELEMENT_DESC *layoutDesc, UINT numElements,
    ID3D11Device *device) {

  HRESULT result;
  Microsoft::WRL::ComPtr<ID3D10Blob> errorMessage;
  Microsoft::WRL::ComPtr<ID3D10Blob> vertexShaderBuffer;
  Microsoft::WRL::ComPtr<ID3D10Blob> pixelShaderBuffer;

  // Compile vertex shader
  result = D3DCompileFromFile(
      vsFilename.c_str(), nullptr, nullptr, vsEntryName.c_str(), "vs_5_0",
      D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage);

  if (FAILED(result)) {
    if (errorMessage) {
      OutputShaderErrorMessage(errorMessage.Get(), hwnd, vsFilename);
    } else {
      Logger::SetModule("ShaderBase");
      Logger::LogError(L"Missing shader file: " + vsFilename);
    }
    return false;
  }

  // Compile pixel shader
  result = D3DCompileFromFile(
      psFilename.c_str(), nullptr, nullptr, psEntryName.c_str(), "ps_5_0",
      D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage);

  if (FAILED(result)) {
    if (errorMessage) {
      OutputShaderErrorMessage(errorMessage.Get(), hwnd, psFilename);
    } else {
      Logger::SetModule("ShaderBase");
      Logger::LogError(L"Missing shader file: " + psFilename);
    }
    return false;
  }

  // Create vertex shader
  result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
                                      vertexShaderBuffer->GetBufferSize(),
                                      nullptr, &vertex_shader_);

  if (FAILED(result)) {
    return false;
  }

  // Create pixel shader
  result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
                                     pixelShaderBuffer->GetBufferSize(),
                                     nullptr, &pixel_shader_);

  if (FAILED(result)) {
    return false;
  }

  reflected_parameters_ =
      ReflectShader(device, vertexShaderBuffer.Get(), pixelShaderBuffer.Get());

  // Create input layout
  result = device->CreateInputLayout(
      layoutDesc, numElements, vertexShaderBuffer->GetBufferPointer(),
      vertexShaderBuffer->GetBufferSize(), &layout_);

  if (FAILED(result)) {
    return false;
  }

  return true;
}

bool ShaderBase::CreateConstantBuffer(UINT byteWidth, ID3D11Buffer **buffer,
                                      ID3D11Device *device) {
  D3D11_BUFFER_DESC bufferDesc = {};
  bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
  bufferDesc.ByteWidth = byteWidth;
  bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

  return SUCCEEDED(device->CreateBuffer(&bufferDesc, nullptr, buffer));
}

bool ShaderBase::CreateSamplerState(ID3D11SamplerState **samplerState,
                                    ID3D11Device *device,
                                    D3D11_TEXTURE_ADDRESS_MODE addressMode) {
  D3D11_SAMPLER_DESC samplerDesc = {};
  samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  samplerDesc.AddressU = addressMode;
  samplerDesc.AddressV = addressMode;
  samplerDesc.AddressW = addressMode;
  samplerDesc.MipLODBias = 0.0f;
  samplerDesc.MaxAnisotropy = 1;
  samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
  samplerDesc.MinLOD = 0;
  samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

  return SUCCEEDED(device->CreateSamplerState(&samplerDesc, samplerState));
}

void ShaderBase::OutputShaderErrorMessage(ID3D10Blob *errorMessage, HWND hwnd,
                                          const std::wstring &shaderFilename) {
  (void)hwnd;
  char *compileErrors = static_cast<char *>(errorMessage->GetBufferPointer());
  SIZE_T bufferSize = errorMessage->GetBufferSize();

  std::ofstream fout("shader-error.txt");
  for (SIZE_T i = 0; i < bufferSize; i++) {
    fout << compileErrors[i];
  }
  fout.close();

  Logger::SetModule("ShaderBase");
  Logger::LogError(
      "Error compiling shader. Check shader-error.txt for message.");
  // Convert wstring to string properly to avoid warning
  std::string filenameStr;
  filenameStr.reserve(shaderFilename.length());
  for (wchar_t wc : shaderFilename) {
    filenameStr.push_back(static_cast<char>(wc));
  }
  Logger::LogError(filenameStr);
}

bool BlurShaderBase::InitializeBlurShader(HWND hwnd,
                                          const std::wstring &vsFilename,
                                          const std::string &vsEntryName,
                                          const std::wstring &psFilename,
                                          const std::string &psEntryName,
                                          ID3D11Device *device) {
  // Define the standard input layout for blur shaders
  D3D11_INPUT_ELEMENT_DESC polygonLayout[2] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
       D3D11_INPUT_PER_VERTEX_DATA, 0}};

  // Initialize base shader components
  if (!InitializeShaderFromFile(hwnd, vsFilename, vsEntryName, psFilename,
                                psEntryName, polygonLayout,
                                _countof(polygonLayout), device)) {
    return false;
  }

  // Create the matrix constant buffer
  if (!CreateConstantBuffer(sizeof(MatrixBufferType),
                            matrix_buffer_.GetAddressOf(), device)) {
    return false;
  }

  // Create the screen size constant buffer
  if (!CreateConstantBuffer(sizeof(ScreenSizeBufferType),
                            screen_size_buffer_.GetAddressOf(), device)) {
    return false;
  }

  // Create sampler state
  if (!CreateSamplerState(sampler_state_.GetAddressOf(), device)) {
    return false;
  }

  return true;
}

bool BlurShaderBase::SetBaseShaderParameters(
    const DirectX::XMMATRIX &worldMatrix, const DirectX::XMMATRIX &viewMatrix,
    const DirectX::XMMATRIX &projectionMatrix,
    ID3D11ShaderResourceView *texture, float screenSize,
    ID3D11DeviceContext *deviceContext) const {
  D3D11_MAPPED_SUBRESOURCE mappedResource;

  // Transpose matrices for the shader
  DirectX::XMMATRIX worldMatrixCopy = DirectX::XMMatrixTranspose(worldMatrix);
  DirectX::XMMATRIX viewMatrixCopy = DirectX::XMMatrixTranspose(viewMatrix);
  DirectX::XMMATRIX projectionMatrixCopy =
      DirectX::XMMatrixTranspose(projectionMatrix);

  // Update matrix buffer
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
  deviceContext->VSSetConstantBuffers(0, 1, matrix_buffer_.GetAddressOf());

  // Update screen size buffer
  result = deviceContext->Map(screen_size_buffer_.Get(), 0,
                              D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  auto sizeDataPtr = static_cast<ScreenSizeBufferType *>(mappedResource.pData);
  sizeDataPtr->screenSize = screenSize;
  sizeDataPtr->padding = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);

  deviceContext->Unmap(screen_size_buffer_.Get(), 0);
  deviceContext->VSSetConstantBuffers(1, 1, screen_size_buffer_.GetAddressOf());

  // Set shader texture resource
  deviceContext->PSSetShaderResources(0, 1, &texture);

  return true;
}

void BlurShaderBase::RenderShader(int indexCount,
                                  ID3D11DeviceContext *deviceContext) const {
  deviceContext->IASetInputLayout(layout_.Get());
  deviceContext->VSSetShader(vertex_shader_.Get(), nullptr, 0);
  deviceContext->PSSetShader(pixel_shader_.Get(), nullptr, 0);
  deviceContext->PSSetSamplers(0, 1, sampler_state_.GetAddressOf());
  deviceContext->DrawIndexed(indexCount, 0, 0);
}