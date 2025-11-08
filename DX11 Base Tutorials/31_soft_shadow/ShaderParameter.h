#pragma once

#include <DirectXMath.h>
#include <d3d11.h>

#include "Logger.h"

#include <cstdint>
#include <functional>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

struct ID3D10Blob;
struct ID3D11Device;

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

inline auto ShaderParameterTypeToString(ShaderParameterType type) -> const
    char * {
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
  ShaderParameterType type = ShaderParameterType::Unknown;
  bool required = true; // Whether this parameter is required (default: true)

  ShaderParameterInfo() = default;

  ShaderParameterInfo(std::string name, ShaderParameterType type,
                      bool required = true)
      : name(std::move(name)), type(type), required(required) {}
};

enum class ShaderStage : std::uint8_t {
  Vertex = 1U << 0U,
  Pixel = 1U << 1U,
  Geometry = 1U << 2U,
  Hull = 1U << 3U,
  Domain = 1U << 4U,
  Compute = 1U << 5U
};

using ShaderStageMask = std::uint8_t;

struct ReflectedParameter : public ShaderParameterInfo {
  ShaderStageMask stage_mask = 0U;

  ReflectedParameter() = default;

  ReflectedParameter(std::string name, ShaderParameterType type,
                     bool required = true, ShaderStageMask stage_mask = 0U)
      : ShaderParameterInfo(std::move(name), type, required),
        stage_mask(stage_mask) {}
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

  class ScopedOriginOverride {
  public:
    ScopedOriginOverride(ShaderParameterContainer &container,
                         ParameterOrigin origin)
        : container_(&container), previous_origin_(container.default_origin_) {
      container_->default_origin_ = origin;
    }

    ~ScopedOriginOverride() {
      if (container_ != nullptr) {
        container_->default_origin_ = previous_origin_;
      }
    }

    ScopedOriginOverride(const ScopedOriginOverride &) = delete;
    ScopedOriginOverride &operator=(const ScopedOriginOverride &) = delete;

    ScopedOriginOverride(ScopedOriginOverride &&other) noexcept
        : container_(other.container_),
          previous_origin_(other.previous_origin_) {
      other.container_ = nullptr;
    }

    ScopedOriginOverride &operator=(ScopedOriginOverride &&) = delete;

  private:
    ShaderParameterContainer *container_;
    ParameterOrigin previous_origin_;
  };

  struct BuildParametersInput {
    const ShaderParameterContainer *base_params = nullptr;
    const ShaderParameterContainer *global_params = nullptr;
    const ShaderParameterContainer *pass_params = nullptr;
    const ShaderParameterContainer *object_params = nullptr;
    const DirectX::XMMATRIX *world_matrix = nullptr;
    std::function<void(ShaderParameterContainer &)> callback;
  };

  ShaderParameterContainer() = default;

  ShaderParameterContainer(const ShaderParameterContainer &) = default;

  ShaderParameterContainer(ShaderParameterContainer &&) noexcept = default;

  auto operator=(const ShaderParameterContainer &)
      -> ShaderParameterContainer & = default;

  auto operator=(ShaderParameterContainer &&) noexcept
      -> ShaderParameterContainer & = default;

  template <typename T>
  auto Set(const std::string &name, const T &value)
      -> ShaderParameterContainer &;

  void SetFloat(const std::string &name, float value,
                ParameterOrigin origin = ParameterOrigin::Manual);

  void SetGlobalDynamicMatrix(const std::string &name,
                              const DirectX::XMMATRIX &matrix);

  void SetMatrix(const std::string &name, const DirectX::XMMATRIX &matrix,
                 ParameterOrigin origin = ParameterOrigin::Manual);

  void SetGlobalDynamicVector3(const std::string &name,
                               const DirectX::XMFLOAT3 &vector);

  void SetVector3(const std::string &name, const DirectX::XMFLOAT3 &vector,
                  ParameterOrigin origin = ParameterOrigin::Manual);

  void SetVector4(const std::string &name, const DirectX::XMFLOAT4 &vector,
                  ParameterOrigin origin = ParameterOrigin::Manual);

  void SetTexture(const std::string &name, ID3D11ShaderResourceView *texture,
                  ParameterOrigin origin = ParameterOrigin::Manual);

  void SetFloatLocked(const std::string &name, float value,
                      ParameterOrigin origin = ParameterOrigin::Manual);

  template <typename T> auto Get(const std::string &name) const -> T;

  template <typename T>
  auto TryGet(const std::string &name, T &out) const -> bool;

  auto GetFloat(const std::string &name) const -> float;

  DirectX::XMMATRIX GetMatrix(const std::string &name) const;

  DirectX::XMFLOAT3 GetVector3(const std::string &name) const;

  DirectX::XMFLOAT4 GetVector4(const std::string &name) const;

