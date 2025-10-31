#pragma once

#include <string>
#include <unordered_map>
#include <vector>

// Unified parameter type definitions used by both Layout and Validator
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

// Layout class for organizing shader parameters
// Now uses unified ShaderParameterInfo structure
class ShaderParameterLayout {
public:
  void AddParameter(const std::string &name, ShaderParameterType type) {
    parameters_.push_back(ShaderParameterInfo(name, type));
  }

  const std::vector<ShaderParameterInfo> &GetParameters() const {
    return parameters_;
  }

private:
  std::vector<ShaderParameterInfo> parameters_;
};