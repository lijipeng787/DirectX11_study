#include "Graphics.h"

#include "../CommonFramework/Camera.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/TypeDefine.h"

#include "RenderableObject.h"
#include "depthshaderclass.h"
#include "horizontalblurshaderclass.h"
#include "lightclass.h"
#include "modelclass.h"
#include "orthowindowclass.h"
#include "rendertextureclass.h"
#include "shadowshaderclass.h"
#include "softshadowshaderclass.h"
#include "textureshaderclass.h"
#include "verticalblurshaderclass.h"

using namespace std;
using namespace DirectX;

bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd) {

  auto directx11_device_ = DirectX11Device::GetD3d11DeviceInstance();

  if (!(directx11_device_->Initialize(screenWidth, screenHeight, VSYNC_ENABLED,
                                      hwnd, FULL_SCREEN, SCREEN_DEPTH,
                                      SCREEN_NEAR))) {
    MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
    return false;
  }

  {
    camera_ = make_shared<Camera>();
    if (!camera_) {
      return false;
    }
    camera_->SetPosition(0.0f, -1.0f, -10.0f);
    camera_->Render();
    camera_->RenderBaseViewMatrix();
  }

  auto result = false;

  {
    // Create the cube model object.
    m_CubeModel = make_shared<ModelClass>();
    if (!m_CubeModel) {
      return false;
    }

    // Initialize the cube model object.
    result = m_CubeModel->Initialize("./data/cube.txt", L"./data/wall01.dds");
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the cube model object.", L"Error",
                 MB_OK);
      return false;
    }
    // Set the position for the cube model.
    m_CubeModel->SetWorldMatrix(
        std::move(XMMatrixTranslation(-2.0f, 2.0f, 0.0f)));
  }

  {
    m_SphereModel = make_shared<ModelClass>();
    if (!m_SphereModel) {
      return false;
    }

    result = m_SphereModel->Initialize("./data/sphere.txt", L"./data/ice.dds");
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the sphere model object.",
                 L"Error", MB_OK);
      return false;
    }

    m_SphereModel->SetWorldMatrix(
        std::move(XMMatrixTranslation(2.0f, 2.0f, 0.0f)));
  }

  {
    m_GroundModel = make_shared<ModelClass>();
    if (!m_GroundModel) {
      return false;
    }

    result =
        m_GroundModel->Initialize("./data/plane01.txt", L"./data/metal001.dds");
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the ground model object.",
                 L"Error", MB_OK);
      return false;
    }

    m_GroundModel->SetWorldMatrix(
        std::move(XMMatrixTranslation(0.0f, 1.0f, 0.0f)));
  }

  {
    // Create the light object.
    light_ = make_shared<LightClass>();
    if (!light_) {
      return false;
    }

    // Initialize the light object.
    light_->SetAmbientColor(0.15f, 0.15f, 0.15f, 1.0f);
    light_->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
    light_->SetLookAt(0.0f, 0.0f, 0.0f);
    light_->GenerateProjectionMatrix(SCREEN_DEPTH, SCREEN_NEAR);
  }

  {
    // Create the render to texture object.
    render_texture_ = make_shared<RenderTextureClass>();
    if (!render_texture_) {
      return false;
    }

    // Initialize the render to texture object.
    result = render_texture_->Initialize(SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT,
                                         SCREEN_DEPTH, SCREEN_NEAR);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the render to texture object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  {
    depth_shader_ = make_shared<DepthShaderClass>();
    if (!depth_shader_) {
      return false;
    }

    result = depth_shader_->Initialize(hwnd);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the depth shader object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  {
    m_BlackWhiteRenderTexture = make_shared<RenderTextureClass>();
    if (!m_BlackWhiteRenderTexture) {
      return false;
    }

    result = m_BlackWhiteRenderTexture->Initialize(
        SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, SCREEN_DEPTH, SCREEN_NEAR);
    if (!result) {
      MessageBox(
          hwnd,
          L"Could not initialize the black and white render to texture object.",
          L"Error", MB_OK);
      return false;
    }
  }

  {
    // Create the shadow shader object.
    m_ShadowShader = make_shared<ShadowShaderClass>();
    if (!m_ShadowShader) {
      return false;
    }

    // Initialize the shadow shader object.
    result = m_ShadowShader->Initialize(hwnd);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the shadow shader object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  // Set the size to sample down to.
  auto downSampleWidth = SHADOWMAP_WIDTH / 2;
  auto downSampleHeight = SHADOWMAP_HEIGHT / 2;

  {
    // Create the down sample render to texture object.
    m_DownSampleTexure = make_shared<RenderTextureClass>();
    if (!m_DownSampleTexure) {
      return false;
    }

    // Initialize the down sample render to texture object.
    result = m_DownSampleTexure->Initialize(downSampleWidth, downSampleHeight,
                                            100.0f, 1.0f);
    if (!result) {
      MessageBox(
          hwnd,
          L"Could not initialize the down sample render to texture object.",
          L"Error", MB_OK);
      return false;
    }
  }

  {
    m_SmallWindow = make_shared<OrthoWindowClass>();
    if (!m_SmallWindow) {
      return false;
    }

    result = m_SmallWindow->Initialize(downSampleWidth, downSampleHeight);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the small ortho window object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  {
    texture_shader_ = make_shared<TextureShaderClass>();
    if (!texture_shader_) {
      return false;
    }

    result = texture_shader_->Initialize(hwnd);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the texture shader object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  {
    m_HorizontalBlurTexture = make_shared<RenderTextureClass>();
    if (!m_HorizontalBlurTexture) {
      return false;
    }

    result = m_HorizontalBlurTexture->Initialize(
        downSampleWidth, downSampleHeight, SCREEN_DEPTH, 0.1f);
    if (!result) {
      MessageBox(
          hwnd,
          L"Could not initialize the horizontal blur render to texture object.",
          L"Error", MB_OK);
      return false;
    }
  }

  {
    m_HorizontalBlurShader = make_shared<HorizontalBlurShaderClass>();
    if (!m_HorizontalBlurShader) {
      return false;
    }

    result = m_HorizontalBlurShader->Initialize(hwnd);
    if (!result) {
      MessageBox(hwnd,
                 L"Could not initialize the horizontal blur shader object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  {
    m_VerticalBlurTexture = make_shared<RenderTextureClass>();
    if (!m_VerticalBlurTexture) {
      return false;
    }

    result = m_VerticalBlurTexture->Initialize(
        downSampleWidth, downSampleHeight, SCREEN_DEPTH, 0.1f);
    if (!result) {
      MessageBox(
          hwnd,
          L"Could not initialize the vertical blur render to texture object.",
          L"Error", MB_OK);
      return false;
    }
  }

  {
    m_VerticalBlurShader = make_shared<VerticalBlurShaderClass>();
    if (!m_VerticalBlurShader) {
      return false;
    }

    result = m_VerticalBlurShader->Initialize(hwnd);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the vertical blur shader object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  {
    m_UpSampleTexure = make_shared<RenderTextureClass>();
    if (!m_UpSampleTexure) {
      return false;
    }

    result = m_UpSampleTexure->Initialize(SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT,
                                          SCREEN_DEPTH, 0.1f);
    if (!result) {
      MessageBox(
          hwnd, L"Could not initialize the up sample render to texture object.",
          L"Error", MB_OK);
      return false;
    }
  }

  {
    m_FullScreenWindow = make_shared<OrthoWindowClass>();
    if (!m_FullScreenWindow) {
      return false;
    }

    result = m_FullScreenWindow->Initialize(SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT);
    if (!result) {
      MessageBox(hwnd,
                 L"Could not initialize the full screen ortho window object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  {
    m_SoftShadowShader = make_shared<SoftShadowShaderClass>();
    if (!m_SoftShadowShader) {
      return false;
    }

    result = m_SoftShadowShader->Initialize(hwnd);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the soft shadow shader object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  SetupRenderPipeline();

  return true;
}

void GraphicsClass::Shutdown() {}

void GraphicsClass::Frame(float deltatime) {

  static float lightPositionX = -5.0f;

  camera_->SetPosition(pos_x_, pos_y_, pos_z_);
  camera_->SetRotation(rot_x_, rot_y_, rot_z_);

  // Update the position of the light each frame.
  lightPositionX += 0.05f;
  if (lightPositionX > 5.0f) {
    lightPositionX = -5.0f;
  }

  // Update the position of the light.
  light_->SetPosition(lightPositionX, 8.0f, -5.0f);

  // Render the graphics scene.
  Render();
}

void GraphicsClass::SetupRenderPipeline() {

  constexpr auto write_depth_tag = "write_depth";
  constexpr auto write_shadow_tag = "write_shadow";
  constexpr auto down_sample_tag = "down_sample";
  constexpr auto horizontal_blur_tag = "horizontal_blur";
  constexpr auto vertical_blur_tag = "vertical_blur";
  constexpr auto up_sample_tag = "up_sample";
  constexpr auto final_tag = "final";

  auto depth_pass = std::make_shared<RenderPass>("DepthPass", depth_shader_);
  depth_pass->AddRenderTag(write_depth_tag);
  depth_pass->SetOutputTexture(render_texture_);
  ShaderParameterContainer depthPassParams;
  light_->GenerateViewMatrix();
  XMMATRIX light_view_matrix;
  light_->GetViewMatrix(light_view_matrix);
  depthPassParams.SetMatrix("lightViewMatrix", light_view_matrix);
  XMMATRIX light_projection_matrix;
  light_->GetProjectionMatrix(light_projection_matrix);
  depthPassParams.SetMatrix("lightProjectionMatrix", light_projection_matrix);
  depth_pass->SetPassParameters(depthPassParams);
  render_pipeline_.AddRenderPass(depth_pass);

  auto shadow_pass = std::make_shared<RenderPass>("ShadowPass", m_ShadowShader);
  shadow_pass->AddRenderTag(write_shadow_tag);
  shadow_pass->AddInputTexture("depthMap", depth_pass->GetOutputTexture());
  shadow_pass->SetOutputTexture(m_BlackWhiteRenderTexture);
  ShaderParameterContainer shadowPassParams;
  XMMATRIX viewMatrix, projectionMatrix;
  camera_->GetViewMatrix(viewMatrix);
  shadowPassParams.SetMatrix("viewMatrix", viewMatrix);
  DirectX11Device::GetD3d11DeviceInstance()->GetProjectionMatrix(
      projectionMatrix);
  shadowPassParams.SetMatrix("projectionMatrix", projectionMatrix);
  shadowPassParams.SetMatrix("lightViewMatrix", light_view_matrix);
  shadowPassParams.SetMatrix("lightProjectionMatrix", light_projection_matrix);
  shadowPassParams.SetVector3("lightPosition", light_->GetPosition());
  shadowPassParams.SetTexture("depthMapTexture",
                              render_texture_->GetShaderResourceView());
  shadow_pass->SetPassParameters(shadowPassParams);
  render_pipeline_.AddRenderPass(shadow_pass);

  auto down_sample_texture_pass =
      std::make_shared<RenderPass>("DownSampleTexture", texture_shader_);
  down_sample_texture_pass->AddRenderTag(down_sample_tag);
  down_sample_texture_pass->AddInputTexture("shadowMap",
                                            shadow_pass->GetOutputTexture());
  down_sample_texture_pass->SetOutputTexture(m_DownSampleTexure);
  down_sample_texture_pass->NeedTurnOffZBuffer();
  XMMATRIX deviceWorldMatrix, baseViewMatrix, orthoMatrix;
  camera_->GetBaseViewMatrix(baseViewMatrix);
  DirectX11Device::GetD3d11DeviceInstance()->GetWorldMatrix(deviceWorldMatrix);
  ShaderParameterContainer DownSampleTexturePassParams;
  DownSampleTexturePassParams.SetMatrix("deviceWorldMatrix", deviceWorldMatrix);
  DownSampleTexturePassParams.SetMatrix("baseViewMatrix", baseViewMatrix);
  m_DownSampleTexure->GetOrthoMatrix(orthoMatrix);
  DownSampleTexturePassParams.SetMatrix("orthoMatrix", orthoMatrix);
  DownSampleTexturePassParams.SetTexture(
      "texture", m_BlackWhiteRenderTexture->GetShaderResourceView());
  down_sample_texture_pass->SetPassParameters(DownSampleTexturePassParams);
  render_pipeline_.AddRenderPass(down_sample_texture_pass);

  auto horizontal_blur_to_texture_pass = std::make_shared<RenderPass>(
      "RenderHorizontalBlurToTexture", m_HorizontalBlurShader);
  horizontal_blur_to_texture_pass->AddRenderTag(horizontal_blur_tag);
  horizontal_blur_to_texture_pass->AddInputTexture(
      "DownSampleTexture", down_sample_texture_pass->GetOutputTexture());
  horizontal_blur_to_texture_pass->SetOutputTexture(m_HorizontalBlurTexture);
  horizontal_blur_to_texture_pass->NeedTurnOffZBuffer();
  ShaderParameterContainer RenderHorizontalBlurToTexturePassParams;
  RenderHorizontalBlurToTexturePassParams.SetMatrix("baseViewMatrix",
                                                    baseViewMatrix);
  m_HorizontalBlurTexture->GetOrthoMatrix(orthoMatrix);
  RenderHorizontalBlurToTexturePassParams.SetMatrix("orthoMatrix", orthoMatrix);
  RenderHorizontalBlurToTexturePassParams.SetFloat(
      "screenWidth", (float)(SHADOWMAP_WIDTH / 2));
  RenderHorizontalBlurToTexturePassParams.SetTexture(
      "texture", m_DownSampleTexure->GetShaderResourceView());
  horizontal_blur_to_texture_pass->SetPassParameters(
      RenderHorizontalBlurToTexturePassParams);
  render_pipeline_.AddRenderPass(horizontal_blur_to_texture_pass);

  auto Vertical_blur_to_texture_pass = std::make_shared<RenderPass>(
      "RenderHorizontalBlurToTexture", m_VerticalBlurShader);
  Vertical_blur_to_texture_pass->AddRenderTag(vertical_blur_tag);
  Vertical_blur_to_texture_pass->AddInputTexture(
      "DownSampleTexture", horizontal_blur_to_texture_pass->GetOutputTexture());
  Vertical_blur_to_texture_pass->SetOutputTexture(m_VerticalBlurTexture);
  Vertical_blur_to_texture_pass->NeedTurnOffZBuffer();
  ShaderParameterContainer RenderVerticalBlurToTexturePassParams;
  RenderHorizontalBlurToTexturePassParams.SetMatrix("baseViewMatrix",
                                                    baseViewMatrix);
  m_VerticalBlurTexture->GetOrthoMatrix(orthoMatrix);
  RenderHorizontalBlurToTexturePassParams.SetMatrix("orthoMatrix", orthoMatrix);
  RenderHorizontalBlurToTexturePassParams.SetFloat(
      "screenHeight", (float)(SHADOWMAP_HEIGHT / 2));
  RenderHorizontalBlurToTexturePassParams.SetTexture(
      "texture", m_HorizontalBlurTexture->GetShaderResourceView());
  Vertical_blur_to_texture_pass->SetPassParameters(
      RenderHorizontalBlurToTexturePassParams);
  render_pipeline_.AddRenderPass(Vertical_blur_to_texture_pass);

  auto up_sample_texture_pass =
      make_shared<RenderPass>("UpSampleTexture", texture_shader_);
  up_sample_texture_pass->AddRenderTag(up_sample_tag);
  up_sample_texture_pass->AddInputTexture(
      "texture", Vertical_blur_to_texture_pass->GetOutputTexture());
  up_sample_texture_pass->SetOutputTexture(m_UpSampleTexure);
  up_sample_texture_pass->NeedTurnOffZBuffer();
  ShaderParameterContainer UpSampleTexturePassParams;
  UpSampleTexturePassParams.SetMatrix("deviceWorldMatrix", deviceWorldMatrix);
  UpSampleTexturePassParams.SetMatrix("baseViewMatrix", baseViewMatrix);
  m_UpSampleTexure->GetOrthoMatrix(orthoMatrix);
  UpSampleTexturePassParams.SetMatrix("orthoMatrix", orthoMatrix);
  UpSampleTexturePassParams.SetTexture(
      "texture", m_VerticalBlurTexture->GetShaderResourceView());
  up_sample_texture_pass->SetPassParameters(UpSampleTexturePassParams);
  render_pipeline_.AddRenderPass(up_sample_texture_pass);

  auto final_pass =
      std::make_shared<RenderPass>("FinalPass", m_SoftShadowShader);
  final_pass->AddRenderTag(final_tag);
  final_pass->AddInputTexture("shadowMap",
                              up_sample_texture_pass->GetOutputTexture());
  ShaderParameterContainer FinalPassParams;
  FinalPassParams.SetMatrix("viewMatrix", viewMatrix);
  FinalPassParams.SetMatrix("projectionMatrix", projectionMatrix);
  FinalPassParams.SetTexture("shadowTexture",
                             m_UpSampleTexure->GetShaderResourceView());
  FinalPassParams.SetVector3("lightPosition", light_->GetPosition());
  FinalPassParams.SetVector4("diffuseColor", light_->GetDiffuseColor());
  FinalPassParams.SetVector4("ambientColor", light_->GetAmbientColor());
  final_pass->SetPassParameters(FinalPassParams);
  render_pipeline_.AddRenderPass(final_pass);

  // Add renderable objects
  auto cube_object =
      std::make_shared<RenderableObject>(m_CubeModel, m_SoftShadowShader);
  cube_object->SetWorldMatrix(XMMatrixTranslation(-2.0f, 2.0f, 0.0f));
  cube_object->AddTag(write_depth_tag);
  cube_object->AddTag(write_shadow_tag);
  cube_object->AddTag(final_tag);
  cube_object->SetParameterCallback([this](ShaderParameterContainer &params) {
    params.SetTexture("texture", m_CubeModel->GetTexture());
  });
  render_pipeline_.AddRenderableObject(cube_object);

  auto sphere_object =
      std::make_shared<RenderableObject>(m_SphereModel, m_SoftShadowShader);
  sphere_object->SetWorldMatrix(XMMatrixTranslation(2.0f, 2.0f, 0.0f));
  sphere_object->AddTag(write_depth_tag);
  sphere_object->AddTag(write_shadow_tag);
  sphere_object->AddTag(final_tag);
  sphere_object->SetParameterCallback([this](ShaderParameterContainer &params) {
    params.SetTexture("texture", m_SphereModel->GetTexture());
  });
  render_pipeline_.AddRenderableObject(sphere_object);

  auto ground_object =
      std::make_shared<RenderableObject>(m_GroundModel, m_SoftShadowShader);
  ground_object->SetWorldMatrix(XMMatrixTranslation(0.0f, 1.0f, 0.0f));
  ground_object->AddTag(write_depth_tag);
  ground_object->AddTag(write_shadow_tag);
  ground_object->AddTag(final_tag);
  ground_object->SetParameterCallback([this](ShaderParameterContainer &params) {
    params.SetTexture("texture", m_GroundModel->GetTexture());
  });
  render_pipeline_.AddRenderableObject(ground_object);

  auto down_sample_object =
      std::make_shared<RenderableObject>(m_SmallWindow, texture_shader_);
  down_sample_object->SetWorldMatrix(deviceWorldMatrix);
  down_sample_object->AddTag(down_sample_tag);
  render_pipeline_.AddRenderableObject(down_sample_object);

  auto horizontal_blur_object =
      std::make_shared<RenderableObject>(m_SmallWindow, m_HorizontalBlurShader);
  horizontal_blur_object->SetWorldMatrix(deviceWorldMatrix);
  horizontal_blur_object->AddTag(horizontal_blur_tag);
  render_pipeline_.AddRenderableObject(horizontal_blur_object);

  auto vertical_blur_object =
      std::make_shared<RenderableObject>(m_SmallWindow, m_VerticalBlurShader);
  vertical_blur_object->SetWorldMatrix(deviceWorldMatrix);
  vertical_blur_object->AddTag(vertical_blur_tag);
  render_pipeline_.AddRenderableObject(vertical_blur_object);

  auto up_sample_object =
      std::make_shared<RenderableObject>(m_FullScreenWindow, texture_shader_);
  up_sample_object->SetWorldMatrix(deviceWorldMatrix);
  up_sample_object->AddTag(up_sample_tag);
  render_pipeline_.AddRenderableObject(up_sample_object);

  // Set global parameters
  ShaderParameterContainer globalParams;
  DirectX11Device::GetD3d11DeviceInstance()->GetWorldMatrix(deviceWorldMatrix);
  DirectX11Device::GetD3d11DeviceInstance()->GetProjectionMatrix(
      projectionMatrix);
  globalParams.SetMatrix("deviceWorldMatrix", deviceWorldMatrix);
  globalParams.SetMatrix("projectionMatrix", projectionMatrix);
  globalParams.Set("ambientColor", light_->GetAmbientColor());
  globalParams.Set("diffuseColor", light_->GetDiffuseColor());
  render_pipeline_.SetGlobalParameters(globalParams);
}

void GraphicsClass::Render() {

  auto directx_device_ = DirectX11Device::GetD3d11DeviceInstance();

  // Generate the view matrix based on the camera's position.
  camera_->Render();
  XMMATRIX viewMatrix, baseViewMatrix;
  camera_->GetViewMatrix(viewMatrix);
  camera_->GetBaseViewMatrix(baseViewMatrix);

  light_->GenerateViewMatrix();
  XMMATRIX lightViewMatrix, lightProjectionMatrix;
  light_->GetViewMatrix(lightViewMatrix);
  light_->GetProjectionMatrix(lightProjectionMatrix);

  ShaderParameterContainer Params;
  Params.SetMatrix("viewMatrix", viewMatrix);
  Params.SetMatrix("baseViewMatrix", baseViewMatrix);
  Params.SetMatrix("lightViewMatrix", lightViewMatrix);
  Params.SetMatrix("lightProjectionMatrix", lightProjectionMatrix);
  Params.SetVector3("lightPosition", light_->GetPosition());

  render_pipeline_.Execute(Params);
}