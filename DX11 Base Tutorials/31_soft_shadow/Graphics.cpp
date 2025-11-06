#include "Graphics.h"

#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "../CommonFramework2/DirectX11Device.h"
#include "../CommonFramework2/TypeDefine.h"

#include "BoundingVolume.h"
#include "Frustum.h"
#include "Interfaces.h"
#include "Logger.h"
#include "RenderableObject.h"
#include "ResourceManager.h"
#include "ResourceRegistry.h"
#include "ShaderBase.h"
#include "ShaderParameterValidator.h"
#include "DepthShader.h"
#include "Font.h"
#include "FontShader.h"
#include "HorizontalBlurShader.h"
#include "Model.h"
#include "OrthoWindow.h"
#include "PbrShader.h"
#include "RefractionShader.h"
#include "RenderTexture.h"
#include "SceneLightShader.h"
#include "ShadowShader.h"
#include "SimpleLightShader.h"
#include "SoftShadowShader.h"
#include "Text.h"
#include "TextureShader.h"
#include "VerticalBlurShader.h"

using namespace std;
using namespace DirectX;
using namespace SceneConfig;

// Shadow map dimensions
static constexpr auto SHADOW_MAP_WIDTH = 1024;
static constexpr auto SHADOW_MAP_HEIGHT = 1024;
static constexpr auto DOWN_SAMPLE_WIDTH = SHADOW_MAP_WIDTH / 2;
static constexpr auto DOWN_SAMPLE_HEIGHT = SHADOW_MAP_HEIGHT / 2;

// Light animation constants
static constexpr auto LIGHT_MOVE_SPEED = 1.0f; // units per second
static constexpr auto LIGHT_X_MIN = -5.0f;
static constexpr auto LIGHT_X_MAX = 5.0f;
static constexpr auto LIGHT_Y_POSITION = 8.0f;
static constexpr auto LIGHT_Z_POSITION = -5.0f;

// Debug resource logging interval (seconds)
#ifdef _DEBUG
static constexpr auto DEBUG_RESOURCE_LOG_INTERVAL = 5.0f;
#endif

// Render tag constants
static constexpr const char *WRITE_DEPTH_TAG = "write_depth";
static constexpr const char *WRITE_SHADOW_TAG = "write_shadow";
static constexpr const char *DOWN_SAMPLE_TAG = "down_sample";
static constexpr const char *HORIZONTAL_BLUR_TAG = "horizontal_blur";
static constexpr const char *VERTICAL_BLUR_TAG = "vertical_blur";
static constexpr const char *UP_SAMPLE_TAG = "up_sample";
static constexpr const char *PBR_TAG = "pbr";
static constexpr const char *FINAL_TAG = "final";
static constexpr const char *REFLECTION_TAG = "reflection";
static constexpr const char *DIFFUSE_LIGHTING_TAG = "diffuse_lighting";

// 辅助函数：统一错误日志记录
namespace {
void LogGraphicsError(const std::wstring &message) {
  Logger::SetModule("Graphics");
  Logger::LogError(message);
}

void LogGraphicsError(const std::string &message) {
  Logger::SetModule("Graphics");
  Logger::LogError(message);
}

std::wstring GetResourceManagerError(const ResourceManager &rm) {
  const auto &last_error = rm.GetLastError();
  if (last_error.empty()) {
    return L"";
  }
  return L"\n" + std::wstring(last_error.begin(), last_error.end());
}
} // namespace

bool Graphics::Initialize(int screenWidth, int screenHeight, HWND hwnd) {
  // Initialize core DirectX device and resources
  if (!InitializeDevice(screenWidth, screenHeight, hwnd)) {
    return false;
  }

  // Setup camera and frustum culling system
  if (!InitializeCamera()) {
    return false;
  }

  // Setup lighting system
  if (!InitializeLight()) {
    return false;
  }

  // Load scene configuration from JSON
  SceneConfig::LoadFromJson(scene_config_, "./data/scene_config.json");

  // Load all rendering resources (models, shaders, render targets, etc.)
  if (!InitializeResources()) {
    return false;
  }

  // Setup rendering pipeline and render graph
  if (!InitializeRenderingPipeline()) {
    return false;
  }

  // Log resource statistics for debugging
  auto &resource_manager = ResourceManager::GetInstance();
  resource_manager.LogResourceStats();

  return true;
}

bool Graphics::InitializeDevice(int screenWidth, int screenHeight, HWND hwnd) {
  auto *directx11_device_ = DirectX11Device::GetD3d11DeviceInstance();

  if (!directx11_device_->Initialize(screenWidth, screenHeight, VSYNC_ENABLED,
                                     hwnd, FULL_SCREEN, SCREEN_DEPTH,
                                     SCREEN_NEAR)) {
    LogGraphicsError(L"Could not initialize Direct3D.");
    return false;
  }

  this->screenWidth = screenWidth;
  this->screenHeight = screenHeight;

  // Initialize ResourceManager for centralized resource management
  auto &resource_manager = ResourceManager::GetInstance();
  auto *device = directx11_device_->GetDevice();
  auto *device_context = directx11_device_->GetDeviceContext();

  if (!resource_manager.Initialize(device, device_context, hwnd)) {
    LogGraphicsError("Could not initialize ResourceManager.");
    return false;
  }

  // Initialize ResourceRegistry for unified resource access
  auto &registry = ResourceRegistry::GetInstance();
  if (!registry.Initialize(device, device_context, hwnd)) {
    LogGraphicsError("Could not initialize ResourceRegistry.");
    return false;
  }

  return true;
}

bool Graphics::InitializeCamera() {
  camera_ = make_unique<Camera>();
  if (nullptr == camera_) {
    return false;
  }

  // Set initial camera position and compute matrices
  camera_->SetPosition(0.0f, -1.0f, -10.0f);
  camera_->Render();
  camera_->RenderBaseViewMatrix();

  // Initialize frustum for visibility culling
  frustum_ = make_unique<FrustumClass>();
  if (nullptr == frustum_) {
    return false;
  }

  return true;
}

