#pragma once

#include <DirectXMath.h>
#include <d3d11.h>

#include "Logger.h"

#include <functional>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
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
  Sampler,
  Float,
  Unknown // Used by Validator for unknown types
};

inline const char *ShaderParameterTypeToString(ShaderParameterType type) {
  switch (type) {
  case ShaderParameterType::Matrix:
    return "Matrix";
  case ShaderParameterType::Vector3:
    return "Vector3";
  case ShaderParameterType::Vector4:
    return "Vector4";
  case ShaderParameterType::Texture:
    return "Texture";
  case ShaderParameterType::Sampler:
    return "Sampler";
  case ShaderParameterType::Float:
    return "Float";
  default:
    return "Unknown";
  }
}

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

using ShaderParameterValueVariant =
    std::variant<DirectX::XMMATRIX, DirectX::XMFLOAT3, DirectX::XMFLOAT4, float,
                 ID3D11ShaderResourceView *>;

// ============================================================================
// Shader Parameter Container
// ============================================================================

class ShaderParameterContainer {
public:
  using ParamValue = ShaderParameterValueVariant;

  enum class ParameterOrigin {
    Unknown,
    Manual,
    Global,
    Pass,
    Object,
    Callback
  };

  struct ParameterEntry {
    std::string name;
    ShaderParameterType type;
  };

  ShaderParameterContainer() = default;
  ShaderParameterContainer(const ShaderParameterContainer &) = default;
  ShaderParameterContainer(ShaderParameterContainer &&) noexcept = default;
  ShaderParameterContainer &
  operator=(const ShaderParameterContainer &) = default;
  ShaderParameterContainer &
  operator=(ShaderParameterContainer &&) noexcept = default;

  template <typename T>
  ShaderParameterContainer &Set(const std::string &name, const T &value);

  void SetFloat(const std::string &name, float value);
  void SetGlobalDynamicMatrix(const std::string &name,
                              const DirectX::XMMATRIX &matrix);
  void SetMatrix(const std::string &name, const DirectX::XMMATRIX &matrix);
  void SetGlobalDynamicVector3(const std::string &name,
                               const DirectX::XMFLOAT3 &vector);
  void SetVector3(const std::string &name, const DirectX::XMFLOAT3 &vector);
  void SetVector4(const std::string &name, const DirectX::XMFLOAT4 &vector);
  void SetTexture(const std::string &name, ID3D11ShaderResourceView *texture);

  template <typename T> T Get(const std::string &name) const;
  template <typename T> bool TryGet(const std::string &name, T &out) const;

  float GetFloat(const std::string &name) const;
  DirectX::XMMATRIX GetMatrix(const std::string &name) const;
  DirectX::XMFLOAT3 GetVector3(const std::string &name) const;
  DirectX::XMFLOAT4 GetVector4(const std::string &name) const;
  ID3D11ShaderResourceView *GetTexture(const std::string &name) const;

  bool HasParameter(const std::string &name) const;
  ShaderParameterType GetType(const std::string &name) const;
  std::optional<ParamValue> TryGet(const std::string &name) const;

  std::vector<ParameterEntry> GetAllParameterEntries() const;
  std::vector<std::string> GetAllParameterNames() const;

  static ShaderParameterContainer
  MergeWithPriority(const ShaderParameterContainer &lower,
                    const ShaderParameterContainer &higher,
                    ParameterOrigin lower_origin = ParameterOrigin::Unknown,
                    ParameterOrigin higher_origin = ParameterOrigin::Unknown);
  static ShaderParameterContainer
  ChainMerge(const ShaderParameterContainer &global,
             const ShaderParameterContainer &pass,
             const ShaderParameterContainer *object = nullptr,
             const ShaderParameterContainer *callback = nullptr);

