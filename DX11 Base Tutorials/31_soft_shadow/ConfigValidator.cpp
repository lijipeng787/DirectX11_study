#include "ConfigValidator.h"

#include "Logger.h"

namespace SceneConfig {

bool ConfigValidator::HasRequiredField(const nlohmann::json &j,
                                       const std::string &field_name,
                                       std::vector<std::string> &errors) {
  if (!j.contains(field_name)) {
    errors.push_back("Missing required field: '" + field_name + "'");
    return false;
  }
  return true;
}

void ConfigValidator::ValidateModelConfig(const nlohmann::json &j,
                                          const std::string &model_name,
                                          ValidationResult &result) {
  if (!j.is_object()) {
    result.errors.push_back("Model '" + model_name + "' is not an object");
    result.success = false;
    return;
  }

  if (!j.contains("model_path")) {
    result.errors.push_back("Model '" + model_name + "' missing 'model_path'");
    result.success = false;
  } else if (!j["model_path"].is_string()) {
    result.errors.push_back("Model '" + model_name +
                            "' 'model_path' must be a string");
    result.success = false;
  }

  // texture_path is optional for PBR models
  if (j.contains("texture_path") && !j["texture_path"].is_string()) {
    result.errors.push_back("Model '" + model_name +
                            "' 'texture_path' must be a string");
    result.success = false;
  }

  // Check PBR-specific fields
  if (j.contains("albedo_path") || j.contains("normal_path") ||
      j.contains("roughmetal_path")) {
    if (!j.contains("albedo_path") || !j.contains("normal_path") ||
        !j.contains("roughmetal_path")) {
      result.warnings.push_back(
          "Model '" + model_name +
          "' has incomplete PBR configuration (missing some texture paths)");
    }
  }
}

void ConfigValidator::ValidateRenderTargetConfig(const nlohmann::json &j,
                                                 const std::string &target_name,
                                                 ValidationResult &result) {
  if (!j.is_object()) {
    result.errors.push_back("RenderTarget '" + target_name +
                            "' is not an object");
    result.success = false;
    return;
  }

  if (!j.contains("width")) {
    result.errors.push_back("RenderTarget '" + target_name +
                            "' missing 'width'");
    result.success = false;
  } else if (!j["width"].is_number()) {
    result.errors.push_back("RenderTarget '" + target_name +
                            "' 'width' must be a number");
    result.success = false;
  }

  if (!j.contains("height")) {
    result.errors.push_back("RenderTarget '" + target_name +
                            "' missing 'height'");
    result.success = false;
  } else if (!j["height"].is_number()) {
    result.errors.push_back("RenderTarget '" + target_name +
                            "' 'height' must be a number");
    result.success = false;
  }

  // Check dimensions
  if (j.contains("width") && j.contains("height") && j["width"].is_number() &&
      j["height"].is_number()) {
    int width = j["width"].get<int>();
    int height = j["height"].get<int>();

    if (width > 0 && height > 0 && width != height) {
      result.warnings.push_back("RenderTarget '" + target_name +
                                "' is not square: " + std::to_string(width) +
                                "x" + std::to_string(height) +
                                " (may cause performance issues)");
    }

    if (width <= 0 && width != -1) {
      result.warnings.push_back(
          "RenderTarget '" + target_name +
          "' has invalid width: " + std::to_string(width));
    }

    if (height <= 0 && height != -1) {
      result.warnings.push_back(
          "RenderTarget '" + target_name +
          "' has invalid height: " + std::to_string(height));
    }
  }

  if (!j.contains("depth")) {
    result.errors.push_back("RenderTarget '" + target_name +
                            "' missing 'depth'");
    result.success = false;
  } else if (!j["depth"].is_number()) {
    result.errors.push_back("RenderTarget '" + target_name +
                            "' 'depth' must be a number");
    result.success = false;
  }

  if (!j.contains("near")) {
    result.errors.push_back("RenderTarget '" + target_name +
                            "' missing 'near'");
    result.success = false;
  } else if (!j["near"].is_number()) {
    result.errors.push_back("RenderTarget '" + target_name +
                            "' 'near' must be a number");
    result.success = false;
  }
}

void ConfigValidator::ValidateOrthoWindowConfig(const nlohmann::json &j,
                                                const std::string &window_name,
                                                ValidationResult &result) {
  if (!j.is_object()) {
    result.errors.push_back("OrthoWindow '" + window_name +
                            "' is not an object");
    result.success = false;
    return;
  }

  if (!j.contains("width")) {
    result.errors.push_back("OrthoWindow '" + window_name +
                            "' missing 'width'");
    result.success = false;
  } else if (!j["width"].is_number()) {
    result.errors.push_back("OrthoWindow '" + window_name +
                            "' 'width' must be a number");
    result.success = false;
  }

  if (!j.contains("height")) {
    result.errors.push_back("OrthoWindow '" + window_name +
                            "' missing 'height'");
    result.success = false;
  } else if (!j["height"].is_number()) {
    result.errors.push_back("OrthoWindow '" + window_name +
                            "' 'height' must be a number");
    result.success = false;
  }
}

ConfigValidator::ValidationResult
ConfigValidator::ValidateSceneConfig(const nlohmann::json &j) {
  ValidationResult result;

  // Check top-level structure
  if (!j.is_object()) {
    result.errors.push_back("Root element must be an object");
    result.success = false;
    return result;
  }

  // Validate models section
  if (j.contains("models")) {
    if (!j["models"].is_object()) {
      result.errors.push_back("'models' must be an object");
      result.success = false;
    } else {
      auto &models = j["models"];
      for (auto &[key, value] : models.items()) {
        if (key == "refraction") {
          // Validate refraction sub-models
          if (value.is_object()) {
            if (value.contains("ground")) {
              ValidateModelConfig(value["ground"], "refraction.ground", result);
            }
            if (value.contains("wall")) {
              ValidateModelConfig(value["wall"], "refraction.wall", result);
            }
            if (value.contains("water")) {
              ValidateModelConfig(value["water"], "refraction.water", result);
            }
          }
        } else {
          ValidateModelConfig(value, key, result);
        }
      }
    }
  } else {
    result.warnings.push_back(
        "Configuration missing 'models' section (will use defaults)");
  }

  // Validate render targets section
  if (j.contains("render_targets")) {
    if (!j["render_targets"].is_object()) {
      result.errors.push_back("'render_targets' must be an object");
      result.success = false;
    } else {
      auto &targets = j["render_targets"];
      for (auto &[key, value] : targets.items()) {
        ValidateRenderTargetConfig(value, key, result);
      }
    }
  } else {
    result.warnings.push_back(
        "Configuration missing 'render_targets' section (will use defaults)");
  }

  // Validate ortho windows section
  if (j.contains("ortho_windows")) {
    if (!j["ortho_windows"].is_object()) {
      result.errors.push_back("'ortho_windows' must be an object");
      result.success = false;
    } else {
      auto &windows = j["ortho_windows"];
      for (auto &[key, value] : windows.items()) {
        ValidateOrthoWindowConfig(value, key, result);
      }
    }
  } else {
    result.warnings.push_back(
        "Configuration missing 'ortho_windows' section (will use defaults)");
  }

  // Validate constants section (optional)
  if (j.contains("constants")) {
    if (!j["constants"].is_object()) {
      result.errors.push_back("'constants' must be an object");
      result.success = false;
    }
  }

  return result;
}

ConfigValidator::ValidationResult
ConfigValidator::ValidateSceneJson(const nlohmann::json &j) {
  ValidationResult result;

  // Check top-level structure
  if (!j.is_object()) {
    result.errors.push_back("Root element must be an object");
    result.success = false;
    return result;
  }

  // Validate objects array
  if (!j.contains("objects")) {
    result.errors.push_back("Missing required field: 'objects'");
    result.success = false;
  } else if (!j["objects"].is_array()) {
    result.errors.push_back("'objects' must be an array");
    result.success = false;
  } else {
    auto &objects = j["objects"];
    for (size_t i = 0; i < objects.size(); ++i) {
      auto &obj = objects[i];
      if (!obj.is_object()) {
        result.errors.push_back("Object at index " + std::to_string(i) +
                                " is not an object");
        result.success = false;
        continue;
      }

      // Check required fields
      if (!obj.contains("type")) {
        result.errors.push_back("Object at index " + std::to_string(i) +
                                " missing 'type'");
        result.success = false;
      }

      // Validate transform if present
      if (obj.contains("transform")) {
        if (!obj["transform"].is_object()) {
          result.errors.push_back("Object at index " + std::to_string(i) +
                                  " 'transform' must be an object");
          result.success = false;
        }
      }

      // Validate animation if present
      if (obj.contains("animation")) {
        if (!obj["animation"].is_object()) {
          result.errors.push_back("Object at index " + std::to_string(i) +
                                  " 'animation' must be an object");
          result.success = false;
        }
      }
    }
  }

  return result;
}

} // namespace SceneConfig
