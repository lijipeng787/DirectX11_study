#include "shadowshaderclass.h"
#include "../CommonFramework/DirectX11Device.h"
#include "ShaderParameterContainer.h"

using namespace DirectX;

bool ShadowShaderClass::Initialize(HWND hwnd) {
  return InitializeShader(hwnd, L"./shadow.vs", L"./shadow.ps");
}

void ShadowShaderClass::Shutdown() { ShutdownShader(); }

bool ShadowShaderClass::Render(
    int indexCount, const ShaderParameterContainer &parameters) const {

  auto worldMatrix = parameters.GetMatrix("worldMatrix");
  auto viewMatrix = parameters.GetMatrix("viewMatrix");
  auto projectionMatrix = parameters.GetMatrix("projectionMatrix");
  auto lightViewMatrix = parameters.GetMatrix("lightViewMatrix");
  auto lightProjectionMatrix = parameters.GetMatrix("lightProjectionMatrix");
  auto depthMapTexture = parameters.GetTexture("depthMapTexture");
  auto lightPosition = parameters.GetVector3("lightPosition");

  auto result = SetShaderParameters(worldMatrix, viewMatrix, projectionMatrix,
                                    lightViewMatrix, lightProjectionMatrix,
                                    depthMapTexture, lightPosition);
  if (!result) {
    return false;
  }

  RenderShader(indexCount);

  return true;
}

bool ShadowShaderClass::InitializeShader(HWND hwnd,
                                         const std::wstring &vsFilename,
                                         const std::wstring &psFilename) {
  HRESULT result;
  Microsoft::WRL::ComPtr<ID3D10Blob> errorMessage;
  Microsoft::WRL::ComPtr<ID3D10Blob> vertexShaderBuffer;
  Microsoft::WRL::ComPtr<ID3D10Blob> pixelShaderBuffer;

  // Compile the vertex shader
  result = D3DCompileFromFile(
      vsFilename.c_str(), nullptr, nullptr, "ShadowVertexShader", "vs_5_0",
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
      psFilename.c_str(), nullptr, nullptr, "ShadowPixelShader", "ps_5_0",
      D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage);
  if (FAILED(result)) {
    if (errorMessage) {
      OutputShaderErrorMessage(errorMessage.Get(), hwnd, psFilename);
    } else {
      MessageBox(hwnd, psFilename.c_str(), L"Missing Shader File", MB_OK);
    }
    return false;
  }

  auto device = DirectX11Device::GetD3d11DeviceInstance()->GetDevice();

  // Create the vertex shader
  result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
                                      vertexShaderBuffer->GetBufferSize(),
                                      nullptr, &vertex_shader_);
  if (FAILED(result)) {
    return false;
  }

  // Create the pixel shader
  result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
                                     pixelShaderBuffer->GetBufferSize(),
                                     nullptr, &pixel_shader_);
  if (FAILED(result)) {
    return false;
  }

  // Create the vertex input layout description
  D3D11_INPUT_ELEMENT_DESC polygonLayout[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
       D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}};

  UINT numElements = ARRAYSIZE(polygonLayout);

  // Create the vertex input layout
  result = device->CreateInputLayout(
      polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
      vertexShaderBuffer->GetBufferSize(), &layout_);
  if (FAILED(result)) {
    return false;
  }

  // Create a constant buffer description
  D3D11_BUFFER_DESC matrixBufferDesc = {};
  matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
  matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
  matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

  // Create the constant buffer pointer
  result = device->CreateBuffer(&matrixBufferDesc, nullptr, &matrix_buffer_);
  if (FAILED(result)) {
    return false;
  }

  // Create a texture sampler state description
  D3D11_SAMPLER_DESC samplerDesc = {};
  samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
  samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
  samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
  samplerDesc.MipLODBias = 0.0f;
  samplerDesc.MaxAnisotropy = 1;
  samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
  samplerDesc.MinLOD = 0;
  samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

  // Create the texture sampler state
  result = device->CreateSamplerState(&samplerDesc, &sample_state_);
  if (FAILED(result)) {
    return false;
  }

  D3D11_BUFFER_DESC lightBufferDesc2;

  // Setup the description of the light dynamic constant buffer that is in the
  // vertex shader.
  lightBufferDesc2.Usage = D3D11_USAGE_DYNAMIC;
  lightBufferDesc2.ByteWidth = sizeof(LightBufferType2);
  lightBufferDesc2.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  lightBufferDesc2.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  lightBufferDesc2.MiscFlags = 0;
  lightBufferDesc2.StructureByteStride = 0;

  result = device->CreateBuffer(&lightBufferDesc2, NULL, &light_buffee_2);
  if (FAILED(result)) {
    return false;
  }

  return true;
}

