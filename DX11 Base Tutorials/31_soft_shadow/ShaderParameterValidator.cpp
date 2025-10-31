#include "ShaderParameterValidator.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>

void ShaderParameterValidator::RegisterShader(
    const std::string &shader_name,
    const std::vector<ShaderParameterInfo> &parameters) {
  shader_parameters_[shader_name] = parameters;
}

bool ShaderParameterValidator::ValidateParameter(
    const std::string &parameter_name, ShaderParameterType expected_type) const {
  // 基础验证：检查参数名格式
  if (!RenderGraphNaming::IsValidParameterName(parameter_name)) {
    return false;
  }
  // 类型验证需要在实际使用中通过ShaderParameterContainer进行
  return true;
}

bool ShaderParameterValidator::ValidatePassParameters(
    const std::string &pass_name, const std::string &shader_name,
    const std::unordered_set<std::string> &provided_parameters,
    ValidationMode mode) const {

  // 检查着色器是否已注册
  auto it = shader_parameters_.find(shader_name);
  if (it == shader_parameters_.end()) {
    if (mode == ValidationMode::Strict) {
      std::cerr << "[ShaderParameterValidator] ERROR: Shader \"" << shader_name
                << "\" used in pass \"" << pass_name
                << "\" is not registered. Cannot validate parameters."
                << std::endl;
      return false;
    }
    return true; // 警告模式：未注册的着色器跳过验证
  }

  const auto &required_params = it->second;
  std::vector<std::string> missing_params;
  std::vector<std::string> invalid_params;

  // 检查必需参数
  for (const auto &param_info : required_params) {
    if (param_info.required) {
      if (provided_parameters.find(param_info.name) ==
          provided_parameters.end()) {
        missing_params.push_back(param_info.name);
      }
    }
  }

  // 检查未知参数（不在注册列表中的参数）
  for (const auto &provided : provided_parameters) {
    bool found = false;
    for (const auto &param_info : required_params) {
      if (param_info.name == provided) {
        found = true;
        break;
      }
    }
    if (!found) {
      invalid_params.push_back(provided);
    }
  }

  // 根据模式处理结果
  bool has_errors = !missing_params.empty();
  bool has_warnings = !invalid_params.empty();

  if (has_errors || has_warnings) {
    std::ostringstream oss;
    oss << "[ShaderParameterValidator] Pass \"" << pass_name
        << "\" (Shader: \"" << shader_name << "\"):\n";

    if (has_errors) {
      oss << "  Missing required parameters: ";
      for (size_t i = 0; i < missing_params.size(); ++i) {
        if (i > 0)
          oss << ", ";
        oss << missing_params[i];
      }
      oss << "\n";
    }

    if (has_warnings) {
      oss << "  Unknown parameters (not registered): ";
      for (size_t i = 0; i < invalid_params.size(); ++i) {
        if (i > 0)
          oss << ", ";
        oss << invalid_params[i];
      }
      oss << "\n";
    }

    if (mode == ValidationMode::Strict) {
      std::cerr << "ERROR: " << oss.str();
      return false;
    } else if (mode == ValidationMode::Warning) {
      std::cerr << "WARNING: " << oss.str();
      return true; // 警告模式不阻止执行
    }
  }

  return true;
}

std::vector<std::string> ShaderParameterValidator::GetMissingParameters(
    const std::string &shader_name,
    const std::unordered_set<std::string> &provided_parameters) const {

  std::vector<std::string> missing;

  auto it = shader_parameters_.find(shader_name);
  if (it == shader_parameters_.end()) {
    return missing;
  }

  for (const auto &param_info : it->second) {
    if (param_info.required &&
        provided_parameters.find(param_info.name) ==
            provided_parameters.end()) {
      missing.push_back(param_info.name);
    }
  }

  return missing;
}

std::vector<std::string> ShaderParameterValidator::GetInvalidParameters(
    const std::string &shader_name,
    const std::unordered_set<std::string> &provided_parameters) const {

  std::vector<std::string> invalid;

  auto it = shader_parameters_.find(shader_name);
  if (it == shader_parameters_.end()) {
    return invalid;
  }

  std::unordered_set<std::string> registered_names;
  for (const auto &param_info : it->second) {
    registered_names.insert(param_info.name);
  }

  for (const auto &provided : provided_parameters) {
    if (registered_names.find(provided) == registered_names.end()) {
      invalid.push_back(provided);
    }
  }

  return invalid;
}

bool ShaderParameterValidator::IsShaderRegistered(
    const std::string &shader_name) const {
  return shader_parameters_.find(shader_name) != shader_parameters_.end();
}

std::vector<ShaderParameterInfo>
ShaderParameterValidator::GetShaderParameters(
    const std::string &shader_name) const {
  auto it = shader_parameters_.find(shader_name);
  if (it != shader_parameters_.end()) {
    return it->second;
  }
  return {};
}

void ShaderParameterValidator::Clear() { shader_parameters_.clear(); }

// 命名约定辅助函数实现
namespace RenderGraphNaming {

std::string ResourceNameToParameterName(const std::string &resource_name) {
  if (resource_name.empty())
    return resource_name;

  std::string result = resource_name;
  // 首字母小写
  if (!result.empty()) {
    result[0] = static_cast<char>(std::tolower(result[0]));
  }
  return result;
}

std::string
ResourceNameToTextureParameterName(const std::string &resource_name) {
  std::string base = ResourceNameToParameterName(resource_name);
  // 如果已经有Texture后缀，不重复添加
  if (base.size() >= 7 &&
      base.substr(base.size() - 7) == "Texture") {
    return base;
  }
  return base + "Texture";
}

bool IsValidParameterName(const std::string &name) {
  if (name.empty())
    return false;

  // 必须是小写字母开头（camelCase）
  if (!std::islower(name[0]))
    return false;

  // 只能包含字母、数字和下划线
  for (char c : name) {
    if (!std::isalnum(c) && c != '_') {
      return false;
    }
  }

  return true;
}

bool IsValidResourceName(const std::string &name) {
  if (name.empty())
    return false;

  // 必须是大写字母开头（PascalCase）
  if (!std::isupper(name[0]))
    return false;

  // 只能包含字母、数字和下划线
  for (char c : name) {
    if (!std::isalnum(c) && c != '_') {
      return false;
    }
  }

  return true;
}

} // namespace RenderGraphNaming

