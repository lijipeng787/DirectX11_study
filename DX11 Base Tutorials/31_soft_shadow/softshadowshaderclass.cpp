

#include "softshadowshaderclass.h"

#include <d3dcompiler.h>
#include <fstream>

#include "../CommonFramework/DirectX11Device.h"

using namespace std;
using namespace DirectX;

bool SoftShadowShaderClass::Initialize(HWND hwnd) {
  bool result;

  result = InitializeShader(hwnd, L"./softshadow.vs", L"./softshadow.ps");
  if (!result) {
    return false;
  }

  return true;
}

void SoftShadowShaderClass::Shutdown() { ShutdownShader(); }

bool SoftShadowShaderClass::Render(
    int indexCount, const ShaderParameterContainer &parameters) const {

  auto worldMatrix = parameters.GetMatrix("worldMatrix");
  auto viewMatrix = parameters.GetMatrix("viewMatrix");
  auto projectionMatrix = parameters.GetMatrix("projectionMatrix");
  auto shadowTexture = parameters.GetTexture("shadowTexture");
  auto lightPosition = parameters.GetVector3("lightPosition");
  auto texture = parameters.GetTexture("texture");
  auto diffuseColor = parameters.GetVector4("diffuseColor");
  auto ambientColor = parameters.GetVector4("ambientColor");

  auto result = SetShaderParameters(worldMatrix, viewMatrix, projectionMatrix,
                                    texture, shadowTexture, lightPosition,
                                    ambientColor, diffuseColor);
  if (!result) {
    return false;
  }

  RenderShader(indexCount);

  return true;
}

