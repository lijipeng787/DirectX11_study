#include "ShaderParameterValidator.h"

#include "Logger.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <limits>
#include <sstream>
#include <vector>

// Undefine Windows macros that conflict with std::min/max
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

void ShaderParameterValidator::RegisterShader(
    const std::string &shader_name,
    const std::vector<ShaderParameterInfo> &parameters) {
  shader_parameters_[shader_name] = parameters;
}

void ShaderParameterValidator::RegisterGlobalParameter(
    const std::string &param_name) {
  global_parameters_.insert(param_name);
}

bool ShaderParameterValidator::IsGlobalParameter(
    const std::string &param_name) const {
  return global_parameters_.find(param_name) != global_parameters_.end();
}

bool ShaderParameterValidator::ValidateParameter(
    const std::string &parameter_name,
    ShaderParameterType expected_type) const {
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
      Logger::SetModule("ShaderParameterValidator");
      Logger::LogError("Shader \"" + shader_name + "\" used in pass \"" +
                       pass_name + "\" is not registered. Cannot validate parameters.");
      return false;
    }
    return true; // 警告模式：未注册的着色器跳过验证
  }

  const auto &required_params = it->second;
  std::vector<std::string> missing_params;
  std::vector<std::string> invalid_params;

  // 检查必需参数（排除全局参数，因为它们会在运行时提供）
  for (const auto &param_info : required_params) {
    if (param_info.required) {
      // 如果是全局参数，跳过验证（运行时提供）
      if (IsGlobalParameter(param_info.name)) {
        continue;
      }
      // 检查是否在Pass级别提供了
      if (provided_parameters.find(param_info.name) ==
          provided_parameters.end()) {
        missing_params.push_back(param_info.name);
      }
    }
  }

  // 检查未知参数（不在注册列表中的参数）
  // 注意：排除全局参数和资源名（资源名不符合camelCase命名约定）
  for (const auto &provided : provided_parameters) {
    // 跳过全局参数
    if (IsGlobalParameter(provided)) {
      continue;
    }
    // 跳过资源名（不符合camelCase命名约定，如"DepthMap"）
    if (!RenderGraphNaming::IsValidParameterName(provided)) {
      continue; // 这是资源名，不是参数名
    }

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
    oss << "[ShaderParameterValidator] Pass \"" << pass_name << "\" (Shader: \""
        << shader_name << "\"):\n";

    if (has_errors) {
      oss << "  Missing required parameters:\n";
      for (const auto &missing : missing_params) {
        // Find parameter info to get type
        ShaderParameterType param_type = ShaderParameterType::Unknown;
        for (const auto &param_info : required_params) {
          if (param_info.name == missing) {
            param_type = param_info.type;
            break;
          }
        }
        oss << "    - " << missing << " (" << GetTypeName(param_type) << ")";
        // Add helpful hints based on parameter name patterns
        if (missing.find("Texture") != std::string::npos) {
          oss << " - Consider using ReadAsParameter() or SetTexture()";
        } else if (IsGlobalParameter(missing)) {
          oss << " - This is a global parameter, should be set in Render()";
        }
        oss << "\n";
      }
    }

    if (has_warnings) {
      oss << "  Unknown parameters (not registered):\n";
      for (const auto &invalid : invalid_params) {
        oss << "    - " << invalid;
        // Try to find similar parameter name
        std::string suggestion = FindSimilarParameter(invalid, required_params);
        if (!suggestion.empty()) {
          oss << " - Did you mean: " << suggestion << "?";
        }
        oss << "\n";
      }
    }

    Logger::SetModule("ShaderParameterValidator");
    if (mode == ValidationMode::Strict) {
      Logger::LogError(oss.str());
      return false;
    } else if (mode == ValidationMode::Warning) {
      Logger::LogWarning(oss.str());
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
    if (param_info.required) {
      // 跳过全局参数（运行时提供）
      if (IsGlobalParameter(param_info.name)) {
        continue;
      }
      if (provided_parameters.find(param_info.name) ==
          provided_parameters.end()) {
        missing.push_back(param_info.name);
      }
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

  // Check for unknown parameters (consistent with ValidatePassParameters logic)
  // Exclude global parameters and resource names (not valid parameter names)
  for (const auto &provided : provided_parameters) {
    // Skip global parameters
    if (IsGlobalParameter(provided)) {
      continue;
    }
    // Skip resource names (don't match camelCase naming convention)
    if (!RenderGraphNaming::IsValidParameterName(provided)) {
      continue; // This is a resource name, not a parameter name
    }

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

std::vector<ShaderParameterInfo> ShaderParameterValidator::GetShaderParameters(
    const std::string &shader_name) const {
  auto it = shader_parameters_.find(shader_name);
  if (it != shader_parameters_.end()) {
    return it->second;
  }
  return {};
}

void ShaderParameterValidator::Clear() {
  shader_parameters_.clear();
  global_parameters_.clear();
}

// Helper methods for enhanced reporting
std::string
ShaderParameterValidator::GetTypeName(ShaderParameterType type) const {
  switch (type) {
  case ShaderParameterType::Matrix:
    return "Matrix";
  case ShaderParameterType::Vector3:
    return "Vector3";
  case ShaderParameterType::Vector4:
    return "Vector4";
  case ShaderParameterType::Texture:
    return "Texture";
  case ShaderParameterType::Float:
    return "Float";
  default:
    return "Unknown";
  }
}

std::string ShaderParameterValidator::FindSimilarParameter(
    const std::string &param_name,
    const std::vector<ShaderParameterInfo> &params) const {
  if (params.empty())
    return "";

  std::string best_match;
  int best_distance = (std::numeric_limits<int>::max)();
  const int max_distance = 3; // Maximum edit distance to consider

  for (const auto &param_info : params) {
    int distance = CalculateLevenshteinDistance(param_name, param_info.name);
    if (distance < best_distance && distance <= max_distance) {
      best_distance = distance;
      best_match = param_info.name;
    }
  }

  return best_match;
}

int ShaderParameterValidator::CalculateLevenshteinDistance(
    const std::string &s1, const std::string &s2) const {
  const size_t len1 = s1.size();
  const size_t len2 = s2.size();

  // Simple optimization: if lengths differ significantly, skip
  if (std::abs(static_cast<int>(len1) - static_cast<int>(len2)) > 5) {
    return 100; // Large distance to skip
  }

  // Create DP table
  std::vector<std::vector<int>> dp(len1 + 1, std::vector<int>(len2 + 1));

  // Initialize base cases
  for (size_t i = 0; i <= len1; ++i)
    dp[i][0] = static_cast<int>(i);
  for (size_t j = 0; j <= len2; ++j)
    dp[0][j] = static_cast<int>(j);

  // Fill DP table
  for (size_t i = 1; i <= len1; ++i) {
    for (size_t j = 1; j <= len2; ++j) {
      if (s1[i - 1] == s2[j - 1]) {
        dp[i][j] = dp[i - 1][j - 1];
      } else {
        dp[i][j] = 1 + (std::min)((std::min)(dp[i - 1][j], dp[i][j - 1]),
                                  dp[i - 1][j - 1]);
      }
    }
  }

  return dp[len1][len2];
}

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
  if (base.size() >= 7 && base.substr(base.size() - 7) == "Texture") {
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
