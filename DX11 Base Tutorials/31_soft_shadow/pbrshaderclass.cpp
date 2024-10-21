#include "pbrshaderclass.h"

#include "../CommonFramework/DirectX11Device.h"

#include <d3dcompiler.h>
#include <fstream>

using namespace DirectX;
using namespace std;

bool PbrShaderClass::Initialize(HWND hwnd) {

  wchar_t vsFilename[128], psFilename[128];

  auto error = wcscpy_s(vsFilename, 128, L"./pbr.vs");
  if (error != 0) {
    return false;
  }

  error = wcscpy_s(psFilename, 128, L"./pbr.ps");
  if (error != 0) {
    return false;
  }

  auto result = InitializeShader(hwnd, vsFilename, psFilename);
  if (!result) {
    return false;
  }

  return true;
}

bool PbrShaderClass::Render(int indexCount,
                            const ShaderParameterContainer &parameters) const {

  auto worldMatrix = parameters.GetMatrix("worldMatrix");
  auto viewMatrix = parameters.GetMatrix("viewMatrix");
  auto projectionMatrix = parameters.GetMatrix("projectionMatrix");
  auto diffuseTexture = parameters.GetTexture("diffuseTexture");
  auto normalMap = parameters.GetTexture("normalMap");
  auto rmTexture = parameters.GetTexture("rmTexture");
  auto lightDirection = parameters.GetVector3("lightDirection");
  auto cameraPosition = parameters.GetVector3("cameraPosition");

  auto result = SetShaderParameters(worldMatrix, viewMatrix, projectionMatrix,
                                    diffuseTexture, normalMap, rmTexture,
                                    lightDirection, cameraPosition);
  if (!result) {
    return false;
  }

  RenderShader(indexCount);

  return true;
}

