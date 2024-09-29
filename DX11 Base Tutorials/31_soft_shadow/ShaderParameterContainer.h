#pragma once

#include <DirectXMath.h>
#include <d3d11.h>

#include <any>
#include <stdexcept>
#include <string>
#include <unordered_map>

class ShaderParameterContainer {
public:
  template <typename T>
  ShaderParameterContainer &Set(const std::string &name, const T &value) {
    parameters_[name] = value;
    return *this;
  }

  void SetFloat(const std::string &name, float f) { parameters_[name] = f; }

  void SetMatrix(const std::string &name, const DirectX::XMMATRIX &matrix) {
    parameters_[name] = matrix;
  }

  void SetVector3(const std::string &name, const DirectX::XMFLOAT3 &vector) {
    parameters_[name] = vector;
  }

  void SetVector4(const std::string &name, const DirectX::XMFLOAT4 &vector) {
    parameters_[name] = vector;
  }

  void SetTexture(const std::string &name, ID3D11ShaderResourceView *texture) {
    parameters_[name] = texture;
  }

  void Merge(const ShaderParameterContainer &other);

  template <typename T> T Get(const std::string &name) const {
    auto it = parameters_.find(name);
    if (it == parameters_.end()) {
      throw std::runtime_error("Parameter not found: " + name);
    }
    try {
      return std::any_cast<T>(it->second);
    } catch (const std::bad_any_cast &) {
      throw std::runtime_error("Type mismatch for parameter: " + name);
    }
  }

  float GetFloat(const std::string &name) const { return Get<float>(name); }

  DirectX::XMMATRIX GetMatrix(const std::string &name) const {
    return Get<DirectX::XMMATRIX>(name);
  }

  DirectX::XMFLOAT3 GetVector3(const std::string &name) const {
    return Get<DirectX::XMFLOAT3>(name);
  }

  DirectX::XMFLOAT4 GetVector4(const std::string &name) const {
    return Get<DirectX::XMFLOAT4>(name);
  }

  ID3D11ShaderResourceView *GetTexture(const std::string &name) const {
    return Get<ID3D11ShaderResourceView *>(name);
  }

  bool HasParameter(const std::string &name) const {
    return parameters_.find(name) != parameters_.end();
  }

private:
  std::unordered_map<std::string, std::any> parameters_;
};