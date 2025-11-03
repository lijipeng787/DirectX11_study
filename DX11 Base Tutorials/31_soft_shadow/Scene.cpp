#include "Scene.h"

#include <DirectXMath.h>
#include <fstream>
#include <unordered_map>

#include "Logger.h"
#include "Model.h"
#include "RenderTexture.h"
#include "RenderableObject.h"
#include "ShaderParameterContainer.h"
#include "orthowindow.h"

#include <nlohmann/json.hpp>

using namespace DirectX;

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

bool Scene::Initialize(const SceneResourceRefs &resources,
                       const SceneConstants &constants,
                       const std::string &scene_file,
                       StandardRenderGroup *cube_group,
                       StandardRenderGroup *pbr_group) {
  Clear();

  // Try to load from JSON file first
  if (!scene_file.empty()) {
    if (LoadFromJson(resources, constants, scene_file, cube_group, pbr_group)) {
      return true;
    }
    // Fall back to hardcoded if JSON loading fails
    Logger::SetModule("Scene");
    Logger::LogError("Failed to load scene from JSON: " + scene_file +
                     ". Using hardcoded scene definition.");
  }

  // Fallback to hardcoded scene definition
  BuildSceneObjects(resources, constants, cube_group, pbr_group);
  return true;
}

bool Scene::LoadFromJson(const SceneResourceRefs &resources,
                         const SceneConstants &constants,
                         const std::string &scene_file,
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

    Clear();
    return BuildSceneObjectsFromJson(j, resources, constants, cube_group,
                                     pbr_group);

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

void Scene::BuildSceneObjects(const SceneResourceRefs &resources,
                              const SceneConstants &constants,
                              StandardRenderGroup *cube_group,
                              StandardRenderGroup *pbr_group) {
  // Add renderable objects
  const auto &cube_model = resources.scene_assets.cube;
  const auto &soft_shadow_shader = resources.shader_assets.soft_shadow;

  {
    auto cube_object = CreateTexturedModelObject(
        cube_model, soft_shadow_shader, XMMatrixTranslation(-2.5f, 2.0f, 0.0f));
    renderable_objects_.push_back(cube_object);
  }

  {
    const auto &sphere_model = resources.scene_assets.sphere;
    auto sphere_object =
        CreateTexturedModelObject(sphere_model, soft_shadow_shader,
                                  XMMatrixTranslation(2.5f, 2.0f, 0.0f));
    renderable_objects_.push_back(sphere_object);
  }

  {
    const auto &sphere_pbr_model = resources.scene_assets.pbr_sphere;
    const auto &pbr_shader = resources.shader_assets.pbr;
    auto pbr_sphere_object = CreatePBRModelObject(
        sphere_pbr_model, pbr_shader, XMMatrixTranslation(0.0f, 2.0f, -2.0f));
    if (pbr_group) {
      pbr_group->AddRenderable(pbr_sphere_object);
    }
    renderable_objects_.push_back(pbr_sphere_object);
  }

  const auto &texture_shader = resources.shader_assets.texture;
  const auto &small_window = resources.ortho_windows.small_window;

  {
    const auto &shadow_tex = resources.render_targets.shadow_map;
    auto down_sample_object = CreatePostProcessObject(
        small_window, texture_shader, down_sample_tag, shadow_tex);
    down_sample_object->AddTag("skip_culling");
    renderable_objects_.push_back(down_sample_object);
  }

  {
    const auto &horizontal_blur_shader =
        resources.shader_assets.horizontal_blur;
    const auto &downsample_tex = resources.render_targets.downsampled_shadow;
    auto horizontal_blur_object =
        CreatePostProcessObject(small_window, horizontal_blur_shader,
                                horizontal_blur_tag, downsample_tex);
    horizontal_blur_object->AddTag("skip_culling");
    renderable_objects_.push_back(horizontal_blur_object);
  }

  {
    const auto &vertical_blur_shader = resources.shader_assets.vertical_blur;
    const auto &h_blur_tex = resources.render_targets.horizontal_blur;
    auto vertical_blur_object = CreatePostProcessObject(
        small_window, vertical_blur_shader, vertical_blur_tag, h_blur_tex);
    vertical_blur_object->AddTag("skip_culling");
    renderable_objects_.push_back(vertical_blur_object);
  }

  {
    const auto &fullscreen_window = resources.ortho_windows.fullscreen_window;
    const auto &v_blur_tex = resources.render_targets.vertical_blur;
    auto up_sample_object = CreatePostProcessObject(
        fullscreen_window, texture_shader, up_sample_tag, v_blur_tex);
    up_sample_object->AddTag("skip_culling");
    renderable_objects_.push_back(up_sample_object);
  }

  {
    const auto &ground_model = resources.scene_assets.ground;
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
    const auto &diffuse_lighting_shader =
        resources.shader_assets.diffuse_lighting;
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

  const auto &refraction_assets = resources.scene_assets.refraction;
  const auto scene_offset = XMMatrixTranslation(
      constants.refraction_scene_offset_x, constants.refraction_scene_offset_y,
      constants.refraction_scene_offset_z);

  if (refraction_assets.ground &&
      resources.shader_assets.refraction.scene_light) {
    auto ground_object = std::make_shared<RenderableObject>(
        refraction_assets.ground,
        resources.shader_assets.refraction.scene_light);
    auto ground_world = XMMatrixScaling(constants.refraction_ground_scale, 1.0f,
                                        constants.refraction_ground_scale) *
                        XMMatrixTranslation(0.0f, 1.0f, 0.0f) * scene_offset;
    ground_object->SetWorldMatrix(ground_world);
    ground_object->AddTag(scene_light_tag);
    ground_object->SetParameterCallback(
        [ground_model =
             refraction_assets.ground](ShaderParameterContainer &params) {
          params.SetTexture("texture", ground_model->GetTexture());
        });
    renderable_objects_.push_back(ground_object);
  }

  if (refraction_assets.wall &&
      resources.shader_assets.refraction.scene_light) {
    auto wall_object = std::make_shared<RenderableObject>(
        refraction_assets.wall, resources.shader_assets.refraction.scene_light);
    wall_object->SetWorldMatrix(XMMatrixTranslation(0.0f, 6.0f, 8.0f) *
                                scene_offset);
    wall_object->AddTag(scene_light_tag);
    wall_object->AddTag(water_reflection_tag);
    wall_object->SetParameterCallback([wall_model = refraction_assets.wall](
                                          ShaderParameterContainer &params) {
      params.SetTexture("texture", wall_model->GetTexture());
    });
    renderable_objects_.push_back(wall_object);
  }
}

// Helper methods implementation
std::shared_ptr<Model>
Scene::GetModelByName(const std::string &name,
                      const SceneResourceRefs &resources) const {
  if (name == "cube")
    return resources.scene_assets.cube;
  if (name == "sphere")
    return resources.scene_assets.sphere;
  if (name == "ground")
    return resources.scene_assets.ground;
  if (name == "refraction_ground" || name == "refraction.ground") {
    return resources.scene_assets.refraction.ground;
  }
  if (name == "refraction_wall" || name == "refraction.wall") {
    return resources.scene_assets.refraction.wall;
  }
  if (name == "refraction_water" || name == "refraction.water") {
    return resources.scene_assets.refraction.water;
  }
  return nullptr;
}

std::shared_ptr<PBRModel>
Scene::GetPBRModelByName(const std::string &name,
                         const SceneResourceRefs &resources) const {
  if (name == "pbr_sphere" || name == "sphere_pbr") {
    return resources.scene_assets.pbr_sphere;
  }
  return nullptr;
}

std::shared_ptr<IShader>
Scene::GetShaderByName(const std::string &name,
                       const SceneResourceRefs &resources) const {
  if (name == "depth")
    return resources.shader_assets.depth;
  if (name == "shadow")
    return resources.shader_assets.shadow;
  if (name == "texture")
    return resources.shader_assets.texture;
  if (name == "horizontal_blur")
    return resources.shader_assets.horizontal_blur;
  if (name == "vertical_blur")
    return resources.shader_assets.vertical_blur;
  if (name == "soft_shadow")
    return resources.shader_assets.soft_shadow;
  if (name == "pbr")
    return resources.shader_assets.pbr;
  if (name == "diffuse_lighting")
    return resources.shader_assets.diffuse_lighting;
  if (name == "scene_light" || name == "refraction.scene_light") {
    return resources.shader_assets.refraction.scene_light;
  }
  if (name == "refraction" || name == "refraction.refraction") {
    return resources.shader_assets.refraction.refraction;
  }
  return nullptr;
}

std::shared_ptr<RenderTexture>
Scene::GetRenderTextureByName(const std::string &name,
                              const SceneResourceRefs &resources) const {
  if (name == "shadow_map")
    return resources.render_targets.shadow_map;
  if (name == "downsampled_shadow")
    return resources.render_targets.downsampled_shadow;
  if (name == "horizontal_blur")
    return resources.render_targets.horizontal_blur;
  if (name == "vertical_blur")
    return resources.render_targets.vertical_blur;
  return nullptr;
}

std::shared_ptr<OrthoWindow>
Scene::GetOrthoWindowByName(const std::string &name,
                            const SceneResourceRefs &resources) const {
  if (name == "small_window")
    return resources.ortho_windows.small_window;
  if (name == "fullscreen_window")
    return resources.ortho_windows.fullscreen_window;
  return nullptr;
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
                                      const SceneResourceRefs &resources,
                                      const SceneConstants &constants,
                                      StandardRenderGroup *cube_group,
                                      StandardRenderGroup *pbr_group) {
  Logger::SetModule("Scene");

  try {
    if (j.find("objects") == j.end() || !j["objects"].is_array()) {
      Logger::LogError("Scene JSON: missing or invalid 'objects' array");
      return false;
    }

    const auto &scene_offset =
        XMMatrixTranslation(constants.refraction_scene_offset_x,
                            constants.refraction_scene_offset_y,
                            constants.refraction_scene_offset_z);

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
          pbr_model = GetPBRModelByName(model_name, resources);
        } else {
          model = GetModelByName(model_name, resources);
        }
      }

      // Get shader
      std::shared_ptr<IShader> shader;
      if (obj_json.find("shader") != obj_json.end()) {
        std::string shader_name = obj_json["shader"].get<std::string>();
        shader = GetShaderByName(shader_name, resources);
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

      // Apply scene offset for refraction objects
      bool is_refraction = obj_json.value("apply_scene_offset", false);
      if (is_refraction) {
        world_matrix = world_matrix * scene_offset;
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
        auto ortho_window = GetOrthoWindowByName(ortho_window_name, resources);
        auto render_texture =
            GetRenderTextureByName(render_texture_name, resources);

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