bool Graphics::InitializeLight() {
  light_ = make_unique<Light>();
  if (nullptr == light_) {
    return false;
  }

  // Configure light properties
  light_->SetAmbientColor(0.15f, 0.15f, 0.15f, 1.0f);
  light_->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
  light_->SetLookAt(0.0f, 0.0f, 0.0f);
  light_->GenerateProjectionMatrix(SCREEN_DEPTH, SCREEN_NEAR);

  return true;
}

bool Graphics::InitializeSceneModels() {
  auto &resource_manager = ResourceManager::GetInstance();

  // Load basic geometry models from configuration
  auto &cube_config = scene_config_.models["cube"];
  scene_assets_.cube = resource_manager.GetModel(
      cube_config.name, cube_config.model_path, cube_config.texture_path);

  auto &sphere_config = scene_config_.models["sphere"];
  scene_assets_.sphere = resource_manager.GetModel(
      sphere_config.name, sphere_config.model_path, sphere_config.texture_path);

  auto &ground_config = scene_config_.models["ground"];
  scene_assets_.ground = resource_manager.GetModel(
      ground_config.name, ground_config.model_path, ground_config.texture_path);

  if (!scene_assets_.cube || !scene_assets_.sphere || !scene_assets_.ground) {
    LogGraphicsError(L"Could not load models." +
                     GetResourceManagerError(resource_manager));
    return false;
  }

  // Load PBR model from configuration
  auto &pbr_config = scene_config_.pbr_models["sphere_pbr"];
  scene_assets_.pbr_sphere = resource_manager.GetPBRModel(
      pbr_config.name, pbr_config.model_path, pbr_config.albedo_path,
      pbr_config.normal_path, pbr_config.roughmetal_path);

  if (!scene_assets_.pbr_sphere) {
    LogGraphicsError(L"Could not load PBR model." +
                     GetResourceManagerError(resource_manager));
    return false;
  }

  return true;
}

bool Graphics::InitializeRenderTargets() {
  auto &resource_manager = ResourceManager::GetInstance();

  // Helper lambda to get width/height with screen fallback
  auto getWidth = [this](const RenderTargetConfig &config) {
    return config.width == -1 ? screenWidth : config.width;
  };
  auto getHeight = [this](const RenderTargetConfig &config) {
    return config.height == -1 ? screenHeight : config.height;
  };

  // Create shadow-related render targets from configuration
  auto &shadow_depth_config = scene_config_.render_targets["shadow_depth"];
  render_targets_.shadow_depth = resource_manager.CreateRenderTexture(
      shadow_depth_config.name, getWidth(shadow_depth_config),
      getHeight(shadow_depth_config), shadow_depth_config.depth,
      shadow_depth_config.near_plane);

  auto &shadow_map_config = scene_config_.render_targets["shadow_map"];
  render_targets_.shadow_map = resource_manager.CreateRenderTexture(
      shadow_map_config.name, getWidth(shadow_map_config),
      getHeight(shadow_map_config), shadow_map_config.depth,
      shadow_map_config.near_plane);

  auto &downsampled_config = scene_config_.render_targets["downsampled_shadow"];
  render_targets_.downsampled_shadow = resource_manager.CreateRenderTexture(
      downsampled_config.name, getWidth(downsampled_config),
      getHeight(downsampled_config), downsampled_config.depth,
      downsampled_config.near_plane);

  auto &horizontal_blur_config =
      scene_config_.render_targets["horizontal_blur"];
  render_targets_.horizontal_blur = resource_manager.CreateRenderTexture(
      horizontal_blur_config.name, getWidth(horizontal_blur_config),
      getHeight(horizontal_blur_config), horizontal_blur_config.depth,
      horizontal_blur_config.near_plane);

  auto &vertical_blur_config = scene_config_.render_targets["vertical_blur"];
  render_targets_.vertical_blur = resource_manager.CreateRenderTexture(
      vertical_blur_config.name, getWidth(vertical_blur_config),
      getHeight(vertical_blur_config), vertical_blur_config.depth,
      vertical_blur_config.near_plane);

  auto &upsampled_config = scene_config_.render_targets["upsampled_shadow"];
  render_targets_.upsampled_shadow = resource_manager.CreateRenderTexture(
      upsampled_config.name, getWidth(upsampled_config),
      getHeight(upsampled_config), upsampled_config.depth,
      upsampled_config.near_plane);

  // Create reflection/refraction render targets from configuration
  auto &reflection_map_config = scene_config_.render_targets["reflection_map"];
  render_targets_.reflection_map = resource_manager.CreateRenderTexture(
      reflection_map_config.name, getWidth(reflection_map_config),
      getHeight(reflection_map_config), reflection_map_config.depth,
      reflection_map_config.near_plane);

  // Validate all render targets were created successfully
  if (!render_targets_.shadow_depth || !render_targets_.shadow_map ||
      !render_targets_.downsampled_shadow || !render_targets_.horizontal_blur ||
      !render_targets_.vertical_blur || !render_targets_.upsampled_shadow ||
      !render_targets_.reflection_map) {
    LogGraphicsError(L"Could not create render textures.");
    return false;
  }

  return true;
}

