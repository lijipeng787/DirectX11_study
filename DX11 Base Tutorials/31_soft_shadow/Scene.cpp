#include "Scene.h"

#include <DirectXMath.h>
#include <fstream>
#include <unordered_map>

#include "ConfigValidator.h"
#include "Logger.h"
#include "Model.h"
#include "RenderTexture.h"
#include "RenderableObject.h"
#include "ResourceRegistry.h"
#include "ShaderParameterContainer.h"
#include "orthowindow.h"

#include <nlohmann/json.hpp>

using namespace DirectX;
using namespace SceneConfig;

// Render tags (moved from Graphics.cpp)
static constexpr auto write_depth_tag = "write_depth";
static constexpr auto write_shadow_tag = "write_shadow";
static constexpr auto down_sample_tag = "down_sample";
static constexpr auto horizontal_blur_tag = "horizontal_blur";
static constexpr auto vertical_blur_tag = "vertical_blur";
static constexpr auto up_sample_tag = "up_sample";
static constexpr auto pbr_tag = "pbr";
static constexpr auto final_tag = "final";
static constexpr auto reflection_tag = "reflection";
static constexpr auto scene_light_tag = "scene_light";
static constexpr auto refraction_pass_tag = "refraction_pass";
static constexpr auto water_reflection_tag = "water_reflection";
static constexpr auto diffuse_lighting_tag = "diffuse_lighting";

bool Scene::Initialize(const std::string &scene_file,
                       StandardRenderGroup *cube_group,
                       StandardRenderGroup *pbr_group) {
  Clear();

  // Try to load from JSON file first
  if (!scene_file.empty()) {
    if (LoadFromJson(scene_file, cube_group, pbr_group)) {
      return true;
    }
    // Fall back to hardcoded if JSON loading fails
    Logger::SetModule("Scene");
    Logger::LogError("Failed to load scene from JSON: " + scene_file +
                     ". Using hardcoded scene definition.");
  }

  // Fallback to hardcoded scene definition
  BuildSceneObjects(cube_group, pbr_group);
  return true;
}

bool Scene::LoadFromJson(const std::string &scene_file,
                         StandardRenderGroup *cube_group,
                         StandardRenderGroup *pbr_group) {
  Logger::SetModule("Scene");

  try {
    std::ifstream file(scene_file);
    if (!file.is_open()) {
      Logger::LogError("Could not open scene file: " + scene_file);
      return false;
    }

    nlohmann::json j;
    file >> j;
    file.close();

    // Validate scene JSON before parsing
    ConfigValidator validator;
    auto validation = validator.ValidateSceneJson(j);

    if (!validation.success) {
      Logger::SetModule("Scene");
      Logger::LogError("Scene JSON validation failed:");
      for (const auto &error : validation.errors) {
        Logger::LogError("  Error: " + error);
      }
      return false;
    }

    // Log warnings
    for (const auto &warning : validation.warnings) {
      Logger::SetModule("Scene");
      Logger::LogError("Validation warning: " + warning);
    }

    Clear();
    return BuildSceneObjectsFromJson(j, cube_group, pbr_group);

  } catch (const std::exception &e) {
    Logger::LogError("Error parsing scene JSON: " + std::string(e.what()));
    return false;
  }
}

void Scene::AddRenderable(std::shared_ptr<IRenderable> renderable) {
  if (renderable) {
    renderable_objects_.push_back(renderable);
  }
}

std::shared_ptr<IRenderable>
Scene::GetRenderable(const std::string &name) const {
  auto it = named_renderables_.find(name);
  if (it != named_renderables_.end()) {
    return it->second;
  }
  return nullptr;
}

void Scene::Clear() {
  renderable_objects_.clear();
  named_renderables_.clear();
  animation_configs_.clear();
  initial_transforms_.clear();
  rotation_states_.clear(); // Clear animation states
}

const AnimationConfig &
Scene::GetAnimationConfig(std::shared_ptr<IRenderable> renderable) const {
  static const AnimationConfig default_config; // Returns disabled config
  auto it = animation_configs_.find(renderable);
  if (it != animation_configs_.end()) {
    return it->second;
  }
  return default_config;
}

const DirectX::XMMATRIX &
Scene::GetInitialTransform(std::shared_ptr<IRenderable> renderable) const {
  static const DirectX::XMMATRIX identity = DirectX::XMMatrixIdentity();
  auto it = initial_transforms_.find(renderable);
  if (it != initial_transforms_.end()) {
    return it->second;
  }
  return identity;
}

