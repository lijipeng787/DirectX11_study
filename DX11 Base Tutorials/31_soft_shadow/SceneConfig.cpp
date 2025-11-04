#include "SceneConfig.h"

#include <fstream>
#include <nlohmann/json.hpp>

#include "Logger.h"
#include "ConfigValidator.h"

namespace SceneConfig {

// Helper functions for string conversion
namespace {
  inline std::wstring ToWString(const std::string& str) {
    return std::wstring(str.begin(), str.end());
  }
  
  inline std::wstring GetWString(const nlohmann::json& j, const std::string& key) {
    if (j.find(key) != j.end() && j[key].is_string()) {
      return ToWString(j[key].get<std::string>());
    }
    return L"";
  }
  
  // Parse basic model configuration
  ModelConfig ParseModelConfig(const nlohmann::json& j, const std::string& name) {
    if (!j.is_object()) return ModelConfig();
    
    std::string model_path = j.value("model_path", "");
    std::wstring texture_path = GetWString(j, "texture_path");
    
    return ModelConfig(name, model_path, texture_path);
  }
  
  // Parse PBR model configuration
  PBRModelConfig ParsePBRModelConfig(const nlohmann::json& j, const std::string& name) {
    if (!j.is_object()) return PBRModelConfig();
    
    return PBRModelConfig(
      name,
      j.value("model_path", ""),
      j.value("albedo_path", ""),
      j.value("normal_path", ""),
      j.value("roughmetal_path", "")
    );
  }
  
  // Parse render target configuration
  RenderTargetConfig ParseRenderTargetConfig(const nlohmann::json& j, const std::string& name) {
    if (!j.is_object()) return RenderTargetConfig();
    
    return RenderTargetConfig(
      name,
      j.value("width", 0),
      j.value("height", 0),
      j.value("depth", 0.0f),
      j.value("near", 0.0f)
    );
  }
  