bool Graphics::InitializeShaders() {
  auto &resource_manager = ResourceManager::GetInstance();

  // Load core rendering shaders from ResourceManager
  shader_assets_.depth = resource_manager.GetShader<DepthShader>("depth");
  shader_assets_.shadow = resource_manager.GetShader<ShadowShader>("shadow");
  shader_assets_.texture = resource_manager.GetShader<TextureShader>("texture");
  shader_assets_.horizontal_blur =
      resource_manager.GetShader<HorizontalBlurShader>("horizontal_blur");
  shader_assets_.vertical_blur =
      resource_manager.GetShader<VerticalBlurShader>("vertical_blur");
  shader_assets_.soft_shadow =
      resource_manager.GetShader<SoftShadowShader>("soft_shadow");
  shader_assets_.pbr = resource_manager.GetShader<PbrShader>("pbr");
  shader_assets_.diffuse_lighting =
      resource_manager.GetShader<SimpleLightShader>("simple_light");

  // Validate all shaders were loaded successfully
  if (!shader_assets_.depth || !shader_assets_.shadow ||
      !shader_assets_.texture || !shader_assets_.horizontal_blur ||
      !shader_assets_.vertical_blur || !shader_assets_.soft_shadow ||
      !shader_assets_.pbr || !shader_assets_.diffuse_lighting) {
    LogGraphicsError(L"Could not load shaders.");
    return false;
  }

  return true;
}

bool Graphics::InitializeFontSystem() {
  auto &resource_manager = ResourceManager::GetInstance();

  // Load font shader
  auto font_shader = resource_manager.GetShader<FontShader>("font");
  if (!font_shader) {
    LogGraphicsError(L"Could not load font shader.");
    return false;
  }

  // Load font resource
  auto font = std::make_shared<Font>();
  if (!font->Initialize("./data/fontdata.txt", L"./data/font.dds",
                        resource_manager.GetDevice())) {
    LogGraphicsError(L"Could not initialize font.");
    return false;
  }

  // Store font and shader for text rendering
  font_shader_ = font_shader;
  font_ = font;

  // Initialize text rendering system
  if (font_ && font_shader_) {
    XMMATRIX baseViewMatrix;
    camera_->GetBaseViewMatrix(baseViewMatrix);

    text_ = make_unique<Text>();
    if (!text_->Initialize(screenWidth, screenHeight, baseViewMatrix, font_,
                           font_shader_, resource_manager.GetDevice())) {
      LogGraphicsError(L"Could not initialize text.");
      return false;
    }
  }

  return true;
}

bool Graphics::InitializeOrthoWindows() {
  auto &resource_manager = ResourceManager::GetInstance();

  // Load ortho windows from configuration
  auto &small_window_config = scene_config_.ortho_windows["small_window"];
  ortho_windows_.small_window = resource_manager.GetOrthoWindow(
      small_window_config.name, small_window_config.width,
      small_window_config.height);

  auto &fullscreen_window_config =
      scene_config_.ortho_windows["fullscreen_window"];
  ortho_windows_.fullscreen_window = resource_manager.GetOrthoWindow(
      fullscreen_window_config.name, fullscreen_window_config.width,
      fullscreen_window_config.height);

  if (!ortho_windows_.small_window || !ortho_windows_.fullscreen_window) {
    LogGraphicsError(L"Could not create ortho windows.");
    return false;
  }

  return true;
}

bool Graphics::InitializeResources() {
  if (!InitializeSceneModels()) {
    return false;
  }

  if (!InitializeRenderTargets()) {
    return false;
  }

  if (!InitializeShaders()) {
    return false;
  }

  if (!InitializeFontSystem()) {
    return false;
  }

  if (!InitializeOrthoWindows()) {
    return false;
  }

  // Object rotation is now JSON-driven, but groups are still needed
  cube_group_ = make_shared<StandardRenderGroup>();
  pbr_group_ = make_shared<StandardRenderGroup>();

  return true;
}

bool Graphics::InitializeRenderingPipeline() {
  if (!SetupRenderGraph()) {
    LogGraphicsError(L"Failed to setup RenderGraph.");
    return false;
  }
  return true;
}

void Graphics::Shutdown() {
  auto &resource_manager = ResourceManager::GetInstance();

  // Display resource usage before shutdown
  cout << "\n=== Resource Usage Before Shutdown ===" << endl;
  cout << "Total cached resources: "
       << resource_manager.GetTotalCachedResources() << endl;

  // Check for unused resources
  auto unusedModels = resource_manager.GetUnusedModels();
  auto unusedShaders = resource_manager.GetUnusedShaders();
  auto unusedRTTs = resource_manager.GetUnusedRenderTextures();

  cout << "Unused models: " << unusedModels.size() << endl;
  for (const auto &name : unusedModels) {
    cout << "  - " << name
         << " (ref count: " << resource_manager.GetModelRefCount(name) << ")"
         << endl;
  }

  cout << "Unused shaders: " << unusedShaders.size() << endl;
  for (const auto &name : unusedShaders) {
    cout << "  - " << name
         << " (ref count: " << resource_manager.GetShaderRefCount(name) << ")"
         << endl;
  }

  cout << "Unused render textures: " << unusedRTTs.size() << endl;
  for (const auto &name : unusedRTTs) {
    cout << "  - " << name
         << " (ref count: " << resource_manager.GetRenderTextureRefCount(name)
         << ")" << endl;
  }

  // Optionally prune unused resources
  size_t pruned = resource_manager.PruneAllUnusedResources();

  cout << "========================================\n" << endl;

  // Shutdown ResourceManager
  resource_manager.Shutdown();
}

