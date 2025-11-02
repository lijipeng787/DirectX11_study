#pragma once

#include <string>
#include <unordered_map>

// Forward declarations
class Model;
class PBRModel;
class RenderTexture;
class OrthoWindow;
class IShader;

namespace SceneConfig {

// Configuration structures for different asset types

struct ModelConfig {
  std::string name;
  std::string model_path;
  std::wstring texture_path;

  ModelConfig() = default;
  ModelConfig(std::string name, std::string model_path,
              std::wstring texture_path)
      : name(std::move(name)), model_path(std::move(model_path)),
        texture_path(std::move(texture_path)) {}
};

struct PBRModelConfig {
  std::string name;
  std::string model_path;
  std::string albedo_path;
  std::string normal_path;
  std::string roughmetal_path;

  PBRModelConfig() = default;
  PBRModelConfig(std::string name, std::string model_path,
                 std::string albedo_path, std::string normal_path,
                 std::string roughmetal_path)
      : name(std::move(name)), model_path(std::move(model_path)),
        albedo_path(std::move(albedo_path)),
        normal_path(std::move(normal_path)),
        roughmetal_path(std::move(roughmetal_path)) {}
};

struct RenderTargetConfig {
  std::string name;
  int width;
  int height;
  float depth;
  float near_plane;

  RenderTargetConfig() = default;
  RenderTargetConfig(std::string name, int width, int height, float depth,
                     float near_plane)
      : name(std::move(name)), width(width), height(height), depth(depth),
        near_plane(near_plane) {}
};

struct OrthoWindowConfig {
  std::string name;
  int width;
  int height;

  OrthoWindowConfig() = default;
  OrthoWindowConfig(std::string name, int width, int height)
      : name(std::move(name)), width(width), height(height) {}
};

// Complete scene configuration
struct SceneConfiguration {
  // Model configurations
  std::unordered_map<std::string, ModelConfig> models;
  std::unordered_map<std::string, PBRModelConfig> pbr_models;

  // Refraction scene models
  struct RefractionModels {
    ModelConfig ground;
    ModelConfig wall;
    ModelConfig bath;
    ModelConfig water;
  } refraction;

  // Render target configurations
  std::unordered_map<std::string, RenderTargetConfig> render_targets;

  // Refraction render targets
  struct RefractionTargets {
    RenderTargetConfig refraction_map;
    RenderTargetConfig water_reflection_map;
  } refraction_targets;

  // Ortho window configurations
  std::unordered_map<std::string, OrthoWindowConfig> ortho_windows;

  // Constant values
  struct Constants {
    float water_plane_height = 2.75f;
    float water_reflect_refract_scale = 0.01f;
    float reflection_plane_height = 1.0f;
    float refraction_scene_offset_x = 15.0f;
    float refraction_scene_offset_y = 0.0f;
    float refraction_scene_offset_z = 0.0f;
    float refraction_ground_scale = 0.5f;
  } constants;

  SceneConfiguration();
};

// Load configuration from JSON file
// Returns true if successful, false otherwise
bool LoadFromJson(SceneConfiguration &config, const std::string &filepath);

// Get default configuration (hardcoded fallback)
SceneConfiguration GetDefaultConfiguration();

} // namespace SceneConfig