void ShadowShaderClass::ShutdownShader() {
  sample_state_.Reset();
  matrix_buffer_.Reset();
  layout_.Reset();
  pixel_shader_.Reset();
  vertex_shader_.Reset();
}

void ShadowShaderClass::OutputShaderErrorMessage(
    ID3D10Blob *errorMessage, HWND hwnd, const std::wstring &shaderFilename) {
  char *compileErrors = static_cast<char *>(errorMessage->GetBufferPointer());
  SIZE_T bufferSize = errorMessage->GetBufferSize();

  std::ofstream fout("shader-error.txt");
  for (SIZE_T i = 0; i < bufferSize; i++) {
    fout << compileErrors[i];
  }
  fout.close();

  MessageBox(hwnd,
             L"Error compiling shader. Check shader-error.txt for message.",
             shaderFilename.c_str(), MB_OK);
}

bool ShadowShaderClass::SetShaderParameters(
    const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix,
    const XMMATRIX &projectionMatrix, const XMMATRIX &lightViewMatrix,
    const XMMATRIX &lightProjectionMatrix,
    ID3D11ShaderResourceView *depthMapTexture,
    const XMFLOAT3 &lightPosition) const {

  D3D11_MAPPED_SUBRESOURCE mappedResource;
  unsigned int buffer_number;
  MatrixBufferType *dataPtr;
  LightBufferType2 *dataPtr3;

  XMMATRIX worldMatrixCopy = worldMatrix;
  XMMATRIX viewMatrixCopy = viewMatrix;
  XMMATRIX projectionMatrixCopy = projectionMatrix;
  XMMATRIX lightViewMatrixCopy = lightViewMatrix;
  XMMATRIX lightProjectionMatrixCopy = lightProjectionMatrix;

  worldMatrixCopy = XMMatrixTranspose(worldMatrix);
  viewMatrixCopy = XMMatrixTranspose(viewMatrix);
  projectionMatrixCopy = XMMatrixTranspose(projectionMatrix);
  lightViewMatrixCopy = XMMatrixTranspose(lightViewMatrix);
  lightProjectionMatrixCopy = XMMatrixTranspose(lightProjectionMatrix);

  auto device_context =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  auto result = device_context->Map(
      matrix_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  dataPtr = (MatrixBufferType *)mappedResource.pData;

  dataPtr->world = worldMatrixCopy;
  dataPtr->view = viewMatrixCopy;
  dataPtr->projection = projectionMatrixCopy;
  dataPtr->lightView = lightViewMatrixCopy;
  dataPtr->lightProjection = lightProjectionMatrixCopy;

  device_context->Unmap(matrix_buffer_.Get(), 0);

  buffer_number = 0;

  device_context->VSSetConstantBuffers(buffer_number, 1,
                                       matrix_buffer_.GetAddressOf());

  device_context->PSSetShaderResources(0, 1, &depthMapTexture);

  // Lock the second light constant buffer so it can be written to.
  result = device_context->Map(light_buffee_2.Get(), 0, D3D11_MAP_WRITE_DISCARD,
                               0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  dataPtr3 = (LightBufferType2 *)mappedResource.pData;

  dataPtr3->lightPosition = lightPosition;
  dataPtr3->padding = 0.0f;

  device_context->Unmap(light_buffee_2.Get(), 0);

  // Set the position of the light constant buffer in the vertex shader.
  buffer_number = 1;

  device_context->VSSetConstantBuffers(buffer_number, 1,
                                       light_buffee_2.GetAddressOf());

  return true;
}

void ShadowShaderClass::RenderShader(int indexCount) const {

  auto device_context =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  // Set the vertex input layout
  device_context->IASetInputLayout(layout_.Get());

  // Set the vertex and pixel shaders that will be used to render this triangle
  device_context->VSSetShader(vertex_shader_.Get(), nullptr, 0);
  device_context->PSSetShader(pixel_shader_.Get(), nullptr, 0);

  // Set the sampler state in the pixel shader
  device_context->PSSetSamplers(0, 1, sample_state_.GetAddressOf());

  // Render the triangle
  device_context->DrawIndexed(indexCount, 0, 0);
}