void Graphics::Frame(float deltaTime) {

  static float lightPositionX = -5.0f;

  camera_->SetPosition(pos_x_, pos_y_, pos_z_);
  camera_->SetRotation(rot_x_, rot_y_, rot_z_);

  // Update light position: moves horizontally at constant speed
  lightPositionX += LIGHT_MOVE_SPEED * deltaTime;
  if (lightPositionX > LIGHT_X_MAX) {
    lightPositionX = LIGHT_X_MIN;
  }
  light_->SetPosition(lightPositionX, LIGHT_Y_POSITION, LIGHT_Z_POSITION);

  // Update animations based on animation config from JSON
  const auto &scene_objects = scene_.GetRenderables();

  for (const auto &renderable : scene_objects) {
    const auto &anim_config = scene_.GetAnimationConfig(renderable);
    if (!anim_config.enabled) {
      continue;
    }

    // Get initial transform from Scene (stored when object was created)
    const XMMATRIX &initial_transform = scene_.GetInitialTransform(renderable);

    // Calculate rotation based on animation config
    // Speed is in degrees per second, convert to radians
    float &rotation_angle = scene_.GetRotationState(renderable);
    rotation_angle +=
        DirectX::XMConvertToRadians(anim_config.speed) * deltaTime;

    // Wrap rotation to [0, 2π)
    const float TWO_PI = 2.0f * DirectX::XM_PI;
    if (rotation_angle >= TWO_PI) {
      rotation_angle -= TWO_PI;
    }

    // Apply initial rotation offset
    float total_rotation =
        rotation_angle + DirectX::XMConvertToRadians(anim_config.initial);

    // Create rotation matrix based on axis
    XMMATRIX rotation_matrix;
    switch (anim_config.axis) {
    case AnimationConfig::RotationAxis::X:
      rotation_matrix = XMMatrixRotationX(total_rotation);
      break;
    case AnimationConfig::RotationAxis::Z:
      rotation_matrix = XMMatrixRotationZ(total_rotation);
      break;
    case AnimationConfig::RotationAxis::Y:
    default:
      rotation_matrix = XMMatrixRotationY(total_rotation);
      break;
    }

    // Decompose initial transform to get translation, rotation, and scale
    XMVECTOR scale_vec, rotation_quat, translation_vec;
    if (XMMatrixDecompose(&scale_vec, &rotation_quat, &translation_vec,
                          initial_transform)) {
      // Rebuild matrix with new rotation: Scale * Rotation * Translation
      XMMATRIX scale_matrix = XMMatrixScalingFromVector(scale_vec);
      XMMATRIX translation_matrix =
          XMMatrixTranslationFromVector(translation_vec);

      // Apply rotation: Scale * NewRotation * Translation
      XMMATRIX new_world_matrix =
          scale_matrix * rotation_matrix * translation_matrix;
      renderable->SetWorldMatrix(new_world_matrix);
    } else {
      // Fallback: if decomposition fails, just apply rotation to initial
      // transform
      renderable->SetWorldMatrix(rotation_matrix * initial_transform);
    }
  }

  // Clean up rotation states for objects that no longer exist
  scene_.CleanupAnimationStates(scene_objects);

#ifdef _DEBUG
  // Periodically log resource usage for debugging
  static float debug_timer = 0.0f;
  debug_timer += deltaTime;
  if (debug_timer >= DEBUG_RESOURCE_LOG_INTERVAL) {
    debug_timer = 0.0f;
    auto &resource_manager = ResourceManager::GetInstance();
    cout << "\n[DEBUG] Resource Usage:" << endl;
    cout << "  Cube model ref count: "
         << resource_manager.GetModelRefCount("cube") << endl;
    cout << "  Sphere model ref count: "
         << resource_manager.GetModelRefCount("sphere") << endl;
    cout << "  Ground model ref count: "
         << resource_manager.GetModelRefCount("ground") << endl;
    cout << "  Total cached: " << resource_manager.GetTotalCachedResources()
         << endl;
  }
#endif
}

bool Graphics::SetupRenderGraph() {
  cout << "\n=== Setting up RenderGraph ===" << endl;

  // Validate required resources are initialized
  if (!scene_assets_.cube || !scene_assets_.sphere || !scene_assets_.ground ||
      !scene_assets_.pbr_sphere || !render_targets_.shadow_depth ||
      !shader_assets_.depth || !shader_assets_.diffuse_lighting ||
      !ortho_windows_.small_window || !ortho_windows_.fullscreen_window) {
    LogGraphicsError("SetupRenderGraph: resources not initialized.");
    return false;
  }

  auto device = DirectX11Device::GetD3d11DeviceInstance()->GetDevice();
  auto context = DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  // Initialize RenderGraph
  render_graph_.Initialize(device, context);

  // Setup parameter validation system
  RegisterShaderParameters();
  render_graph_.SetParameterValidator(&parameter_validator_);
  render_graph_.EnableParameterValidation(true);

  // Setup render passes
  SetupRenderPasses();

  // Register all resources to ResourceRegistry
  auto &registry = ResourceRegistry::GetInstance();

  // Register models
  registry.Register("cube", scene_assets_.cube);
  registry.Register("sphere", scene_assets_.sphere);
  registry.Register("ground", scene_assets_.ground);
  registry.Register("pbr_sphere", scene_assets_.pbr_sphere);
  registry.Register("refraction_ground", scene_assets_.refraction.ground);
  registry.Register("refraction_wall", scene_assets_.refraction.wall);
  registry.Register("refraction_water", scene_assets_.refraction.water);

  // Register shaders (cast to IShader interface)
  registry.Register<IShader>("depth", shader_assets_.depth);
  registry.Register<IShader>("shadow", shader_assets_.shadow);
  registry.Register<IShader>("texture", shader_assets_.texture);
  registry.Register<IShader>("horizontal_blur", shader_assets_.horizontal_blur);
  registry.Register<IShader>("vertical_blur", shader_assets_.vertical_blur);
  registry.Register<IShader>("soft_shadow", shader_assets_.soft_shadow);
  registry.Register<IShader>("pbr", shader_assets_.pbr);
  registry.Register<IShader>("diffuse_lighting", shader_assets_.diffuse_lighting);

  // Register render textures
  registry.Register("shadow_map", render_targets_.shadow_map);
  registry.Register("downsampled_shadow", render_targets_.downsampled_shadow);
  registry.Register("horizontal_blur", render_targets_.horizontal_blur);
  registry.Register("vertical_blur", render_targets_.vertical_blur);

  // Register ortho windows
  registry.Register("small_window", ortho_windows_.small_window);
  registry.Register("fullscreen_window", ortho_windows_.fullscreen_window);

  // Load scene from JSON configuration
  if (!scene_.Initialize("./data/scene.json", cube_group_.get(), pbr_group_.get())) {
    LogGraphicsError("Failed to initialize Scene!");
    return false;
  }

  // Compile render graph to validate dependencies and build execution order
  if (!render_graph_.Compile()) {
    LogGraphicsError("Failed to compile RenderGraph!");
    return false;
  }

  cout << "=== RenderGraph Setup Complete ===\n" << endl;

  render_graph_.PrintGraph();
  return true;
}

