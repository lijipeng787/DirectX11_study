#include "Graphics.h"

#include "../CommonFramework2/DirectX11Device.h"
#include "../CommonFramework2/TypeDefine.h"

#include "BoundingVolume.h"
#include "Frustum.h"
#include "Interfaces.h"
#include "Logger.h"
#include "RenderableObject.h"
#include "ResourceManager.h"
#include "ShaderParameterValidator.h"
#include "depthshader.h"
#include "font.h"
#include "fontshader.h"
#include "horizontalblurshader.h"
#include "model.h"
#include "orthowindow.h"
#include "pbrshader.h"
#include "refractionshader.h"
#include "rendertexture.h"
#include "scenelightshader.h"
#include "shadowshader.h"
#include "simplelightshader.h"
#include "softshadowshader.h"
#include "text.h"
#include "textureshader.h"
#include "verticalblurshader.h"
#include "watershader.h"

#include <iostream>

using namespace std;
using namespace DirectX;

using namespace SceneConfig;

static constexpr auto SHADOW_MAP_WIDTH = 1024;
static constexpr auto SHADOW_MAP_HEIGHT = 1024;
// Set the size to sample down to.
static constexpr auto downSampleWidth = SHADOW_MAP_WIDTH / 2;
static constexpr auto downSampleHeight = SHADOW_MAP_HEIGHT / 2;

bool Graphics::Initialize(int screenWidth, int screenHeight, HWND hwnd) {

  if (!InitializeDevice(screenWidth, screenHeight, hwnd)) {
    return false;
  }

  if (!InitializeCamera()) {
    return false;
  }

  if (!InitializeLight()) {
    return false;
  }

  // Load scene configuration from JSON or use default
  SceneConfig::LoadFromJson(scene_config_, "./data/scene_config.json");

  if (!InitializeResources(hwnd)) {
    return false;
  }

  if (!InitializeRenderingPipeline()) {
    return false;
  }

  // Log resource statistics
  auto &resource_manager = ResourceManager::GetInstance();
  resource_manager.LogResourceStats();

  return true;
}

bool Graphics::InitializeDevice(int screenWidth, int screenHeight, HWND hwnd) {
  auto *directx11_device_ = DirectX11Device::GetD3d11DeviceInstance();

  if (!(directx11_device_->Initialize(screenWidth, screenHeight, VSYNC_ENABLED,
                                      hwnd, FULL_SCREEN, SCREEN_DEPTH,
                                      SCREEN_NEAR))) {
    Logger::SetModule("Graphics");
    Logger::LogError(L"Could not initialize Direct3D.");
    return false;
  }

  this->screenWidth = screenWidth;
  this->screenHeight = screenHeight;

  // Initialize ResourceManager
  auto &resource_manager = ResourceManager::GetInstance();
  auto *device = directx11_device_->GetDevice();
  auto *device_context = directx11_device_->GetDeviceContext();

  if (!resource_manager.Initialize(device, device_context, hwnd)) {
    Logger::LogError("Could not initialize ResourceManager.");
    return false;
  }

  return true;
}

bool Graphics::InitializeCamera() {
  camera_ = make_unique<Camera>();
  if (nullptr == camera_) {
    return false;
  }
  camera_->SetPosition(0.0f, -1.0f, -10.0f);
  camera_->Render();
  camera_->RenderBaseViewMatrix();

  // Initialize frustum for culling
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

  light_->SetAmbientColor(0.15f, 0.15f, 0.15f, 1.0f);
  light_->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
  light_->SetLookAt(0.0f, 0.0f, 0.0f);
  light_->GenerateProjectionMatrix(SCREEN_DEPTH, SCREEN_NEAR);
  return true;
}

