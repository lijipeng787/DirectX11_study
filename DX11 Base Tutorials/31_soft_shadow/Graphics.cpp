#include "Graphics.h"

#include <new>

#include "../CommonFramework/Camera.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/TypeDefine.h"

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

GraphicsClass::GraphicsClass() {}

GraphicsClass::~GraphicsClass() {}

bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd) {

  bool result;
  int downSampleWidth, downSampleHeight;

  auto directx11_device_ = DirectX11Device::GetD3d11DeviceInstance();

  if (!(directx11_device_->Initialize(screenWidth, screenHeight, VSYNC_ENABLED,
                                      hwnd, FULL_SCREEN, SCREEN_DEPTH,
                                      SCREEN_NEAR))) {
    MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
    return false;
  }

  {
    camera_ = (Camera *)_aligned_malloc(sizeof(Camera), 16);
    new (camera_) Camera();
    if (!camera_) {
      return false;
    }
    camera_->SetPosition(0.0f, -1.0f, -10.0f);
    camera_->Render();
    camera_->RenderBaseViewMatrix();
  }

  {
    // Create the cube model object.
    m_CubeModel = new ModelClass();
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
    // m_CubeModel->SetPosition(-2.0f, 2.0f, 0.0f);
    m_CubeModel->SetWorldMatrix(
        std::move(XMMatrixTranslation(-2.0f, 2.0f, 0.0f)));
  }

  {
    // Create the sphere model object.
    m_SphereModel = new ModelClass();
    if (!m_SphereModel) {
      return false;
    }

    // Initialize the sphere model object.
    result = m_SphereModel->Initialize("./data/sphere.txt", L"./data/ice.dds");
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the sphere model object.",
                 L"Error", MB_OK);
      return false;
    }

    // Set the position for the sphere model.
    // m_SphereModel->SetPosition(2.0f, 2.0f, 0.0f);
    m_SphereModel->SetWorldMatrix(
        std::move(XMMatrixTranslation(2.0f, 2.0f, 0.0f)));
  }

  {
    // Create the ground model object.
    m_GroundModel = new ModelClass();
    if (!m_GroundModel) {
      return false;
    }

    // Initialize the ground model object.
    result =
        m_GroundModel->Initialize("./data/plane01.txt", L"./data/metal001.dds");
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the ground model object.",
                 L"Error", MB_OK);
      return false;
    }

    // Set the position for the ground model.
    // m_GroundModel->SetPosition(0.0f, 1.0f, 0.0f);
    m_GroundModel->SetWorldMatrix(
        std::move(XMMatrixTranslation(0.0f, 1.0f, 0.0f)));
  }

  {
    // Create the light object.
    light_ = (LightClass *)_aligned_malloc(sizeof(LightClass), 16);
    new (light_) LightClass();
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
    render_texture_ =
        (RenderTextureClass *)_aligned_malloc(sizeof(RenderTextureClass), 16);
    new (render_texture_) RenderTextureClass();
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
    // Create the depth shader object.
    depth_shader_ =
        (DepthShaderClass *)_aligned_malloc(sizeof(DepthShaderClass), 16);
    new (depth_shader_) DepthShaderClass();
    if (!depth_shader_) {
      return false;
    }

    // Initialize the depth shader object.
    result = depth_shader_->Initialize(hwnd);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the depth shader object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  {
    // Create the black and white render to texture object.
    m_BlackWhiteRenderTexture =
        (RenderTextureClass *)_aligned_malloc(sizeof(RenderTextureClass), 16);
    new (m_BlackWhiteRenderTexture) RenderTextureClass();
    if (!m_BlackWhiteRenderTexture) {
      return false;
    }

    // Initialize the black and white render to texture object.
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
    m_ShadowShader =
        (ShadowShaderClass *)_aligned_malloc(sizeof(ShadowShaderClass), 16);
    new (m_ShadowShader) ShadowShaderClass();
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
  downSampleWidth = SHADOWMAP_WIDTH / 2;
  downSampleHeight = SHADOWMAP_HEIGHT / 2;

  {
    // Create the down sample render to texture object.
    m_DownSampleTexure =
        (RenderTextureClass *)_aligned_malloc(sizeof(RenderTextureClass), 16);
    new (m_DownSampleTexure) RenderTextureClass();
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
    // Create the small ortho window object.
    m_SmallWindow = new OrthoWindowClass();
    if (!m_SmallWindow) {
      return false;
    }

    // Initialize the small ortho window object.
    result = m_SmallWindow->Initialize(downSampleWidth, downSampleHeight);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the small ortho window object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  {
    // Create the texture shader object.
    texture_shader_ =
        (TextureShaderClass *)_aligned_malloc(sizeof(TextureShaderClass), 16);
    new (texture_shader_) TextureShaderClass();
    if (!texture_shader_) {
      return false;
    }

    // Initialize the texture shader object.
    result = texture_shader_->Initialize(hwnd);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the texture shader object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  {
    // Create the horizontal blur render to texture object.
    m_HorizontalBlurTexture =
        (RenderTextureClass *)_aligned_malloc(sizeof(RenderTextureClass), 16);
    new (m_HorizontalBlurTexture) RenderTextureClass();
    if (!m_HorizontalBlurTexture) {
      return false;
    }

    // Initialize the horizontal blur render to texture object.
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
    // Create the horizontal blur shader object.
    m_HorizontalBlurShader = (HorizontalBlurShaderClass *)_aligned_malloc(
        sizeof(HorizontalBlurShaderClass), 16);
    new (m_HorizontalBlurShader) HorizontalBlurShaderClass();
    if (!m_HorizontalBlurShader) {
      return false;
    }

    // Initialize the horizontal blur shader object.
    result = m_HorizontalBlurShader->Initialize(hwnd);
    if (!result) {
      MessageBox(hwnd,
                 L"Could not initialize the horizontal blur shader object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  {
    // Create the vertical blur render to texture object.
    m_VerticalBlurTexture =
        (RenderTextureClass *)_aligned_malloc(sizeof(RenderTextureClass), 16);
    new (m_VerticalBlurTexture) RenderTextureClass();
    if (!m_VerticalBlurTexture) {
      return false;
    }

    // Initialize the vertical blur render to texture object.
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
    // Create the vertical blur shader object.
    m_VerticalBlurShader = (VerticalBlurShaderClass *)_aligned_malloc(
        sizeof(VerticalBlurShaderClass), 16);
    new (m_VerticalBlurShader) VerticalBlurShaderClass();
    if (!m_VerticalBlurShader) {
      return false;
    }

    // Initialize the vertical blur shader object.
    result = m_VerticalBlurShader->Initialize(hwnd);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the vertical blur shader object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  {
    // Create the up sample render to texture object.
    m_UpSampleTexure =
        (RenderTextureClass *)_aligned_malloc(sizeof(RenderTextureClass), 16);
    new (m_UpSampleTexure) RenderTextureClass();
    if (!m_UpSampleTexure) {
      return false;
    }

    // Initialize the up sample render to texture object.
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
    // Create the full screen ortho window object.
    m_FullScreenWindow = new OrthoWindowClass();
    if (!m_FullScreenWindow) {
      return false;
    }

    // Initialize the full screen ortho window object.
    result = m_FullScreenWindow->Initialize(SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT);
    if (!result) {
      MessageBox(hwnd,
                 L"Could not initialize the full screen ortho window object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  {
    // Create the soft shadow shader object.
    m_SoftShadowShader = (SoftShadowShaderClass *)_aligned_malloc(
        sizeof(SoftShadowShaderClass), 16);
    new (m_SoftShadowShader) SoftShadowShaderClass();
    if (!m_SoftShadowShader) {
      return false;
    }

    // Initialize the soft shadow shader object.
    result = m_SoftShadowShader->Initialize(hwnd);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the soft shadow shader object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  return true;
}

void GraphicsClass::Shutdown() {

  // Release the soft shadow shader object.
  if (m_SoftShadowShader) {
    m_SoftShadowShader->Shutdown();
    m_SoftShadowShader->~SoftShadowShaderClass();
    _aligned_free(m_SoftShadowShader);
    m_SoftShadowShader = 0;
  }

  if (m_FullScreenWindow) {
    m_FullScreenWindow->Shutdown();
    delete m_FullScreenWindow;
    m_FullScreenWindow = 0;
  }

  if (m_UpSampleTexure) {
    m_UpSampleTexure->Shutdown();
    m_UpSampleTexure->~RenderTextureClass();
    _aligned_free(m_UpSampleTexure);
    m_UpSampleTexure = 0;
  }

  if (m_VerticalBlurShader) {
    m_VerticalBlurShader->Shutdown();
    m_VerticalBlurShader->~VerticalBlurShaderClass();
    _aligned_free(m_VerticalBlurShader);
    m_VerticalBlurShader = 0;
  }

  if (m_VerticalBlurTexture) {
    m_VerticalBlurTexture->Shutdown();
    m_VerticalBlurTexture->~RenderTextureClass();
    _aligned_free(m_VerticalBlurTexture);
    m_VerticalBlurTexture = 0;
  }

  if (m_HorizontalBlurShader) {
    m_HorizontalBlurShader->Shutdown();
    m_HorizontalBlurShader->~HorizontalBlurShaderClass();
    _aligned_free(m_HorizontalBlurShader);
    m_HorizontalBlurShader = 0;
  }

  if (m_HorizontalBlurTexture) {
    m_HorizontalBlurTexture->Shutdown();
    m_HorizontalBlurTexture->~RenderTextureClass();
    _aligned_free(m_HorizontalBlurTexture);
    m_HorizontalBlurTexture = 0;
  }

  if (texture_shader_) {
    texture_shader_->Shutdown();
    texture_shader_->~TextureShaderClass();
    _aligned_free(texture_shader_);
    texture_shader_ = 0;
  }

  if (m_SmallWindow) {
    m_SmallWindow->Shutdown();
    delete m_SmallWindow;
    m_SmallWindow = 0;
  }

  if (m_DownSampleTexure) {
    m_DownSampleTexure->Shutdown();
    m_DownSampleTexure->~RenderTextureClass();
    _aligned_free(m_DownSampleTexure);
    m_DownSampleTexure = 0;
  }

  // Release the shadow shader object.
  if (m_ShadowShader) {
    m_ShadowShader->Shutdown();
    m_ShadowShader->~ShadowShaderClass();
    _aligned_free(m_ShadowShader);
    m_ShadowShader = 0;
  }

  // Release the black and white render to texture.
  if (m_BlackWhiteRenderTexture) {
    m_BlackWhiteRenderTexture->Shutdown();
    m_BlackWhiteRenderTexture->~RenderTextureClass();
    _aligned_free(m_BlackWhiteRenderTexture);
    m_BlackWhiteRenderTexture = 0;
  }

  // Release the depth shader object.
  if (depth_shader_) {
    depth_shader_->Shutdown();
    depth_shader_->~DepthShaderClass();
    _aligned_free(depth_shader_);
    depth_shader_ = 0;
  }

  if (render_texture_) {
    render_texture_->Shutdown();
    render_texture_->~RenderTextureClass();
    _aligned_free(render_texture_);
    render_texture_ = 0;
  }

  if (light_) {
    light_->~LightClass();
    _aligned_free(light_);
    light_ = nullptr;
    ;
  }

  // Release the ground model object.
  if (m_GroundModel) {
    m_GroundModel->Shutdown();
    delete m_GroundModel;
    m_GroundModel = 0;
  }

  // Release the sphere model object.
  if (m_SphereModel) {
    m_SphereModel->Shutdown();
    delete m_SphereModel;
    m_SphereModel = 0;
  }

  // Release the cube model object.
  if (m_CubeModel) {
    m_CubeModel->Shutdown();
    delete m_CubeModel;
    m_CubeModel = 0;
  }

  if (camera_) {
    camera_->~Camera();
    _aligned_free(camera_);
    camera_ = nullptr;
  }
}

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

bool GraphicsClass::RenderSceneToTexture() {

  auto directx_device_ = DirectX11Device::GetD3d11DeviceInstance();

  render_texture_->SetRenderTarget(directx_device_->GetDeviceContext());

  render_texture_->ClearRenderTarget(0.0f, 0.0f, 0.0f, 1.0f);

  light_->GenerateViewMatrix();

  XMMATRIX lightViewMatrix, lightProjectionMatrix;
  light_->GetViewMatrix(lightViewMatrix);
  light_->GetProjectionMatrix(lightProjectionMatrix);

  auto worldMatrix = m_CubeModel->GetWorldMatrix();

  ShaderParameterContainer parameters;
  parameters.SetMatrix("worldMatrix", worldMatrix);
  parameters.SetMatrix("viewMatrix", lightViewMatrix);
  parameters.SetMatrix("projectionMatrix", lightProjectionMatrix);

  m_CubeModel->Render(*depth_shader_, parameters);

  worldMatrix = m_SphereModel->GetWorldMatrix();
  parameters.SetMatrix("worldMatrix", worldMatrix);

  m_SphereModel->Render(*depth_shader_, parameters);

  worldMatrix = m_GroundModel->GetWorldMatrix();
  parameters.SetMatrix("worldMatrix", worldMatrix);

  m_GroundModel->Render(*depth_shader_, parameters);

  directx_device_->SetBackBufferRenderTarget();

  directx_device_->ResetViewport();

  return true;
}

bool GraphicsClass::RenderBlackAndWhiteShadows() {

  auto directx_device_ = DirectX11Device::GetD3d11DeviceInstance();

  // Set the render target to be the render to texture.
  m_BlackWhiteRenderTexture->SetRenderTarget(
      directx_device_->GetDeviceContext());

  // Clear the render to texture.
  m_BlackWhiteRenderTexture->ClearRenderTarget(0.0f, 0.0f, 0.0f, 1.0f);

  // Generate the view matrix based on the camera's position.
  camera_->Render();

  // Generate the light view matrix based on the light's position.
  light_->GenerateViewMatrix();

  // Get the world, view, and projection matrices from the camera and d3d
  // objects.
  XMMATRIX viewMatrix, projectionMatrix;
  camera_->GetViewMatrix(viewMatrix);
  directx_device_->GetProjectionMatrix(projectionMatrix);

  // Get the light's view and projection matrices from the light object.
  XMMATRIX lightViewMatrix, lightProjectionMatrix;
  light_->GetViewMatrix(lightViewMatrix);
  light_->GetProjectionMatrix(lightProjectionMatrix);

  // Setup the world matrix for the cube model.
  auto worldMatrix = m_CubeModel->GetWorldMatrix();

  ShaderParameterContainer parameters;
  parameters.SetMatrix("worldMatrix", worldMatrix);
  parameters.SetMatrix("viewMatrix", viewMatrix);
  parameters.SetMatrix("projectionMatrix", projectionMatrix);
  parameters.SetMatrix("lightViewMatrix", lightViewMatrix);
  parameters.SetMatrix("lightProjectionMatrix", lightProjectionMatrix);
  parameters.SetTexture("depthMapTexture",
                        render_texture_->GetShaderResourceView());
  parameters.SetVector3("lightPosition", light_->GetPosition());
  // Render the cube model using the shadow shader.
  m_CubeModel->Render(*m_ShadowShader, parameters);

  worldMatrix = m_SphereModel->GetWorldMatrix();
  parameters.SetMatrix("worldMatrix", worldMatrix);

  m_SphereModel->Render(*m_ShadowShader, parameters);

  worldMatrix = m_GroundModel->GetWorldMatrix();
  parameters.SetMatrix("worldMatrix", worldMatrix);

  m_GroundModel->Render(*m_ShadowShader, parameters);

  // Reset the render target back to the original back buffer and not the render
  // to texture anymore.
  directx_device_->SetBackBufferRenderTarget();

  // Reset the viewport back to the original.
  directx_device_->ResetViewport();

  return true;
}

bool GraphicsClass::DownSampleTexture() {

  auto directx_device_ = DirectX11Device::GetD3d11DeviceInstance();

  m_DownSampleTexure->SetRenderTarget(directx_device_->GetDeviceContext());

  // Clear the render to texture.
  m_DownSampleTexure->ClearRenderTarget(0.0f, 0.0f, 0.0f, 1.0f);

  // Generate the view matrix based on the camera's position.
  camera_->Render();

  // Get the world and view matrices from the camera and d3d objects.
  XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;
  directx_device_->GetWorldMatrix(worldMatrix);
  camera_->GetBaseViewMatrix(baseViewMatrix);

  // Get the ortho matrix from the render to texture since texture has different
  // dimensions being that it is smaller.
  m_DownSampleTexure->GetOrthoMatrix(orthoMatrix);

  // Turn off the Z buffer to begin all 2D rendering.
  directx_device_->TurnZBufferOff();

  // Put the small ortho window vertex and index buffers on the graphics
  // pipeline to prepare them for drawing.
  m_SmallWindow->Render(directx_device_->GetDeviceContext());

  // Render the small ortho window using the texture shader and the render to
  // texture of the scene as the texture resource.
  ShaderParameterContainer parameters;
  parameters.SetMatrix("worldMatrix", worldMatrix);
  parameters.SetMatrix("baseViewMatrix", baseViewMatrix);
  parameters.SetMatrix("orthoMatrix", orthoMatrix);
  parameters.SetTexture("texture",
                        m_BlackWhiteRenderTexture->GetShaderResourceView());

  auto result =
      texture_shader_->Render(m_SmallWindow->GetIndexCount(), parameters);
  if (!result) {
    return false;
  }

  // Turn the Z buffer back on now that all 2D rendering has completed.
  directx_device_->TurnZBufferOn();

  // Reset the render target back to the original back buffer and not the render
  // to texture anymore.
  directx_device_->SetBackBufferRenderTarget();

  // Reset the viewport back to the original.
  directx_device_->ResetViewport();

  return true;
}

bool GraphicsClass::RenderHorizontalBlurToTexture() {

  // Store the screen width in a float that will be used in the horizontal blur
  // shader.
  auto screenSizeX = (float)(SHADOWMAP_WIDTH / 2);

  auto directx_device_ = DirectX11Device::GetD3d11DeviceInstance();

  // Set the render target to be the render to texture.
  m_HorizontalBlurTexture->SetRenderTarget(directx_device_->GetDeviceContext());

  // Clear the render to texture.
  m_HorizontalBlurTexture->ClearRenderTarget(0.0f, 0.0f, 0.0f, 1.0f);

  // Generate the view matrix based on the camera's position.
  camera_->Render();

  // Get the world and view matrices from the camera and d3d objects.
  XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;
  directx_device_->GetWorldMatrix(worldMatrix);
  camera_->GetBaseViewMatrix(baseViewMatrix);

  // Get the ortho matrix from the render to texture since texture has different
  // dimensions.
  m_HorizontalBlurTexture->GetOrthoMatrix(orthoMatrix);

  // Turn off the Z buffer to begin all 2D rendering.
  directx_device_->TurnZBufferOff();

  // Put the small ortho window vertex and index buffers on the graphics
  // pipeline to prepare them for drawing.
  m_SmallWindow->Render(directx_device_->GetDeviceContext());

  // Render the small ortho window using the horizontal blur shader and the down
  // sampled render to texture resource.

  ShaderParameterContainer parameters;
  parameters.SetMatrix("worldMatrix", worldMatrix);
  parameters.SetMatrix("viewMatrix", baseViewMatrix);
  parameters.SetMatrix("projectionMatrix", orthoMatrix);
  parameters.SetFloat("screenWidth", screenSizeX);
  parameters.SetTexture("texture", m_DownSampleTexure->GetShaderResourceView());

  auto result = m_HorizontalBlurShader->Render(m_SmallWindow->GetIndexCount(),
                                               parameters);
  if (!result) {
    return false;
  }

  // Turn the Z buffer back on now that all 2D rendering has completed.
  directx_device_->TurnZBufferOn();

  // Reset the render target back to the original back buffer and not the render
  // to texture anymore.
  directx_device_->SetBackBufferRenderTarget();

  // Reset the viewport back to the original.
  directx_device_->ResetViewport();

  return true;
}

bool GraphicsClass::RenderVerticalBlurToTexture() {

  // Store the screen height in a float that will be used in the vertical blur
  // shader.
  auto screenSizeY = (float)(SHADOWMAP_HEIGHT / 2);

  auto directx_device_ = DirectX11Device::GetD3d11DeviceInstance();

  // Set the render target to be the render to texture.
  m_VerticalBlurTexture->SetRenderTarget(directx_device_->GetDeviceContext());

  // Clear the render to texture.
  m_VerticalBlurTexture->ClearRenderTarget(0.0f, 0.0f, 0.0f, 1.0f);

  // Generate the view matrix based on the camera's position.
  camera_->Render();

  // Get the world and view matrices from the camera and d3d objects.
  XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;
  directx_device_->GetWorldMatrix(worldMatrix);
  camera_->GetBaseViewMatrix(baseViewMatrix);

  // Get the ortho matrix from the render to texture since texture has different
  // dimensions.
  m_VerticalBlurTexture->GetOrthoMatrix(orthoMatrix);

  // Turn off the Z buffer to begin all 2D rendering.
  directx_device_->TurnZBufferOff();

  // Put the small ortho window vertex and index buffers on the graphics
  // pipeline to prepare them for drawing.
  m_SmallWindow->Render(directx_device_->GetDeviceContext());

  ShaderParameterContainer parameters;
  parameters.SetMatrix("worldMatrix", worldMatrix);
  parameters.SetMatrix("viewMatrix", baseViewMatrix);
  parameters.SetMatrix("projectionMatrix", orthoMatrix);
  parameters.SetFloat("screenHeight", screenSizeY);
  parameters.SetTexture("texture",
                        m_HorizontalBlurTexture->GetShaderResourceView());

  // Render the small ortho window using the vertical blur shader and the
  // horizontal blurred render to texture resource.
  auto result =
      m_VerticalBlurShader->Render(m_SmallWindow->GetIndexCount(), parameters);
  if (!result) {
    return false;
  }

  // Turn the Z buffer back on now that all 2D rendering has completed.
  directx_device_->TurnZBufferOn();

  // Reset the render target back to the original back buffer and not the render
  // to texture anymore.
  directx_device_->SetBackBufferRenderTarget();

  // Reset the viewport back to the original.
  directx_device_->ResetViewport();

  return true;
}

bool GraphicsClass::UpSampleTexture() {

  auto directx_device_ = DirectX11Device::GetD3d11DeviceInstance();

  // Set the render target to be the render to texture.
  m_UpSampleTexure->SetRenderTarget(directx_device_->GetDeviceContext());

  // Clear the render to texture.
  m_UpSampleTexure->ClearRenderTarget(0.0f, 0.0f, 0.0f, 1.0f);

  // Generate the view matrix based on the camera's position.
  camera_->Render();

  // Get the world and view matrices from the camera and d3d objects.
  XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;
  directx_device_->GetWorldMatrix(worldMatrix);
  camera_->GetBaseViewMatrix(baseViewMatrix);

  // Get the ortho matrix from the render to texture since texture has different
  // dimensions.
  m_UpSampleTexure->GetOrthoMatrix(orthoMatrix);

  // Turn off the Z buffer to begin all 2D rendering.
  directx_device_->TurnZBufferOff();

  // Put the full screen ortho window vertex and index buffers on the graphics
  // pipeline to prepare them for drawing.
  m_FullScreenWindow->Render(directx_device_->GetDeviceContext());

  // Render the full screen ortho window using the texture shader and the small
  // sized final blurred render to texture resource.

  ShaderParameterContainer parameters;
  parameters.SetMatrix("worldMatrix", worldMatrix);
  parameters.SetMatrix("baseViewMatrix", baseViewMatrix);
  parameters.SetMatrix("orthoMatrix", orthoMatrix);
  parameters.SetTexture("texture",
                        m_VerticalBlurTexture->GetShaderResourceView());

  auto result =
      texture_shader_->Render(m_FullScreenWindow->GetIndexCount(), parameters);
  if (!result) {
    return false;
  }

  // Turn the Z buffer back on now that all 2D rendering has completed.
  directx_device_->TurnZBufferOn();

  // Reset the render target back to the original back buffer and not the render
  // to texture anymore.
  directx_device_->SetBackBufferRenderTarget();

  // Reset the viewport back to the original.
  directx_device_->ResetViewport();

  return true;
}

void GraphicsClass::Render() {

  // First render the scene to a texture.
  auto result = RenderSceneToTexture();
  if (!result) {
    return;
  }

  // Next render the shadowed scene in black and white.
  result = RenderBlackAndWhiteShadows();
  if (!result) {
    return;
  }

  // Then down sample the black and white scene texture.
  result = DownSampleTexture();
  if (!result) {
    return;
  }

  // Perform a horizontal blur on the down sampled texture.
  result = RenderHorizontalBlurToTexture();
  if (!result) {
    return;
  }

  // Now perform a vertical blur on the texture.
  result = RenderVerticalBlurToTexture();
  if (!result) {
    return;
  }

  // Finally up sample the final blurred render to texture that can now be used
  // in the soft shadow shader.
  result = UpSampleTexture();
  if (!result) {
    return;
  }

  auto directx_device_ = DirectX11Device::GetD3d11DeviceInstance();

  // Clear the buffers to begin the scene.
  directx_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

  // Generate the view matrix based on the camera's position.
  camera_->Render();

  // Get the world, view, and projection matrices from the camera and d3d
  // objects.
  XMMATRIX viewMatrix, projectionMatrix;
  camera_->GetViewMatrix(viewMatrix);
  directx_device_->GetProjectionMatrix(projectionMatrix);

  // Setup the world matrix for the cube model.
  auto worldMatrix = m_CubeModel->GetWorldMatrix();

  ShaderParameterContainer parameters;
  parameters.SetMatrix("worldMatrix", worldMatrix);
  parameters.SetMatrix("viewMatrix", viewMatrix);
  parameters.SetMatrix("projectionMatrix", projectionMatrix);
  parameters.SetTexture("shadowTexture",
                        m_UpSampleTexure->GetShaderResourceView());
  parameters.SetVector3("lightPosition", light_->GetPosition());
  parameters.SetTexture("texture", m_CubeModel->GetTexture());
  parameters.SetVector4("diffuseColor", light_->GetDiffuseColor());
  parameters.SetVector4("ambientColor", light_->GetAmbientColor());

  // Render the cube model using the soft shadow shader.
  m_CubeModel->Render(*m_SoftShadowShader, parameters);

  worldMatrix = m_SphereModel->GetWorldMatrix();

  parameters.SetMatrix("worldMatrix", worldMatrix);
  parameters.SetTexture("texture", m_SphereModel->GetTexture());

  m_SphereModel->Render(*m_SoftShadowShader, parameters);

  worldMatrix = m_GroundModel->GetWorldMatrix();

  parameters.SetMatrix("worldMatrix", worldMatrix);
  parameters.SetTexture("texture", m_GroundModel->GetTexture());

  m_GroundModel->Render(*m_SoftShadowShader, parameters);

  result =
      m_SoftShadowShader->Render(m_GroundModel->GetIndexCount(), parameters);
  if (!result) {
    return;
  }

  // Present the rendered scene to the screen.
  directx_device_->EndScene();
}