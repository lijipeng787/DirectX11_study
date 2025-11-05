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

void ShaderParameterValidator::RegisterShader(
    const std::string &shader_name,
    std::initializer_list<ShaderParameterInfo> parameters) {
  RegisterShader(shader_name, std::vector<ShaderParameterInfo>(
                                  parameters.begin(), parameters.end()));
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
  // Basic validation: check parameter name format
  if (!RenderGraphNaming::IsValidParameterName(parameter_name)) {
    return false;
  }
  // Type validation needs to be done through ShaderParameterContainer in actual
  // use
  return true;
}

bool ShaderParameterValidator::ValidatePassParameters(
    const std::string &pass_name, const std::string &shader_name,
    const std::unordered_set<std::string> &provided_parameters,
    ValidationMode mode) const {
  std::unordered_map<std::string, ShaderParameterType> typed_parameters;
  typed_parameters.reserve(provided_parameters.size());
  for (const auto &name : provided_parameters) {
    typed_parameters.emplace(name, ShaderParameterType::Unknown);
  }
  return ValidatePassParametersInternal(pass_name, shader_name,
                                        typed_parameters, mode);
}

bool ShaderParameterValidator::ValidatePassParameters(
    const std::string &pass_name, const std::string &shader_name,
    const std::unordered_map<std::string, ShaderParameterType>
        &provided_parameters,
    ValidationMode mode) const {
  return ValidatePassParametersInternal(pass_name, shader_name,
                                        provided_parameters, mode);
}

bool ShaderParameterValidator::ValidatePassParametersInternal(
    const std::string &pass_name, const std::string &shader_name,
    const std::unordered_map<std::string, ShaderParameterType>
        &provided_parameters,
    ValidationMode mode) const {

  auto shader_it = shader_parameters_.find(shader_name);
  if (shader_it == shader_parameters_.end()) {
    if (mode == ValidationMode::Strict) {
      Logger::SetModule("ShaderParameterValidator");
      Logger::LogError("Shader \"" + shader_name + "\" used in pass \"" +
                       pass_name +
                       "\" is not registered. Cannot validate parameters.");
      return false;
    }
    return true;
  }

  const auto &required_params = shader_it->second;
  const ValidationSummary summary =
      AnalyzeProvidedParameters(required_params, provided_parameters);

  if (!summary.HasIssues()) {
    return true;
  }

  const std::string message =
      BuildValidationReport(pass_name, shader_name, required_params, summary);

  Logger::SetModule("ShaderParameterValidator");
  if (mode == ValidationMode::Strict) {
    Logger::LogError(message);
    return false;
  }
  if (mode == ValidationMode::Warning) {
    Logger::LogWarning(message);
  }

  return true;
}

ShaderParameterValidator::ValidationSummary
ShaderParameterValidator::AnalyzeProvidedParameters(
    const std::vector<ShaderParameterInfo> &required_params,
    const std::unordered_map<std::string, ShaderParameterType>
        &provided_parameters) const {
  ValidationSummary summary;
  summary.missing.reserve(required_params.size());

  for (const auto &param_info : required_params) {
    if (!param_info.required || IsGlobalParameter(param_info.name)) {
      continue;
    }
    if (provided_parameters.find(param_info.name) ==
        provided_parameters.end()) {
      summary.missing.push_back(&param_info);
    }
  }

  for (const auto &param_info : required_params) {
    const auto provided_iter = provided_parameters.find(param_info.name);
    if (provided_iter == provided_parameters.end()) {
      continue;
    }

    const auto actual_type = provided_iter->second;
    if (actual_type != ShaderParameterType::Unknown &&
        actual_type != param_info.type) {
      std::ostringstream mismatch_stream;
      mismatch_stream << param_info.name << " (expected "
                      << GetTypeName(param_info.type) << ", actual "
                      << GetTypeName(actual_type) << ")";
      summary.type_mismatches.push_back(mismatch_stream.str());
    }
  }

  std::unordered_set<std::string> registered_names;
  registered_names.reserve(required_params.size());
  for (const auto &param_info : required_params) {
    registered_names.insert(param_info.name);
  }

  for (const auto &provided : provided_parameters) {
    const std::string &provided_name = provided.first;
    if (IsGlobalParameter(provided_name)) {
      continue;
    }
    if (!RenderGraphNaming::IsValidParameterName(provided_name)) {
      continue;
    }
    if (registered_names.find(provided_name) == registered_names.end()) {
      summary.unknown.push_back(provided_name);
    }
  }

  return summary;
}

