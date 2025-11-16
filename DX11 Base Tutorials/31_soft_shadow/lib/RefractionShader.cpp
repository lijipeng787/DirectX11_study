#include "RefractionShader.h"

#include <d3d11.h>

using namespace DirectX;

bool RefractionShader::Initialize(HWND hwnd, ID3D11Device *device) {

  shader_name_ = "RefractionShader";
  D3D11_INPUT_ELEMENT_DESC polygonLayout[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
       D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}};

  if (!InitializeShaderFromFile(hwnd, L"./shader/refraction.vs",
                                "RefractionVertexShader", L"./shader/refraction.ps",
                                "RefractionPixelShader", polygonLayout,
                                _countof(polygonLayout), device)) {
    return false;
  }

  if (!CreateConstantBuffer(sizeof(MatrixBufferType),
                            matrix_buffer_.GetAddressOf(), device)) {
    return false;
  }

  if (!CreateConstantBuffer(sizeof(ClipPlaneBufferType),
                            clip_plane_buffer_.GetAddressOf(), device)) {
    return false;
  }

  if (!CreateConstantBuffer(sizeof(LightBufferType),
                            light_buffer_.GetAddressOf(), device)) {
    return false;
  }

  if (!CreateSamplerState(sampler_state_.GetAddressOf(), device)) {
    return false;
  }

  return true;
}

void RefractionShader::Shutdown() { ShaderBase::Shutdown(); }

bool RefractionShader::Render(int indexCount,
                              const ShaderParameterContainer &parameters,
                              ID3D11DeviceContext *deviceContext) const {

  auto worldMatrix = parameters.GetMatrix("worldMatrix");
  auto viewMatrix = parameters.GetMatrix("viewMatrix");
  auto projectionMatrix = parameters.GetMatrix("projectionMatrix");

  auto texture = parameters.GetTexture("texture");
  auto ambientColor = parameters.GetVector4("ambientColor");
  auto diffuseColor = parameters.GetVector4("diffuseColor");
  auto lightDirection = parameters.GetVector3("lightDirection");
  auto clipPlane = parameters.Get<DirectX::XMFLOAT4>("clipPlane");

  if (!SetShaderParameters(worldMatrix, viewMatrix, projectionMatrix, texture,
                           ambientColor, diffuseColor, lightDirection,
                           clipPlane, deviceContext)) {
    return false;
  }

  deviceContext->IASetInputLayout(layout_.Get());
  deviceContext->VSSetShader(vertex_shader_.Get(), nullptr, 0);
  deviceContext->PSSetShader(pixel_shader_.Get(), nullptr, 0);
  deviceContext->PSSetSamplers(0, 1, sampler_state_.GetAddressOf());
  deviceContext->DrawIndexed(indexCount, 0, 0);

  return true;
}

bool RefractionShader::SetShaderParameters(
    const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix,
    const XMMATRIX &projectionMatrix, ID3D11ShaderResourceView *texture,
    const XMFLOAT4 &ambientColor, const XMFLOAT4 &diffuseColor,
    const XMFLOAT3 &lightDirection, const XMFLOAT4 &clipPlane,
    ID3D11DeviceContext *deviceContext) const {

  D3D11_MAPPED_SUBRESOURCE mappedResource;

  auto worldT = XMMatrixTranspose(worldMatrix);
  auto viewT = XMMatrixTranspose(viewMatrix);
  auto projectionT = XMMatrixTranspose(projectionMatrix);

  auto result = deviceContext->Map(matrix_buffer_.Get(), 0,
                                   D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  auto matrixPtr = static_cast<MatrixBufferType *>(mappedResource.pData);
  matrixPtr->world = worldT;
  matrixPtr->view = viewT;
  matrixPtr->projection = projectionT;

  deviceContext->Unmap(matrix_buffer_.Get(), 0);
  deviceContext->VSSetConstantBuffers(0, 1, matrix_buffer_.GetAddressOf());

  result = deviceContext->Map(clip_plane_buffer_.Get(), 0,
                              D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  auto clipPtr = static_cast<ClipPlaneBufferType *>(mappedResource.pData);
  clipPtr->clipPlane = clipPlane;

  deviceContext->Unmap(clip_plane_buffer_.Get(), 0);
  deviceContext->VSSetConstantBuffers(1, 1, clip_plane_buffer_.GetAddressOf());

  result = deviceContext->Map(light_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD,
                              0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  auto lightPtr = static_cast<LightBufferType *>(mappedResource.pData);
  lightPtr->ambientColor = ambientColor;
  lightPtr->diffuseColor = diffuseColor;
  lightPtr->lightDirection = lightDirection;
  lightPtr->padding = 0.0f;

  deviceContext->Unmap(light_buffer_.Get(), 0);
  deviceContext->PSSetConstantBuffers(0, 1, light_buffer_.GetAddressOf());

  deviceContext->PSSetShaderResources(0, 1, &texture);

  return true;
}