float &Scene::GetRotationState(std::shared_ptr<IRenderable> renderable) {
  return rotation_states_[renderable];
}

void Scene::CleanupAnimationStates(
    const std::vector<std::shared_ptr<IRenderable>> &active_objects) {
  // Remove rotation states for objects that no longer exist
  for (auto it = rotation_states_.begin(); it != rotation_states_.end();) {
    if (std::find(active_objects.begin(), active_objects.end(), it->first) ==
        active_objects.end()) {
      it = rotation_states_.erase(it);
    } else {
      ++it;
    }
  }
}

void Scene::Update(float deltaTime) {
  // Placeholder for future scene updates (animations, etc.)
  (void)deltaTime;
}

std::shared_ptr<RenderableObject> Scene::CreateTexturedModelObject(
    std::shared_ptr<Model> model, std::shared_ptr<IShader> shader,
    const XMMATRIX &worldMatrix, bool enable_reflection) {
  auto obj = std::make_shared<RenderableObject>(model, shader);
  obj->SetWorldMatrix(worldMatrix);
  obj->AddTag(write_depth_tag);
  obj->AddTag(write_shadow_tag);
  obj->AddTag(final_tag);
  if (enable_reflection) {
    obj->AddTag(reflection_tag);
  }
  obj->SetParameterCallback([model](ShaderParameterContainer &params) {
    params.SetTexture("texture", model->GetTexture());
  });
  return obj;
}

std::shared_ptr<RenderableObject> Scene::CreatePostProcessObject(
    std::shared_ptr<OrthoWindow> window, std::shared_ptr<IShader> shader,
    const std::string &tag, std::shared_ptr<RenderTexture> texture) {
  auto obj = std::make_shared<RenderableObject>(window, shader);
  obj->AddTag(tag);
  obj->SetParameterCallback([texture](ShaderParameterContainer &p) {
    p.SetTexture("texture", texture->GetShaderResourceView());
  });
  return obj;
}

std::shared_ptr<RenderableObject>
Scene::CreatePBRModelObject(std::shared_ptr<PBRModel> model,
                            std::shared_ptr<IShader> shader,
                            const XMMATRIX &worldMatrix) {
  auto obj = std::make_shared<RenderableObject>(model, shader);
  obj->SetWorldMatrix(worldMatrix);
  obj->AddTag(write_depth_tag);
  obj->AddTag(write_shadow_tag);
  obj->AddTag(pbr_tag);
  return obj;
}