std::string ShaderParameterValidator::BuildValidationReport(
    const std::string &pass_name, const std::string &shader_name,
    const std::vector<ShaderParameterInfo> &required_params,
    const ValidationSummary &summary) const {
  std::ostringstream report_stream;
  report_stream << "[ShaderParameterValidator] Pass \"" << pass_name
                << "\" (Shader: \"" << shader_name << "\"):\n";

  if (!summary.missing.empty()) {
    report_stream << "  Missing required parameters:\n";
    for (const auto *missing : summary.missing) {
      report_stream << "    - " << missing->name << " ("
                    << GetTypeName(missing->type) << ")";
      if (missing->name.find("Texture") != std::string::npos) {
        report_stream << " - Consider using ReadAsParameter() or SetTexture()";
      }
      report_stream << "\n";
    }
  }

  if (!summary.type_mismatches.empty()) {
    report_stream << "  Type mismatches:\n";
    for (const auto &entry : summary.type_mismatches) {
      report_stream << "    - " << entry << "\n";
    }
  }

  if (!summary.unknown.empty()) {
    report_stream << "  Unknown parameters (not registered):\n";
    for (const auto &unknown_name : summary.unknown) {
      report_stream << "    - " << unknown_name;
      const std::string suggestion =
          FindSimilarParameter(unknown_name, required_params);
      if (!suggestion.empty()) {
        report_stream << " - Did you mean: " << suggestion << "?";
      }
      report_stream << "\n";
    }
  }

  return report_stream.str();
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

bool ShaderParameterValidator::ValidateParameterType(
    const ShaderParameterContainer &container,
    const std::string &parameter_name,
    ShaderParameterType expected_type) const {
  return container.GetType(parameter_name) == expected_type;
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
  case ShaderParameterType::Sampler:
    return "Sampler";
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
    const std::string &source, const std::string &target) const {
  const size_t len1 = source.size();
  const size_t len2 = target.size();

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
      if (source[i - 1] == target[j - 1]) {
        dp[i][j] = dp[i - 1][j - 1];
      } else {
        dp[i][j] = 1 + (std::min)((std::min)(dp[i - 1][j], dp[i][j - 1]),
                                  dp[i - 1][j - 1]);
      }
    }
  }

  return dp[len1][len2];
}

// Naming convention helper function implementations
namespace RenderGraphNaming {

std::string ResourceNameToParameterName(const std::string &resource_name) {
  if (resource_name.empty())
    return resource_name;

  std::string result = resource_name;
  // Lowercase first letter
  if (!result.empty()) {
    result[0] = static_cast<char>(std::tolower(result[0]));
  }
  return result;
}

std::string
ResourceNameToTextureParameterName(const std::string &resource_name) {
  std::string base = ResourceNameToParameterName(resource_name);
  // If Texture suffix already exists, don't add again
  if (base.size() >= 7 && base.substr(base.size() - 7) == "Texture") {
    return base;
  }
  return base + "Texture";
}

bool IsValidParameterName(const std::string &name) {
  if (name.empty())
    return false;

  // Must start with lowercase letter (camelCase)
  if (!std::islower(name[0]))
    return false;

  // Can only contain letters, numbers, and underscores
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

  // Must start with uppercase letter (PascalCase)
  if (!std::isupper(name[0]))
    return false;

  // Can only contain letters, numbers, and underscores
  for (char c : name) {
    if (!std::isalnum(c) && c != '_') {
      return false;
    }
  }

  return true;
}

} // namespace RenderGraphNaming