  static void SetTypeMismatchLoggingEnabled(bool enabled);
  static void SetOverrideLoggingEnabled(bool enabled);

private:
  void AssignValue(const std::string &name, const ParamValue &value,
                   ParameterOrigin origin = ParameterOrigin::Manual);
  void ApplyOverrides(const ShaderParameterContainer &other,
                      ParameterOrigin origin = ParameterOrigin::Unknown);
  static ShaderParameterType DeduceType(const ParamValue &value);
  static const char *ParameterOriginToString(ParameterOrigin origin);

  std::unordered_map<std::string, ParamValue> typed_parameters_;
  std::unordered_map<std::string, ParameterOrigin> parameter_origins_;
  static bool type_mismatch_logging_enabled_;
  static bool override_logging_enabled_;
};

// ============================================================================
// Shader Parameter Callback
// ============================================================================

// Callback function type for customizing shader parameters per object
using ShaderParameterCallback = std::function<void(ShaderParameterContainer &)>;

// ============================================================================
// Template Implementations
// ============================================================================

namespace detail {
template <typename T> struct always_false : std::false_type {};
} // namespace detail

template <typename T>
ShaderParameterContainer &ShaderParameterContainer::Set(const std::string &name,
                                                        const T &value) {
  if constexpr (std::is_same_v<std::decay_t<T>, DirectX::XMMATRIX>) {
    SetMatrix(name, value);
  } else if constexpr (std::is_same_v<std::decay_t<T>, DirectX::XMFLOAT3>) {
    SetVector3(name, value);
  } else if constexpr (std::is_same_v<std::decay_t<T>, DirectX::XMFLOAT4>) {
    SetVector4(name, value);
  } else if constexpr (std::is_same_v<std::decay_t<T>, float>) {
    SetFloat(name, value);
  } else if constexpr (std::is_same_v<std::decay_t<T>,
                                      ID3D11ShaderResourceView *>) {
    SetTexture(name, value);
  } else if constexpr (std::is_same_v<std::decay_t<T>, std::nullptr_t>) {
    SetTexture(name, nullptr);
  } else {
    static_assert(
        detail::always_false<T>::value,
        "Unsupported parameter type for ShaderParameterContainer::Set");
  }
  return *this;
}

template <typename T>
T ShaderParameterContainer::Get(const std::string &name) const {
  auto it = typed_parameters_.find(name);
  if (it == typed_parameters_.end()) {
    throw std::runtime_error("Parameter not found: " + name);
  }
  if (const auto *value = std::get_if<T>(&it->second)) {
    return *value;
  }
  throw std::runtime_error("Type mismatch for parameter: " + name);
}

template <typename T>
bool ShaderParameterContainer::TryGet(const std::string &name, T &out) const {
  auto it = typed_parameters_.find(name);
  if (it == typed_parameters_.end()) {
    return false;
  }
  if (const auto *value = std::get_if<T>(&it->second)) {
    out = *value;
    return true;
  }
  return false;
}

inline void ShaderParameterContainer::SetFloat(const std::string &name,
                                               float value) {
  auto param_value = decltype(typed_parameters_)::mapped_type(value);
  AssignValue(name, param_value);
}

inline void ShaderParameterContainer::SetGlobalDynamicMatrix(
    const std::string &name, const DirectX::XMMATRIX &matrix) {
  SetMatrix(name, matrix);
}

inline void
ShaderParameterContainer::SetMatrix(const std::string &name,
                                    const DirectX::XMMATRIX &matrix) {
  auto param_value = decltype(typed_parameters_)::mapped_type(matrix);
  AssignValue(name, param_value);
}

inline void ShaderParameterContainer::SetGlobalDynamicVector3(
    const std::string &name, const DirectX::XMFLOAT3 &vector) {
  SetVector3(name, vector);
}

inline void
ShaderParameterContainer::SetVector3(const std::string &name,
                                     const DirectX::XMFLOAT3 &vector) {
  auto param_value = decltype(typed_parameters_)::mapped_type(vector);
  AssignValue(name, param_value);
}

