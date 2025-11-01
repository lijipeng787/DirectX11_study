#pragma once

#include <DirectXMath.h>
#include <d3d11.h>

#include <any>
#include <functional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

// ============================================================================
// Shader Parameter Type Definitions
// ============================================================================

// Unified parameter type definitions used by Layout and Validator
enum class ShaderParameterType {
  Matrix,
  Vector3,
  Vector4,
  Texture,
  Float,
  Unknown // Used by Validator for unknown types
};

// Unified parameter info structure
// Supports both simple layout (name + type) and validation (name + type +
// required)
struct ShaderParameterInfo {
  std::string name;
  ShaderParameterType type;
  bool required; // Whether this parameter is required (default: true)

  // Constructor for simple usage (layout)
  explicit ShaderParameterInfo(std::string name, ShaderParameterType type)
      : name(std::move(name)), type(type), required(true) {}

  // Constructor for validation usage (with required flag)
  ShaderParameterInfo(std::string name, ShaderParameterType type, bool required)
      : name(std::move(name)), type(type), required(required) {}
};

// ============================================================================
// Shader Parameter Container
// ============================================================================

class ShaderParameterContainer {
public:
  template <typename T>
  ShaderParameterContainer &Set(const std::string &name, const T &value) {
    parameters_[name] = value;
    return *this;
  }

  inline void SetFloat(const std::string &name, float f) {
    parameters_[name] = f;
  }

  inline void SetGlobalDynamicMatrix(const std::string &name,
                                     const DirectX::XMMATRIX &matrix) {
    SetMatrix(name, matrix);
  }

  inline void SetMatrix(const std::string &name,
                        const DirectX::XMMATRIX &matrix) {
    parameters_[name] = matrix;
  }

  inline void SetGlobalDynamicVector3(const std::string &name,
                                      const DirectX::XMFLOAT3 &vector) {
    SetVector3(name, vector);
  }

  inline void SetVector3(const std::string &name,
                         const DirectX::XMFLOAT3 &vector) {
    parameters_[name] = vector;
  }

  inline void SetVector4(const std::string &name,
                         const DirectX::XMFLOAT4 &vector) {
    parameters_[name] = vector;
  }

  inline void SetTexture(const std::string &name,
                         ID3D11ShaderResourceView *texture) {
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

  inline float GetFloat(const std::string &name) const {
    return Get<float>(name);
  }

  inline DirectX::XMMATRIX GetMatrix(const std::string &name) const {
    return Get<DirectX::XMMATRIX>(name);
  }

  inline DirectX::XMFLOAT3 GetVector3(const std::string &name) const {
    return Get<DirectX::XMFLOAT3>(name);
  }

  inline DirectX::XMFLOAT4 GetVector4(const std::string &name) const {
    return Get<DirectX::XMFLOAT4>(name);
  }

  inline ID3D11ShaderResourceView *GetTexture(const std::string &name) const {
    return Get<ID3D11ShaderResourceView *>(name);
  }

  inline bool HasParameter(const std::string &name) const {
    return parameters_.find(name) != parameters_.end();
  }

  // Get all parameter names (for validation)
  std::vector<std::string> GetAllParameterNames() const {
    std::vector<std::string> names;
    names.reserve(parameters_.size());
    for (const auto &kv : parameters_) {
      names.push_back(kv.first);
    }
    return names;
  }

private:
  std::unordered_map<std::string, std::any> parameters_;
};

// ============================================================================
// Shader Parameter Callback
// ============================================================================

// Callback function type for customizing shader parameters per object
using ShaderParameterCallback = std::function<void(ShaderParameterContainer &)>;