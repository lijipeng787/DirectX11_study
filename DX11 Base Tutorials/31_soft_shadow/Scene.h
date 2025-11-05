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
  // Initialize scene from JSON configuration
  // Resources are loaded via ResourceRegistry
  bool Initialize(const std::string &scene_file = "",
                  StandardRenderGroup *cube_group = nullptr,
                  StandardRenderGroup *pbr_group = nullptr);

  // Load from JSON (explicit)
  bool LoadFromJson(const std::string &scene_file,
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
  void BuildSceneObjects(StandardRenderGroup *cube_group,
                         StandardRenderGroup *pbr_group);

  // Build scene objects from JSON object (internal use, j is already parsed)
  bool BuildSceneObjectsFromJson(const nlohmann::json &j,
                                 StandardRenderGroup *cube_group,
                                 StandardRenderGroup *pbr_group);

  // Helper: Get model reference by name from ResourceRegistry
  std::shared_ptr<Model> GetModelByName(const std::string &name) const;
  std::shared_ptr<PBRModel> GetPBRModelByName(const std::string &name) const;

  // Helper: Get shader reference by name from ResourceRegistry
  std::shared_ptr<IShader> GetShaderByName(const std::string &name) const;

  // Helper: Get render texture reference by name from ResourceRegistry
  std::shared_ptr<RenderTexture>
  GetRenderTextureByName(const std::string &name) const;

  // Helper: Get ortho window reference by name from ResourceRegistry
  std::shared_ptr<OrthoWindow>
  GetOrthoWindowByName(const std::string &name) const;

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
};
