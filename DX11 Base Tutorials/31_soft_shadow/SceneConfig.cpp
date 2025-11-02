#include "SceneConfig.h"

#include <fstream>
#include <nlohmann/json.hpp>

#include "Logger.h"

namespace SceneConfig {

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
  config.refraction.bath =
      ModelConfig("refraction_bath", "./data/bath.txt", L"./data/marble01.dds");
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

    // Clear existing config
    config = SceneConfiguration();

    // Parse models
    if (j.find("models") != j.end() && j["models"].is_object()) {
      auto &models = j["models"];

      // Parse basic models (cube, sphere, ground)
      if (models.find("cube") != models.end() && models["cube"].is_object()) {
        auto &m = models["cube"];
        std::string texture_path_str = m["texture_path"].get<std::string>();
        config.models["cube"] = ModelConfig(
            "cube", m["model_path"].get<std::string>(),
            std::wstring(texture_path_str.begin(), texture_path_str.end()));
      }

      if (models.find("sphere") != models.end() &&
          models["sphere"].is_object()) {
        auto &m = models["sphere"];
        std::string texture_path_str = m["texture_path"].get<std::string>();
        config.models["sphere"] = ModelConfig(
            "sphere", m["model_path"].get<std::string>(),
            std::wstring(texture_path_str.begin(), texture_path_str.end()));
      }

      if (models.find("ground") != models.end() &&
          models["ground"].is_object()) {
        auto &m = models["ground"];
        std::string texture_path_str = m["texture_path"].get<std::string>();
        config.models["ground"] = ModelConfig(
            "ground", m["model_path"].get<std::string>(),
            std::wstring(texture_path_str.begin(), texture_path_str.end()));
      }

      // Parse PBR model
      if (models.find("pbr_sphere") != models.end() &&
          models["pbr_sphere"].is_object()) {
        auto &m = models["pbr_sphere"];
        config.pbr_models["sphere_pbr"] =
            PBRModelConfig("sphere_pbr", m["model_path"].get<std::string>(),
                           m["albedo_path"].get<std::string>(),
                           m["normal_path"].get<std::string>(),
                           m["roughmetal_path"].get<std::string>());
      }

      // Parse refraction models
      if (models.find("refraction") != models.end() &&
          models["refraction"].is_object()) {
        auto &ref = models["refraction"];

        if (ref.find("ground") != ref.end()) {
          auto &m = ref["ground"];
          std::string texture_path_str = m["texture_path"].get<std::string>();
          config.refraction.ground = ModelConfig(
              "refraction_ground", m["model_path"].get<std::string>(),
              std::wstring(texture_path_str.begin(), texture_path_str.end()));
        }

        if (ref.find("wall") != ref.end()) {
          auto &m = ref["wall"];
          std::string texture_path_str = m["texture_path"].get<std::string>();
          config.refraction.wall = ModelConfig(
              "refraction_wall", m["model_path"].get<std::string>(),
              std::wstring(texture_path_str.begin(), texture_path_str.end()));
        }

        if (ref.find("bath") != ref.end()) {
          auto &m = ref["bath"];
          std::string texture_path_str = m["texture_path"].get<std::string>();
          config.refraction.bath = ModelConfig(
              "refraction_bath", m["model_path"].get<std::string>(),
              std::wstring(texture_path_str.begin(), texture_path_str.end()));
        }

        if (ref.find("water") != ref.end()) {
          auto &m = ref["water"];
          std::string texture_path_str = m["texture_path"].get<std::string>();
          config.refraction.water = ModelConfig(
              "refraction_water", m["model_path"].get<std::string>(),
              std::wstring(texture_path_str.begin(), texture_path_str.end()));
        }
      }
    }

    // Parse render targets
    if (j.find("render_targets") != j.end() &&
        j["render_targets"].is_object()) {
      auto &targets = j["render_targets"];

      auto parseRenderTarget = [&](const std::string &name) {
        if (targets.find(name) != targets.end() && targets[name].is_object()) {
          auto &t = targets[name];
          config.render_targets[name] = RenderTargetConfig(
              name, t["width"].get<int>(), t["height"].get<int>(),
              t["depth"].get<float>(), t["near"].get<float>());
        }
      };

      parseRenderTarget("shadow_depth");
      parseRenderTarget("shadow_map");
      parseRenderTarget("downsampled_shadow");
      parseRenderTarget("horizontal_blur");
      parseRenderTarget("vertical_blur");
      parseRenderTarget("upsampled_shadow");
      parseRenderTarget("reflection_map");
      parseRenderTarget("water_refraction");
      parseRenderTarget("water_reflection");
    }

    // Parse ortho windows
    if (j.find("ortho_windows") != j.end() && j["ortho_windows"].is_object()) {
      auto &windows = j["ortho_windows"];

      if (windows.find("small_window") != windows.end() &&
          windows["small_window"].is_object()) {
        auto &w = windows["small_window"];
        config.ortho_windows["small_window"] = OrthoWindowConfig(
            "small_window", w["width"].get<int>(), w["height"].get<int>());
      }

      if (windows.find("fullscreen_window") != windows.end() &&
          windows["fullscreen_window"].is_object()) {
        auto &w = windows["fullscreen_window"];
        config.ortho_windows["fullscreen_window"] = OrthoWindowConfig(
            "fullscreen_window", w["width"].get<int>(), w["height"].get<int>());
      }
    }

    // Parse constants
    if (j.find("constants") != j.end() && j["constants"].is_object()) {
      auto &constants = j["constants"];

      if (constants.find("water_plane_height") != constants.end())
        config.constants.water_plane_height =
            constants["water_plane_height"].get<float>();
      if (constants.find("water_reflect_refract_scale") != constants.end())
        config.constants.water_reflect_refract_scale =
            constants["water_reflect_refract_scale"].get<float>();
      if (constants.find("reflection_plane_height") != constants.end())
        config.constants.reflection_plane_height =
            constants["reflection_plane_height"].get<float>();
      if (constants.find("refraction_scene_offset_x") != constants.end())
        config.constants.refraction_scene_offset_x =
            constants["refraction_scene_offset_x"].get<float>();
      if (constants.find("refraction_scene_offset_y") != constants.end())
        config.constants.refraction_scene_offset_y =
            constants["refraction_scene_offset_y"].get<float>();
      if (constants.find("refraction_scene_offset_z") != constants.end())
        config.constants.refraction_scene_offset_z =
            constants["refraction_scene_offset_z"].get<float>();
      if (constants.find("refraction_ground_scale") != constants.end())
        config.constants.refraction_ground_scale =
            constants["refraction_ground_scale"].get<float>();
    }

    Logger::SetModule("SceneConfig");
    Logger::LogError("Successfully loaded configuration from: " + filepath);
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
