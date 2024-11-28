#include "projectionshaderclass.h"

#include <d3dcompiler.h>
#include <fstream>

#include "../CommonFramework/DirectX11Device.h"

using namespace std;

struct MatrixBufferType {
  XMMATRIX world;
  XMMATRIX view;
  XMMATRIX projection;
  XMMATRIX view2;
  XMMATRIX projection2;
};

struct LightPositionBufferType {
  XMFLOAT3 lightPosition;
  float padding;
};

struct LightBufferType {
  XMFLOAT4 ambientColor;
  XMFLOAT4 diffuseColor;
};

ProjectionShaderClass::ProjectionShaderClass() {
  vertex_shader_ = nullptr;
  pixel_shader_ = nullptr;
  layout_ = nullptr;
  sample_state_ = nullptr;
  matrix_buffer_ = nullptr;
  light_buffer_ = nullptr;
  m_lightPositionBuffer = 0;
}

ProjectionShaderClass::ProjectionShaderClass(
    const ProjectionShaderClass &other) {}

ProjectionShaderClass::~ProjectionShaderClass() {}

bool ProjectionShaderClass::Initialize(HWND hwnd) {

  auto result = InitializeShader(hwnd, L"projection.vs", L"projection.ps");
  if (!result) {
    return false;
  }

  return true;
}

void ProjectionShaderClass::Shutdown() { ShutdownShader(); }

bool ProjectionShaderClass::Render(
    int indexCount, const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix,
    const XMMATRIX &projectionMatrix, ID3D11ShaderResourceView *texture,
    const XMFLOAT4 &ambientColor, const XMFLOAT4 &diffuseColor,
    const XMFLOAT3 &lightPosition, const XMMATRIX &viewMatrix2,
    const XMMATRIX &projectionMatrix2,
    ID3D11ShaderResourceView *projectionTexture) {

  auto result =
      SetShaderParameters(worldMatrix, viewMatrix, projectionMatrix, texture,
                          ambientColor, diffuseColor, lightPosition,
                          viewMatrix2, projectionMatrix2, projectionTexture);
  if (!result) {
    return false;
  }

  RenderShader(indexCount);

  return true;
}