inline void
ShaderParameterContainer::SetVector4(const std::string &name,
                                     const DirectX::XMFLOAT4 &vector) {
  auto param_value = decltype(typed_parameters_)::mapped_type(vector);
  AssignValue(name, param_value);
}

inline void
ShaderParameterContainer::SetTexture(const std::string &name,
                                     ID3D11ShaderResourceView *texture) {
  auto param_value = decltype(typed_parameters_)::mapped_type(texture);
  AssignValue(name, param_value);
}

inline float ShaderParameterContainer::GetFloat(const std::string &name) const {
  return Get<float>(name);
}

inline DirectX::XMMATRIX
ShaderParameterContainer::GetMatrix(const std::string &name) const {
  return Get<DirectX::XMMATRIX>(name);
}

inline DirectX::XMFLOAT3
ShaderParameterContainer::GetVector3(const std::string &name) const {
  return Get<DirectX::XMFLOAT3>(name);
}

inline DirectX::XMFLOAT4
ShaderParameterContainer::GetVector4(const std::string &name) const {
  return Get<DirectX::XMFLOAT4>(name);
}

inline ID3D11ShaderResourceView *
ShaderParameterContainer::GetTexture(const std::string &name) const {
  return Get<ID3D11ShaderResourceView *>(name);
}

inline bool
ShaderParameterContainer::HasParameter(const std::string &name) const {
  return typed_parameters_.find(name) != typed_parameters_.end();
}

inline ShaderParameterType
ShaderParameterContainer::GetType(const std::string &name) const {
  auto iter = typed_parameters_.find(name);
  if (iter == typed_parameters_.end()) {
    return ShaderParameterType::Unknown;
  }
  return DeduceType(iter->second);
}

inline std::optional<ShaderParameterContainer::ParamValue>
ShaderParameterContainer::TryGet(const std::string &name) const {
  auto iter = typed_parameters_.find(name);
  if (iter == typed_parameters_.end()) {
    return std::nullopt;
  }
  return iter->second;
}

inline std::vector<ShaderParameterContainer::ParameterEntry>
ShaderParameterContainer::GetAllParameterEntries() const {
  std::vector<ParameterEntry> entries;
  entries.reserve(typed_parameters_.size());
  for (const auto &kv : typed_parameters_) {
    entries.push_back({kv.first, DeduceType(kv.second)});
  }
  return entries;
}

inline std::vector<std::string>
ShaderParameterContainer::GetAllParameterNames() const {
  std::vector<std::string> names;
  names.reserve(typed_parameters_.size());
  for (const auto &kv : typed_parameters_) {
    names.push_back(kv.first);
  }
  return names;
}

inline ShaderParameterContainer ShaderParameterContainer::MergeWithPriority(
    const ShaderParameterContainer &lower,
    const ShaderParameterContainer &higher, ParameterOrigin lower_origin,
    ParameterOrigin higher_origin) {
  ShaderParameterContainer result;
  result.ApplyOverrides(lower, lower_origin);
  result.ApplyOverrides(higher, higher_origin);
  return result;
}

inline ShaderParameterContainer
ShaderParameterContainer::ChainMerge(const ShaderParameterContainer &global,
                                     const ShaderParameterContainer &pass,
                                     const ShaderParameterContainer *object,
                                     const ShaderParameterContainer *callback) {
  ShaderParameterContainer result;
  result.ApplyOverrides(global, ParameterOrigin::Global);
  result.ApplyOverrides(pass, ParameterOrigin::Pass);
  if (object != nullptr) {
    result.ApplyOverrides(*object, ParameterOrigin::Object);
  }
  if (callback != nullptr) {
    result.ApplyOverrides(*callback, ParameterOrigin::Callback);
  }
  return result;
}