void Scene::BuildSceneObjects(StandardRenderGroup *cube_group,
                              StandardRenderGroup *pbr_group) {
  // Add renderable objects
  auto cube_model = ResourceRegistry::GetInstance().Get<Model>("cube");
  auto soft_shadow_shader = ResourceRegistry::GetInstance().Get<IShader>("soft_shadow");

  {
    auto cube_object = CreateTexturedModelObject(
        cube_model, soft_shadow_shader, XMMatrixTranslation(-2.5f, 2.0f, 0.0f));
    renderable_objects_.push_back(cube_object);
  }

  {
    auto sphere_model = ResourceRegistry::GetInstance().Get<Model>("sphere");
    auto sphere_object =
        CreateTexturedModelObject(sphere_model, soft_shadow_shader,
                                  XMMatrixTranslation(2.5f, 2.0f, 0.0f));
    renderable_objects_.push_back(sphere_object);
  }

  {
    auto sphere_pbr_model = ResourceRegistry::GetInstance().Get<PBRModel>("pbr_sphere");
    auto pbr_shader = ResourceRegistry::GetInstance().Get<IShader>("pbr");
    auto pbr_sphere_object = CreatePBRModelObject(
        sphere_pbr_model, pbr_shader, XMMatrixTranslation(0.0f, 2.0f, -2.0f));
    if (pbr_group) {
      pbr_group->AddRenderable(pbr_sphere_object);
    }
    renderable_objects_.push_back(pbr_sphere_object);
  }

  auto texture_shader = ResourceRegistry::GetInstance().Get<IShader>("texture");
  auto small_window = ResourceRegistry::GetInstance().Get<OrthoWindow>("small_window");

  {
    auto shadow_tex = ResourceRegistry::GetInstance().Get<RenderTexture>("shadow_map");
    auto down_sample_object = CreatePostProcessObject(
        small_window, texture_shader, down_sample_tag, shadow_tex);
    down_sample_object->AddTag("skip_culling");
    renderable_objects_.push_back(down_sample_object);
  }

  {
    auto horizontal_blur_shader =
        ResourceRegistry::GetInstance().Get<IShader>("horizontal_blur");
    auto downsample_tex = 
        ResourceRegistry::GetInstance().Get<RenderTexture>("downsampled_shadow");
    auto horizontal_blur_object =
        CreatePostProcessObject(small_window, horizontal_blur_shader,
                                horizontal_blur_tag, downsample_tex);
    horizontal_blur_object->AddTag("skip_culling");
    renderable_objects_.push_back(horizontal_blur_object);
  }

  {
    auto vertical_blur_shader = 
        ResourceRegistry::GetInstance().Get<IShader>("vertical_blur");
    auto h_blur_tex = 
        ResourceRegistry::GetInstance().Get<RenderTexture>("horizontal_blur");
    auto vertical_blur_object = CreatePostProcessObject(
        small_window, vertical_blur_shader, vertical_blur_tag, h_blur_tex);
    vertical_blur_object->AddTag("skip_culling");
    renderable_objects_.push_back(vertical_blur_object);
  }

  {
    auto fullscreen_window = ResourceRegistry::GetInstance().Get<OrthoWindow>("fullscreen_window");
    auto v_blur_tex = ResourceRegistry::GetInstance().Get<RenderTexture>("vertical_blur");
    auto up_sample_object = CreatePostProcessObject(
        fullscreen_window, texture_shader, up_sample_tag, v_blur_tex);
    up_sample_object->AddTag("skip_culling");
    renderable_objects_.push_back(up_sample_object);
  }

  {
    auto ground_model = ResourceRegistry::GetInstance().Get<Model>("ground");
    auto ground_object =
        CreateTexturedModelObject(ground_model, soft_shadow_shader,
                                  XMMatrixTranslation(0.0f, 1.0f, 0.0f), false);
    ground_object->SetParameterCallback(
        [ground_model](ShaderParameterContainer &params) {
          params.SetTexture("texture", ground_model->GetTexture());
          params.SetFloat("reflectionBlend", 0.5f);
        });
    renderable_objects_.push_back(ground_object);
  }

  // Add diffuse lighting shader demo object
  {
    auto diffuse_lighting_shader =
        ResourceRegistry::GetInstance().Get<IShader>("diffuse_lighting");
    auto diffuse_lighting_cube_obj =
        std::make_shared<RenderableObject>(cube_model, diffuse_lighting_shader);
    diffuse_lighting_cube_obj->SetWorldMatrix(
        XMMatrixTranslation(6.0f, 2.0f, 3.0f));
    diffuse_lighting_cube_obj->AddTag(diffuse_lighting_tag);
    diffuse_lighting_cube_obj->SetParameterCallback(
        [cube_model](ShaderParameterContainer &params) {
          params.SetTexture("texture", cube_model->GetTexture());
        });
    named_renderables_["diffuse_lighting_cube"] = diffuse_lighting_cube_obj;
    renderable_objects_.push_back(diffuse_lighting_cube_obj);
  }

  for (int i = 0; i < 5; i++) {
    float xPos = -6.5f + i * 3;
    float yPos = 5.5f + i * 1;
    float zPos = -12.0f;

    auto cube_obj =
        CreateTexturedModelObject(cube_model, soft_shadow_shader,
                                  XMMatrixTranslation(xPos, yPos, zPos) *
                                      XMMatrixScaling(0.3f, 0.3f, 0.3f),
                                  true);

    if (cube_group) {
      cube_group->AddRenderable(cube_obj);
    }
    renderable_objects_.push_back(cube_obj);
  }
}

// Helper methods implementation - use ResourceRegistry directly
std::shared_ptr<Model>
Scene::GetModelByName(const std::string &name) const {
  return ResourceRegistry::GetInstance().Get<Model>(name);
}

std::shared_ptr<PBRModel>
Scene::GetPBRModelByName(const std::string &name) const {
  return ResourceRegistry::GetInstance().Get<PBRModel>(name);
}

std::shared_ptr<IShader>
Scene::GetShaderByName(const std::string &name) const {
  return ResourceRegistry::GetInstance().Get<IShader>(name);
}

std::shared_ptr<RenderTexture>
Scene::GetRenderTextureByName(const std::string &name) const {
  return ResourceRegistry::GetInstance().Get<RenderTexture>(name);
}

std::shared_ptr<OrthoWindow>
Scene::GetOrthoWindowByName(const std::string &name) const {
  return ResourceRegistry::GetInstance().Get<OrthoWindow>(name);
}