bool ProjectionShaderClass::InitializeShader(HWND hwnd, WCHAR *vsFilename,
                                             WCHAR *psFilename) {

  ID3D10Blob *errorMessage;
  ID3D10Blob *vertexShaderBuffer;

  auto result = D3DCompileFromFile(
      vsFilename, NULL, NULL, "ProjectionVertexShader", "vs_5_0",
      D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage);
  if (FAILED(result)) {

    if (errorMessage) {
      OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
    } else {
      MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
    }

    return false;
  }

  ID3D10Blob *pixelShaderBuffer;

  result = D3DCompileFromFile(psFilename, NULL, NULL, "ProjectionPixelShader",
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

  D3D11_INPUT_ELEMENT_DESC polygonLayout[3];

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

  result = device->CreateSamplerState(&samplerDesc, &sample_state_);
  if (FAILED(result)) {
    return false;
  }

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

  D3D11_BUFFER_DESC lightBufferDesc;

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

  D3D11_BUFFER_DESC lightPositionBufferDesc;

  lightPositionBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
  lightPositionBufferDesc.ByteWidth = sizeof(LightPositionBufferType);
  lightPositionBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  lightPositionBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  lightPositionBufferDesc.MiscFlags = 0;
  lightPositionBufferDesc.StructureByteStride = 0;

  result = device->CreateBuffer(&lightPositionBufferDesc, NULL,
                                &m_lightPositionBuffer);
  if (FAILED(result)) {
    return false;
  }

  return true;
}

void ProjectionShaderClass::ShutdownShader() {
  // Release the light position constant buffer.
  if (m_lightPositionBuffer) {
    m_lightPositionBuffer->Release();
    m_lightPositionBuffer = 0;
  }

  if (light_buffer_) {
    light_buffer_->Release();
    light_buffer_ = nullptr;
  }

  if (matrix_buffer_) {
    matrix_buffer_->Release();
    matrix_buffer_ = nullptr;
  }

  if (sample_state_) {
    sample_state_->Release();
    sample_state_ = nullptr;
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

void ProjectionShaderClass::OutputShaderErrorMessage(ID3D10Blob *errorMessage,
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

bool ProjectionShaderClass::SetShaderParameters(
    const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix,
    const XMMATRIX &projectionMatrix, ID3D11ShaderResourceView *texture,
    const XMFLOAT4 &ambientColor, const XMFLOAT4 &diffuseColor,
    const XMFLOAT3 &lightPosition, const XMMATRIX &viewMatrix2,
    const XMMATRIX &projectionMatrix2,
    ID3D11ShaderResourceView *projectionTexture) {

  XMMATRIX worldMatrixCopy = worldMatrix;
  XMMATRIX viewMatrixCopy = viewMatrix;
  XMMATRIX projectionMatrixCopy = projectionMatrix;
  XMMATRIX viewMatrix2Copy = viewMatrix2;
  XMMATRIX projectionMatrix2Copy = projectionMatrix2;

  worldMatrixCopy = XMMatrixTranspose(worldMatrix);
  viewMatrixCopy = XMMatrixTranspose(viewMatrix);
  projectionMatrixCopy = XMMatrixTranspose(projectionMatrix);
  viewMatrix2Copy = XMMatrixTranspose(viewMatrix2);
  projectionMatrix2Copy = XMMatrixTranspose(projectionMatrix2);

  D3D11_MAPPED_SUBRESOURCE mappedResource;

  auto device_context =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  result = device_context->Map(matrix_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0,
                               &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  MatrixBufferType *dataPtr;
  dataPtr = (MatrixBufferType *)mappedResource.pData;

  dataPtr->world = worldMatrixCopy;
  dataPtr->view = viewMatrixCopy;
  dataPtr->projection = projectionMatrixCopy;
  dataPtr->view2 = viewMatrix2Copy;
  dataPtr->projection2 = projectionMatrix2Copy;

  device_context->Unmap(matrix_buffer_, 0);

  unsigned int buffer_number;

  buffer_number = 0;

  device_context->VSSetConstantBuffers(buffer_number, 1, &matrix_buffer_);

  auto device_context =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  result = device_context->Map(light_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0,
                               &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  LightBufferType *dataPtr2;

  dataPtr2 = (LightBufferType *)mappedResource.pData;

  dataPtr2->ambientColor = ambientColor;
  dataPtr2->diffuseColor = diffuseColor;

  device_context->Unmap(light_buffer_, 0);

  buffer_number = 0;

  device_context->PSSetConstantBuffers(buffer_number, 1, &light_buffer_);

  auto device_context =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  result = device_context->Map(m_lightPositionBuffer, 0,
                               D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  LightPositionBufferType *dataPtr3;

  dataPtr3 = (LightPositionBufferType *)mappedResource.pData;

  dataPtr3->lightPosition = lightPosition;
  dataPtr3->padding = 0.0f;

  device_context->Unmap(m_lightPositionBuffer, 0);

  buffer_number = 1;

  device_context->VSSetConstantBuffers(buffer_number, 1,
                                       &m_lightPositionBuffer);
  device_context->PSSetShaderResources(0, 1, &texture);
  device_context->PSSetShaderResources(1, 1, &projectionTexture);

  return true;
}

void ProjectionShaderClass::RenderShader(int indexCount) {

  auto device_context =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  device_context->IASetInputLayout(layout_);
  device_context->VSSetShader(vertex_shader_, NULL, 0);
  device_context->PSSetShader(pixel_shader_, NULL, 0);
  device_context->PSSetSamplers(0, 1, &sample_state_);

  device_context->DrawIndexed(indexCount, 0, 0);
}