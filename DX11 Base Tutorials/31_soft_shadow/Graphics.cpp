#include "Graphics.h"

#include "../CommonFramework2/DirectX11Device.h"
#include "../CommonFramework2/TypeDefine.h"

#include "RenderableObject.h"
#include "ResourceManager.h"
#include "StandardRenderGroup.h"
#include "depthshader.h"
#include "horizontalblurshader.h"
#include "model.h"
#include "orthowindow.h"
#include "pbrshader.h"
#include "rendertexture.h"
#include "shadowshader.h"
#include "softshadowshader.h"
#include "textureshader.h"
#include "verticalblurshader.h"

#include <iostream>

using namespace std;
using namespace DirectX;

static constexpr auto SHADOW_MAP_WIDTH = 1024;
static constexpr auto SHADOW_MAP_HEIGHT = 1024;
// Set the size to sample down to.
static constexpr auto downSampleWidth = SHADOW_MAP_WIDTH / 2;
static constexpr auto downSampleHeight = SHADOW_MAP_HEIGHT / 2;

bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd) {

  if (!InitializeDevice(screenWidth, screenHeight, hwnd)) {
    return false;
  }

  if (!InitializeCamera()) {
    return false;
  }

  if (!InitializeLight()) {
    return false;
  }

  if (!InitializeResources(hwnd)) {
    return false;
  }

  if (!InitializeRenderingSystem()) {
    return false;
  }

  // Log resource statistics
  auto &resource_manager = ResourceManager::GetInstance();
  resource_manager.LogResourceStats();

  return true;
}

bool GraphicsClass::InitializeDevice(int screenWidth, int screenHeight,
                                     HWND hwnd) {
  auto directx11_device_ = DirectX11Device::GetD3d11DeviceInstance();

  if (!(directx11_device_->Initialize(screenWidth, screenHeight, VSYNC_ENABLED,
                                      hwnd, FULL_SCREEN, SCREEN_DEPTH,
                                      SCREEN_NEAR))) {
    MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
    return false;
  }

  // Initialize ResourceManager
  auto &resource_manager = ResourceManager::GetInstance();
  auto device = directx11_device_->GetDevice();
  auto device_context = directx11_device_->GetDeviceContext();

  if (!resource_manager.Initialize(device, device_context, hwnd)) {
    MessageBox(hwnd, L"Could not initialize ResourceManager.", L"Error",
               MB_OK);
    return false;
  }

  // Initialize render pipeline
  render_pipeline_.Initialize(device, device_context);

  return true;
}

bool GraphicsClass::InitializeCamera() {
  camera_ = make_unique<Camera>();
  if (nullptr == camera_) {
    return false;
  }
  camera_->SetPosition(0.0f, -1.0f, -10.0f);
  camera_->Render();
  camera_->RenderBaseViewMatrix();
  return true;
}

