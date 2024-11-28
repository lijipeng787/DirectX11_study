#include "fireshaderclass.h"
#include "../CommonFramework/DirectX11Device.h"

#include <d3dcompiler.h>
#include <fstream>

using namespace std;
using namespace DirectX;

struct MatrixBufferType {
  XMMATRIX world;
  XMMATRIX view;
  XMMATRIX projection;
};

struct NoiseBufferType {
  float frameTime;
  XMFLOAT3 scrollSpeeds;
  XMFLOAT3 scales;
  float padding;
};

struct DistortionBufferType {
  XMFLOAT2 distortion1;
  XMFLOAT2 distortion2;
  XMFLOAT2 distortion3;
  float distortionScale;
  float distortionBias;
};

bool FireShaderClass::Initialize(HWND hwnd) {

  auto result = InitializeShader(hwnd, L"fire.hlsl", L"fire.hlsl");
  if (!result) {
    return false;
  }

  return true;
}

void FireShaderClass::Shutdown() { ShutdownShader(); }

bool FireShaderClass::Render(
    int indexCount, const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix,
    const XMMATRIX &projectionMatrix, ID3D11ShaderResourceView *fireTexture,
    ID3D11ShaderResourceView *noiseTexture,
    ID3D11ShaderResourceView *alphaTexture, float frameTime,
    const XMFLOAT3 &scrollSpeeds, const XMFLOAT3 &scales,
    const XMFLOAT2 &distortion1, const XMFLOAT2 &distortion2,
    const XMFLOAT2 &distortion3, float distortionScale, float distortionBias) {

  auto result = SetShaderParameters(
      worldMatrix, viewMatrix, projectionMatrix, fireTexture, noiseTexture,
      alphaTexture, frameTime, scrollSpeeds, scales, distortion1, distortion2,
      distortion3, distortionScale, distortionBias);
  if (!result) {
    return false;
  }

  RenderShader(indexCount);

  return true;
}

bool FireShaderClass::InitializeShader(HWND hwnd, WCHAR *vsFilename,
                                       WCHAR *psFilename) {

  ID3D10Blob *errorMessage = nullptr;
  ID3D10Blob *vertexShaderBuffer = nullptr;

  auto result = D3DCompileFromFile(vsFilename, NULL, NULL, "FireVertexShader",
                                   "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
                                   &vertexShaderBuffer, &errorMessage);
  if (FAILED(result)) {
    if (errorMessage) {
      OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
    } else {
      MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
    }

    return false;
  }

  ID3D10Blob *pixelShaderBuffer = nullptr;

  result = D3DCompileFromFile(psFilename, NULL, NULL, "FirePixelShader",
                              "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
                              &pixelShaderBuffer, &errorMessage);
  if (FAILED(result)) {
    if (errorMessage) {
      OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
    } else {
      MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
    }

    return false;
  }

  auto device = DirectX11Device::GetD3d11DeviceInstance()->GetDevice();

  result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
                                      vertexShaderBuffer->GetBufferSize(), NULL,
                                      &vertex_shader_);
  if (FAILED(result)) {
    return false;
  }

  result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
                                     pixelShaderBuffer->GetBufferSize(), NULL,
                                     &pixel_shader_);
  if (FAILED(result)) {
    return false;
  }

  D3D11_INPUT_ELEMENT_DESC polygonLayout[2];

  polygonLayout[0].SemanticName = "POSITION";
  polygonLayout[0].SemanticIndex = 0;
  polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
  polygonLayout[0].InputSlot = 0;
  polygonLayout[0].AlignedByteOffset = 0;
  polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  polygonLayout[0].InstanceDataStepRate = 0;

  polygonLayout[1].SemanticName = "TEXCOORD";
  polygonLayout[1].SemanticIndex = 0;
  polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
  polygonLayout[1].InputSlot = 0;
  polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
  polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  polygonLayout[1].InstanceDataStepRate = 0;

  unsigned int numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

  result = device->CreateInputLayout(
      polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
      vertexShaderBuffer->GetBufferSize(), &layout_);
  if (FAILED(result)) {
    return false;
  }

  vertexShaderBuffer->Release();
  vertexShaderBuffer = 0;

  pixelShaderBuffer->Release();
  pixelShaderBuffer = 0;

  D3D11_BUFFER_DESC matrixBufferDesc;

  matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
  matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
  matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  matrixBufferDesc.MiscFlags = 0;
  matrixBufferDesc.StructureByteStride = 0;

  result = device->CreateBuffer(&matrixBufferDesc, NULL, &matrix_buffer_);
  if (FAILED(result)) {
    return false;
  }

  D3D11_BUFFER_DESC noiseBufferDesc;

  noiseBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
  noiseBufferDesc.ByteWidth = sizeof(NoiseBufferType);
  noiseBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  noiseBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  noiseBufferDesc.MiscFlags = 0;
  noiseBufferDesc.StructureByteStride = 0;

  result = device->CreateBuffer(&noiseBufferDesc, NULL, &noise_buffer_);
  if (FAILED(result)) {
    return false;
  }

  D3D11_SAMPLER_DESC samplerDesc;

  samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.MipLODBias = 0.0f;
  samplerDesc.MaxAnisotropy = 1;
  samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
  samplerDesc.BorderColor[0] = 0;
  samplerDesc.BorderColor[1] = 0;
  samplerDesc.BorderColor[2] = 0;
  samplerDesc.BorderColor[3] = 0;
  samplerDesc.MinLOD = 0;
  samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

  D3D11_SAMPLER_DESC samplerDesc2;

  result = device->CreateSamplerState(&samplerDesc, &sample_state_);
  if (FAILED(result)) {
    return false;
  }

  samplerDesc2.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  samplerDesc2.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
  samplerDesc2.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
  samplerDesc2.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
  samplerDesc2.MipLODBias = 0.0f;
  samplerDesc2.MaxAnisotropy = 1;
  samplerDesc2.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
  samplerDesc2.BorderColor[0] = 0;
  samplerDesc2.BorderColor[1] = 0;
  samplerDesc2.BorderColor[2] = 0;
  samplerDesc2.BorderColor[3] = 0;
  samplerDesc2.MinLOD = 0;
  samplerDesc2.MaxLOD = D3D11_FLOAT32_MAX;

  result = device->CreateSamplerState(&samplerDesc2, &sample_state_2_);
  if (FAILED(result)) {
    return false;
  }

  D3D11_BUFFER_DESC distortionBufferDesc;

  distortionBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
  distortionBufferDesc.ByteWidth = sizeof(DistortionBufferType);
  distortionBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  distortionBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  distortionBufferDesc.MiscFlags = 0;
  distortionBufferDesc.StructureByteStride = 0;

  result =
      device->CreateBuffer(&distortionBufferDesc, NULL, &distortion_buffer_);
  if (FAILED(result)) {
    return false;
  }

  return true;
}

