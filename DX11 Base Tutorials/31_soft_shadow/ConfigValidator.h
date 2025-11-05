#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace SceneConfig {

class ConfigValidator {
public:
  struct ValidationResult {
    bool success;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;

    ValidationResult() : success(true) {}
  };

  // Validate scene configuration JSON
  ValidationResult ValidateSceneConfig(const nlohmann::json &j);

  // Validate scene JSON
  ValidationResult ValidateSceneJson(const nlohmann::json &j);

private:
  // Helper: Check if JSON contains required field
  bool HasRequiredField(const nlohmann::json &j, const std::string &field_name,
                        std::vector<std::string> &errors);

  // Helper: Validate model configuration
  void ValidateModelConfig(const nlohmann::json &j,
                           const std::string &model_name,
                           ValidationResult &result);

  // Helper: Validate render target configuration
  void ValidateRenderTargetConfig(const nlohmann::json &j,
                                  const std::string &target_name,
                                  ValidationResult &result);

  // Helper: Validate ortho window configuration
  void ValidateOrthoWindowConfig(const nlohmann::json &j,
                                 const std::string &window_name,
                                 ValidationResult &result);
};

} // namespace SceneConfig
