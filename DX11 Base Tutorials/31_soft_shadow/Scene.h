#pragma once

#include <DirectXMath.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Interfaces.h"
#include "RenderableObject.h"

// Forward declarations
class Model;
class PBRModel;
class RenderTexture;
class OrthoWindow;
class IShader;

// Include nlohmann/json in header to avoid symbol conflicts
#include <nlohmann/json.hpp>

// Animation configuration structure
struct AnimationConfig {
  enum class RotationAxis { X, Y, Z };

  bool enabled = false;
  RotationAxis axis = RotationAxis::Y;
  float speed = 0.0f;   // Degrees per second
  float initial = 0.0f; // Initial angle in degrees

  AnimationConfig() = default;
  AnimationConfig(RotationAxis axis, float speed, float initial = 0.0f)
      : enabled(true), axis(axis), speed(speed), initial(initial) {}
};

// Scene resource references (non-owning)
struct SceneResourceRefs {
  struct SceneAssets {
    std::shared_ptr<Model> cube;
    std::shared_ptr<Model> sphere;
    std::shared_ptr<Model> ground;
    std::shared_ptr<PBRModel> pbr_sphere;

    struct RefractionAssets {
      std::shared_ptr<Model> ground;
      std::shared_ptr<Model> wall;
      std::shared_ptr<Model> water;
    } refraction;
  };

  struct ShaderAssets {
    std::shared_ptr<IShader> depth;
    std::shared_ptr<IShader> shadow;
    std::shared_ptr<IShader> texture;
    std::shared_ptr<IShader> horizontal_blur;
    std::shared_ptr<IShader> vertical_blur;
    std::shared_ptr<IShader> soft_shadow;
    std::shared_ptr<IShader> pbr;
    std::shared_ptr<IShader> diffuse_lighting;

    struct RefractionShaders {
      std::shared_ptr<IShader> scene_light;
      std::shared_ptr<IShader> refraction;
    } refraction;
  };

  struct RenderTargetAssets {
    std::shared_ptr<RenderTexture> shadow_map;
    std::shared_ptr<RenderTexture> downsampled_shadow;
    std::shared_ptr<RenderTexture> horizontal_blur;
    std::shared_ptr<RenderTexture> vertical_blur;
  };

  struct OrthoWindowAssets {
    std::shared_ptr<OrthoWindow> small_window;
    std::shared_ptr<OrthoWindow> fullscreen_window;
  };

  SceneAssets scene_assets;
  ShaderAssets shader_assets;
  RenderTargetAssets render_targets;
  OrthoWindowAssets ortho_windows;
};

// Scene configuration constants (non-owning)
struct SceneConstants {
  float refraction_scene_offset_x;
  float refraction_scene_offset_y;
  float refraction_scene_offset_z;
  float refraction_ground_scale;
  float water_plane_height;
};

// StandardRenderGroup forward declaration
class StandardRenderGroup;

class Scene {
public:
  Scene() = default;
  ~Scene() = default;

  // Non-copyable
  Scene(const Scene &) = delete;
  Scene &operator=(const Scene &) = delete;

public:
  // Initialize scene with resource references
  // This separates scene object creation from resource management
  // Option 1: Load from JSON file (preferred)
  bool Initialize(const SceneResourceRefs &resources,
                  const SceneConstants &constants,
                  const std::string &scene_file = "",
                  StandardRenderGroup *cube_group = nullptr,
                  StandardRenderGroup *pbr_group = nullptr);

  // Option 2: Load from JSON (explicit)
  bool LoadFromJson(const SceneResourceRefs &resources,
                    const SceneConstants &constants,
                    const std::string &scene_file,
                    StandardRenderGroup *cube_group = nullptr,
                    StandardRenderGroup *pbr_group = nullptr);

  // Get all renderable objects in the scene
  const std::vector<std::shared_ptr<IRenderable>> &GetRenderables() const {
    return renderable_objects_;
  }

  // Get specific renderable (for special handling like diffuse_lighting_cube)
  std::shared_ptr<IRenderable> GetRenderable(const std::string &name) const;

  // Add a renderable object to the scene
  void AddRenderable(std::shared_ptr<IRenderable> renderable);

  // Clear all renderable objects
  void Clear();

  // Update scene (for animations, etc.)
  void Update(float deltaTime);

public:
  // Get animation configuration for a renderable object
  const AnimationConfig &
  GetAnimationConfig(std::shared_ptr<IRenderable> renderable) const;