void Graphics::SetupRenderPasses() {
  // Import existing textures into RenderGraph for pass dependencies
  const auto &depth_tex = render_targets_.shadow_depth;
  render_graph_.ImportTexture("DepthMap", depth_tex);

  const auto &shadow_tex = render_targets_.shadow_map;
  render_graph_.ImportTexture("ShadowMap", shadow_tex);

  const auto &downsample_tex = render_targets_.downsampled_shadow;
  render_graph_.ImportTexture("DownsampledShadow", downsample_tex);

  const auto &h_blur_tex = render_targets_.horizontal_blur;
  render_graph_.ImportTexture("HorizontalBlur", h_blur_tex);

  const auto &v_blur_tex = render_targets_.vertical_blur;
  render_graph_.ImportTexture("VerticalBlur", v_blur_tex);

  const auto &upsample_tex = render_targets_.upsampled_shadow;
  render_graph_.ImportTexture("UpsampledShadow", upsample_tex);

  const auto &reflection_tex = render_targets_.reflection_map;
  render_graph_.ImportTexture("ReflectionMap", reflection_tex);

  // Pass 1: Depth Pass - Render depth map from light's perspective
  const auto &depth_shader = shader_assets_.depth;
  render_graph_.AddPass("DepthPass")
      .SetShader(depth_shader)
      .Write("DepthMap")
      .AddRenderTag(WRITE_DEPTH_TAG);

  // Pass 2: Shadow Pass - Generate shadow map using depth map
  const auto &shadow_shader = shader_assets_.shadow;
  render_graph_.AddPass("ShadowPass")
      .SetShader(shadow_shader)
      .ReadAsParameter("DepthMap")
      .Write("ShadowMap")
      .AddRenderTag(WRITE_SHADOW_TAG);

  // Pass 3: Downsample - Reduce shadow map resolution for blur processing
  XMMATRIX orthoMatrix;
  downsample_tex->GetOrthoMatrix(orthoMatrix);

  const auto &texture_shader = shader_assets_.texture;
  render_graph_.AddPass("DownsamplePass")
      .SetShader(texture_shader)
      .ReadAsParameter("ShadowMap")
      .Write("DownsampledShadow")
      .AddRenderTag(DOWN_SAMPLE_TAG)
      .DisableZBuffer(true)
      .SetParameter("orthoMatrix", orthoMatrix);

  // Pass 4: Horizontal Blur - Apply horizontal Gaussian blur
  h_blur_tex->GetOrthoMatrix(orthoMatrix);

  const auto &horizontal_blur_shader = shader_assets_.horizontal_blur;
  render_graph_.AddPass("HorizontalBlurPass")
      .SetShader(horizontal_blur_shader)
      .ReadAsParameter("DownsampledShadow")
      .Write("HorizontalBlur")
      .AddRenderTag(HORIZONTAL_BLUR_TAG)
      .DisableZBuffer(true)
      .SetParameter("orthoMatrix", orthoMatrix)
      .SetParameter("screenWidth", static_cast<float>(DOWN_SAMPLE_WIDTH));

  // Pass 5: Vertical Blur - Apply vertical Gaussian blur (completes 2D blur)
  v_blur_tex->GetOrthoMatrix(orthoMatrix);

  const auto &vertical_blur_shader = shader_assets_.vertical_blur;
  render_graph_.AddPass("VerticalBlurPass")
      .SetShader(vertical_blur_shader)
      .ReadAsParameter("HorizontalBlur")
      .Write("VerticalBlur")
      .AddRenderTag(VERTICAL_BLUR_TAG)
      .DisableZBuffer(true)
      .SetParameter("orthoMatrix", orthoMatrix)
      .SetParameter("screenHeight", static_cast<float>(DOWN_SAMPLE_HEIGHT));

  // Pass 6: Upsample - Scale blurred shadow map back to original resolution
  upsample_tex->GetOrthoMatrix(orthoMatrix);

  render_graph_.AddPass("UpsamplePass")
      .SetShader(texture_shader)
      .ReadAsParameter("VerticalBlur")
      .Write("UpsampledShadow")
      .AddRenderTag(UP_SAMPLE_TAG)
      .DisableZBuffer(true)
      .SetParameter("orthoMatrix", orthoMatrix);

  // Pass 7: Reflection Scene Pass - Render scene from reflection camera view
  const auto &soft_shadow_shader = shader_assets_.soft_shadow;
  render_graph_.AddPass("ReflectionScenePass")
      .SetShader(soft_shadow_shader)
      .ReadAsParameter("UpsampledShadow", "shadowTexture")
      .Write("ReflectionMap")
      .AddRenderTag(REFLECTION_TAG)
      .SetParameter("ambientColor", light_->GetAmbientColor())
      .SetParameter("diffuseColor", light_->GetDiffuseColor())
      .SetParameter("reflectionBlend", 0.0f)
      .SetParameter("shadowStrength", 0.0f)
      .Execute([this](RenderPassContext &ctx) {
        if (!ctx.shader)
          return;

        ShaderParameterContainer merged = ShaderParameterContainer::ChainMerge(
            *ctx.global_params, *ctx.pass_params);

        // Override view matrix with reflection matrix if available
        if (ctx.global_params->HasParameter("reflectionMatrix")) {
          merged.SetMatrix("viewMatrix",
                           ctx.global_params->GetMatrix("reflectionMatrix"));
        }

        // Ensure projection matrix is set
        if (ctx.global_params->HasParameter("projectionMatrix")) {
          merged.SetMatrix("projectionMatrix",
                           ctx.global_params->GetMatrix("projectionMatrix"));
        }

        // Render only objects tagged for reflection
        for (const auto &renderable : *ctx.renderables) {
          if (!renderable->HasTag(REFLECTION_TAG))
            continue;

          ShaderParameterContainer objParams = merged;
          objParams.SetMatrix("worldMatrix", renderable->GetWorldMatrix());
          if (auto cb = renderable->GetParameterCallback()) {
            cb(objParams);
          }
          // Disable shadowing and recursive reflection for reflection pass
          objParams.SetFloat("shadowStrength", 0.0f);
          objParams.SetFloat("reflectionBlend", 0.0f);
          renderable->Render(*ctx.shader, objParams, ctx.device_context);
        }
      });

  // Pass 8: Final Pass - Render main scene with soft shadows and reflections
  render_graph_.AddPass("FinalPass")
      .SetShader(soft_shadow_shader)
      .ReadAsParameter("UpsampledShadow", "shadowTexture")
      .ReadAsParameter("ReflectionMap", "reflectionTexture")
      .AddRenderTag(FINAL_TAG)
      .SetParameter("diffuseColor", light_->GetDiffuseColor())
      .SetParameter("ambientColor", light_->GetAmbientColor())
      .SetParameter("reflectionBlend", 0.0f)
      .SetParameter("shadowStrength", 1.0f);

  // Pass 9: PBR Pass - Render physically-based rendered objects
  const auto &sphere_pbr_model = scene_assets_.pbr_sphere;
  const auto &pbr_shader = shader_assets_.pbr;
  render_graph_.AddPass("PBRPass")
      .SetShader(pbr_shader)
      .AddRenderTag(PBR_TAG)
      .SetTexture("diffuseTexture", sphere_pbr_model->GetTexture(0))
      .SetTexture("normalMap", sphere_pbr_model->GetTexture(1))
      .SetTexture("rmTexture", sphere_pbr_model->GetTexture(2));

  // Pass 10: Diffuse Lighting Pass - Render objects with simple diffuse
  // lighting
  const auto &diffuse_lighting_shader = shader_assets_.diffuse_lighting;
  render_graph_.AddPass("DiffuseLightingPass")
      .SetShader(diffuse_lighting_shader)
      .AddRenderTag(DIFFUSE_LIGHTING_TAG)
      .SetParameter("ambientColor", light_->GetAmbientColor())
      .SetParameter("diffuseColor", light_->GetDiffuseColor())
      .SetParameter("lightDirection", light_->GetDirection());

  // Pass 11: Text Overlay - Render debug text and UI elements
  render_graph_.AddPass("TextOverlayPass")
      .DisableZBuffer(true)
      .Execute([this](RenderPassContext &ctx) {
        if (!text_) {
          return;
        }

        auto dx = DirectX11Device::GetD3d11DeviceInstance();
        dx->SetBackBufferRenderTarget();
        dx->ResetViewport();
        dx->TurnOnAlphaBlending();

        XMMATRIX worldMatrix, orthoMatrix;
        dx->GetWorldMatrix(worldMatrix);
        dx->GetOrthoMatrix(orthoMatrix);

        text_->Render(worldMatrix, orthoMatrix, ctx.device_context);

        dx->TurnOffAlphaBlending();
      });
}