inline void ShaderParameterContainer::AssignValue(
    const std::string &name,
    const typename decltype(typed_parameters_)::mapped_type &value,
    ParameterOrigin origin) {
  auto iter = typed_parameters_.find(name);
  if (iter != typed_parameters_.end()) {
    const auto previous_origin_iter = parameter_origins_.find(name);
    const ParameterOrigin previous_origin =
        previous_origin_iter != parameter_origins_.end()
            ? previous_origin_iter->second
            : ParameterOrigin::Unknown;

    ParameterOrigin resolved_origin = origin;
    if (resolved_origin == ParameterOrigin::Unknown) {
      resolved_origin = previous_origin != ParameterOrigin::Unknown
                            ? previous_origin
                            : ParameterOrigin::Manual;
    }

    auto existing_type = DeduceType(iter->second);
    auto incoming_type = DeduceType(value);
    if (existing_type != incoming_type) {
      std::ostringstream oss;
      oss << "Parameter \"" << name << "\" type mismatch: existing="
          << ShaderParameterTypeToString(existing_type)
          << ", incoming=" << ShaderParameterTypeToString(incoming_type);
      if (type_mismatch_logging_enabled_) {
        Logger::SetModule("ShaderParameterContainer");
        Logger::LogError(oss.str());
      }
      throw std::runtime_error(oss.str());
    }

    if (override_logging_enabled_ && previous_origin != resolved_origin &&
        resolved_origin != ParameterOrigin::Unknown &&
        previous_origin != ParameterOrigin::Unknown) {
      std::ostringstream oss;
      oss << "Parameter \"" << name
          << "\" overridden: " << ParameterOriginToString(previous_origin)
          << " -> " << ParameterOriginToString(resolved_origin);
      Logger::SetModule("ShaderParameterContainer");
      Logger::LogWarning(oss.str());
    }

    iter->second = value;
    parameter_origins_[name] = resolved_origin;
    return;
  }
  ParameterOrigin resolved_origin = origin;
  if (resolved_origin == ParameterOrigin::Unknown) {
    resolved_origin = ParameterOrigin::Manual;
  }
  typed_parameters_.emplace(name, value);
  parameter_origins_[name] = resolved_origin;
}

inline void
ShaderParameterContainer::ApplyOverrides(const ShaderParameterContainer &other,
                                         ParameterOrigin origin) {
  for (const auto &kv : other.typed_parameters_) {
    ParameterOrigin resolved_origin = origin;
    if (resolved_origin == ParameterOrigin::Unknown) {
      auto origin_iter = other.parameter_origins_.find(kv.first);
      if (origin_iter != other.parameter_origins_.end()) {
        resolved_origin = origin_iter->second;
      } else {
        resolved_origin = ParameterOrigin::Manual;
      }
    }
    AssignValue(kv.first, kv.second, resolved_origin);
  }
}

inline ShaderParameterType ShaderParameterContainer::DeduceType(
    const typename decltype(typed_parameters_)::mapped_type &value) {
  return std::visit(
      [](auto &&arg) -> ShaderParameterType {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, DirectX::XMMATRIX>) {
          return ShaderParameterType::Matrix;
        } else if constexpr (std::is_same_v<T, DirectX::XMFLOAT3>) {
          return ShaderParameterType::Vector3;
        } else if constexpr (std::is_same_v<T, DirectX::XMFLOAT4>) {
          return ShaderParameterType::Vector4;
        } else if constexpr (std::is_same_v<T, float>) {
          return ShaderParameterType::Float;
        } else if constexpr (std::is_same_v<T, ID3D11ShaderResourceView *>) {
          return ShaderParameterType::Texture;
        }
        return ShaderParameterType::Unknown;
      },
      value);
}

inline const char *
ShaderParameterContainer::ParameterOriginToString(ParameterOrigin origin) {
  switch (origin) {
  case ParameterOrigin::Manual:
    return "Manual";
  case ParameterOrigin::Global:
    return "Global";
  case ParameterOrigin::Pass:
    return "Pass";
  case ParameterOrigin::Object:
    return "Object";
  case ParameterOrigin::Callback:
    return "Callback";
  default:
    return "Unknown";
  }
}