  ID3D11ShaderResourceView *GetTexture(const std::string &name) const;

  bool HasParameter(const std::string &name) const;

  ShaderParameterType GetType(const std::string &name) const;

  std::optional<ParamValue> TryGet(const std::string &name) const;

  std::vector<ShaderParameterInfo> GetAllParameterEntries() const;

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

  static ShaderParameterContainer
  BuildFinalParameters(const BuildParametersInput &input);

  static void SetTypeMismatchLoggingEnabled(bool enabled);

  static void SetOverrideLoggingEnabled(bool enabled);

  ScopedOriginOverride OverrideDefaultOrigin(ParameterOrigin origin);

  bool IsLocked(const std::string &name) const;

  inline std::vector<std::pair<std::string, std::string>>
  GetPrefixedNamePairs() const {
    std::vector<std::pair<std::string, std::string>> out;
    out.reserve(typed_parameters_.size());
    auto prefixFor = [](ParameterOrigin o) -> const char * {
      switch (o) {
      case ParameterOrigin::Global:
        return "global_";
      case ParameterOrigin::Pass:
        return "pass_";
      case ParameterOrigin::Object:
        return "object_";
      case ParameterOrigin::Callback:
        return "cb_";
      case ParameterOrigin::Manual:
        return "manual_";
      default:
        return "unknown_";
      }
    };
    for (const auto &kv : typed_parameters_) {
      auto origin_it = parameter_origins_.find(kv.first);
      ParameterOrigin origin = origin_it == parameter_origins_.end()
                                   ? ParameterOrigin::Unknown
                                   : origin_it->second;
      std::string prefixed = std::string(prefixFor(origin)) + kv.first;
      if (IsLocked(kv.first)) {
        prefixed += "[locked]";
      }
      out.emplace_back(std::move(prefixed), kv.first);
    }
    return out;
  }

  inline std::string DumpWithOriginPrefixes() const {
    std::ostringstream oss;
    oss << "ShaderParameterContainer Debug Dump (with prefixes)\n";
    for (const auto &p : GetPrefixedNamePairs()) {
      const auto &prefixed = p.first;
      const auto &original = p.second;
      oss << "  " << prefixed << " = ";
      auto it = typed_parameters_.find(original);
      if (it != typed_parameters_.end()) {
        switch (DeduceType(it->second)) {
        case ShaderParameterType::Float:
          oss << std::get<float>(it->second);
          break;
        case ShaderParameterType::Vector3: {
          auto v = std::get<DirectX::XMFLOAT3>(it->second);
          oss << "(" << v.x << "," << v.y << "," << v.z << ")";
          break;
        }
        case ShaderParameterType::Vector4: {
          auto v = std::get<DirectX::XMFLOAT4>(it->second);
          oss << "(" << v.x << "," << v.y << "," << v.z << "," << v.w << ")";
          break;
        }
        case ShaderParameterType::Matrix:
          oss << "<matrix>";
          break;
        case ShaderParameterType::Texture:
          oss << "<texture>";
          break;
        default:
          oss << "<unknown>";
          break;
        }
      }
      oss << "\n";
    }
    return oss.str();
  }

private:
  void AssignValue(const std::string &name, const ParamValue &value,
                   ParameterOrigin origin = ParameterOrigin::Manual);

  void ApplyOverrides(const ShaderParameterContainer &other,
                      ParameterOrigin origin = ParameterOrigin::Unknown);

  static ShaderParameterType DeduceType(const ParamValue &value);

  static const char *ParameterOriginToString(ParameterOrigin origin);

  void LockParameter(const std::string &name);

  std::unordered_map<std::string, ParamValue> typed_parameters_;

  std::unordered_map<std::string, ParameterOrigin> parameter_origins_;

  ParameterOrigin default_origin_ = ParameterOrigin::Manual;

  std::unordered_set<std::string> locked_parameters_;

  static bool type_mismatch_logging_enabled_;

  static bool override_logging_enabled_;

  static bool strict_validation_enabled_;

public:
  static void SetStrictValidationEnabled(bool enabled) {
    strict_validation_enabled_ = enabled;
  }
  static bool IsStrictValidationEnabled() { return strict_validation_enabled_; }
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
                                               float value,
                                               ParameterOrigin origin) {
  auto param_value = decltype(typed_parameters_)::mapped_type(value);
  AssignValue(name, param_value, origin);
}

inline void ShaderParameterContainer::SetGlobalDynamicMatrix(
    const std::string &name, const DirectX::XMMATRIX &matrix) {
  SetMatrix(name, matrix);
}

inline void ShaderParameterContainer::SetMatrix(const std::string &name,
                                                const DirectX::XMMATRIX &matrix,
                                                ParameterOrigin origin) {
  auto param_value = decltype(typed_parameters_)::mapped_type(matrix);
  AssignValue(name, param_value, origin);
}

