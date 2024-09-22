#include "ShaderParameterContainer.h"

#include <stdexcept>

void ShaderParameterContainer::SetFloat(const std::string &name, float f) {
  parameters_[name] = f;
}

void ShaderParameterContainer::SetMatrix(const std::string &name,
                                         const DirectX::XMMATRIX &matrix) {
  parameters_[name] = matrix;
}

void ShaderParameterContainer::SetVector3(const std::string &name,
                                          const DirectX::XMFLOAT3 &vector) {
  parameters_[name] = vector;
}

void ShaderParameterContainer::SetVector4(const std::string &name,
                                          const DirectX::XMFLOAT4 &vector) {
  parameters_[name] = vector;
}

void ShaderParameterContainer::SetTexture(const std::string &name,
                                          ID3D11ShaderResourceView *texture) {
  parameters_[name] = texture;
}