bool GraphicsClass::InitializeLight() {
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

bool GraphicsClass::InitializeResources(HWND hwnd) {
  auto &resource_manager = ResourceManager::GetInstance();

  // 1. 模型与几何体资源
  scene_assets_.cube =
      resource_manager.GetModel("cube", "./data/cube.txt", L"./data/wall01.dds");
  scene_assets_.sphere =
      resource_manager.GetModel("sphere", "./data/sphere.txt", L"./data/ice.dds");
  scene_assets_.ground = resource_manager.GetModel("ground", "./data/plane01.txt",
                                                   L"./data/metal001.dds");

  if (!scene_assets_.cube || !scene_assets_.sphere || !scene_assets_.ground) {
    std::wstring error_msg = L"Could not load models.\n";
    error_msg += std::wstring(resource_manager.GetLastError().begin(),
                              resource_manager.GetLastError().end());
    MessageBox(hwnd, error_msg.c_str(), L"Resource Error", MB_OK);
    return false;
  }

  scene_assets_.pbr_sphere = resource_manager.GetPBRModel(
      "sphere_pbr", "./data/sphere.txt", "./data/pbr_albedo.tga",
      "./data/pbr_normal.tga", "./data/pbr_roughmetal.tga");

  if (!scene_assets_.pbr_sphere) {
    std::wstring error_msg = L"Could not load PBR model.\n";
    error_msg += std::wstring(resource_manager.GetLastError().begin(),
                              resource_manager.GetLastError().end());
    MessageBox(hwnd, error_msg.c_str(), L"Resource Error", MB_OK);
    return false;
  }

  // 2. 渲染目标
  render_targets_.shadow_depth = resource_manager.CreateRenderTexture(
      "shadow_depth", SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, SCREEN_DEPTH,
      SCREEN_NEAR);
  render_targets_.shadow_map = resource_manager.CreateRenderTexture(
      "shadow_map", SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, SCREEN_DEPTH,
      SCREEN_NEAR);
  render_targets_.downsampled_shadow = resource_manager.CreateRenderTexture(
      "downsample", downSampleWidth, downSampleHeight, 100.0f, 1.0f);
  render_targets_.horizontal_blur = resource_manager.CreateRenderTexture(
      "horizontal_blur", downSampleWidth, downSampleHeight, SCREEN_DEPTH, 0.1f);
  render_targets_.vertical_blur = resource_manager.CreateRenderTexture(
      "vertical_blur", downSampleWidth, downSampleHeight, SCREEN_DEPTH, 0.1f);
  render_targets_.upsampled_shadow = resource_manager.CreateRenderTexture(
      "upsample", SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, SCREEN_DEPTH, 0.1f);

  if (!render_targets_.shadow_depth || !render_targets_.shadow_map ||
      !render_targets_.downsampled_shadow || !render_targets_.horizontal_blur ||
      !render_targets_.vertical_blur || !render_targets_.upsampled_shadow) {
    MessageBox(hwnd, L"Could not create render textures.", L"Error", MB_OK);
    return false;
  }

  // 3. 着色器
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

  if (!shader_assets_.depth || !shader_assets_.shadow ||
      !shader_assets_.texture || !shader_assets_.horizontal_blur ||
      !shader_assets_.vertical_blur || !shader_assets_.soft_shadow ||
      !shader_assets_.pbr) {
    MessageBox(hwnd, L"Could not load shaders.", L"Error", MB_OK);
    return false;
  }

  // 4. 正交屏幕窗口
  ortho_windows_.small_window =
      resource_manager.GetOrthoWindow("small_window", downSampleWidth,
                                      downSampleHeight);
  ortho_windows_.fullscreen_window = resource_manager.GetOrthoWindow(
      "fullscreen_window", SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);

  if (!ortho_windows_.small_window || !ortho_windows_.fullscreen_window) {
    MessageBox(hwnd, L"Could not create ortho windows.", L"Error", MB_OK);
    return false;
  }

  // 5. 预先创建渲染组
  cube_group_ = make_shared<StandardRenderGroup>();

  return true;
}

bool GraphicsClass::InitializeRenderingSystem() {
  // Setup rendering system based on compile-time constant
  if constexpr (use_render_graph_) {
    SetupRenderGraph();
  } else {
    SetupRenderPipeline();
  }
  return true;
}

void GraphicsClass::Shutdown() {
  auto &resource_manager = ResourceManager::GetInstance();

  // Display resource usage before shutdown
  cout << "\n=== Resource Usage Before Shutdown ===" << endl;
  cout << "Total cached resources: " << resource_manager.GetTotalCachedResources() << endl;

  // Check for unused resources
  auto unusedModels = resource_manager.GetUnusedModels();
  auto unusedShaders = resource_manager.GetUnusedShaders();
  auto unusedRTTs = resource_manager.GetUnusedRenderTextures();

  cout << "Unused models: " << unusedModels.size() << endl;
  for (const auto &name : unusedModels) {
    cout << "  - " << name << " (ref count: " << resource_manager.GetModelRefCount(name)
         << ")" << endl;
  }

  cout << "Unused shaders: " << unusedShaders.size() << endl;
  for (const auto &name : unusedShaders) {
    cout << "  - " << name << " (ref count: " << resource_manager.GetShaderRefCount(name)
         << ")" << endl;
  }

  cout << "Unused render textures: " << unusedRTTs.size() << endl;
  for (const auto &name : unusedRTTs) {
    cout << "  - " << name
         << " (ref count: " << resource_manager.GetRenderTextureRefCount(name) << ")" << endl;
  }

  // Optionally prune unused resources
  // size_t pruned = resourceManager.PruneAllUnusedResources();

  cout << "========================================\n" << endl;

  // Shutdown ResourceManager
  resource_manager.Shutdown();
}

void GraphicsClass::Frame(float deltaTime) {

  static float lightPositionX = -5.0f;

  camera_->SetPosition(pos_x_, pos_y_, pos_z_);
  camera_->SetRotation(rot_x_, rot_y_, rot_z_);

  // Update the position of the light each frame.
  lightPositionX += 0.001f * deltaTime;
  if (lightPositionX > 5.0f) {
    lightPositionX = -5.0f;
  }
  light_->SetPosition(lightPositionX, 8.0f, -5.0f);

  static float rotation_y = 0.0f;
  if (0.000001 < 360.0f - rotation_y)
    rotation_y = 0.0f;
  rotation_y += DirectX::XM_PI * 0.001f * deltaTime;

  XMMATRIX rotationMatrix =
      XMMatrixRotationRollPitchYaw(0.0f, rotation_y, 0.0f);

  XMMATRIX translationMatrix;
  for (const auto &renderable : cube_group_->GetRenderables()) {
    translationMatrix = renderable->GetWorldMatrix();
    renderable->SetWorldMatrix(rotationMatrix * translationMatrix);
  }

#ifdef _DEBUG
  // Debug: periodically log resource usage (every 5 seconds)
  static float debugTimer = 0.0f;
  debugTimer += deltaTime / 1000.0f;
  if (debugTimer > 5.0f) {
    debugTimer = 0.0f;
    auto &resource_manager = ResourceManager::GetInstance();
    cout << "\n[DEBUG] Resource Usage:" << endl;
    cout << "  Cube model ref count: " << resource_manager.GetModelRefCount("cube") << endl;
    cout << "  Sphere model ref count: " << resource_manager.GetModelRefCount("sphere")
         << endl;
    cout << "  Ground model ref count: " << resource_manager.GetModelRefCount("ground")
         << endl;
    cout << "  Total cached: " << resource_manager.GetTotalCachedResources() << endl;
  }
#endif

  // Render the graphics scene.
  Render();
}

static constexpr auto write_depth_tag = "write_depth";
static constexpr auto write_shadow_tag = "write_shadow";
static constexpr auto down_sample_tag = "down_sample";
static constexpr auto horizontal_blur_tag = "horizontal_blur";
static constexpr auto vertical_blur_tag = "vertical_blur";
static constexpr auto up_sample_tag = "up_sample";
static constexpr auto pbr_tag = "pbr_tag";
static constexpr auto final_tag = "final";

void GraphicsClass::SetupRenderPipeline() {

  if (!scene_assets_.cube || !scene_assets_.sphere || !scene_assets_.ground ||
      !scene_assets_.pbr_sphere || !render_targets_.shadow_depth ||
      !shader_assets_.depth || !ortho_windows_.small_window ||
      !ortho_windows_.fullscreen_window) {
    cerr << "SetupRenderPipeline: resources not initialized." << endl;
    return;
  }

  const auto &cube_model = scene_assets_.cube;
  const auto &sphere_model = scene_assets_.sphere;
  const auto &ground_model = scene_assets_.ground;
  const auto &sphere_pbr_model = scene_assets_.pbr_sphere;

  const auto &shadow_depth_target = render_targets_.shadow_depth;
  const auto &shadow_map_target = render_targets_.shadow_map;
  const auto &downsample_target = render_targets_.downsampled_shadow;
  const auto &horizontal_blur_target = render_targets_.horizontal_blur;
  const auto &vertical_blur_target = render_targets_.vertical_blur;
  const auto &upsample_target = render_targets_.upsampled_shadow;

  const auto &depth_shader = shader_assets_.depth;
  const auto &shadow_shader = shader_assets_.shadow;
  const auto &texture_shader = shader_assets_.texture;
  const auto &horizontal_blur_shader = shader_assets_.horizontal_blur;
  const auto &vertical_blur_shader = shader_assets_.vertical_blur;
  const auto &soft_shadow_shader = shader_assets_.soft_shadow;
  const auto &pbr_shader = shader_assets_.pbr;

  const auto &small_window = ortho_windows_.small_window;
  const auto &fullscreen_window = ortho_windows_.fullscreen_window;

  auto depth_pass = std::make_shared<RenderPass>("DepthPass", depth_shader);
  {
    depth_pass->AddRenderTag(write_depth_tag);
    depth_pass->SetOutputTexture(shadow_depth_target);

    render_pipeline_.AddRenderPass(depth_pass);
  }

  auto shadow_pass = std::make_shared<RenderPass>("ShadowPass", shadow_shader);
  {
    shadow_pass->AddRenderTag(write_shadow_tag);
    shadow_pass->AddInputTexture("depthMap", depth_pass->GetOutputTexture());
    shadow_pass->SetOutputTexture(shadow_map_target);

    ShaderParameterContainer shadowPassParams;
    shadowPassParams.SetTexture("depthMapTexture",
                                shadow_depth_target->GetShaderResourceView());
    shadow_pass->SetPassParameters(shadowPassParams);

    render_pipeline_.AddRenderPass(shadow_pass);
  }

  XMMATRIX orthoMatrix;

  auto down_sample_texture_pass =
      std::make_shared<RenderPass>("DownSampleTexture", texture_shader);
  {
    down_sample_texture_pass->AddRenderTag(down_sample_tag);
    down_sample_texture_pass->AddInputTexture("shadowMap",
                                              shadow_pass->GetOutputTexture());
    down_sample_texture_pass->SetOutputTexture(downsample_target);
    down_sample_texture_pass->NeedTurnOffZBuffer();

    ShaderParameterContainer downsampleParams;
    downsample_target->GetOrthoMatrix(orthoMatrix);
    downsampleParams.SetMatrix("orthoMatrix", orthoMatrix);
    downsampleParams.SetTexture(
        "texture", shadow_map_target->GetShaderResourceView());
    down_sample_texture_pass->SetPassParameters(downsampleParams);

    render_pipeline_.AddRenderPass(down_sample_texture_pass);
  }

  auto horizontal_blur_to_texture_pass = std::make_shared<RenderPass>(
      "RenderHorizontalBlurToTexture", horizontal_blur_shader);
  {
    horizontal_blur_to_texture_pass->AddRenderTag(horizontal_blur_tag);
    horizontal_blur_to_texture_pass->AddInputTexture(
        "DownSampleTexture", down_sample_texture_pass->GetOutputTexture());
    horizontal_blur_to_texture_pass->SetOutputTexture(horizontal_blur_target);
    horizontal_blur_to_texture_pass->NeedTurnOffZBuffer();

    ShaderParameterContainer horizontalBlurParams;
    horizontal_blur_target->GetOrthoMatrix(orthoMatrix);
    horizontalBlurParams.SetMatrix("orthoMatrix", orthoMatrix);
    horizontalBlurParams.SetFloat("screenWidth",
                                  static_cast<float>(SHADOW_MAP_WIDTH / 2));
    horizontalBlurParams.SetTexture(
        "texture", downsample_target->GetShaderResourceView());
    horizontal_blur_to_texture_pass->SetPassParameters(horizontalBlurParams);

    render_pipeline_.AddRenderPass(horizontal_blur_to_texture_pass);
  }

  auto vertical_blur_to_texture_pass = std::make_shared<RenderPass>(
      "RenderVerticalBlurToTexture", vertical_blur_shader);
  {
    vertical_blur_to_texture_pass->AddRenderTag(vertical_blur_tag);
    vertical_blur_to_texture_pass->AddInputTexture(
        "DownSampleTexture",
        horizontal_blur_to_texture_pass->GetOutputTexture());
    vertical_blur_to_texture_pass->SetOutputTexture(vertical_blur_target);
    vertical_blur_to_texture_pass->NeedTurnOffZBuffer();

    ShaderParameterContainer verticalBlurParams;
    vertical_blur_target->GetOrthoMatrix(orthoMatrix);
    verticalBlurParams.SetMatrix("orthoMatrix", orthoMatrix);
    verticalBlurParams.SetFloat("screenHeight",
                                static_cast<float>(SHADOW_MAP_HEIGHT / 2));
    verticalBlurParams.SetTexture(
        "texture", horizontal_blur_target->GetShaderResourceView());
    vertical_blur_to_texture_pass->SetPassParameters(verticalBlurParams);

    render_pipeline_.AddRenderPass(vertical_blur_to_texture_pass);
  }

  auto up_sample_texture_pass =
      make_shared<RenderPass>("UpSampleTexture", texture_shader);
  {
    up_sample_texture_pass->AddRenderTag(up_sample_tag);
    up_sample_texture_pass->AddInputTexture(
        "texture", vertical_blur_to_texture_pass->GetOutputTexture());
    up_sample_texture_pass->SetOutputTexture(upsample_target);
    up_sample_texture_pass->NeedTurnOffZBuffer();

    ShaderParameterContainer upsampleParams;
    upsample_target->GetOrthoMatrix(orthoMatrix);
    upsampleParams.SetMatrix("orthoMatrix", orthoMatrix);
    upsampleParams.SetTexture("texture",
                              vertical_blur_target->GetShaderResourceView());
    up_sample_texture_pass->SetPassParameters(upsampleParams);

    render_pipeline_.AddRenderPass(up_sample_texture_pass);
  }

  auto final_pass =
      std::make_shared<RenderPass>("FinalPass", soft_shadow_shader);
  {
    final_pass->AddRenderTag(final_tag);
    final_pass->AddInputTexture("shadowMap",
                                up_sample_texture_pass->GetOutputTexture());

    ShaderParameterContainer finalParams;
    finalParams.SetTexture("shadowTexture",
                           upsample_target->GetShaderResourceView());
    finalParams.SetTexture("texture",
                           upsample_target->GetShaderResourceView());
    finalParams.SetVector4("diffuseColor", light_->GetDiffuseColor());
    finalParams.SetVector4("ambientColor", light_->GetAmbientColor());
    final_pass->SetPassParameters(finalParams);

    render_pipeline_.AddRenderPass(final_pass);
  }

  auto pbr_pass = std::make_shared<RenderPass>("PBRPass", pbr_shader);
  {
    pbr_pass->AddRenderTag(pbr_tag);

    ShaderParameterContainer pbrParams;
    pbrParams.SetTexture("diffuseTexture", sphere_pbr_model->GetTexture(0));
    pbrParams.SetTexture("normalMap", sphere_pbr_model->GetTexture(1));
    pbrParams.SetTexture("rmTexture", sphere_pbr_model->GetTexture(2));
    pbr_pass->SetPassParameters(pbrParams);

    render_pipeline_.AddRenderPass(pbr_pass);
  }

  // Add renderable objects
  auto cube_object =
      std::make_shared<RenderableObject>(cube_model, soft_shadow_shader);
  cube_object->SetWorldMatrix(XMMatrixTranslation(-2.5f, 2.0f, 0.0f));
  cube_object->AddTag(write_depth_tag);
  cube_object->AddTag(write_shadow_tag);
  cube_object->AddTag(final_tag);
  cube_object->SetParameterCallback(
      [cube_model](ShaderParameterContainer &params) {
        params.SetTexture("texture", cube_model->GetTexture());
      });
  render_pipeline_.AddRenderableObject(cube_object);

  auto sphere_object =
      std::make_shared<RenderableObject>(sphere_model, soft_shadow_shader);
  sphere_object->SetWorldMatrix(XMMatrixTranslation(2.5f, 2.0f, 0.0f));
  sphere_object->AddTag(write_depth_tag);
  sphere_object->AddTag(write_shadow_tag);
  sphere_object->AddTag(final_tag);
  sphere_object->SetParameterCallback(
      [sphere_model](ShaderParameterContainer &params) {
        params.SetTexture("texture", sphere_model->GetTexture());
      });
  render_pipeline_.AddRenderableObject(sphere_object);

  auto pbr_sphere_object =
      std::make_shared<RenderableObject>(sphere_pbr_model, pbr_shader);
  pbr_sphere_object->SetWorldMatrix(XMMatrixTranslation(0.0f, 2.0f, -2.0f));
  pbr_sphere_object->AddTag(write_depth_tag);
  pbr_sphere_object->AddTag(write_shadow_tag);
  pbr_sphere_object->AddTag(pbr_tag);
  render_pipeline_.AddRenderableObject(pbr_sphere_object);

  auto ground_object =
      std::make_shared<RenderableObject>(ground_model, soft_shadow_shader);
  ground_object->SetWorldMatrix(XMMatrixTranslation(0.0f, 1.0f, 0.0f));
  ground_object->AddTag(write_depth_tag);
  ground_object->AddTag(write_shadow_tag);
  ground_object->AddTag(final_tag);
  ground_object->SetParameterCallback(
      [ground_model](ShaderParameterContainer &params) {
        params.SetTexture("texture", ground_model->GetTexture());
      });
  render_pipeline_.AddRenderableObject(ground_object);

  auto down_sample_object =
      std::make_shared<RenderableObject>(small_window, texture_shader);
  down_sample_object->AddTag(down_sample_tag);
  render_pipeline_.AddRenderableObject(down_sample_object);

  auto horizontal_blur_object = std::make_shared<RenderableObject>(
      small_window, horizontal_blur_shader);
  horizontal_blur_object->AddTag(horizontal_blur_tag);
  render_pipeline_.AddRenderableObject(horizontal_blur_object);

  auto vertical_blur_object =
      std::make_shared<RenderableObject>(small_window, vertical_blur_shader);
  vertical_blur_object->AddTag(vertical_blur_tag);
  render_pipeline_.AddRenderableObject(vertical_blur_object);

  auto up_sample_object = std::make_shared<RenderableObject>(
      fullscreen_window, texture_shader);
  up_sample_object->AddTag(up_sample_tag);
  render_pipeline_.AddRenderableObject(up_sample_object);

  for (int i = 0; i < 5; i++) {
    float xPos = -6.5f + i * 3;
    float yPos = 5.5f + i * 1;
    float zPos = -12.0f;

    auto dynamic_cube =
        std::make_shared<RenderableObject>(cube_model, soft_shadow_shader);
    dynamic_cube->SetWorldMatrix(XMMatrixTranslation(xPos, yPos, zPos) *
                                 XMMatrixScaling(0.3f, 0.3f, 0.3f));

    dynamic_cube->AddTag(write_depth_tag);
    dynamic_cube->AddTag(write_shadow_tag);
    dynamic_cube->AddTag(final_tag);

    dynamic_cube->SetParameterCallback(
        [cube_model](ShaderParameterContainer &params) {
          params.SetTexture("texture", cube_model->GetTexture());
        });

    cube_group_->AddRenderable(dynamic_cube);
  }

  for (const auto &renderable : cube_group_->GetRenderables()) {
    render_pipeline_.AddRenderableObject(renderable);
  }

  ShaderParameterContainer globalParams;
  XMMATRIX deviceWorldMatrix, projectionMatrix;
  DirectX11Device::GetD3d11DeviceInstance()->GetWorldMatrix(deviceWorldMatrix);
  DirectX11Device::GetD3d11DeviceInstance()->GetProjectionMatrix(
      projectionMatrix);
  globalParams.SetMatrix("deviceWorldMatrix", deviceWorldMatrix);
  globalParams.SetMatrix("projectionMatrix", projectionMatrix);
  render_pipeline_.SetGlobalParameters(globalParams);
}

void GraphicsClass::SetupRenderGraph() {
  cout << "\n=== Setting up RenderGraph ===" << endl;

  if (!scene_assets_.cube || !scene_assets_.sphere || !scene_assets_.ground ||
      !scene_assets_.pbr_sphere || !render_targets_.shadow_depth ||
      !shader_assets_.depth || !ortho_windows_.small_window ||
      !ortho_windows_.fullscreen_window) {
    cerr << "SetupRenderGraph: resources not initialized." << endl;
    return;
  }

  auto device = DirectX11Device::GetD3d11DeviceInstance()->GetDevice();
  auto context = DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  // Initialize RenderGraph
  render_graph_.Initialize(device, context);

  const auto &cube_model = scene_assets_.cube;
  const auto &sphere_model = scene_assets_.sphere;
  const auto &ground_model = scene_assets_.ground;
  const auto &sphere_pbr_model = scene_assets_.pbr_sphere;

  const auto &depth_shader = shader_assets_.depth;
  const auto &shadow_shader = shader_assets_.shadow;
  const auto &texture_shader = shader_assets_.texture;
  const auto &horizontal_blur_shader = shader_assets_.horizontal_blur;
  const auto &vertical_blur_shader = shader_assets_.vertical_blur;
  const auto &soft_shadow_shader = shader_assets_.soft_shadow;
  const auto &pbr_shader = shader_assets_.pbr;

  const auto &small_window = ortho_windows_.small_window;
  const auto &fullscreen_window = ortho_windows_.fullscreen_window;

  const auto &depth_tex = render_targets_.shadow_depth;
  const auto &shadow_tex = render_targets_.shadow_map;
  const auto &downsample_tex = render_targets_.downsampled_shadow;
  const auto &h_blur_tex = render_targets_.horizontal_blur;
  const auto &v_blur_tex = render_targets_.vertical_blur;
  const auto &upsample_tex = render_targets_.upsampled_shadow;

  // Import existing textures into RenderGraph
  render_graph_.ImportTexture("DepthMap", depth_tex);
  render_graph_.ImportTexture("ShadowMap", shadow_tex);
  render_graph_.ImportTexture("DownsampledShadow", downsample_tex);
  render_graph_.ImportTexture("HorizontalBlur", h_blur_tex);
  render_graph_.ImportTexture("VerticalBlur", v_blur_tex);
  render_graph_.ImportTexture("UpsampledShadow", upsample_tex);

  constexpr auto write_depth_tag = "write_depth";
  constexpr auto write_shadow_tag = "write_shadow";
  constexpr auto down_sample_tag = "down_sample";
  constexpr auto horizontal_blur_tag = "horizontal_blur";
  constexpr auto vertical_blur_tag = "vertical_blur";
  constexpr auto up_sample_tag = "up_sample";
  constexpr auto pbr_tag = "pbr_tag";
  constexpr auto final_tag = "final";

  // Pass 1: Depth Pass
  render_graph_.AddPass("DepthPass")
      .SetShader(depth_shader)
      .Write("DepthMap")
      .AddRenderTag(write_depth_tag);

  // Pass 2: Shadow Pass (standard execution, bind depth map as parameter)
  render_graph_.AddPass("ShadowPass")
      .SetShader(shadow_shader)
      .Read("DepthMap")
      .Write("ShadowMap")
      .AddRenderTag(write_shadow_tag)
      .SetTexture("depthMapTexture", depth_tex->GetShaderResourceView());

  // Pass 3: Downsample
  XMMATRIX orthoMatrix;
  downsample_tex->GetOrthoMatrix(orthoMatrix);

  render_graph_.AddPass("DownsamplePass")
      .SetShader(texture_shader)
      .Read("ShadowMap")
      .Write("DownsampledShadow")
      .AddRenderTag(down_sample_tag)
      .DisableZBuffer(true)
      .SetParameter("orthoMatrix", orthoMatrix)
      .SetTexture("texture", shadow_tex->GetShaderResourceView());

  // Pass 4: Horizontal Blur
  h_blur_tex->GetOrthoMatrix(orthoMatrix);

  render_graph_.AddPass("HorizontalBlurPass")
      .SetShader(horizontal_blur_shader)
      .Read("DownsampledShadow")
      .Write("HorizontalBlur")
      .AddRenderTag(horizontal_blur_tag)
      .DisableZBuffer(true)
      .SetParameter("orthoMatrix", orthoMatrix)
      .SetParameter("screenWidth", static_cast<float>(SHADOW_MAP_WIDTH / 2))
      .SetTexture("texture", downsample_tex->GetShaderResourceView());

  // Pass 5: Vertical Blur
  v_blur_tex->GetOrthoMatrix(orthoMatrix);

  render_graph_.AddPass("VerticalBlurPass")
      .SetShader(vertical_blur_shader)
      .Read("HorizontalBlur")
      .Write("VerticalBlur")
      .AddRenderTag(vertical_blur_tag)
      .DisableZBuffer(true)
      .SetParameter("orthoMatrix", orthoMatrix)
      .SetParameter("screenHeight", static_cast<float>(SHADOW_MAP_HEIGHT / 2))
      .SetTexture("texture", h_blur_tex->GetShaderResourceView());

  // Pass 6: Upsample
  upsample_tex->GetOrthoMatrix(orthoMatrix);

  render_graph_.AddPass("UpsamplePass")
      .SetShader(texture_shader)
      .Read("VerticalBlur")
      .Write("UpsampledShadow")
      .AddRenderTag(up_sample_tag)
      .DisableZBuffer(true)
      .SetParameter("orthoMatrix", orthoMatrix)
      .SetTexture("texture", v_blur_tex->GetShaderResourceView());

  // Pass 7: Final Pass (soft shadow)
  render_graph_.AddPass("FinalPass")
      .SetShader(soft_shadow_shader)
      .Read("UpsampledShadow")
      .AddRenderTag(final_tag)
      .SetTexture("shadowTexture", upsample_tex->GetShaderResourceView())
      .SetParameter("diffuseColor", light_->GetDiffuseColor())
      .SetParameter("ambientColor", light_->GetAmbientColor());

  // Pass 8: PBR Pass (standard execution)
  render_graph_.AddPass("PBRPass")
      .SetShader(pbr_shader)
      .AddRenderTag(pbr_tag)
      .SetTexture("diffuseTexture", sphere_pbr_model->GetTexture(0))
      .SetTexture("normalMap", sphere_pbr_model->GetTexture(1))
      .SetTexture("rmTexture", sphere_pbr_model->GetTexture(2));

  // Add renderable objects
  auto cube_object =
      std::make_shared<RenderableObject>(cube_model, soft_shadow_shader);
  cube_object->SetWorldMatrix(XMMatrixTranslation(-2.5f, 2.0f, 0.0f));
  cube_object->AddTag(write_depth_tag);
  cube_object->AddTag(write_shadow_tag);
  cube_object->AddTag(final_tag);
  cube_object->SetParameterCallback(
      [cube_model](ShaderParameterContainer &params) {
        params.SetTexture("texture", cube_model->GetTexture());
      });
  renderable_objects_.push_back(cube_object);

  auto sphere_object =
      std::make_shared<RenderableObject>(sphere_model, soft_shadow_shader);
  sphere_object->SetWorldMatrix(XMMatrixTranslation(2.5f, 2.0f, 0.0f));
  sphere_object->AddTag(write_depth_tag);
  sphere_object->AddTag(write_shadow_tag);
  sphere_object->AddTag(final_tag);
  sphere_object->SetParameterCallback(
      [sphere_model](ShaderParameterContainer &params) {
        params.SetTexture("texture", sphere_model->GetTexture());
      });
  renderable_objects_.push_back(sphere_object);

  auto pbr_sphere_object =
      std::make_shared<RenderableObject>(sphere_pbr_model, pbr_shader);
  pbr_sphere_object->SetWorldMatrix(XMMatrixTranslation(0.0f, 2.0f, -2.0f));
  pbr_sphere_object->AddTag(write_depth_tag);
  pbr_sphere_object->AddTag(write_shadow_tag);
  pbr_sphere_object->AddTag(pbr_tag);
  renderable_objects_.push_back(pbr_sphere_object);

  auto down_sample_object =
      std::make_shared<RenderableObject>(small_window, texture_shader);
  down_sample_object->AddTag(down_sample_tag);
  down_sample_object->SetParameterCallback(
      [shadow_tex](ShaderParameterContainer &p) {
        p.SetTexture("texture", shadow_tex->GetShaderResourceView());
      });
  renderable_objects_.push_back(down_sample_object);

  auto horizontal_blur_object = std::make_shared<RenderableObject>(
      small_window, horizontal_blur_shader);
  horizontal_blur_object->AddTag(horizontal_blur_tag);
  horizontal_blur_object->SetParameterCallback(
      [downsample_tex](ShaderParameterContainer &p) {
        p.SetTexture("texture", downsample_tex->GetShaderResourceView());
      });
  renderable_objects_.push_back(horizontal_blur_object);

  auto vertical_blur_object = std::make_shared<RenderableObject>(
      small_window, vertical_blur_shader);
  vertical_blur_object->AddTag(vertical_blur_tag);
  vertical_blur_object->SetParameterCallback(
      [h_blur_tex](ShaderParameterContainer &p) {
        p.SetTexture("texture", h_blur_tex->GetShaderResourceView());
      });
  renderable_objects_.push_back(vertical_blur_object);

  auto up_sample_object = std::make_shared<RenderableObject>(
      fullscreen_window, texture_shader);
  up_sample_object->AddTag(up_sample_tag);
  up_sample_object->SetParameterCallback(
      [v_blur_tex](ShaderParameterContainer &p) {
        p.SetTexture("texture", v_blur_tex->GetShaderResourceView());
      });
  renderable_objects_.push_back(up_sample_object);

  auto ground_object =
      std::make_shared<RenderableObject>(ground_model, soft_shadow_shader);
  ground_object->SetWorldMatrix(XMMatrixTranslation(0.0f, 1.0f, 0.0f));
  ground_object->AddTag(write_depth_tag);
  ground_object->AddTag(write_shadow_tag);
  ground_object->AddTag(final_tag);
  ground_object->SetParameterCallback(
      [ground_model](ShaderParameterContainer &params) {
        params.SetTexture("texture", ground_model->GetTexture());
      });
  renderable_objects_.push_back(ground_object);

  for (int i = 0; i < 5; i++) {
    float xPos = -6.5f + i * 3;
    float yPos = 5.5f + i * 1;
    float zPos = -12.0f;

    auto cube_obj =
        std::make_shared<RenderableObject>(cube_model, soft_shadow_shader);
    cube_obj->SetWorldMatrix(XMMatrixTranslation(xPos, yPos, zPos) *
                             XMMatrixScaling(0.3f, 0.3f, 0.3f));
    cube_obj->AddTag(write_depth_tag);
    cube_obj->AddTag(write_shadow_tag);
    cube_obj->AddTag(final_tag);
    cube_obj->SetParameterCallback(
        [cube_model](ShaderParameterContainer &params) {
          params.SetTexture("texture", cube_model->GetTexture());
        });

    cube_group_->AddRenderable(cube_obj);
    renderable_objects_.push_back(cube_obj);
  }

  if (!render_graph_.Compile()) {
    cerr << "Failed to compile RenderGraph!" << endl;
  }

  cout << "=== RenderGraph Setup Complete ===\n" << endl;

  render_graph_.PrintGraph();
}

void GraphicsClass::Render() {

  auto directx_device_ = DirectX11Device::GetD3d11DeviceInstance();

  // Update the view matrix based on the camera's position.
  camera_->Render();
  XMMATRIX viewMatrix, baseViewMatrix;
  camera_->GetViewMatrix(viewMatrix);
  camera_->GetBaseViewMatrix(baseViewMatrix);

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

  // Add device matrices
  XMMATRIX deviceWorldMatrix, projectionMatrix;
  directx_device_->GetWorldMatrix(deviceWorldMatrix);
  directx_device_->GetProjectionMatrix(projectionMatrix);
  Params.SetMatrix("deviceWorldMatrix", deviceWorldMatrix);
  Params.SetMatrix("projectionMatrix", projectionMatrix);

  if (use_render_graph_) {
    render_graph_.Execute(renderable_objects_, Params);
  } else {
    render_pipeline_.Execute(Params);
  }
}