void FireShaderClass::ShutdownShader() {

  if (distortion_buffer_) {
    distortion_buffer_->Release();
    distortion_buffer_ = 0;
  }

  if (sample_state_2_) {
    sample_state_2_->Release();
    sample_state_2_ = 0;
  }

  if (sample_state_) {
    sample_state_->Release();
    sample_state_ = nullptr;
  }

  if (noise_buffer_) {
    noise_buffer_->Release();
    noise_buffer_ = 0;
  }

  if (matrix_buffer_) {
    matrix_buffer_->Release();
    matrix_buffer_ = nullptr;
  }

  if (layout_) {
    layout_->Release();
    layout_ = nullptr;
  }

  if (pixel_shader_) {
    pixel_shader_->Release();
    pixel_shader_ = nullptr;
  }

  if (vertex_shader_) {
    vertex_shader_->Release();
    vertex_shader_ = nullptr;
  }
}

void FireShaderClass::OutputShaderErrorMessage(ID3D10Blob *errorMessage,
                                               HWND hwnd,
                                               WCHAR *shaderFilename) {

  auto compileErrors = (char *)(errorMessage->GetBufferPointer());

  auto bufferSize = errorMessage->GetBufferSize();

  ofstream fout;
  fout.open("shader-error.txt");

  int i = 0;
  for (i = 0; i < bufferSize; i++) {
    fout << compileErrors[i];
  }

  fout.close();

  errorMessage->Release();
  errorMessage = 0;

  MessageBox(hwnd,
             L"Error compiling shader.  Check shader-error.txt for message.",
             shaderFilename, MB_OK);
}

bool FireShaderClass::SetShaderParameters(
    const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix,
    const XMMATRIX &projectionMatrix, ID3D11ShaderResourceView *fireTexture,
    ID3D11ShaderResourceView *noiseTexture,
    ID3D11ShaderResourceView *alphaTexture, float frameTime,
    const XMFLOAT3 &scrollSpeeds, const XMFLOAT3 &scales,
    const XMFLOAT2 &distortion1, const XMFLOAT2 &distortion2,
    const XMFLOAT2 &distortion3, float distortionScale, float distortionBias) {

  D3D11_MAPPED_SUBRESOURCE mappedResource;

  auto device_context =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  auto result = device_context->Map(matrix_buffer_, 0, D3D11_MAP_WRITE_DISCARD,
                                    0, &mappedResource);
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

  device_context->Unmap(matrix_buffer_, 0);

  unsigned int buffer_number = 0;

  device_context->VSSetConstantBuffers(buffer_number, 1, &matrix_buffer_);

  auto device_context =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  result = device_context->Map(noise_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0,
                               &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  NoiseBufferType *dataPtr2;
  dataPtr2 = (NoiseBufferType *)mappedResource.pData;

  dataPtr2->frameTime = frameTime;
  dataPtr2->scrollSpeeds = scrollSpeeds;
  dataPtr2->scales = scales;
  dataPtr2->padding = 0.0f;

  device_context->Unmap(noise_buffer_, 0);

  buffer_number = 1;

  device_context->VSSetConstantBuffers(buffer_number, 1, &noise_buffer_);

  device_context->PSSetShaderResources(0, 1, &fireTexture);
  device_context->PSSetShaderResources(1, 1, &noiseTexture);
  device_context->PSSetShaderResources(2, 1, &alphaTexture);

  auto device_context =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  result = device_context->Map(distortion_buffer_, 0, D3D11_MAP_WRITE_DISCARD,
                               0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  DistortionBufferType *dataPtr3;
  dataPtr3 = (DistortionBufferType *)mappedResource.pData;

  dataPtr3->distortion1 = distortion1;
  dataPtr3->distortion2 = distortion2;
  dataPtr3->distortion3 = distortion3;
  dataPtr3->distortionScale = distortionScale;
  dataPtr3->distortionBias = distortionBias;

  device_context->Unmap(distortion_buffer_, 0);

  buffer_number = 0;

  device_context->PSSetConstantBuffers(buffer_number, 1, &distortion_buffer_);

  return true;
}

void FireShaderClass::RenderShader(int indexCount) {

  auto device_context =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  device_context->IASetInputLayout(layout_);

  device_context->VSSetShader(vertex_shader_, NULL, 0);

  device_context->PSSetShader(pixel_shader_, NULL, 0);

  // Set the sampler states in the pixel shader.
  device_context->PSSetSamplers(0, 1, &sample_state_);

  device_context->PSSetSamplers(1, 1, &sample_state_2_);

  device_context->DrawIndexed(indexCount, 0, 0);
}
