#pragma once

#include <string>
#include <unordered_map>
#include <vector>

enum class ShaderParameterType { Matrix, Vector3, Vector4, Texture, Float };

class ShaderParameterInfo {
public:
  explicit ShaderParameterInfo(std::string name, ShaderParameterType type)
      : name_(name), type_(type) {}

public:
  std::string name_;
  ShaderParameterType type_;
};

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