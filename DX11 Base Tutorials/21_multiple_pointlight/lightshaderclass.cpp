#include "lightshaderclass.h"

#include <d3dcompiler.h>
#include <fstream>

#include "../CommonFramework/DirectX11Device.h"

using namespace std;

struct MatrixBufferType {
  XMMATRIX world;
  XMMATRIX view;
  XMMATRIX projection;
};

struct LightColorBufferType {
  XMFLOAT4 diffuseColor[NUM_LIGHTS];
};

struct LightPositionBufferType {
  XMFLOAT4 lightPosition[NUM_LIGHTS];
};

LightShaderClass::LightShaderClass() {
  vertex_shader_ = nullptr;
  pixel_shader_ = nullptr;
  layout_ = nullptr;
  sample_state_ = nullptr;
  matrix_buffer_ = nullptr;
  m_lightColorBuffer = 0;
  m_lightPositionBuffer = 0;
}

LightShaderClass::LightShaderClass(const LightShaderClass &other) {}

LightShaderClass::~LightShaderClass() {}

bool LightShaderClass::Initialize(HWND hwnd) {

  auto result = InitializeShader(hwnd, L"light.vs", L"light.ps");
  if (!result) {
    return false;
  }

  return true;
}

void LightShaderClass::Shutdown() { ShutdownShader(); }

bool LightShaderClass::Render(int indexCount, const XMMATRIX &worldMatrix,
                              const XMMATRIX &viewMatrix,
                              const XMMATRIX &projectionMatrix,
                              ID3D11ShaderResourceView *texture,
                              const XMFLOAT4 diffuseColor[],
                              const XMFLOAT4 lightPosition[]) {

  auto result = SetShaderParameters(worldMatrix, viewMatrix, projectionMatrix,
                                    texture, diffuseColor, lightPosition);
  if (!result) {
    return false;
  }

  RenderShader(indexCount);

  return true;
}

bool LightShaderClass::InitializeShader(HWND hwnd, WCHAR *vsFilename,
                                        WCHAR *psFilename) {

  ID3D10Blob *errorMessage;
  ID3D10Blob *vertexShaderBuffer;

  auto result = D3DCompileFromFile(vsFilename, NULL, NULL, "LightVertexShader",
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

  ID3D10Blob *pixelShaderBuffer;

  result = D3DCompileFromFile(psFilename, NULL, NULL, "LightPixelShader",
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

  D3D11_BUFFER_DESC lightColorBufferDesc;

  lightColorBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
  lightColorBufferDesc.ByteWidth = sizeof(LightColorBufferType);
  lightColorBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  lightColorBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  lightColorBufferDesc.MiscFlags = 0;
  lightColorBufferDesc.StructureByteStride = 0;

  result =
      device->CreateBuffer(&lightColorBufferDesc, NULL, &m_lightColorBuffer);
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

void LightShaderClass::ShutdownShader() {
  // Release the light constant buffers.
  if (m_lightColorBuffer) {
    m_lightColorBuffer->Release();
    m_lightColorBuffer = 0;
  }

  if (m_lightPositionBuffer) {
    m_lightPositionBuffer->Release();
    m_lightPositionBuffer = 0;
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

void LightShaderClass::OutputShaderErrorMessage(ID3D10Blob *errorMessage,
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

bool LightShaderClass::SetShaderParameters(const XMMATRIX &worldMatrix,
                                           const XMMATRIX &viewMatrix,
                                           const XMMATRIX &projectionMatrix,
                                           ID3D11ShaderResourceView *texture,
                                           const XMFLOAT4 diffuseColor[],
                                           const XMFLOAT4 lightPosition[]) {

  XMMATRIX worldMatrixCopy = worldMatrix;
  XMMATRIX viewMatrixCopy = viewMatrix;
  XMMATRIX projectionMatrixCopy = projectionMatrix;

  worldMatrixCopy = XMMatrixTranspose(worldMatrix);
  viewMatrixCopy = XMMatrixTranspose(viewMatrix);
  projectionMatrixCopy = XMMatrixTranspose(projectionMatrix);

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

  dataPtr->world = worldMatrixCopy;
  dataPtr->view = viewMatrixCopy;
  dataPtr->projection = projectionMatrixCopy;

  device_context->Unmap(matrix_buffer_, 0);

  unsigned int buffer_number = 0;

  device_context->VSSetConstantBuffers(buffer_number, 1, &matrix_buffer_);

  // Lock the light position constant buffer so it can be written to.
  result = device_context->Map(m_lightPositionBuffer, 0,
                               D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  LightPositionBufferType *dataPtr2;

  dataPtr2 = (LightPositionBufferType *)mappedResource.pData;

  // Copy the light position variables into the constant buffer.
  dataPtr2->lightPosition[0] = lightPosition[0];
  dataPtr2->lightPosition[1] = lightPosition[1];
  dataPtr2->lightPosition[2] = lightPosition[2];
  dataPtr2->lightPosition[3] = lightPosition[3];

  device_context->Unmap(m_lightPositionBuffer, 0);

  buffer_number = 1;

  // Finally set the constant buffer in the vertex shader with the updated
  // values.
  device_context->VSSetConstantBuffers(buffer_number, 1,
                                       &m_lightPositionBuffer);

  device_context->PSSetShaderResources(0, 1, &texture);

  // Lock the light color constant buffer so it can be written to.
  result = device_context->Map(m_lightColorBuffer, 0, D3D11_MAP_WRITE_DISCARD,
                               0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  LightColorBufferType *dataPtr3;

  dataPtr3 = (LightColorBufferType *)mappedResource.pData;

  // Copy the light color variables into the constant buffer.
  dataPtr3->diffuseColor[0] = diffuseColor[0];
  dataPtr3->diffuseColor[1] = diffuseColor[1];
  dataPtr3->diffuseColor[2] = diffuseColor[2];
  dataPtr3->diffuseColor[3] = diffuseColor[3];

  device_context->Unmap(m_lightColorBuffer, 0);

  // Set the position of the constant buffer in the pixel shader.
  buffer_number = 0;

  // Finally set the constant buffer in the pixel shader with the updated
  // values.
  device_context->PSSetConstantBuffers(buffer_number, 1, &m_lightColorBuffer);

  return true;
}

void LightShaderClass::RenderShader(int indexCount) {

  auto device_context =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  device_context->IASetInputLayout(layout_);
  device_context->VSSetShader(vertex_shader_, NULL, 0);
  device_context->PSSetShader(pixel_shader_, NULL, 0);
  device_context->PSSetSamplers(0, 1, &sample_state_);

  device_context->DrawIndexed(indexCount, 0, 0);
}