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

  auto directx11_device_ = DirectX11Device::GetD3d11DeviceInstance();

  if (!(directx11_device_->Initialize(screenWidth, screenHeight, VSYNC_ENABLED,
                                      hwnd, FULL_SCREEN, SCREEN_DEPTH,
                                      SCREEN_NEAR))) {
    MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
    return false;
  }

  // Initialize ResourceManager
  auto &rm = ResourceManager::GetInstance();
  auto device = directx11_device_->GetDevice();
  auto device_context = directx11_device_->GetDeviceContext();

  if (!rm.Initialize(device, device_context, hwnd)) {
    MessageBox(hwnd, L"Could not initialize ResourceManager.", L"Error", MB_OK);
    return false;
  }

  // Initialize camera
  {
    camera_ = make_unique<Camera>();
    if (nullptr == camera_) {
      return false;
    }
    camera_->SetPosition(0.0f, -1.0f, -10.0f);
    camera_->Render();
    camera_->RenderBaseViewMatrix();
  }

  render_pipeline_.Initialize(device, device_context);

  // Load models through ResourceManager
  auto cube_model =
      rm.GetModel("cube", "./data/cube.txt", L"./data/wall01.dds");
  auto sphere_model =
      rm.GetModel("sphere", "./data/sphere.txt", L"./data/ice.dds");
  auto ground_model =
      rm.GetModel("ground", "./data/plane01.txt", L"./data/metal001.dds");

  if (!cube_model || !sphere_model || !ground_model) {
    std::wstring error_msg = L"Could not load models.\n";
    error_msg +=
        std::wstring(rm.GetLastError().begin(), rm.GetLastError().end());
    MessageBox(hwnd, error_msg.c_str(), L"Resource Error", MB_OK);
    return false;
  }

  // Load PBR model
  auto sphere_pbr_model =
      rm.GetPBRModel("sphere_pbr", "./data/sphere.txt", "./data/pbr_albedo.tga",
                     "./data/pbr_normal.tga", "./data/pbr_roughmetal.tga");

  if (!sphere_pbr_model) {
    std::wstring error_msg = L"Could not load PBR model.\n";
    error_msg +=
        std::wstring(rm.GetLastError().begin(), rm.GetLastError().end());
    MessageBox(hwnd, error_msg.c_str(), L"Resource Error", MB_OK);
    return false;
  }

  // Initialize light
  {
    light_ = make_unique<Light>();
    if (nullptr == light_) {
      return false;
    }

    light_->SetAmbientColor(0.15f, 0.15f, 0.15f, 1.0f);
    light_->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
    light_->SetLookAt(0.0f, 0.0f, 0.0f);
    light_->GenerateProjectionMatrix(SCREEN_DEPTH, SCREEN_NEAR);
  }

  // Create RenderTextures through ResourceManager
  auto render_texture =
      rm.CreateRenderTexture("shadow_depth", SHADOW_MAP_WIDTH,
                             SHADOW_MAP_HEIGHT, SCREEN_DEPTH, SCREEN_NEAR);

  auto blackwhiter_render_texture =
      rm.CreateRenderTexture("shadow_map", SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT,
                             SCREEN_DEPTH, SCREEN_NEAR);

  auto downsample_texture = rm.CreateRenderTexture(
      "downsample", downSampleWidth, downSampleHeight, 100.0f, 1.0f);

  auto horizontal_blur_texture = rm.CreateRenderTexture(
      "horizontal_blur", downSampleWidth, downSampleHeight, SCREEN_DEPTH, 0.1f);

  auto vertical_blur_texture = rm.CreateRenderTexture(
      "vertical_blur", downSampleWidth, downSampleHeight, SCREEN_DEPTH, 0.1f);

  auto upsample_texture = rm.CreateRenderTexture(
      "upsample", SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, SCREEN_DEPTH, 0.1f);

  if (!render_texture || !blackwhiter_render_texture || !downsample_texture ||
      !horizontal_blur_texture || !vertical_blur_texture || !upsample_texture) {
    MessageBox(hwnd, L"Could not create render textures.", L"Error", MB_OK);
    return false;
  }

  // Load shaders through ResourceManager
  auto depth_shader = rm.GetShader<DepthShader>("depth");
  auto shadow_shader = rm.GetShader<ShadowShader>("shadow");
  auto texture_shader = rm.GetShader<TextureShader>("texture");
  auto horizontal_blur_shader =
      rm.GetShader<HorizontalBlurShader>("horizontal_blur");
  auto vertical_blur_shader = rm.GetShader<VerticalBlurShader>("vertical_blur");
  auto soft_shadow_shader = rm.GetShader<SoftShadowShader>("soft_shadow");
  auto pbr_shader = rm.GetShader<PbrShader>("pbr");

  if (!depth_shader || !shadow_shader || !texture_shader ||
      !horizontal_blur_shader || !vertical_blur_shader || !soft_shadow_shader ||
      !pbr_shader) {
    MessageBox(hwnd, L"Could not load shaders.", L"Error", MB_OK);
    return false;
  }

  // Create ortho windows
  auto small_window =
      rm.GetOrthoWindow("small_window", downSampleWidth, downSampleHeight);
  auto fullscreen_window = rm.GetOrthoWindow(
      "fullscreen_window", SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);

  if (!small_window || !fullscreen_window) {
    MessageBox(hwnd, L"Could not create ortho windows.", L"Error", MB_OK);
    return false;
  }

  cube_group_ = make_shared<StandardRenderGroup>();

  // Setup rendering system
  if (use_render_graph_) {
    SetupRenderGraph();
  } else {
    SetupRenderPipeline();
  }

  // Log resource statistics
  rm.LogResourceStats();

  return true;
}