inline void ShaderParameterContainer::SetGlobalDynamicVector3(
    const std::string &name, const DirectX::XMFLOAT3 &vector) {
  SetVector3(name, vector);
}

inline void
ShaderParameterContainer::SetVector3(const std::string &name,
                                     const DirectX::XMFLOAT3 &vector,
                                     ParameterOrigin origin) {
  auto param_value = decltype(typed_parameters_)::mapped_type(vector);
  AssignValue(name, param_value, origin);
}

inline void
ShaderParameterContainer::SetVector4(const std::string &name,
                                     const DirectX::XMFLOAT4 &vector,
                                     ParameterOrigin origin) {
  auto param_value = decltype(typed_parameters_)::mapped_type(vector);
  AssignValue(name, param_value, origin);
}

inline void
ShaderParameterContainer::SetTexture(const std::string &name,
                                     ID3D11ShaderResourceView *texture,
                                     ParameterOrigin origin) {
  auto param_value = decltype(typed_parameters_)::mapped_type(texture);
  AssignValue(name, param_value, origin);
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

inline std::vector<ShaderParameterInfo>
ShaderParameterContainer::GetAllParameterEntries() const {
  std::vector<ShaderParameterInfo> entries;
  entries.reserve(typed_parameters_.size());
  for (const auto &kv : typed_parameters_) {
    entries.emplace_back(kv.first, DeduceType(kv.second));
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

inline ShaderParameterContainer ShaderParameterContainer::BuildFinalParameters(
    const BuildParametersInput &input) {
  ShaderParameterContainer result;

  if (input.base_params) {
    result = *input.base_params;
  } else {
    if (input.global_params) {
      result.ApplyOverrides(*input.global_params, ParameterOrigin::Global);
    }
    if (input.pass_params) {
      result.ApplyOverrides(*input.pass_params, ParameterOrigin::Pass);
    }
  }

  if (input.object_params) {
    result.ApplyOverrides(*input.object_params, ParameterOrigin::Object);
  }

  if (input.world_matrix) {
    result.SetMatrix("worldMatrix", *input.world_matrix,
                     ParameterOrigin::Object);
  }

  if (input.callback) {
    auto origin_guard = result.OverrideDefaultOrigin(ParameterOrigin::Callback);
    input.callback(result);
  }

  return result;
}

inline ShaderParameterContainer::ScopedOriginOverride
ShaderParameterContainer::OverrideDefaultOrigin(ParameterOrigin origin) {
  return ScopedOriginOverride(*this, origin);
}

inline void ShaderParameterContainer::AssignValue(const std::string &name,
                                                  const ParamValue &value,
                                                  ParameterOrigin origin) {
  ParameterOrigin effective_origin = origin;
  if (effective_origin == ParameterOrigin::Manual &&
      default_origin_ != ParameterOrigin::Manual) {
    effective_origin = default_origin_;
  }
  auto iter = typed_parameters_.find(name);
  if (locked_parameters_.find(name) != locked_parameters_.end() &&
      iter != typed_parameters_.end()) {
    if (override_logging_enabled_) {
      std::ostringstream oss;
      oss << "Parameter \"" << name
          << "\" locked; ignoring override attempt from "
          << ParameterOriginToString(origin) << ".";
      Logger::SetModule("ShaderParameterContainer");
      Logger::LogWarning(oss.str());
    }
    if (strict_validation_enabled_) {
      throw std::runtime_error(
          std::string(
              "StrictValidation: attempt to override locked parameter: ") +
          name);
    }
    return;
  }
  if (iter != typed_parameters_.end()) {
    const auto previous_origin_iter = parameter_origins_.find(name);
    const ParameterOrigin previous_origin =
        previous_origin_iter != parameter_origins_.end()
            ? previous_origin_iter->second
            : ParameterOrigin::Unknown;

    ParameterOrigin resolved_origin = effective_origin;
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
  ParameterOrigin resolved_origin = effective_origin;
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
    if (other.IsLocked(kv.first)) {
      LockParameter(kv.first);
    }
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

inline void ShaderParameterContainer::SetFloatLocked(const std::string &name,
                                                     float value,
                                                     ParameterOrigin origin) {
  SetFloat(name, value, origin);
  LockParameter(name);
}

inline bool ShaderParameterContainer::IsLocked(const std::string &name) const {
  return locked_parameters_.find(name) != locked_parameters_.end();
}

inline void ShaderParameterContainer::LockParameter(const std::string &name) {
  locked_parameters_.insert(name);
}

std::vector<ReflectedParameter>
ReflectShader(ID3D11Device *device, ID3D10Blob *vs_blob, ID3D10Blob *ps_blob);