bool Graphics::InitializeSceneModels(HWND hwnd) {
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
    std::wstring error_msg = L"Could not load models.";
    const auto &last_error = resource_manager.GetLastError();
    if (!last_error.empty()) {
      error_msg += L"\n" + std::wstring(last_error.begin(), last_error.end());
    }
    Logger::SetModule("Graphics");
    Logger::LogError(error_msg);
    return false;
  }

  // Load PBR model from configuration
  auto &pbr_config = scene_config_.pbr_models["sphere_pbr"];
  scene_assets_.pbr_sphere = resource_manager.GetPBRModel(
      pbr_config.name, pbr_config.model_path, pbr_config.albedo_path,
      pbr_config.normal_path, pbr_config.roughmetal_path);

  if (!scene_assets_.pbr_sphere) {
    std::wstring error_msg = L"Could not load PBR model.";
    const auto &last_error = resource_manager.GetLastError();
    if (!last_error.empty()) {
      error_msg += L"\n" + std::wstring(last_error.begin(), last_error.end());
    }
    Logger::SetModule("Graphics");
    Logger::LogError(error_msg);
    return false;
  }

  // Load refraction scene models from configuration
  auto &ref_ground_config = scene_config_.refraction.ground;
  scene_assets_.refraction.ground = resource_manager.GetModel(
      ref_ground_config.name, ref_ground_config.model_path,
      ref_ground_config.texture_path);

  auto &ref_wall_config = scene_config_.refraction.wall;
  scene_assets_.refraction.wall = resource_manager.GetModel(
      ref_wall_config.name, ref_wall_config.model_path,
      ref_wall_config.texture_path);

  auto &ref_bath_config = scene_config_.refraction.bath;
  scene_assets_.refraction.bath = resource_manager.GetModel(
      ref_bath_config.name, ref_bath_config.model_path,
      ref_bath_config.texture_path);

  auto &ref_water_config = scene_config_.refraction.water;
  scene_assets_.refraction.water = resource_manager.GetModel(
      ref_water_config.name, ref_water_config.model_path,
      ref_water_config.texture_path);

  if (!scene_assets_.refraction.ground || !scene_assets_.refraction.wall ||
      !scene_assets_.refraction.bath || !scene_assets_.refraction.water) {
    std::wstring error_msg = L"Could not load refraction scene models.";
    const auto &last_error = resource_manager.GetLastError();
    if (!last_error.empty()) {
      error_msg += L"\n" + std::wstring(last_error.begin(), last_error.end());
    }
    Logger::SetModule("Graphics");
    Logger::LogError(error_msg);
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

  auto &water_refraction_config =
      scene_config_.render_targets["water_refraction"];
  render_targets_.refraction.refraction_map =
      resource_manager.CreateRenderTexture(
          water_refraction_config.name, getWidth(water_refraction_config),
          getHeight(water_refraction_config), water_refraction_config.depth,
          water_refraction_config.near_plane);

  auto &water_reflection_config =
      scene_config_.render_targets["water_reflection"];
  render_targets_.refraction.water_reflection_map =
      resource_manager.CreateRenderTexture(
          water_reflection_config.name, getWidth(water_reflection_config),
          getHeight(water_reflection_config), water_reflection_config.depth,
          water_reflection_config.near_plane);

  if (!render_targets_.shadow_depth || !render_targets_.shadow_map ||
      !render_targets_.downsampled_shadow || !render_targets_.horizontal_blur ||
      !render_targets_.vertical_blur || !render_targets_.upsampled_shadow ||
      !render_targets_.reflection_map ||
      !render_targets_.refraction.refraction_map ||
      !render_targets_.refraction.water_reflection_map) {
    Logger::SetModule("Graphics");
    Logger::LogError(L"Could not create render textures.");
    return false;
  }

  return true;
}

bool Graphics::InitializeShaders() {
  auto &resource_manager = ResourceManager::GetInstance();

  // Load core rendering shaders
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

  // Load refraction/water shaders
  shader_assets_.refraction.scene_light =
      resource_manager.GetShader<SceneLightShader>("scene_light");
  shader_assets_.refraction.refraction =
      resource_manager.GetShader<RefractionShader>("refraction");
  shader_assets_.refraction.water =
      resource_manager.GetShader<WaterShader>("water");

  if (!shader_assets_.depth || !shader_assets_.shadow ||
      !shader_assets_.texture || !shader_assets_.horizontal_blur ||
      !shader_assets_.vertical_blur || !shader_assets_.soft_shadow ||
      !shader_assets_.pbr || !shader_assets_.diffuse_lighting ||
      !shader_assets_.refraction.scene_light ||
      !shader_assets_.refraction.refraction ||
      !shader_assets_.refraction.water) {
    Logger::SetModule("Graphics");
    Logger::LogError(L"Could not load shaders.");
    return false;
  }

  return true;
}