void Graphics::RegisterShaderParameters() {
  // Set validation mode to Warning (report issues but don't block execution)
  parameter_validator_.SetValidationMode(ValidationMode::Warning);
  ShaderParameterContainer::SetOverrideLoggingEnabled(true);

  // Register global parameters (provided at runtime by Render() or per-object)
  // These parameters are automatically available to all shaders and don't need
  // to be set at Pass level
  // Set per-object
  parameter_validator_.RegisterGlobalParameter("worldMatrix");
  // From Render()
  parameter_validator_.RegisterGlobalParameter("viewMatrix");
  parameter_validator_.RegisterGlobalParameter("projectionMatrix"); 
  parameter_validator_.RegisterGlobalParameter("baseViewMatrix");
  parameter_validator_.RegisterGlobalParameter("deviceWorldMatrix");
  parameter_validator_.RegisterGlobalParameter("lightViewMatrix");
  parameter_validator_.RegisterGlobalParameter("lightProjectionMatrix");
  parameter_validator_.RegisterGlobalParameter("lightPosition");
  parameter_validator_.RegisterGlobalParameter("lightDirection");
  parameter_validator_.RegisterGlobalParameter("cameraPosition");
  parameter_validator_.RegisterGlobalParameter("reflectionMatrix");
  parameter_validator_.RegisterGlobalParameter("ambientColor");
  parameter_validator_.RegisterGlobalParameter("diffuseColor");

  const auto register_with_reflection =
      [this](const std::string &shader_name,
             const std::shared_ptr<ShaderBase> &shader,
             std::initializer_list<const char *> optional_parameters = {},
             std::initializer_list<const char *> ignored_parameters = {},
             std::initializer_list<std::pair<const char *, const char *>>
                 alias_parameters = {}) {
        if (!shader) {
          return false;
        }

        const auto &reflected = shader->GetReflectedParameters();
        if (reflected.empty()) {
          return false;
        }

        std::unordered_set<std::string> optional;
        for (const auto &name : optional_parameters) {
          optional.emplace(name);
        }

        std::unordered_set<std::string> ignored;
        for (const auto &name : ignored_parameters) {
          ignored.emplace(name);
        }

        std::unordered_map<std::string, std::string> aliases;
        for (const auto &pair : alias_parameters) {
          aliases.emplace(pair.first, pair.second);
        }

        std::vector<ReflectedParameter> adjusted;
        adjusted.reserve(reflected.size());
        std::unordered_set<std::string> seen_names;
        for (const auto &parameter : reflected) {
          if (ignored.find(parameter.name) != ignored.end()) {
            continue;
          }

          ReflectedParameter entry = parameter;
          const auto alias_it = aliases.find(entry.name);
          if (alias_it != aliases.end()) {
            entry.name = alias_it->second;
          }

          if (optional.find(entry.name) != optional.end() ||
              optional.find(parameter.name) != optional.end()) {
            entry.required = false;
          }

          if (!seen_names.insert(entry.name).second) {
            continue;
          }
          adjusted.push_back(entry);
        }

        if (adjusted.empty()) {
          return false;
        }

        parameter_validator_.RegisterShader(shader_name, adjusted);
        return true;
      };

  // Register DepthShader parameters
  if (!register_with_reflection(
          "DepthShader",
          std::static_pointer_cast<ShaderBase>(shader_assets_.depth))) {
    parameter_validator_.RegisterShader(
        "DepthShader",
        {{"worldMatrix", ShaderParameterType::Matrix, true},
         {"lightViewMatrix", ShaderParameterType::Matrix, true},
         {"lightProjectionMatrix", ShaderParameterType::Matrix, true}});
  }

  // Register ShadowShader parameters
  if (!register_with_reflection(
          "ShadowShader",
          std::static_pointer_cast<ShaderBase>(shader_assets_.shadow))) {
    parameter_validator_.RegisterShader(
        "ShadowShader",
        {{"worldMatrix", ShaderParameterType::Matrix, true},
         {"viewMatrix", ShaderParameterType::Matrix, true},
         {"projectionMatrix", ShaderParameterType::Matrix, true},
         {"lightViewMatrix", ShaderParameterType::Matrix, true},
         {"lightProjectionMatrix", ShaderParameterType::Matrix, true},
         {"lightPosition", ShaderParameterType::Vector3, true},
         {"depthMapTexture", ShaderParameterType::Texture, true}});
  }

  // Register SoftShadowShader parameters
  // Note: "texture" is set via object callbacks, not at Pass level
  if (!register_with_reflection(
          "SoftShadowShader",
          std::static_pointer_cast<ShaderBase>(shader_assets_.soft_shadow),
          {"texture", "reflectionTexture", "reflectionBlend", "shadowStrength"},
          {}, {{"shaderTexture", "texture"}})) {
    parameter_validator_.RegisterShader(
        "SoftShadowShader",
        {{"worldMatrix", ShaderParameterType::Matrix, true},
         {"viewMatrix", ShaderParameterType::Matrix, true},
         {"projectionMatrix", ShaderParameterType::Matrix, true},
         {"texture", ShaderParameterType::Texture, false},
         {"shadowTexture", ShaderParameterType::Texture, true},
         {"ambientColor", ShaderParameterType::Vector4, true},
         {"diffuseColor", ShaderParameterType::Vector4, true},
         {"lightPosition", ShaderParameterType::Vector3, true},
         {"reflectionMatrix", ShaderParameterType::Matrix, true},
         {"reflectionTexture", ShaderParameterType::Texture, false},
         {"reflectionBlend", ShaderParameterType::Float, false},
         {"shadowStrength", ShaderParameterType::Float, false}});
  }

  // Register PbrShader parameters
  if (!register_with_reflection(
          "PbrShader",
          std::static_pointer_cast<ShaderBase>(shader_assets_.pbr))) {
    parameter_validator_.RegisterShader(
        "PbrShader", {{"worldMatrix", ShaderParameterType::Matrix, true},
                      {"viewMatrix", ShaderParameterType::Matrix, true},
                      {"projectionMatrix", ShaderParameterType::Matrix, true},
                      {"diffuseTexture", ShaderParameterType::Texture, true},
                      {"normalMap", ShaderParameterType::Texture, true},
                      {"rmTexture", ShaderParameterType::Texture, true},
                      {"lightDirection", ShaderParameterType::Vector3, true},
                      {"cameraPosition", ShaderParameterType::Vector3, true}});
  }

  // Register TextureShader parameters
  if (!register_with_reflection(
          "TextureShader",
          std::static_pointer_cast<ShaderBase>(shader_assets_.texture), {}, {},
          {{"projectionMatrix", "orthoMatrix"}})) {
    parameter_validator_.RegisterShader(
        "TextureShader",
        {{"deviceWorldMatrix", ShaderParameterType::Matrix, true},
         {"baseViewMatrix", ShaderParameterType::Matrix, true},
         {"orthoMatrix", ShaderParameterType::Matrix, true},
         {"shaderTexture", ShaderParameterType::Texture, true}});
  }

  // Register HorizontalBlurShader parameters
  if (!register_with_reflection(
          "HorizontalBlurShader",
          std::static_pointer_cast<ShaderBase>(shader_assets_.horizontal_blur),
          {}, {},
          {{"projectionMatrix", "orthoMatrix"}})) {
    parameter_validator_.RegisterShader(
        "HorizontalBlurShader",
        {{"worldMatrix", ShaderParameterType::Matrix, true},
         {"baseViewMatrix", ShaderParameterType::Matrix, true},
         {"orthoMatrix", ShaderParameterType::Matrix, true},
         {"screenWidth", ShaderParameterType::Float, true},
         {"shaderTexture", ShaderParameterType::Texture, true}});
  }

  // Register VerticalBlurShader parameters
  if (!register_with_reflection(
          "VerticalBlurShader",
          std::static_pointer_cast<ShaderBase>(shader_assets_.vertical_blur),
          {}, {},
          {{"projectionMatrix", "orthoMatrix"}})) {
    parameter_validator_.RegisterShader(
        "VerticalBlurShader",
        {{"worldMatrix", ShaderParameterType::Matrix, true},
         {"baseViewMatrix", ShaderParameterType::Matrix, true},
         {"orthoMatrix", ShaderParameterType::Matrix, true},
         {"screenHeight", ShaderParameterType::Float, true},
         {"shaderTexture", ShaderParameterType::Texture, true}});
  }

  // Register SimpleLightShader parameters (diffuse lighting shader demo)
  if (!register_with_reflection(
          "SimpleLightShader",
          std::static_pointer_cast<ShaderBase>(shader_assets_.diffuse_lighting),
          {"texture"}, {}, {{"shaderTexture", "texture"}})) {
    parameter_validator_.RegisterShader(
        "SimpleLightShader",
        {{"worldMatrix", ShaderParameterType::Matrix, true},
         {"viewMatrix", ShaderParameterType::Matrix, true},
         {"projectionMatrix", ShaderParameterType::Matrix, true},
         {"texture", ShaderParameterType::Texture, false},
         {"ambientColor", ShaderParameterType::Vector4, true},
         {"diffuseColor", ShaderParameterType::Vector4, true},
         {"lightDirection", ShaderParameterType::Vector3, true}});
  }

  cout << "[Graphics] Registered shader parameters for validation" << endl;
}