bool PbrShaderClass::InitializeShader(HWND hwnd, WCHAR *vsFilename,
                                      WCHAR *psFilename) {
  HRESULT result;
  ID3D10Blob *errorMessage;
  ID3D10Blob *vertexShaderBuffer;
  ID3D10Blob *pixelShaderBuffer;
  D3D11_INPUT_ELEMENT_DESC polygonLayout[5];
  unsigned int numElements;
  D3D11_SAMPLER_DESC samplerDesc;
  D3D11_BUFFER_DESC matrixBufferDesc;
  D3D11_BUFFER_DESC cameraBufferDesc;
  D3D11_BUFFER_DESC lightBufferDesc;

  errorMessage = 0;
  vertexShaderBuffer = 0;
  pixelShaderBuffer = 0;

  result = D3DCompileFromFile(vsFilename, NULL, NULL, "PbrVertexShader",
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

  result = D3DCompileFromFile(psFilename, NULL, NULL, "PbrPixelShader",
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
                                      &m_vertexShader);
  if (FAILED(result)) {
    return false;
  }

  result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
                                     pixelShaderBuffer->GetBufferSize(), NULL,
                                     &m_pixelShader);
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

  polygonLayout[3].SemanticName = "TANGENT";
  polygonLayout[3].SemanticIndex = 0;
  polygonLayout[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
  polygonLayout[3].InputSlot = 0;
  polygonLayout[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
  polygonLayout[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  polygonLayout[3].InstanceDataStepRate = 0;

  polygonLayout[4].SemanticName = "BINORMAL";
  polygonLayout[4].SemanticIndex = 0;
  polygonLayout[4].Format = DXGI_FORMAT_R32G32B32_FLOAT;
  polygonLayout[4].InputSlot = 0;
  polygonLayout[4].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
  polygonLayout[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  polygonLayout[4].InstanceDataStepRate = 0;

  numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

  result = device->CreateInputLayout(
      polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
      vertexShaderBuffer->GetBufferSize(), &m_layout);
  if (FAILED(result)) {
    return false;
  }

  vertexShaderBuffer->Release();
  vertexShaderBuffer = 0;

  pixelShaderBuffer->Release();
  pixelShaderBuffer = 0;

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

  result = device->CreateSamplerState(&samplerDesc, &m_sampleState);
  if (FAILED(result)) {
    return false;
  }

  matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
  matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
  matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  matrixBufferDesc.MiscFlags = 0;
  matrixBufferDesc.StructureByteStride = 0;

  result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
  if (FAILED(result)) {
    return false;
  }

  cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
  cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
  cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  cameraBufferDesc.MiscFlags = 0;
  cameraBufferDesc.StructureByteStride = 0;

  result = device->CreateBuffer(&cameraBufferDesc, NULL, &m_cameraBuffer);
  if (FAILED(result)) {
    return false;
  }

  lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
  lightBufferDesc.ByteWidth = sizeof(LightBufferType);
  lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  lightBufferDesc.MiscFlags = 0;
  lightBufferDesc.StructureByteStride = 0;

  result = device->CreateBuffer(&lightBufferDesc, NULL, &m_lightBuffer);
  if (FAILED(result)) {
    return false;
  }

  return true;
}

void PbrShaderClass::OutputShaderErrorMessage(ID3D10Blob *errorMessage,
                                              HWND hwnd,
                                              WCHAR *shaderFilename) {
  char *compileErrors;
  unsigned __int64 bufferSize, i;
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

bool PbrShaderClass::SetShaderParameters(
    XMMATRIX &worldMatrix, XMMATRIX &viewMatrix, XMMATRIX &projectionMatrix,
    ID3D11ShaderResourceView *diffuseTexture,
    ID3D11ShaderResourceView *normalMap, ID3D11ShaderResourceView *rmTexture,
    const XMFLOAT3 &lightDirection, const XMFLOAT3 &cameraPosition) const {

  HRESULT result;
  D3D11_MAPPED_SUBRESOURCE mappedResource;
  unsigned int bufferNumber;
  MatrixBufferType *dataPtr;
  LightBufferType *dataPtr2;
  CameraBufferType *dataPtr3;

  worldMatrix = XMMatrixTranspose(worldMatrix);
  viewMatrix = XMMatrixTranspose(viewMatrix);
  projectionMatrix = XMMatrixTranspose(projectionMatrix);

  auto deviceContext =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  result = deviceContext->Map(m_matrixBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD,
                              0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  dataPtr = (MatrixBufferType *)mappedResource.pData;

  dataPtr->world = worldMatrix;
  dataPtr->view = viewMatrix;
  dataPtr->projection = projectionMatrix;

  deviceContext->Unmap(m_matrixBuffer.Get(), 0);

  bufferNumber = 0;

  deviceContext->VSSetConstantBuffers(bufferNumber, 1,
                                      m_matrixBuffer.GetAddressOf());

  result = deviceContext->Map(m_cameraBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD,
                              0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  dataPtr3 = (CameraBufferType *)mappedResource.pData;

  dataPtr3->cameraPosition = cameraPosition;
  dataPtr3->padding = 0.0f;

  deviceContext->Unmap(m_cameraBuffer.Get(), 0);

  bufferNumber = 1;

  deviceContext->VSSetConstantBuffers(bufferNumber, 1,
                                      m_cameraBuffer.GetAddressOf());

  deviceContext->PSSetShaderResources(0, 1, &diffuseTexture);
  deviceContext->PSSetShaderResources(1, 1, &normalMap);
  deviceContext->PSSetShaderResources(2, 1, &rmTexture);

  result = deviceContext->Map(m_lightBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD,
                              0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  dataPtr2 = (LightBufferType *)mappedResource.pData;

  dataPtr2->lightDirection = lightDirection;
  dataPtr2->padding = 0.0f;

  deviceContext->Unmap(m_lightBuffer.Get(), 0);

  bufferNumber = 0;

  deviceContext->PSSetConstantBuffers(bufferNumber, 1,
                                      m_lightBuffer.GetAddressOf());

  return true;
}

void PbrShaderClass::RenderShader(int indexCount) const {

  auto deviceContext =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  deviceContext->IASetInputLayout(m_layout.Get());

  deviceContext->VSSetShader(m_vertexShader.Get(), NULL, 0);
  deviceContext->PSSetShader(m_pixelShader.Get(), NULL, 0);

  deviceContext->PSSetSamplers(0, 1, m_sampleState.GetAddressOf());

  deviceContext->DrawIndexed(indexCount, 0, 0);
}