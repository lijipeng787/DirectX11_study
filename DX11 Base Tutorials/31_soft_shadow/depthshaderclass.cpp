#include "depthshaderclass.h"
#include "../CommonFramework/DirectX11Device.h"

#include <d3dcompiler.h>
#include <fstream>

using namespace std;
using namespace DirectX;

bool DepthShaderClass::Initialize(HWND hwnd) {
  bool result;

  result = InitializeShader(hwnd, L"./depth.vs", L"./depth.ps");
  if (!result) {
    return false;
  }

  return true;
}

void DepthShaderClass::Shutdown() { ShutdownShader(); }

bool DepthShaderClass::Render(
    int indexCount, const ShaderParameterContainer &parameters) const {

  auto worldMatrix = parameters.GetMatrix("worldMatrix");
  auto viewMatrix = parameters.GetMatrix("viewMatrix");
  auto projectionMatrix = parameters.GetMatrix("projectionMatrix");

  auto result = SetShaderParameters(worldMatrix, viewMatrix, projectionMatrix);
  if (!result) {
    return false;
  }

  RenderShader(indexCount);

  return true;
}

bool DepthShaderClass::InitializeShader(HWND hwnd, WCHAR *vsFilename,
                                        WCHAR *psFilename) {
  HRESULT result;
  ID3D10Blob *errorMessage;
  ID3D10Blob *vertexShaderBuffer;
  ID3D10Blob *pixelShaderBuffer;
  D3D11_INPUT_ELEMENT_DESC polygonLayout[1];
  unsigned int numElements;
  D3D11_BUFFER_DESC matrixBufferDesc;

  errorMessage = 0;
  vertexShaderBuffer = 0;
  pixelShaderBuffer = 0;

  result = D3DCompileFromFile(vsFilename, NULL, NULL, "DepthVertexShader",
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

  result = D3DCompileFromFile(psFilename, NULL, NULL, "DepthPixelShader",
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

  return true;
}

void DepthShaderClass::ShutdownShader() {}

void DepthShaderClass::OutputShaderErrorMessage(ID3D10Blob *errorMessage,
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

bool DepthShaderClass::SetShaderParameters(
    const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix,
    const XMMATRIX &projectionMatrix) const {
  HRESULT result;
  D3D11_MAPPED_SUBRESOURCE mappedResource;
  unsigned int buffer_number;
  MatrixBufferType *dataPtr;

  XMMATRIX worldMatrixCopy = worldMatrix;
  XMMATRIX viewMatrixCopy = viewMatrix;
  XMMATRIX projectionMatrixCopy = projectionMatrix;

  worldMatrixCopy = XMMatrixTranspose(worldMatrix);
  viewMatrixCopy = XMMatrixTranspose(viewMatrix);
  projectionMatrixCopy = XMMatrixTranspose(projectionMatrix);

  auto device_context =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  result = device_context->Map(matrix_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD,
                               0, &mappedResource);
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

  return true;
}

void DepthShaderClass::RenderShader(int indexCount) const {
  auto device_context =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  device_context->IASetInputLayout(layout_.Get());

  device_context->VSSetShader(vertex_shader_.Get(), NULL, 0);
  device_context->PSSetShader(pixel_shader_.Get(), NULL, 0);

  device_context->DrawIndexed(indexCount, 0, 0);
}