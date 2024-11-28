#include <d3dcompiler.h>
#include <fstream>

#include "../CommonFramework/DirectX11Device.h"
#include "colorshaderclass.h"

using namespace std;
using namespace DirectX;

struct MatrixBufferType {
  XMMATRIX world;
  XMMATRIX view;
  XMMATRIX projection;
};

bool ColorShaderClass::Initialize(HWND hwnd) {

  auto result =
      InitializeShader(hwnd, L"vertex_colors.hlsl", L"vertex_colors.hlsl");
  if (!result) {
    return false;
  }

  return true;
}

void ColorShaderClass::Shutdown() { ShutdownShader(); }

bool ColorShaderClass::Render(int indexCount, const XMMATRIX &worldMatrix,
                              const XMMATRIX &viewMatrix,
                              const XMMATRIX &projectionMatrix) {

  auto result = SetShaderParameters(worldMatrix, viewMatrix, projectionMatrix);
  if (!result) {
    return false;
  }

  RenderShader(indexCount);

  return true;
}

bool ColorShaderClass::InitializeShader(HWND hwnd, WCHAR *vsFilename,
                                        WCHAR *psFilename) {

  ID3D10Blob *errorMessage = nullptr;
  ID3D10Blob *vertexShaderBuffer = nullptr;

  auto result = D3DCompileFromFile(vsFilename, NULL, NULL, "ColorVertexShader",
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

  result = D3DCompileFromFile(psFilename, NULL, NULL, "ColorPixelShader",
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

  polygonLayout[1].SemanticName = "COLOR";
  polygonLayout[1].SemanticIndex = 0;
  polygonLayout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
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

  return true;
}

void ColorShaderClass::ShutdownShader() {

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

void ColorShaderClass::OutputShaderErrorMessage(ID3D10Blob *errorMessage,
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

bool ColorShaderClass::SetShaderParameters(const XMMATRIX &worldMatrix,
                                           const XMMATRIX &viewMatrix,
                                           const XMMATRIX &projectionMatrix) {

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

  return true;
}

void ColorShaderClass::RenderShader(int indexCount) {

  auto device_context =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  device_context->IASetInputLayout(layout_);

  device_context->VSSetShader(vertex_shader_, NULL, 0);

  device_context->PSSetShader(pixel_shader_, NULL, 0);

  device_context->DrawIndexed(indexCount, 0, 0);
}