bool Graphics::InitializeFontSystem(HWND hwnd) {
  auto &resource_manager = ResourceManager::GetInstance();

  // Load font shader
  auto font_shader = resource_manager.GetShader<FontShader>("font");
  if (!font_shader) {
    Logger::SetModule("Graphics");
    Logger::LogError(L"Could not load font shader.");
    return false;
  }

  // Load font
  auto font = std::make_shared<Font>();
  if (!font->Initialize("./data/fontdata.txt", L"./data/font.dds",
                        resource_manager.GetDevice())) {
    Logger::SetModule("Graphics");
    Logger::LogError(L"Could not initialize font.");
    return false;
  }

  // Store font and shader
  font_shader_ = font_shader;
  font_ = font;

  // Initialize text rendering
  if (font_ && font_shader_) {
    XMMATRIX baseViewMatrix;
    camera_->GetBaseViewMatrix(baseViewMatrix);

    text_ = make_unique<Text>();
    if (!text_->Initialize(screenWidth, screenHeight, baseViewMatrix, font_,
                           font_shader_, resource_manager.GetDevice())) {
      Logger::SetModule("Graphics");
      Logger::LogError(L"Could not initialize text.");
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
    Logger::SetModule("Graphics");
    Logger::LogError(L"Could not create ortho windows.");
    return false;
  }

  return true;
}

bool Graphics::InitializeResources(HWND hwnd) {
  // Delegate to specialized initialization methods for better organization
  // and testability

  if (!InitializeSceneModels(hwnd)) {
    return false;
  }

  if (!InitializeRenderTargets()) {
    return false;
  }

  if (!InitializeShaders()) {
    return false;
  }

  if (!InitializeFontSystem(hwnd)) {
    return false;
  }

  if (!InitializeOrthoWindows()) {
    return false;
  }

  // Pre-create render groups
  cube_group_ = make_shared<StandardRenderGroup>();
  pbr_group_ = make_shared<StandardRenderGroup>();

  return true;
}

bool Graphics::InitializeRenderingPipeline() {
  if (!SetupRenderGraph()) {
    Logger::SetModule("Graphics");
    Logger::LogError(L"Failed to setup RenderGraph.");
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
  // size_t pruned = resourceManager.PruneAllUnusedResources();

  cout << "========================================\n" << endl;

  // Shutdown ResourceManager
  resource_manager.Shutdown();
}

static constexpr float water_translation_speed = 0.001f;

void Graphics::Frame(float deltaTime) {

  static float lightPositionX = -5.0f;

  camera_->SetPosition(pos_x_, pos_y_, pos_z_);
  camera_->SetRotation(rot_x_, rot_y_, rot_z_);

  water_translation_ += water_translation_speed;
  if (water_translation_ > 1.0f) {
    water_translation_ -= 1.0f;
  }

  // Update the position of the light each frame.
  // deltaTime is now in seconds (converted from milliseconds in System.cpp)
  // Light moves at 1 unit per second
  lightPositionX += 1.0f * deltaTime;
  if (lightPositionX > 5.0f) {
    lightPositionX = -5.0f;
  }
  light_->SetPosition(lightPositionX, 8.0f, -5.0f);

  // Update rotation for 5 cubes group
  // Rotate at approximately PI radians per second (180 degrees per second)
  // Previously: XM_PI * 0.001f * deltaTime_ms = XM_PI * 0.001f * (deltaTime_s *
  // 1000) = XM_PI * deltaTime_s Now deltaTime is in seconds, so we use: XM_PI *
  // deltaTime
  static float rotation_y = 0.0f;
  if (0.000001 < 360.0f - rotation_y)
    rotation_y = 0.0f;
  rotation_y += DirectX::XM_PI * deltaTime;

  XMMATRIX rotationMatrix =
      XMMatrixRotationRollPitchYaw(0.0f, rotation_y, 0.0f);

  XMMATRIX translationMatrix;
  for (const auto &renderable : cube_group_->GetRenderables()) {
    translationMatrix = renderable->GetWorldMatrix();
    renderable->SetWorldMatrix(rotationMatrix * translationMatrix);
  }

  // Update diffuse lighting demo cube rotation
  // Rotate at 90 degrees per second (π/2 radians per second)
  if (diffuse_lighting_cube_) {
    const float rotationSpeed = XM_PI / 2.0f; // Radians per second
    diffuse_lighting_rotation_ += rotationSpeed * deltaTime;

    const float TWO_PI = 2.0f * XM_PI;
    if (diffuse_lighting_rotation_ > TWO_PI) {
      diffuse_lighting_rotation_ -= TWO_PI;
    }

    XMMATRIX diffuse_lighting_rotation =
        XMMatrixRotationY(diffuse_lighting_rotation_);
    // Position diffuse lighting cube in a separate area to avoid overlapping
    // with soft shadow demo Located to the right and forward:
    // (6.0f, 2.0f, 3.0f)
    XMMATRIX diffuse_lighting_translation =
        XMMatrixTranslation(6.0f, 2.0f, 3.0f);
    diffuse_lighting_cube_->SetWorldMatrix(diffuse_lighting_rotation *
                                           diffuse_lighting_translation);
  }

  for (const auto &renderable : pbr_group_->GetRenderables()) {
    translationMatrix = renderable->GetWorldMatrix();
    renderable->SetWorldMatrix(rotationMatrix * translationMatrix);
  }

#ifdef _DEBUG
  // Debug: periodically log resource usage (every 5 seconds)
  static float debugTimer = 0.0f;
  // deltaTime is now in seconds, no conversion needed
  debugTimer += deltaTime;
  if (debugTimer > 5.0f) {
    debugTimer = 0.0f;
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
static constexpr auto water_surface_tag = "water_surface";
static constexpr auto diffuse_lighting_tag = "diffuse_lighting";
// Note: Scene constants are now loaded from JSON configuration
// These default values are kept for reference only
// static constexpr float water_plane_height = 2.75f;
// static constexpr float water_reflect_refract_scale = 0.01f;
// static constexpr float reflection_plane_height = 1.0f;
// static constexpr float refraction_scene_offset_x = 15.0f;
// static constexpr float refraction_scene_offset_y = 0.0f;
// static constexpr float refraction_scene_offset_z = 0.0f;
// static constexpr float refraction_ground_scale = 0.5f;

bool Graphics::SetupRenderGraph() {
  cout << "\n=== Setting up RenderGraph ===" << endl;

  if (!scene_assets_.cube || !scene_assets_.sphere || !scene_assets_.ground ||
      !scene_assets_.pbr_sphere || !render_targets_.shadow_depth ||
      !shader_assets_.depth || !shader_assets_.diffuse_lighting ||
      !ortho_windows_.small_window || !ortho_windows_.fullscreen_window) {
    Logger::SetModule("Graphics");
    Logger::LogError("SetupRenderGraph: resources not initialized.");
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

  // Setup renderable objects
  SetupRenderableObjects();

  if (!render_graph_.Compile()) {
    Logger::SetModule("Graphics");
    Logger::LogError("Failed to compile RenderGraph!");
    return false;
  }

  cout << "=== RenderGraph Setup Complete ===\n" << endl;

  render_graph_.PrintGraph();
  return true;
}

void Graphics::SetupRenderPasses() {

  // Import existing textures into RenderGraph
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

  const auto &water_refraction_tex = render_targets_.refraction.refraction_map;
  render_graph_.ImportTexture("WaterRefraction", water_refraction_tex);

  const auto &water_reflection_tex =
      render_targets_.refraction.water_reflection_map;
  render_graph_.ImportTexture("WaterReflection", water_reflection_tex);

  // Pass 1: Depth Pass
  const auto &depth_shader = shader_assets_.depth;
  render_graph_.AddPass("DepthPass")
      .SetShader(depth_shader)
      .Write("DepthMap")
      .AddRenderTag(write_depth_tag);

  // Pass 2: Shadow Pass (standard execution, auto-bind depth map as parameter)
  const auto &shadow_shader = shader_assets_.shadow;
  render_graph_.AddPass("ShadowPass")
      .SetShader(shadow_shader)
      .ReadAsParameter("DepthMap",
                       "depthMapTexture") // Auto-bind resource to parameter
      .Write("ShadowMap")
      .AddRenderTag(write_shadow_tag);

  // Pass 3: Downsample
  XMMATRIX orthoMatrix;
  downsample_tex->GetOrthoMatrix(orthoMatrix);

  const auto &texture_shader = shader_assets_.texture;
  render_graph_.AddPass("DownsamplePass")
      .SetShader(texture_shader)
      .ReadAsParameter("ShadowMap",
                       "texture") // Auto-bind: ShadowMap -> texture
      .Write("DownsampledShadow")
      .AddRenderTag(down_sample_tag)
      .DisableZBuffer(true)
      .SetParameter("orthoMatrix", orthoMatrix);

  // Pass 4: Horizontal Blur
  h_blur_tex->GetOrthoMatrix(orthoMatrix);

  const auto &horizontal_blur_shader = shader_assets_.horizontal_blur;
  render_graph_.AddPass("HorizontalBlurPass")
      .SetShader(horizontal_blur_shader)
      .ReadAsParameter("DownsampledShadow", "texture") // Auto-bind resource
      .Write("HorizontalBlur")
      .AddRenderTag(horizontal_blur_tag)
      .DisableZBuffer(true)
      .SetParameter("orthoMatrix", orthoMatrix)
      .SetParameter("screenWidth", static_cast<float>(downSampleWidth));

  // Pass 5: Vertical Blur
  v_blur_tex->GetOrthoMatrix(orthoMatrix);

  const auto &vertical_blur_shader = shader_assets_.vertical_blur;
  render_graph_.AddPass("VerticalBlurPass")
      .SetShader(vertical_blur_shader)
      .ReadAsParameter("HorizontalBlur", "texture") // Auto-bind resource
      .Write("VerticalBlur")
      .AddRenderTag(vertical_blur_tag)
      .DisableZBuffer(true)
      .SetParameter("orthoMatrix", orthoMatrix)
      .SetParameter("screenHeight", static_cast<float>(downSampleHeight));

  // Pass 6: Upsample
  upsample_tex->GetOrthoMatrix(orthoMatrix);

  render_graph_.AddPass("UpsamplePass")
      .SetShader(texture_shader)
      .ReadAsParameter("VerticalBlur", "texture") // Auto-bind resource
      .Write("UpsampledShadow")
      .AddRenderTag(up_sample_tag)
      .DisableZBuffer(true)
      .SetParameter("orthoMatrix", orthoMatrix);

  // Pass 7: Reflection scene pass (render reflected objects)
  const auto &soft_shadow_shader = shader_assets_.soft_shadow;
  render_graph_.AddPass("ReflectionScenePass")
      .SetShader(soft_shadow_shader)
      .ReadAsParameter("UpsampledShadow", "shadowTexture")
      .Write("ReflectionMap")
      .AddRenderTag(reflection_tag)
      .SetParameter("ambientColor", light_->GetAmbientColor())
      .SetParameter("diffuseColor", light_->GetDiffuseColor())
      .SetParameter("reflectionBlend", 0.0f)
      .SetParameter("shadowStrength", 0.0f)
      .Execute([this](RenderPassContext &ctx) {
        if (!ctx.shader)
          return;

        ShaderParameterContainer merged = *ctx.pass_params;
        merged.Merge(*ctx.global_params);

        if (ctx.global_params->HasParameter("reflectionMatrix")) {
          merged.SetMatrix("viewMatrix",
                           ctx.global_params->GetMatrix("reflectionMatrix"));
        }

        if (ctx.global_params->HasParameter("projectionMatrix")) {
          merged.SetMatrix("projectionMatrix",
                           ctx.global_params->GetMatrix("projectionMatrix"));
        }

        for (const auto &renderable : *ctx.renderables) {
          if (!renderable->HasTag(reflection_tag))
            continue;

          ShaderParameterContainer objParams = merged;
          objParams.Set("worldMatrix", renderable->GetWorldMatrix());
          if (auto cb = renderable->GetParameterCallback()) {
            cb(objParams);
          }
          // Disable shadowing and recursive reflection for reflection pass.
          objParams.SetFloat("shadowStrength", 0.0f);
          objParams.SetFloat("reflectionBlend", 0.0f);
          renderable->Render(*ctx.shader, objParams, ctx.device_context);
        }
      });

  // Pass 8: Final Pass (soft shadow)
  render_graph_.AddPass("FinalPass")
      .SetShader(soft_shadow_shader)
      .ReadAsParameter("UpsampledShadow", "shadowTexture") // Auto-bind resource
      .ReadAsParameter("ReflectionMap", "reflectionTexture")
      .AddRenderTag(final_tag)
      .SetParameter("diffuseColor", light_->GetDiffuseColor())
      .SetParameter("ambientColor", light_->GetAmbientColor())
      .SetParameter("reflectionBlend", 0.0f)
      .SetParameter("shadowStrength", 1.0f);

  // Pass 9: PBR Pass (standard execution)
  const auto &sphere_pbr_model = scene_assets_.pbr_sphere;
  const auto &pbr_shader = shader_assets_.pbr;
  render_graph_.AddPass("PBRPass")
      .SetShader(pbr_shader)
      .AddRenderTag(pbr_tag)
      .SetTexture("diffuseTexture", sphere_pbr_model->GetTexture(0))
      .SetTexture("normalMap", sphere_pbr_model->GetTexture(1))
      .SetTexture("rmTexture", sphere_pbr_model->GetTexture(2));

  // Pass 10: DiffuseLighting Pass (diffuse lighting shader demo)
  const auto &diffuse_lighting_shader = shader_assets_.diffuse_lighting;
  render_graph_.AddPass("DiffuseLightingPass")
      .SetShader(diffuse_lighting_shader)
      .AddRenderTag(diffuse_lighting_tag)
      .SetParameter("ambientColor", light_->GetAmbientColor())
      .SetParameter("diffuseColor", light_->GetDiffuseColor())
      .SetParameter("lightDirection", light_->GetDirection());

  const auto &refraction_shader = shader_assets_.refraction.refraction;
  render_graph_.AddPass("WaterRefractionPass")
      .SetShader(refraction_shader)
      .Write("WaterRefraction")
      .AddRenderTag(refraction_pass_tag)
      .SetParameter(
          "clipPlane",
          DirectX::XMFLOAT4(0.0f, -1.0f, 0.0f,
                            scene_config_.constants.water_plane_height + 0.1f));

  const auto &scene_light_shader = shader_assets_.refraction.scene_light;
  render_graph_.AddPass("WaterReflectionPass")
      .SetShader(scene_light_shader)
      .Write("WaterReflection")
      .AddRenderTag(water_reflection_tag)
      .Execute([this](RenderPassContext &ctx) {
        if (!ctx.shader)
          return;

        ShaderParameterContainer merged = *ctx.pass_params;
        merged.Merge(*ctx.global_params);

        if (ctx.global_params->HasParameter("waterReflectionMatrix")) {
          merged.SetMatrix("viewMatrix", ctx.global_params->GetMatrix(
                                             "waterReflectionMatrix"));
        }

        if (ctx.global_params->HasParameter("projectionMatrix")) {
          merged.SetMatrix("projectionMatrix",
                           ctx.global_params->GetMatrix("projectionMatrix"));
        }

        for (const auto &renderable : *ctx.renderables) {
          if (!renderable->HasTag(water_reflection_tag))
            continue;

          ShaderParameterContainer objParams = merged;
          objParams.Set("worldMatrix", renderable->GetWorldMatrix());
          if (auto cb = renderable->GetParameterCallback()) {
            cb(objParams);
          }
          renderable->Render(*ctx.shader, objParams, ctx.device_context);
        }
      });

  render_graph_.AddPass("SceneLightPass")
      .SetShader(scene_light_shader)
      .AddRenderTag(scene_light_tag);

  const auto &water_shader = shader_assets_.refraction.water;
  const auto &water_model = scene_assets_.refraction.water;
  render_graph_.AddPass("WaterSurfacePass")
      .SetShader(water_shader)
      .ReadAsParameter("WaterReflection", "reflectionTexture")
      .ReadAsParameter("WaterRefraction", "refractionTexture")
      .AddRenderTag(water_surface_tag)
      .SetTexture("normalTexture", water_model->GetTexture())
      .SetParameter("reflectRefractScale",
                    scene_config_.constants.water_reflect_refract_scale);

  // Pass 10: Text overlay (executed via custom lambda)
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

std::shared_ptr<RenderableObject> CreateTexturedModelObject(
    std::shared_ptr<Model> model, std::shared_ptr<IShader> shader,
    const XMMATRIX &worldMatrix, bool enable_reflection = true) {
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

std::shared_ptr<RenderableObject>
CreatePostProcessObject(std::shared_ptr<OrthoWindow> window,
                        std::shared_ptr<IShader> shader, const std::string &tag,
                        std::shared_ptr<RenderTexture> texture) {
  auto obj = std::make_shared<RenderableObject>(window, shader);
  obj->AddTag(tag);
  obj->SetParameterCallback([texture](ShaderParameterContainer &p) {
    p.SetTexture("texture", texture->GetShaderResourceView());
  });
  return obj;
}

std::shared_ptr<RenderableObject>
CreatePBRModelObject(std::shared_ptr<PBRModel> model,
                     std::shared_ptr<IShader> shader,
                     const XMMATRIX &worldMatrix) {
  auto obj = std::make_shared<RenderableObject>(model, shader);
  obj->SetWorldMatrix(worldMatrix);
  obj->AddTag(write_depth_tag);
  obj->AddTag(write_shadow_tag);
  obj->AddTag(pbr_tag);
  return obj;
}

void Graphics::SetupRenderableObjects() {

  // Add renderable objects
  const auto &cube_model = scene_assets_.cube;
  const auto &soft_shadow_shader = shader_assets_.soft_shadow;
  {
    auto cube_object = CreateTexturedModelObject(
        cube_model, soft_shadow_shader, XMMatrixTranslation(-2.5f, 2.0f, 0.0f));
    renderable_objects_.push_back(cube_object);
  }

  {
    const auto &sphere_model = scene_assets_.sphere;
    auto sphere_object =
        CreateTexturedModelObject(sphere_model, soft_shadow_shader,
                                  XMMatrixTranslation(2.5f, 2.0f, 0.0f));
    renderable_objects_.push_back(sphere_object);
  }

  {
    const auto &sphere_pbr_model = scene_assets_.pbr_sphere;
    const auto &pbr_shader = shader_assets_.pbr;
    auto pbr_sphere_object = CreatePBRModelObject(
        sphere_pbr_model, pbr_shader, XMMatrixTranslation(0.0f, 2.0f, -2.0f));
    pbr_group_->AddRenderable(pbr_sphere_object);
    renderable_objects_.push_back(pbr_sphere_object);
  }

  const auto &texture_shader = shader_assets_.texture;
  const auto &small_window = ortho_windows_.small_window;
  {
    const auto &shadow_tex = render_targets_.shadow_map;
    auto down_sample_object = CreatePostProcessObject(
        small_window, texture_shader, down_sample_tag, shadow_tex);
    down_sample_object->AddTag(
        "skip_culling"); // Post-processing objects should not be culled
    renderable_objects_.push_back(down_sample_object);
  }

  {
    const auto &horizontal_blur_shader = shader_assets_.horizontal_blur;
    const auto &downsample_tex = render_targets_.downsampled_shadow;
    auto horizontal_blur_object =
        CreatePostProcessObject(small_window, horizontal_blur_shader,
                                horizontal_blur_tag, downsample_tex);
    horizontal_blur_object->AddTag(
        "skip_culling"); // Post-processing objects should not be culled
    renderable_objects_.push_back(horizontal_blur_object);
  }

  {
    const auto &vertical_blur_shader = shader_assets_.vertical_blur;
    const auto &h_blur_tex = render_targets_.horizontal_blur;
    auto vertical_blur_object = CreatePostProcessObject(
        small_window, vertical_blur_shader, vertical_blur_tag, h_blur_tex);
    vertical_blur_object->AddTag(
        "skip_culling"); // Post-processing objects should not be culled
    renderable_objects_.push_back(vertical_blur_object);
  }

  {
    const auto &fullscreen_window = ortho_windows_.fullscreen_window;
    const auto &v_blur_tex = render_targets_.vertical_blur;
    auto up_sample_object = CreatePostProcessObject(
        fullscreen_window, texture_shader, up_sample_tag, v_blur_tex);
    up_sample_object->AddTag(
        "skip_culling"); // Post-processing objects should not be culled
    renderable_objects_.push_back(up_sample_object);
  }

  {
    const auto &ground_model = scene_assets_.ground;
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
  // Position in a separate area (right and forward) to avoid overlapping with
  // soft shadow demo
  {
    const auto &diffuse_lighting_shader = shader_assets_.diffuse_lighting;
    auto diffuse_lighting_cube_obj =
        std::make_shared<RenderableObject>(cube_model, diffuse_lighting_shader);
    diffuse_lighting_cube_obj->SetWorldMatrix(
        XMMatrixTranslation(6.0f, 2.0f, 3.0f));
    diffuse_lighting_cube_obj->AddTag(diffuse_lighting_tag);
    diffuse_lighting_cube_obj->SetParameterCallback(
        [cube_model](ShaderParameterContainer &params) {
          params.SetTexture("texture", cube_model->GetTexture());
        });
    diffuse_lighting_cube_ = diffuse_lighting_cube_obj;
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

    cube_group_->AddRenderable(cube_obj);
    renderable_objects_.push_back(cube_obj);
  }

  const auto &refraction_assets = scene_assets_.refraction;
  const auto scene_offset =
      XMMatrixTranslation(scene_config_.constants.refraction_scene_offset_x,
                          scene_config_.constants.refraction_scene_offset_y,
                          scene_config_.constants.refraction_scene_offset_z);

  if (refraction_assets.ground && shader_assets_.refraction.scene_light) {
    auto ground_object = std::make_shared<RenderableObject>(
        refraction_assets.ground, shader_assets_.refraction.scene_light);
    auto ground_world =
        XMMatrixScaling(scene_config_.constants.refraction_ground_scale, 1.0f,
                        scene_config_.constants.refraction_ground_scale) *
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

  if (refraction_assets.wall && shader_assets_.refraction.scene_light) {
    auto wall_object = std::make_shared<RenderableObject>(
        refraction_assets.wall, shader_assets_.refraction.scene_light);
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

  if (refraction_assets.bath && shader_assets_.refraction.scene_light) {
    auto bath_object = std::make_shared<RenderableObject>(
        refraction_assets.bath, shader_assets_.refraction.scene_light);
    bath_object->SetWorldMatrix(XMMatrixTranslation(0.0f, 2.0f, 0.0f) *
                                scene_offset);
    bath_object->AddTag(scene_light_tag);
    bath_object->AddTag(refraction_pass_tag);
    bath_object->SetParameterCallback([bath_model = refraction_assets.bath](
                                          ShaderParameterContainer &params) {
      params.SetTexture("texture", bath_model->GetTexture());
    });
    renderable_objects_.push_back(bath_object);
  }

  if (refraction_assets.water && shader_assets_.refraction.water) {
    auto water_object = std::make_shared<RenderableObject>(
        refraction_assets.water, shader_assets_.refraction.water);
    water_object->SetWorldMatrix(
        XMMatrixTranslation(0.0f, scene_config_.constants.water_plane_height,
                            0.0f) *
        scene_offset);
    water_object->AddTag(water_surface_tag);
    renderable_objects_.push_back(water_object);
  }
}

void Graphics::RegisterShaderParameters() {
  // Set validation mode to Warning (report issues but don't block execution)
  parameter_validator_.SetValidationMode(ValidationMode::Warning);

  // Register global parameters (provided at runtime by Render() or per-object)
  // These parameters are automatically available to all shaders and don't need
  // to be set at Pass level
  parameter_validator_.RegisterGlobalParameter("worldMatrix"); // Set per-object
  parameter_validator_.RegisterGlobalParameter("viewMatrix");  // From Render()
  parameter_validator_.RegisterGlobalParameter(
      "projectionMatrix"); // From Render()
  parameter_validator_.RegisterGlobalParameter(
      "baseViewMatrix"); // From Render()
  parameter_validator_.RegisterGlobalParameter(
      "deviceWorldMatrix"); // From Render()
  parameter_validator_.RegisterGlobalParameter(
      "lightViewMatrix"); // From Render()
  parameter_validator_.RegisterGlobalParameter(
      "lightProjectionMatrix"); // From Render()
  parameter_validator_.RegisterGlobalParameter(
      "lightPosition"); // From Render()
  parameter_validator_.RegisterGlobalParameter(
      "lightDirection"); // From Render()
  parameter_validator_.RegisterGlobalParameter(
      "cameraPosition"); // From Render()
  parameter_validator_.RegisterGlobalParameter(
      "reflectionMatrix"); // From Render()
  parameter_validator_.RegisterGlobalParameter(
      "waterReflectionMatrix");                                 // From Render()
  parameter_validator_.RegisterGlobalParameter("ambientColor"); // From Render()
  parameter_validator_.RegisterGlobalParameter("diffuseColor"); // From Render()
  parameter_validator_.RegisterGlobalParameter(
      "waterTranslation"); // From Render()
  parameter_validator_.RegisterGlobalParameter(
      "reflectRefractScale"); // From Render()

  // Register DepthShader parameters
  parameter_validator_.RegisterShader(
      "DepthShader",
      {{"worldMatrix", ShaderParameterType::Matrix, true},
       {"lightViewMatrix", ShaderParameterType::Matrix, true},
       {"lightProjectionMatrix", ShaderParameterType::Matrix, true}});

  // Register ShadowShader parameters
  parameter_validator_.RegisterShader(
      "ShadowShader",
      {{"worldMatrix", ShaderParameterType::Matrix, true},
       {"viewMatrix", ShaderParameterType::Matrix, true},
       {"projectionMatrix", ShaderParameterType::Matrix, true},
       {"lightViewMatrix", ShaderParameterType::Matrix, true},
       {"lightProjectionMatrix", ShaderParameterType::Matrix, true},
       {"lightPosition", ShaderParameterType::Vector3, true},
       {"depthMapTexture", ShaderParameterType::Texture, true}});

  // Register SoftShadowShader parameters
  // Note: "texture" is set via object callbacks, not at Pass level
  parameter_validator_.RegisterShader(
      "SoftShadowShader",
      {{"worldMatrix", ShaderParameterType::Matrix, true},
       {"viewMatrix", ShaderParameterType::Matrix, true},
       {"projectionMatrix", ShaderParameterType::Matrix, true},
       {"texture", ShaderParameterType::Texture, false}, // Set via callback
       {"shadowTexture", ShaderParameterType::Texture, true},
       {"ambientColor", ShaderParameterType::Vector4, true},
       {"diffuseColor", ShaderParameterType::Vector4, true},
       {"lightPosition", ShaderParameterType::Vector3, true},
       {"reflectionMatrix", ShaderParameterType::Matrix, true},
       {"reflectionTexture", ShaderParameterType::Texture, false},
       {"reflectionBlend", ShaderParameterType::Float, false},
       {"shadowStrength", ShaderParameterType::Float, false}});

  // Register PbrShader parameters
  parameter_validator_.RegisterShader(
      "PbrShader", {{"worldMatrix", ShaderParameterType::Matrix, true},
                    {"viewMatrix", ShaderParameterType::Matrix, true},
                    {"projectionMatrix", ShaderParameterType::Matrix, true},
                    {"diffuseTexture", ShaderParameterType::Texture, true},
                    {"normalMap", ShaderParameterType::Texture, true},
                    {"rmTexture", ShaderParameterType::Texture, true},
                    {"lightDirection", ShaderParameterType::Vector3, true},
                    {"cameraPosition", ShaderParameterType::Vector3, true}});

  // Register TextureShader parameters
  parameter_validator_.RegisterShader(
      "TextureShader",
      {{"deviceWorldMatrix", ShaderParameterType::Matrix, true},
       {"baseViewMatrix", ShaderParameterType::Matrix, true},
       {"orthoMatrix", ShaderParameterType::Matrix, true},
       {"texture", ShaderParameterType::Texture, true}});

  // Register HorizontalBlurShader parameters
  parameter_validator_.RegisterShader(
      "HorizontalBlurShader",
      {{"worldMatrix", ShaderParameterType::Matrix, true},
       {"baseViewMatrix", ShaderParameterType::Matrix, true},
       {"orthoMatrix", ShaderParameterType::Matrix, true},
       {"screenWidth", ShaderParameterType::Float, true},
       {"texture", ShaderParameterType::Texture, true}});

  // Register VerticalBlurShader parameters
  parameter_validator_.RegisterShader(
      "VerticalBlurShader",
      {{"worldMatrix", ShaderParameterType::Matrix, true},
       {"baseViewMatrix", ShaderParameterType::Matrix, true},
       {"orthoMatrix", ShaderParameterType::Matrix, true},
       {"screenHeight", ShaderParameterType::Float, true},
       {"texture", ShaderParameterType::Texture, true}});

  parameter_validator_.RegisterShader(
      "SceneLightShader",
      {{"worldMatrix", ShaderParameterType::Matrix, true},
       {"viewMatrix", ShaderParameterType::Matrix, true},
       {"projectionMatrix", ShaderParameterType::Matrix, true},
       {"texture", ShaderParameterType::Texture, false}, // Set via callback
       {"ambientColor", ShaderParameterType::Vector4, true},
       {"diffuseColor", ShaderParameterType::Vector4, true},
       {"lightDirection", ShaderParameterType::Vector3, true}});

  // Register SimpleLightShader parameters (diffuse lighting shader demo)
  parameter_validator_.RegisterShader(
      "SimpleLightShader",
      {{"worldMatrix", ShaderParameterType::Matrix, true},
       {"viewMatrix", ShaderParameterType::Matrix, true},
       {"projectionMatrix", ShaderParameterType::Matrix, true},
       {"texture", ShaderParameterType::Texture, false}, // Set via callback
       {"ambientColor", ShaderParameterType::Vector4, true},
       {"diffuseColor", ShaderParameterType::Vector4, true},
       {"lightDirection", ShaderParameterType::Vector3, true}});

  parameter_validator_.RegisterShader(
      "RefractionShader",
      {{"worldMatrix", ShaderParameterType::Matrix, true},
       {"viewMatrix", ShaderParameterType::Matrix, true},
       {"projectionMatrix", ShaderParameterType::Matrix, true},
       {"texture", ShaderParameterType::Texture, false}, // Set via callback
       {"ambientColor", ShaderParameterType::Vector4, true},
       {"diffuseColor", ShaderParameterType::Vector4, true},
       {"lightDirection", ShaderParameterType::Vector3, true},
       {"clipPlane", ShaderParameterType::Vector4, true}});

  parameter_validator_.RegisterShader(
      "WaterShader",
      {{"worldMatrix", ShaderParameterType::Matrix, true},
       {"viewMatrix", ShaderParameterType::Matrix, true},
       {"projectionMatrix", ShaderParameterType::Matrix, true},
       {"waterReflectionMatrix", ShaderParameterType::Matrix, true},
       {"reflectionTexture", ShaderParameterType::Texture, true},
       {"refractionTexture", ShaderParameterType::Texture, true},
       {"normalTexture", ShaderParameterType::Texture, true},
       {"waterTranslation", ShaderParameterType::Float, true},
       {"reflectRefractScale", ShaderParameterType::Float, true}});

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

  // Update the view matrix based on the camera's position.
  camera_->Render();
  camera_->RenderReflection(scene_config_.constants.reflection_plane_height);
  XMMATRIX viewMatrix, baseViewMatrix;
  camera_->GetViewMatrix(viewMatrix);
  camera_->GetBaseViewMatrix(baseViewMatrix);
  auto reflectionMatrix = camera_->GetReflectionViewMatrix();

  camera_->RenderReflection(scene_config_.constants.water_plane_height);
  auto waterReflectionMatrix = camera_->GetReflectionViewMatrix();

  // Restore the original reflection matrix for soft shadow pipeline.
  camera_->RenderReflection(scene_config_.constants.reflection_plane_height);

  // Update the light
  light_->GenerateViewMatrix();
  XMMATRIX lightViewMatrix, lightProjectionMatrix;
  light_->GetViewMatrix(lightViewMatrix);
  light_->GetProjectionMatrix(lightProjectionMatrix);
  // light_->SetDirection(0.0f - light_->GetPosition().x, 2.0f -
  // light_->GetPosition().y, -2.0f - light_->GetPosition().z);
  light_->SetDirection(0.5f, 0.5f, 0.5f);

  ShaderParameterContainer Params;
  Params.SetGlobalDynamicMatrix("viewMatrix", viewMatrix);
  Params.SetGlobalDynamicMatrix("baseViewMatrix", baseViewMatrix);
  Params.SetGlobalDynamicMatrix("lightViewMatrix", lightViewMatrix);
  Params.SetGlobalDynamicMatrix("lightProjectionMatrix", lightProjectionMatrix);
  Params.SetGlobalDynamicVector3("lightPosition", light_->GetPosition());
  Params.SetGlobalDynamicVector3("lightDirection", light_->GetDirection());
  Params.SetGlobalDynamicVector3("cameraPosition", camera_->GetPosition());
  Params.SetMatrix("reflectionMatrix", reflectionMatrix);
  Params.SetMatrix("waterReflectionMatrix", waterReflectionMatrix);
  Params.SetVector4("ambientColor", light_->GetAmbientColor());
  Params.SetVector4("diffuseColor", light_->GetDiffuseColor());
  Params.SetFloat("waterTranslation", water_translation_);
  Params.SetFloat("reflectRefractScale",
                  scene_config_.constants.water_reflect_refract_scale);

  // Add device matrices
  XMMATRIX deviceWorldMatrix, projectionMatrix;
  directx_device_->GetWorldMatrix(deviceWorldMatrix);
  directx_device_->GetProjectionMatrix(projectionMatrix);
  Params.SetMatrix("deviceWorldMatrix", deviceWorldMatrix);
  Params.SetMatrix("projectionMatrix", projectionMatrix);

  // Construct frustum for culling
  if (frustum_) {
    frustum_->ConstructFrustum(SCREEN_DEPTH, projectionMatrix, viewMatrix);
  }

  // Perform frustum culling: filter renderable objects
  std::vector<std::shared_ptr<IRenderable>> culled_objects;
  if (frustum_) {
    for (const auto &renderable : renderable_objects_) {
      if (IsObjectVisible(renderable, *frustum_)) {
        culled_objects.push_back(renderable);
      }
    }
  } else {
    // If frustum not initialized, render all objects
    culled_objects = renderable_objects_;
  }

  // Update render count before rendering
  int renderCount = static_cast<int>(culled_objects.size());
  if (text_) {
    text_->SetRenderCount(renderCount);
  }

  render_graph_.Execute(culled_objects, Params);
}