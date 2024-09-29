#pragma once

#include <string>
#include <unordered_map>
#include <vector>

enum class ShaderParameterType { Matrix, Vector3, Vector4, Texture, Float };

class ShaderParameterInfo {
public:
  ShaderParameterInfo(std::string name, ShaderParameterType type) {
    this->name = name;
    this->type = type;
  }

public:
  std::string name;
  ShaderParameterType type;
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