  // Parse ortho window configuration
  OrthoWindowConfig ParseOrthoWindowConfig(const nlohmann::json& j, const std::string& name) {
    if (!j.is_object()) return OrthoWindowConfig();
    
    return OrthoWindowConfig(
      name,
      j.value("width", 0),
      j.value("height", 0)
    );
  }
}

SceneConfiguration::SceneConfiguration() {
  // Initialize with default configurations
  // This serves as fallback when JSON loading is not available
}

SceneConfiguration GetDefaultConfiguration() {
  SceneConfiguration config;

  // Default model configurations
  config.models["cube"] =
      ModelConfig("cube", "./data/cube.txt", L"./data/wall01.dds");
  config.models["sphere"] =
      ModelConfig("sphere", "./data/sphere.txt", L"./data/ice.dds");
  config.models["ground"] =
      ModelConfig("ground", "./data/plane01.txt", L"./data/blue01.dds");

  // Default PBR model configuration
  config.pbr_models["sphere_pbr"] =
      PBRModelConfig("sphere_pbr", "./data/sphere.txt", "./data/pbr_albedo.tga",
                     "./data/pbr_normal.tga", "./data/pbr_roughmetal.tga");

  // Default refraction scene models
  config.refraction.ground =
      ModelConfig("refraction_ground", "./data/ground_refraction.txt",
                  L"./data/ground01.dds");
  config.refraction.wall =
      ModelConfig("refraction_wall", "./data/wall.txt", L"./data/wall01.dds");
  config.refraction.water = ModelConfig("refraction_water", "./data/water.txt",
                                        L"./data/water01.dds");

  // Default render target configurations
  config.render_targets["shadow_depth"] =
      RenderTargetConfig("shadow_depth", 1024, 1024, 1000.0f, 1.0f);
  config.render_targets["shadow_map"] =
      RenderTargetConfig("shadow_map", 1024, 1024, 1000.0f, 1.0f);
  config.render_targets["downsampled_shadow"] =
      RenderTargetConfig("downsample", 512, 512, 100.0f, 1.0f);
  config.render_targets["horizontal_blur"] =
      RenderTargetConfig("horizontal_blur", 512, 512, 1000.0f, 0.1f);
  config.render_targets["vertical_blur"] =
      RenderTargetConfig("vertical_blur", 512, 512, 1000.0f, 0.1f);
  config.render_targets["upsampled_shadow"] =
      RenderTargetConfig("upsample", 1024, 1024, 1000.0f, 0.1f);

  // Default ortho window configurations
  config.ortho_windows["small_window"] =
      OrthoWindowConfig("small_window", 512, 512);
  config.ortho_windows["fullscreen_window"] =
      OrthoWindowConfig("fullscreen_window", 1024, 1024);

  // Constants remain at default values
  // (already initialized in struct)

  return config;
}

bool LoadFromJson(SceneConfiguration &config, const std::string &filepath) {
  Logger::SetModule("SceneConfig");

  try {
    std::ifstream file(filepath);
    if (!file.is_open()) {
      Logger::LogError("Could not open config file: " + filepath +
                       ". Using default configuration.");
      config = GetDefaultConfiguration();
      return false;
    }

    nlohmann::json j;
    file >> j;
    file.close();

    // Validate configuration before parsing
    ConfigValidator validator;
    auto validation = validator.ValidateSceneConfig(j);
    
    if (!validation.success) {
      Logger::SetModule("SceneConfig");
      Logger::LogError("Configuration validation failed:");
      for (const auto& error : validation.errors) {
        Logger::LogError("  Error: " + error);
      }
      config = GetDefaultConfiguration();
      return false;
    }
    
    // Log warnings
    for (const auto& warning : validation.warnings) {
      Logger::SetModule("SceneConfig");
      Logger::LogError("Validation warning: " + warning);
    }

    // Clear existing config
    config = SceneConfiguration();

    // Parse models
    if (j.find("models") != j.end() && j["models"].is_object()) {
      auto &models = j["models"];

      // Parse basic models using iterator (excluding special cases)
      for (auto& [key, value] : models.items()) {
        if (key == "refraction" || key == "pbr_sphere") {
          continue; // Handle separately
        }
        
        if (value.is_object()) {
          config.models[key] = ParseModelConfig(value, key);
        }
      }

      // Parse PBR model
      if (models.find("pbr_sphere") != models.end() &&
          models["pbr_sphere"].is_object()) {
        config.pbr_models["sphere_pbr"] = 
            ParsePBRModelConfig(models["pbr_sphere"], "sphere_pbr");
      }

      // Parse refraction models
      if (models.find("refraction") != models.end() &&
          models["refraction"].is_object()) {
        auto &ref = models["refraction"];

        if (ref.find("ground") != ref.end() && ref["ground"].is_object()) {
          config.refraction.ground = ParseModelConfig(ref["ground"], "refraction_ground");
        }

        if (ref.find("wall") != ref.end() && ref["wall"].is_object()) {
          config.refraction.wall = ParseModelConfig(ref["wall"], "refraction_wall");
        }

        if (ref.find("water") != ref.end() && ref["water"].is_object()) {
          config.refraction.water = ParseModelConfig(ref["water"], "refraction_water");
        }
      }
    }

    // Parse render targets
    if (j.find("render_targets") != j.end() &&
        j["render_targets"].is_object()) {
      auto &targets = j["render_targets"];

      // Parse all render targets using iterator
      for (auto& [key, value] : targets.items()) {
        if (value.is_object()) {
          config.render_targets[key] = ParseRenderTargetConfig(value, key);
        }
      }
    }

    // Parse ortho windows
    if (j.find("ortho_windows") != j.end() && j["ortho_windows"].is_object()) {
      auto &windows = j["ortho_windows"];

      // Parse all ortho windows using iterator
      for (auto& [key, value] : windows.items()) {
        if (value.is_object()) {
          config.ortho_windows[key] = ParseOrthoWindowConfig(value, key);
        }
      }
    }

    // Parse constants
    if (j.find("constants") != j.end() && j["constants"].is_object()) {
      auto &constants = j["constants"];

      // Parse constants using value() method for safe access
      if (constants.find("water_plane_height") != constants.end()) {
        config.constants.water_plane_height = constants["water_plane_height"].get<float>();
      }
      if (constants.find("water_reflect_refract_scale") != constants.end()) {
        config.constants.water_reflect_refract_scale = constants["water_reflect_refract_scale"].get<float>();
      }
      if (constants.find("reflection_plane_height") != constants.end()) {
        config.constants.reflection_plane_height = constants["reflection_plane_height"].get<float>();
      }
      if (constants.find("refraction_scene_offset_x") != constants.end()) {
        config.constants.refraction_scene_offset_x = constants["refraction_scene_offset_x"].get<float>();
      }
      if (constants.find("refraction_scene_offset_y") != constants.end()) {
        config.constants.refraction_scene_offset_y = constants["refraction_scene_offset_y"].get<float>();
      }
      if (constants.find("refraction_scene_offset_z") != constants.end()) {
        config.constants.refraction_scene_offset_z = constants["refraction_scene_offset_z"].get<float>();
      }
      if (constants.find("refraction_ground_scale") != constants.end()) {
        config.constants.refraction_ground_scale = constants["refraction_ground_scale"].get<float>();
      }
    }

    Logger::SetModule("SceneConfig");
  Logger::LogInfo("Successfully loaded configuration from: " + filepath);
    return true;

  } catch (const std::exception &e) {
    Logger::SetModule("SceneConfig");
    Logger::LogError("Error parsing JSON config: " + std::string(e.what()) +
                     ". Using default configuration.");
    config = GetDefaultConfiguration();
    return false;
  }
}

} // namespace SceneConfig