// Helper function to check if a renderable object is visible in the frustum
bool Graphics::IsObjectVisible(std::shared_ptr<IRenderable> renderable,
                               const FrustumClass &frustum) const {
  if (!renderable) {
    return true; // If object is null, skip it
  }

  // Check if object has "skip_culling" tag (for UI elements, post-processing,
  // etc.)
  if (renderable->HasTag("skip_culling")) {
    return true;
  }

  // Prefer Model's bounding volume data (if available)
  // First try to cast directly to Model
  auto model = std::dynamic_pointer_cast<Model>(renderable);
  if (model) {
    // Get world-space bounding volume
    BoundingVolume worldBounds = model->GetWorldBoundingVolume();

    // Use optimized bounding volume test (AABB + bounding sphere)
    return frustum.CheckBoundingVolume(worldBounds);
  }

  // If wrapped by RenderableObject, use RenderableObject's bounding volume
  auto renderable_obj = std::dynamic_pointer_cast<RenderableObject>(renderable);
  if (renderable_obj) {
    BoundingVolume worldBounds = renderable_obj->GetWorldBoundingVolume();

    // Check if bounding volume is valid (non-empty)
    if (worldBounds.sphere_radius > 0.0f) {
      // Use optimized bounding volume test
      return frustum.CheckBoundingVolume(worldBounds);
    }
    // If bounding volume is invalid, continue with fallback method
  }

  // Fallback to default method: use world matrix position and default radius
  XMMATRIX worldMatrix = renderable->GetWorldMatrix();
  XMFLOAT3 position;
  XMStoreFloat3(&position, worldMatrix.r[3]);

  // Default radius (for objects without bounding volume like OrthoWindow)
  float boundingRadius = 2.0f;

  // Use smaller radius for small objects
  if (renderable->HasTag("final")) {
    XMVECTOR scale;
    XMVECTOR rotation;
    XMVECTOR translation;
    XMMatrixDecompose(&scale, &rotation, &translation, worldMatrix);
    float scaleX = XMVectorGetX(scale);
    if (scaleX < 0.5f) {
      boundingRadius = 0.5f;
    }
  }

  return frustum.CheckSphere(position.x, position.y, position.z,
                             boundingRadius);
}