AnimationConfig
Scene::ParseAnimation(const nlohmann::json &animation_json) const {
  AnimationConfig config;

  if (!animation_json.is_object()) {
    return config;
  }

  // Parse rotation animation
  if (animation_json.find("rotate") != animation_json.end()) {
    const auto &rotate_json = animation_json["rotate"];
    if (rotate_json.is_object()) {
      config.enabled = true;

      // Parse axis (default to Y)
      std::string axis_str = rotate_json.value("axis", "y");
      if (axis_str == "x" || axis_str == "X") {
        config.axis = AnimationConfig::RotationAxis::X;
      } else if (axis_str == "z" || axis_str == "Z") {
        config.axis = AnimationConfig::RotationAxis::Z;
      } else {
        config.axis = AnimationConfig::RotationAxis::Y; // Default to Y
      }

      // Parse speed (degrees per second)
      if (rotate_json.find("speed") != rotate_json.end()) {
        config.speed = rotate_json["speed"].get<float>();
      }

      // Parse initial angle (optional, default to 0)
      if (rotate_json.find("initial") != rotate_json.end()) {
        config.initial = rotate_json["initial"].get<float>();
      }
    }
  }

  return config;
}

DirectX::XMMATRIX
Scene::ParseTransform(const nlohmann::json &transform_json) const {
  if (!transform_json.is_object()) {
    return XMMatrixIdentity();
  }

  // Initialize matrices
  XMMATRIX translation = XMMatrixIdentity();
  XMMATRIX rotation = XMMatrixIdentity();
  XMMATRIX scale = XMMatrixIdentity();

  // Parse position
  if (transform_json.find("position") != transform_json.end()) {
    const auto &pos = transform_json["position"];
    if (pos.is_array() && pos.size() >= 3) {
      float x = pos[0].get<float>();
      float y = pos[1].get<float>();
      float z = pos[2].get<float>();
      translation = XMMatrixTranslation(x, y, z);
    }
  }

  // Parse rotation
  if (transform_json.find("rotation") != transform_json.end()) {
    const auto &rot = transform_json["rotation"];
    if (rot.is_array() && rot.size() >= 3) {
      float rx = rot[0].get<float>();
      float ry = rot[1].get<float>();
      float rz = rot[2].get<float>();
      // Assume rotation is in radians (can be extended to support degrees)
      rotation = XMMatrixRotationRollPitchYaw(rx, ry, rz);
    }
  }

  // Parse scale
  if (transform_json.find("scale") != transform_json.end()) {
    const auto &s = transform_json["scale"];
    if (s.is_number()) {
      float uniform_scale = s.get<float>();
      scale = XMMatrixScaling(uniform_scale, uniform_scale, uniform_scale);
    } else if (s.is_array() && s.size() >= 3) {
      float sx = s[0].get<float>();
      float sy = s[1].get<float>();
      float sz = s[2].get<float>();
      scale = XMMatrixScaling(sx, sy, sz);
    }
  }

  // Combine: Scale * Rotation * Translation (SRT order)
  return scale * rotation * translation;
}