  // Get initial transform for a renderable object (for animation)
  const DirectX::XMMATRIX &
  GetInitialTransform(std::shared_ptr<IRenderable> renderable) const;

  // Animation state management
  // Get rotation state for an animated object (used by Graphics::Frame)
  float &GetRotationState(std::shared_ptr<IRenderable> renderable);

  // Clean up animation states for objects that no longer exist
  void CleanupAnimationStates(
      const std::vector<std::shared_ptr<IRenderable>> &active_objects);

private:
  // Helper functions for creating different types of objects
  std::shared_ptr<RenderableObject> CreateTexturedModelObject(
      std::shared_ptr<Model> model, std::shared_ptr<IShader> shader,
      const DirectX::XMMATRIX &worldMatrix, bool enable_reflection = true);

  std::shared_ptr<RenderableObject> CreatePostProcessObject(
      std::shared_ptr<OrthoWindow> window, std::shared_ptr<IShader> shader,
      const std::string &tag, std::shared_ptr<RenderTexture> texture);

  std::shared_ptr<RenderableObject>
  CreatePBRModelObject(std::shared_ptr<PBRModel> model,
                       std::shared_ptr<IShader> shader,
                       const DirectX::XMMATRIX &worldMatrix);

  // Build all scene objects from hardcoded definition (fallback)
  void BuildSceneObjects(const SceneResourceRefs &resources,
                         const SceneConstants &constants,
                         StandardRenderGroup *cube_group,
                         StandardRenderGroup *pbr_group);

  // Build scene objects from JSON object (internal use, j is already parsed)
  bool BuildSceneObjectsFromJson(const nlohmann::json &j,
                                 const SceneResourceRefs &resources,
                                 const SceneConstants &constants,
                                 StandardRenderGroup *cube_group,
                                 StandardRenderGroup *pbr_group);

  // Helper: Get model reference by name
  std::shared_ptr<Model>
  GetModelByName(const std::string &name,
                 const SceneResourceRefs &resources) const;
  std::shared_ptr<PBRModel>
  GetPBRModelByName(const std::string &name,
                    const SceneResourceRefs &resources) const;

  // Helper: Get shader reference by name
  std::shared_ptr<IShader>
  GetShaderByName(const std::string &name,
                  const SceneResourceRefs &resources) const;

  // Helper: Get render texture reference by name
  std::shared_ptr<RenderTexture>
  GetRenderTextureByName(const std::string &name,
                         const SceneResourceRefs &resources) const;

  // Helper: Get ortho window reference by name
  std::shared_ptr<OrthoWindow>
  GetOrthoWindowByName(const std::string &name,
                       const SceneResourceRefs &resources) const;

  // Helper: Parse transform from JSON
  DirectX::XMMATRIX ParseTransform(const nlohmann::json &transform_json) const;

private:
  // Helper: Parse animation from JSON
  AnimationConfig ParseAnimation(const nlohmann::json &animation_json) const;

private:
  std::vector<std::shared_ptr<IRenderable>> renderable_objects_;
  std::unordered_map<std::string, std::shared_ptr<IRenderable>>
      named_renderables_;
  // Store animation configs for each renderable object
  std::unordered_map<std::shared_ptr<IRenderable>, AnimationConfig>
      animation_configs_;
  // Store initial transforms for animated objects
  std::unordered_map<std::shared_ptr<IRenderable>, DirectX::XMMATRIX>
      initial_transforms_;
  // Store rotation states for animated objects (used by Graphics::Frame)
  std::unordered_map<std::shared_ptr<IRenderable>, float> rotation_states_;

  // Resource lookup caches (built once during initialization)
  mutable std::unordered_map<std::string, std::shared_ptr<Model>> model_cache_;
  mutable std::unordered_map<std::string, std::shared_ptr<PBRModel>>
      pbr_model_cache_;
  mutable std::unordered_map<std::string, std::shared_ptr<IShader>>
      shader_cache_;
  mutable std::unordered_map<std::string, std::shared_ptr<RenderTexture>>
      render_texture_cache_;
  mutable std::unordered_map<std::string, std::shared_ptr<OrthoWindow>>
      ortho_window_cache_;

  // Helper: Build resource caches from SceneResourceRefs
  void BuildResourceCaches(const SceneResourceRefs &resources) const;
};