void GraphicsClass::Shutdown() {
  auto &rm = ResourceManager::GetInstance();

  // Display resource usage before shutdown
  cout << "\n=== Resource Usage Before Shutdown ===" << endl;
  cout << "Total cached resources: " << rm.GetTotalCachedResources() << endl;

  // Check for unused resources
  auto unusedModels = rm.GetUnusedModels();
  auto unusedShaders = rm.GetUnusedShaders();
  auto unusedRTTs = rm.GetUnusedRenderTextures();

  cout << "Unused models: " << unusedModels.size() << endl;
  for (const auto &name : unusedModels) {
    cout << "  - " << name << " (ref count: " << rm.GetModelRefCount(name)
         << ")" << endl;
  }

  cout << "Unused shaders: " << unusedShaders.size() << endl;
  for (const auto &name : unusedShaders) {
    cout << "  - " << name << " (ref count: " << rm.GetShaderRefCount(name)
         << ")" << endl;
  }

  cout << "Unused render textures: " << unusedRTTs.size() << endl;
  for (const auto &name : unusedRTTs) {
    cout << "  - " << name
         << " (ref count: " << rm.GetRenderTextureRefCount(name) << ")" << endl;
  }

  // Optionally prune unused resources
  // size_t pruned = rm.PruneAllUnusedResources();

  cout << "========================================\n" << endl;

  // Shutdown ResourceManager
  rm.Shutdown();
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
    auto &rm = ResourceManager::GetInstance();
    cout << "\n[DEBUG] Resource Usage:" << endl;
    cout << "  Cube model ref count: " << rm.GetModelRefCount("cube") << endl;
    cout << "  Sphere model ref count: " << rm.GetModelRefCount("sphere")
         << endl;
    cout << "  Ground model ref count: " << rm.GetModelRefCount("ground")
         << endl;
    cout << "  Total cached: " << rm.GetTotalCachedResources() << endl;
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

  auto &rm = ResourceManager::GetInstance();

  // Get resources from ResourceManager
  auto cube_model =
      rm.GetModel("cube", "./data/cube.txt", L"./data/wall01.dds");
  auto sphere_model =
      rm.GetModel("sphere", "./data/sphere.txt", L"./data/ice.dds");
  auto ground_model =
      rm.GetModel("ground", "./data/plane01.txt", L"./data/metal001.dds");
  auto sphere_pbr_model =
      rm.GetPBRModel("sphere_pbr", "./data/sphere.txt", "./data/pbr_albedo.tga",
                     "./data/pbr_normal.tga", "./data/pbr_roughmetal.tga");

  auto render_texture = rm.GetRenderTexture("shadow_depth");
  auto blackwhiter_render_texture = rm.GetRenderTexture("shadow_map");
  auto downsample_texture = rm.GetRenderTexture("downsample");
  auto horizontal_blur_texture = rm.GetRenderTexture("horizontal_blur");
  auto vertical_blur_texture = rm.GetRenderTexture("vertical_blur");
  auto upsample_texture = rm.GetRenderTexture("upsample");

  auto depth_shader = rm.GetShader<DepthShader>("depth");
  auto shadow_shader = rm.GetShader<ShadowShader>("shadow");
  auto texture_shader = rm.GetShader<TextureShader>("texture");
  auto horizontal_blur_shader =
      rm.GetShader<HorizontalBlurShader>("horizontal_blur");
  auto vertical_blur_shader = rm.GetShader<VerticalBlurShader>("vertical_blur");
  auto soft_shadow_shader = rm.GetShader<SoftShadowShader>("soft_shadow");
  auto pbr_shader = rm.GetShader<PbrShader>("pbr");

  auto small_window =
      rm.GetOrthoWindow("small_window", downSampleWidth, downSampleHeight);
  auto fullscreen_window = rm.GetOrthoWindow(
      "fullscreen_window", SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);

  auto depth_pass = std::make_shared<RenderPass>("DepthPass", depth_shader);
  {
    depth_pass->AddRenderTag(write_depth_tag);
    depth_pass->SetOutputTexture(render_texture);

    render_pipeline_.AddRenderPass(depth_pass);
  }

  auto shadow_pass = std::make_shared<RenderPass>("ShadowPass", shadow_shader);
  {
    shadow_pass->AddRenderTag(write_shadow_tag);
    shadow_pass->AddInputTexture("depthMap", depth_pass->GetOutputTexture());
    shadow_pass->SetOutputTexture(blackwhiter_render_texture);

    // required parameter by shadow shader
    ShaderParameterContainer shadowPassParams;
    shadowPassParams.SetTexture("depthMapTexture",
                                render_texture->GetShaderResourceView());
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
    down_sample_texture_pass->SetOutputTexture(downsample_texture);
    down_sample_texture_pass->NeedTurnOffZBuffer();

    ShaderParameterContainer DownSampleTexturePassParams;
    downsample_texture->GetOrthoMatrix(orthoMatrix);
    DownSampleTexturePassParams.SetMatrix("orthoMatrix", orthoMatrix);
    DownSampleTexturePassParams.SetTexture(
        "texture", blackwhiter_render_texture->GetShaderResourceView());
    down_sample_texture_pass->SetPassParameters(DownSampleTexturePassParams);

    render_pipeline_.AddRenderPass(down_sample_texture_pass);
  }

  auto horizontal_blur_to_texture_pass = std::make_shared<RenderPass>(
      "RenderHorizontalBlurToTexture", horizontal_blur_shader);
  {
    horizontal_blur_to_texture_pass->AddRenderTag(horizontal_blur_tag);
    horizontal_blur_to_texture_pass->AddInputTexture(
        "DownSampleTexture", down_sample_texture_pass->GetOutputTexture());
    horizontal_blur_to_texture_pass->SetOutputTexture(horizontal_blur_texture);
    horizontal_blur_to_texture_pass->NeedTurnOffZBuffer();

    ShaderParameterContainer RenderHorizontalBlurToTexturePassParams;
    horizontal_blur_texture->GetOrthoMatrix(orthoMatrix);
    RenderHorizontalBlurToTexturePassParams.SetMatrix("orthoMatrix",
                                                      orthoMatrix);
    RenderHorizontalBlurToTexturePassParams.SetFloat(
        "screenWidth", (float)(SHADOW_MAP_WIDTH / 2));
    RenderHorizontalBlurToTexturePassParams.SetTexture(
        "texture", downsample_texture->GetShaderResourceView());
    horizontal_blur_to_texture_pass->SetPassParameters(
        RenderHorizontalBlurToTexturePassParams);

    render_pipeline_.AddRenderPass(horizontal_blur_to_texture_pass);
  }

  auto Vertical_blur_to_texture_pass = std::make_shared<RenderPass>(
      "RenderHorizontalBlurToTexture", vertical_blur_shader);
  {
    Vertical_blur_to_texture_pass->AddRenderTag(vertical_blur_tag);
    Vertical_blur_to_texture_pass->AddInputTexture(
        "DownSampleTexture",
        horizontal_blur_to_texture_pass->GetOutputTexture());
    Vertical_blur_to_texture_pass->SetOutputTexture(vertical_blur_texture);
    Vertical_blur_to_texture_pass->NeedTurnOffZBuffer();

    ShaderParameterContainer RenderVerticalBlurToTexturePassParams;
    vertical_blur_texture->GetOrthoMatrix(orthoMatrix);
    RenderVerticalBlurToTexturePassParams.SetMatrix("orthoMatrix", orthoMatrix);
    RenderVerticalBlurToTexturePassParams.SetFloat(
        "screenHeight", (float)(SHADOW_MAP_HEIGHT / 2));
    RenderVerticalBlurToTexturePassParams.SetTexture(
        "texture", horizontal_blur_texture->GetShaderResourceView());
    Vertical_blur_to_texture_pass->SetPassParameters(
        RenderVerticalBlurToTexturePassParams);

    render_pipeline_.AddRenderPass(Vertical_blur_to_texture_pass);
  }

  auto up_sample_texture_pass =
      make_shared<RenderPass>("UpSampleTexture", texture_shader);
  {
    up_sample_texture_pass->AddRenderTag(up_sample_tag);
    up_sample_texture_pass->AddInputTexture(
        "texture", Vertical_blur_to_texture_pass->GetOutputTexture());
    up_sample_texture_pass->SetOutputTexture(upsample_texture);
    up_sample_texture_pass->NeedTurnOffZBuffer();

    ShaderParameterContainer UpSampleTexturePassParams;
    upsample_texture->GetOrthoMatrix(orthoMatrix);
    UpSampleTexturePassParams.SetMatrix("orthoMatrix", orthoMatrix);
    UpSampleTexturePassParams.SetTexture(
        "texture", vertical_blur_texture->GetShaderResourceView());
    up_sample_texture_pass->SetPassParameters(UpSampleTexturePassParams);

    render_pipeline_.AddRenderPass(up_sample_texture_pass);
  }

  auto final_pass =
      std::make_shared<RenderPass>("FinalPass", soft_shadow_shader);
  {
    final_pass->AddRenderTag(final_tag);
    final_pass->AddInputTexture("shadowMap",
                                up_sample_texture_pass->GetOutputTexture());

    ShaderParameterContainer FinalPassParams;
    FinalPassParams.SetTexture("shadowTexture",
                               upsample_texture->GetShaderResourceView());
    FinalPassParams.SetTexture("texture",
                               upsample_texture->GetShaderResourceView());
    FinalPassParams.SetVector4("diffuseColor", light_->GetDiffuseColor());
    FinalPassParams.SetVector4("ambientColor", light_->GetAmbientColor());
    final_pass->SetPassParameters(FinalPassParams);

    render_pipeline_.AddRenderPass(final_pass);
  }

  auto pbr_pass = std::make_shared<RenderPass>("PBRPass", pbr_shader);
  {
    pbr_pass->AddRenderTag(pbr_tag);

    ShaderParameterContainer PBRPassParams;
    PBRPassParams.SetTexture("diffuseTexture", sphere_pbr_model->GetTexture(0));
    PBRPassParams.SetTexture("normalMap", sphere_pbr_model->GetTexture(1));
    PBRPassParams.SetTexture("rmTexture", sphere_pbr_model->GetTexture(2));
    pbr_pass->SetPassParameters(PBRPassParams);

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

  auto horizontal_blur_object =
      std::make_shared<RenderableObject>(small_window, horizontal_blur_shader);
  horizontal_blur_object->AddTag(horizontal_blur_tag);
  render_pipeline_.AddRenderableObject(horizontal_blur_object);

  auto vertical_blur_object =
      std::make_shared<RenderableObject>(small_window, vertical_blur_shader);
  vertical_blur_object->AddTag(vertical_blur_tag);
  render_pipeline_.AddRenderableObject(vertical_blur_object);

  auto up_sample_object =
      std::make_shared<RenderableObject>(fullscreen_window, texture_shader);
  up_sample_object->AddTag(up_sample_tag);
  render_pipeline_.AddRenderableObject(up_sample_object);

  cube_group_ = make_shared<StandardRenderGroup>();

  for (int i = 0; i < 5; i++) {

    float xPos = -6.5f + i * 3;
    float yPos = 5.5f + i * 1;
    float zPos = -12.0f;

    auto cube_object =
        std::make_shared<RenderableObject>(cube_model, soft_shadow_shader);
    cube_object->SetWorldMatrix(XMMatrixTranslation(xPos, yPos, zPos) *
                                XMMatrixScaling(0.3f, 0.3f, 0.3f));

    cube_object->AddTag(write_depth_tag);
    cube_object->AddTag(write_shadow_tag);
    cube_object->AddTag(final_tag);

    cube_object->SetParameterCallback(
        [cube_model](ShaderParameterContainer &params) {
          params.SetTexture("texture", cube_model->GetTexture());
        });

    cube_group_->AddRenderable(cube_object);
  }

  for (const auto &renderable : cube_group_->GetRenderables()) {
    render_pipeline_.AddRenderableObject(renderable);
  }

  // Set global static parameters
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

  auto &rm = ResourceManager::GetInstance();
  auto device = DirectX11Device::GetD3d11DeviceInstance()->GetDevice();
  auto context = DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  // Initialize RenderGraph
  render_graph_.Initialize(device, context);

  // Get resources from ResourceManager
  auto cube_model =
      rm.GetModel("cube", "./data/cube.txt", L"./data/wall01.dds");
  auto sphere_model =
      rm.GetModel("sphere", "./data/sphere.txt", L"./data/ice.dds");
  auto ground_model =
      rm.GetModel("ground", "./data/plane01.txt", L"./data/metal001.dds");
  auto sphere_pbr_model =
      rm.GetPBRModel("sphere_pbr", "./data/sphere.txt", "./data/pbr_albedo.tga",
                     "./data/pbr_normal.tga", "./data/pbr_roughmetal.tga");

  auto depth_shader = rm.GetShader<DepthShader>("depth");
  auto shadow_shader = rm.GetShader<ShadowShader>("shadow");
  auto texture_shader = rm.GetShader<TextureShader>("texture");
  auto horizontal_blur_shader =
      rm.GetShader<HorizontalBlurShader>("horizontal_blur");
  auto vertical_blur_shader = rm.GetShader<VerticalBlurShader>("vertical_blur");
  auto soft_shadow_shader = rm.GetShader<SoftShadowShader>("soft_shadow");
  auto pbr_shader = rm.GetShader<PbrShader>("pbr");

  auto small_window =
      rm.GetOrthoWindow("small_window", downSampleWidth, downSampleHeight);
  auto fullscreen_window = rm.GetOrthoWindow(
      "fullscreen_window", SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);

  // Get or create RenderTextures from ResourceManager
  // These are the actual textures that will be used
  auto depth_tex = rm.GetRenderTexture("shadow_depth");
  auto shadow_tex = rm.GetRenderTexture("shadow_map");
  auto downsample_tex = rm.GetRenderTexture("downsample");
  auto h_blur_tex = rm.GetRenderTexture("horizontal_blur");
  auto v_blur_tex = rm.GetRenderTexture("vertical_blur");
  auto upsample_tex = rm.GetRenderTexture("upsample");

  if (!depth_tex || !shadow_tex || !downsample_tex || !h_blur_tex ||
      !v_blur_tex || !upsample_tex) {
    cerr << "Error: Failed to get render textures from ResourceManager" << endl;
    return;
  }

  // Import existing textures into RenderGraph
  render_graph_.ImportTexture("DepthMap", depth_tex);
  render_graph_.ImportTexture("ShadowMap", shadow_tex);
  render_graph_.ImportTexture("DownsampledShadow", downsample_tex);
  render_graph_.ImportTexture("HorizontalBlur", h_blur_tex);
  render_graph_.ImportTexture("VerticalBlur", v_blur_tex);
  render_graph_.ImportTexture("UpsampledShadow", upsample_tex);

  // Tags
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
      .SetParameter("screenWidth", (float)(SHADOW_MAP_WIDTH / 2))
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
      .SetParameter("screenHeight", (float)(SHADOW_MAP_HEIGHT / 2))
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
  // Pass 7: Final Pass (standard execution)
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

  auto pbr_sphere_object = std::make_shared<RenderableObject>(sphere_pbr_model, pbr_shader);
  pbr_sphere_object->SetWorldMatrix(XMMatrixTranslation(0.0f, 2.0f, -2.0f));
  pbr_sphere_object->AddTag(write_depth_tag);
  pbr_sphere_object->AddTag(write_shadow_tag);
  pbr_sphere_object->AddTag(pbr_tag);
  renderable_objects_.push_back(pbr_sphere_object);

  // Fullscreen quad / small quad renderables for post-process passes (rendered via standard tag filtering)
  auto down_sample_object = std::make_shared<RenderableObject>(small_window, texture_shader);
  down_sample_object->AddTag(down_sample_tag);
  down_sample_object->SetParameterCallback([shadow_tex](ShaderParameterContainer &p){ p.SetTexture("texture", shadow_tex->GetShaderResourceView()); });
  renderable_objects_.push_back(down_sample_object);

  auto horizontal_blur_object = std::make_shared<RenderableObject>(small_window, horizontal_blur_shader);
  horizontal_blur_object->AddTag(horizontal_blur_tag);
  horizontal_blur_object->SetParameterCallback([downsample_tex](ShaderParameterContainer &p){ p.SetTexture("texture", downsample_tex->GetShaderResourceView()); });
  renderable_objects_.push_back(horizontal_blur_object);

  auto vertical_blur_object = std::make_shared<RenderableObject>(small_window, vertical_blur_shader);
  vertical_blur_object->AddTag(vertical_blur_tag);
  vertical_blur_object->SetParameterCallback([h_blur_tex](ShaderParameterContainer &p){ p.SetTexture("texture", h_blur_tex->GetShaderResourceView()); });
  renderable_objects_.push_back(vertical_blur_object);

  auto up_sample_object = std::make_shared<RenderableObject>(fullscreen_window, texture_shader);
  up_sample_object->AddTag(up_sample_tag);
  up_sample_object->SetParameterCallback([v_blur_tex](ShaderParameterContainer &p){ p.SetTexture("texture", v_blur_tex->GetShaderResourceView()); });
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

  // Add dynamic cubes
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

  // Compile the graph
  if (!render_graph_.Compile()) {
    cerr << "Failed to compile RenderGraph!" << endl;
  }

  cout << "=== RenderGraph Setup Complete ===\n" << endl;
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