bool Scene::BuildSceneObjectsFromJson(const nlohmann::json &j,
                                      StandardRenderGroup *cube_group,
                                      StandardRenderGroup *pbr_group) {
  Logger::SetModule("Scene");

  try {
    if (j.find("objects") == j.end() || !j["objects"].is_array()) {
      Logger::LogError("Scene JSON: missing or invalid 'objects' array");
      return false;
    }

    for (const auto &obj_json : j["objects"]) {
      if (!obj_json.is_object())
        continue;

      std::string type = obj_json.value("type", "");
      std::string name = obj_json.value("name", "");

      // Get model
      std::shared_ptr<Model> model;
      std::shared_ptr<PBRModel> pbr_model;
      if (obj_json.find("model") != obj_json.end()) {
        std::string model_name = obj_json["model"].get<std::string>();
        if (type == "PBRModel") {
          pbr_model = GetPBRModelByName(model_name);
        } else {
          model = GetModelByName(model_name);
        }
      }

      // Get shader
      std::shared_ptr<IShader> shader;
      if (obj_json.find("shader") != obj_json.end()) {
        std::string shader_name = obj_json["shader"].get<std::string>();
        shader = GetShaderByName(shader_name);
      }

      if (!shader) {
        Logger::LogError("Scene object missing shader: " + name);
        continue;
      }

      // Parse transform
      XMMATRIX world_matrix = XMMatrixIdentity();
      if (obj_json.find("transform") != obj_json.end()) {
        const nlohmann::json &transform_json = obj_json["transform"];
        world_matrix = ParseTransform(transform_json);
      }

      // Create object
      std::shared_ptr<RenderableObject> obj;
      if (type == "PBRModel" && pbr_model) {
        obj = CreatePBRModelObject(pbr_model, shader, world_matrix);
      } else if (type == "PostProcess") {
        // Post-processing object
        std::string ortho_window_name =
            obj_json.value("ortho_window", "small_window");
        std::string render_texture_name = obj_json.value("render_texture", "");
        auto ortho_window = GetOrthoWindowByName(ortho_window_name);
        auto render_texture =
            GetRenderTextureByName(render_texture_name);

        if (ortho_window && render_texture &&
            obj_json.find("tag") != obj_json.end()) {
          std::string tag = obj_json["tag"].get<std::string>();
          obj = CreatePostProcessObject(ortho_window, shader, tag,
                                        render_texture);
          // PostProcess objects always skip culling
          obj->AddTag("skip_culling");
        } else {
          Logger::LogError("Scene PostProcess object missing resources: " +
                           name);
          continue;
        }
      } else if (model) {
        // Regular textured model
        bool enable_reflection = obj_json.value("enable_reflection", true);
        obj = CreateTexturedModelObject(model, shader, world_matrix,
                                        enable_reflection);
      } else {
        Logger::LogError("Scene object missing model: " + name);
        continue;
      }

      if (!obj) {
        Logger::LogError("Failed to create scene object: " + name);
        continue;
      }

      // Parse tags (add additional tags beyond default ones)
      if (obj_json.find("tags") != obj_json.end() &&
          obj_json["tags"].is_array()) {
        for (const auto &tag : obj_json["tags"]) {
          if (tag.is_string()) {
            std::string tag_str = tag.get<std::string>();
            // Skip default tags that are already added by Create* functions
            if (tag_str != "write_depth" && tag_str != "write_shadow" &&
                tag_str != "final" && tag_str != "reflection" &&
                tag_str != "pbr") {
              obj->AddTag(tag_str);
            }
          }
        }
      }

      // Parse parameters callback (for texture, reflectionBlend, etc.)
      if (obj_json.find("parameters") != obj_json.end()) {
        const auto &params_json = obj_json["parameters"];
        if (params_json.is_object()) {
          // Capture model and params_json by copy for lambda
          std::shared_ptr<Model> captured_model = model;
          nlohmann::json captured_params_json = params_json;

          obj->SetParameterCallback(
              [captured_params_json,
               captured_model](ShaderParameterContainer &container_params) {
                if (captured_params_json.find("texture") !=
                        captured_params_json.end() &&
                    captured_model) {
                  container_params.SetTexture("texture",
                                              captured_model->GetTexture());
                }
                if (captured_params_json.find("reflectionBlend") !=
                    captured_params_json.end()) {
                  container_params.SetFloat(
                      "reflectionBlend",
                      captured_params_json["reflectionBlend"].get<float>());
                }
              });
        }
      }

      // Parse animation configuration
      AnimationConfig animation_config;
      if (obj_json.find("animation") != obj_json.end()) {
        animation_config = ParseAnimation(obj_json["animation"]);
        if (animation_config.enabled) {
          animation_configs_[obj] = animation_config;
          // Store initial transform for animated objects
          initial_transforms_[obj] = world_matrix;
        }
      }

      // Add to groups (for backward compatibility, but animation takes
      // precedence)
      if (obj_json.find("groups") != obj_json.end() &&
          obj_json["groups"].is_array()) {
        for (const auto &group_name : obj_json["groups"]) {
          if (group_name.is_string()) {
            std::string group_str = group_name.get<std::string>();
            if (group_str == "cube_group" && cube_group) {
              cube_group->AddRenderable(obj);
            } else if (group_str == "pbr_group" && pbr_group) {
              pbr_group->AddRenderable(obj);
            }
          }
        }
      }

      // Store named objects
      if (!name.empty()) {
        named_renderables_[name] = obj;
      }

      renderable_objects_.push_back(obj);
    }

    Logger::LogInfo("Loaded " + std::to_string(renderable_objects_.size()) +
                    " objects from JSON");
    return true;

  } catch (const std::exception &e) {
    Logger::LogError("Error building scene from JSON: " +
                     std::string(e.what()));
    return false;
  }
}