bool SoftShadowShaderClass::InitializeShader(HWND hwnd, WCHAR *vsFilename,
                                             WCHAR *psFilename) {
  HRESULT result;
  ID3D10Blob *errorMessage;
  ID3D10Blob *vertexShaderBuffer;
  ID3D10Blob *pixelShaderBuffer;
  D3D11_INPUT_ELEMENT_DESC polygonLayout[3];
  unsigned int numElements;
  D3D11_SAMPLER_DESC samplerDesc;
  D3D11_BUFFER_DESC matrixBufferDesc;
  D3D11_BUFFER_DESC lightBufferDesc;
  D3D11_BUFFER_DESC lightBufferDesc2;

  errorMessage = 0;
  vertexShaderBuffer = 0;
  pixelShaderBuffer = 0;

  result = D3DCompileFromFile(vsFilename, NULL, NULL, "SoftShadowVertexShader",
                              "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
                              &vertexShaderBuffer, &errorMessage);
  if (FAILED(result)) {

    if (errorMessage) {
      OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
    }

    else {
      MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
    }

    return false;
  }

  result = D3DCompileFromFile(psFilename, NULL, NULL, "SoftShadowPixelShader",
                              "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
                              &pixelShaderBuffer, &errorMessage);
  if (FAILED(result)) {

    if (errorMessage) {
      OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
    }

    else {
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

  polygonLayout[2].SemanticName = "NORMAL";
  polygonLayout[2].SemanticIndex = 0;
  polygonLayout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
  polygonLayout[2].InputSlot = 0;
  polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
  polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  polygonLayout[2].InstanceDataStepRate = 0;

  numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

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

  // Create a wrap texture sampler state description.
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

  result = device->CreateSamplerState(&samplerDesc, &m_sampleStateWrap);
  if (FAILED(result)) {
    return false;
  }

  // Create a clamp texture sampler state description.
  samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
  samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
  samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

  result = device->CreateSamplerState(&samplerDesc, &m_sampleStateClamp);
  if (FAILED(result)) {
    return false;
  }

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

  lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
  lightBufferDesc.ByteWidth = sizeof(LightBufferType);
  lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  lightBufferDesc.MiscFlags = 0;
  lightBufferDesc.StructureByteStride = 0;

  result = device->CreateBuffer(&lightBufferDesc, NULL, &light_buffer_);
  if (FAILED(result)) {
    return false;
  }

  // Setup the description of the light dynamic constant buffer that is in the
  // vertex shader.
  lightBufferDesc2.Usage = D3D11_USAGE_DYNAMIC;
  lightBufferDesc2.ByteWidth = sizeof(LightBufferType2);
  lightBufferDesc2.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  lightBufferDesc2.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  lightBufferDesc2.MiscFlags = 0;
  lightBufferDesc2.StructureByteStride = 0;

  result = device->CreateBuffer(&lightBufferDesc2, NULL, &m_lightBuffer2);
  if (FAILED(result)) {
    return false;
  }

  return true;
}

void SoftShadowShaderClass::ShutdownShader() {}

void SoftShadowShaderClass::OutputShaderErrorMessage(ID3D10Blob *errorMessage,
                                                     HWND hwnd,
                                                     WCHAR *shaderFilename) {
  char *compileErrors;
  SIZE_T bufferSize, i;
  ofstream fout;

  compileErrors = (char *)(errorMessage->GetBufferPointer());

  bufferSize = errorMessage->GetBufferSize();

  fout.open("shader-error.txt");

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

bool SoftShadowShaderClass::SetShaderParameters(
    const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix,
    const XMMATRIX &projectionMatrix, ID3D11ShaderResourceView *texture,
    ID3D11ShaderResourceView *shadowTexture, const XMFLOAT3 &lightPosition,
    const XMFLOAT4 &ambientColor, const XMFLOAT4 &diffuseColor) const {

  D3D11_MAPPED_SUBRESOURCE mappedResource;
  unsigned int buffer_number;
  MatrixBufferType *dataPtr;
  LightBufferType *dataPtr2;
  LightBufferType2 *dataPtr3;

  XMMATRIX worldMatrixCopy = worldMatrix;
  XMMATRIX viewMatrixCopy = viewMatrix;
  XMMATRIX projectionMatrixCopy = projectionMatrix;

  worldMatrixCopy = XMMatrixTranspose(worldMatrix);
  viewMatrixCopy = XMMatrixTranspose(viewMatrix);
  projectionMatrixCopy = XMMatrixTranspose(projectionMatrix);

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

  device_context->Unmap(matrix_buffer_.Get(), 0);

  buffer_number = 0;

  device_context->VSSetConstantBuffers(buffer_number, 1,
                                       matrix_buffer_.GetAddressOf());

  device_context->PSSetShaderResources(0, 1, &texture);
  device_context->PSSetShaderResources(1, 1, &shadowTexture);

  result = device_context->Map(light_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD,
                               0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  dataPtr2 = (LightBufferType *)mappedResource.pData;

  dataPtr2->ambientColor = ambientColor;
  dataPtr2->diffuseColor = diffuseColor;

  device_context->Unmap(light_buffer_.Get(), 0);

  buffer_number = 0;

  device_context->PSSetConstantBuffers(buffer_number, 1,
                                       light_buffer_.GetAddressOf());

  // Lock the second light constant buffer so it can be written to.

  result = device_context->Map(m_lightBuffer2.Get(), 0, D3D11_MAP_WRITE_DISCARD,
                               0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  dataPtr3 = (LightBufferType2 *)mappedResource.pData;

  dataPtr3->lightPosition = lightPosition;
  dataPtr3->padding = 0.0f;

  device_context->Unmap(m_lightBuffer2.Get(), 0);

  // Set the position of the light constant buffer in the vertex shader.
  buffer_number = 1;

  device_context->VSSetConstantBuffers(buffer_number, 1,
                                       m_lightBuffer2.GetAddressOf());

  return true;
}

void SoftShadowShaderClass::RenderShader(int indexCount) const {

  auto device_context =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  device_context->IASetInputLayout(layout_.Get());

  device_context->VSSetShader(vertex_shader_.Get(), NULL, 0);
  device_context->PSSetShader(pixel_shader_.Get(), NULL, 0);

  // Set the sampler states in the pixel shader.
  device_context->PSSetSamplers(0, 1, m_sampleStateClamp.GetAddressOf());
  device_context->PSSetSamplers(1, 1, m_sampleStateWrap.GetAddressOf());

  device_context->DrawIndexed(indexCount, 0, 0);
}