void Graphics::Render() {

  auto directx_device_ = DirectX11Device::GetD3d11DeviceInstance();

  // Update camera matrices
  camera_->Render();
  camera_->RenderReflection(scene_config_.constants.reflection_plane_height);
  XMMATRIX viewMatrix, baseViewMatrix;
  camera_->GetViewMatrix(viewMatrix);
  camera_->GetBaseViewMatrix(baseViewMatrix);
  auto reflectionMatrix = camera_->GetReflectionViewMatrix();

  // Update light matrices
  light_->GenerateViewMatrix();
  XMMATRIX lightViewMatrix, lightProjectionMatrix;
  light_->GetViewMatrix(lightViewMatrix);
  light_->GetProjectionMatrix(lightProjectionMatrix);
  light_->SetDirection(0.5f, 0.5f, 0.5f);

  // Build global shader parameters
  ShaderParameterContainer globalParams;
  globalParams.SetGlobalDynamicMatrix("viewMatrix", viewMatrix);
  globalParams.SetGlobalDynamicMatrix("baseViewMatrix", baseViewMatrix);
  globalParams.SetGlobalDynamicMatrix("lightViewMatrix", lightViewMatrix);
  globalParams.SetGlobalDynamicMatrix("lightProjectionMatrix",
                                      lightProjectionMatrix);
  globalParams.SetGlobalDynamicVector3("lightPosition", light_->GetPosition());
  globalParams.SetGlobalDynamicVector3("lightDirection",
                                       light_->GetDirection());
  globalParams.SetGlobalDynamicVector3("cameraPosition",
                                       camera_->GetPosition());
  globalParams.SetMatrix("reflectionMatrix", reflectionMatrix);
  globalParams.SetVector4("ambientColor", light_->GetAmbientColor());
  globalParams.SetVector4("diffuseColor", light_->GetDiffuseColor());

  // Add device matrices
  XMMATRIX deviceWorldMatrix, projectionMatrix;
  directx_device_->GetWorldMatrix(deviceWorldMatrix);
  directx_device_->GetProjectionMatrix(projectionMatrix);
  globalParams.SetMatrix("deviceWorldMatrix", deviceWorldMatrix);
  globalParams.SetMatrix("projectionMatrix", projectionMatrix);

  // Construct frustum for culling
  if (frustum_) {
    frustum_->ConstructFrustum(SCREEN_DEPTH, projectionMatrix, viewMatrix);
  }

  // Perform frustum culling: filter renderable objects from scene
  std::vector<std::shared_ptr<IRenderable>> culled_objects;
  const auto &scene_objects = scene_.GetRenderables();
  if (frustum_) {
    for (const auto &renderable : scene_objects) {
      if (IsObjectVisible(renderable, *frustum_)) {
        culled_objects.push_back(renderable);
      }
    }
  } else {
    // If frustum not initialized, render all objects
    culled_objects = scene_objects;
  }

  // Update render count for debug display
  if (text_) {
    text_->SetRenderCount(static_cast<int>(culled_objects.size()));
  }

  // Execute render graph with culled objects
  render_graph_.Execute(culled_objects, globalParams);
}