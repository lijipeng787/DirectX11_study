#include "Graphics.h"

#include <new>

#include "../CommonFramework/Camera.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/TypeDefine.h"

#include "depthshaderclass.h"
#include "lightclass.h"
#include "modelclass.h"
#include "rendertextureclass.h"
#include "shadowshaderclass.h"
#include "transparentdepthshaderclass.h"
#include "treeclass.h"

GraphicsClass::GraphicsClass() {}

GraphicsClass::~GraphicsClass() {}

bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd) {

  bool result;

  {
    directx_device_ = new DirectX11Device;
    if (!directx_device_) {
      return false;
    }
    result = directx_device_->Initialize(screenWidth, screenHeight,
                                         VSYNC_ENABLED, hwnd, FULL_SCREEN,
                                         SCREEN_DEPTH, SCREEN_NEAR);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
      return false;
    }
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
    light_ = (LightClass *)_aligned_malloc(sizeof(LightClass), 16);
    new (light_) LightClass();
    if (!light_) {
      return false;
    }
    light_->GenerateOrthoMatrix(15.0f, 15.0f, SHADOWMAP_DEPTH, SHADOWMAP_NEAR);
  }

  {
    m_GroundModel = new ModelClass;
    if (!m_GroundModel) {
      return false;
    }
    result = m_GroundModel->Initialize("../../tut49/data/plane01.txt",
                                       L"../../tut49/data/dirt.dds", 2.0f);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the ground model object.",
                 L"Error", MB_OK);
      return false;
    }
    m_GroundModel->SetPosition(0.0f, 1.0f, 0.0f);
  }

  {
    m_Tree = new TreeClass;
    if (!m_Tree) {
      return false;
    }
    result = m_Tree->Initialize("../../tut49/data/trees/trunk001.txt",
                                L"../../tut49/data/trees/trunk001.dds",
                                "../../tut49/data/trees/leaf001.txt",
                                L"../../tut49/data/trees/leaf001.dds", 0.1f);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the tree object.", L"Error",
                 MB_OK);
      return false;
    }
    m_Tree->SetPosition(0.0f, 1.0f, 0.0f);
  }

  {
    render_texture_ =
        (RenderTextureClass *)_aligned_malloc(sizeof(RenderTextureClass), 16);
    new (render_texture_) RenderTextureClass();
    if (!render_texture_) {
      return false;
    }
    result = render_texture_->Initialize(SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT,
                                         SHADOWMAP_DEPTH, SHADOWMAP_NEAR);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the render to texture object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  {
    depth_shader_ =
        (DepthShaderClass *)_aligned_malloc(sizeof(DepthShaderClass), 16);
    new (depth_shader_) DepthShaderClass();
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
    m_TransparentDepthShader = (TransparentDepthShaderClass *)_aligned_malloc(
        sizeof(TransparentDepthShaderClass), 16);
    new (m_TransparentDepthShader) TransparentDepthShaderClass();
    if (!m_TransparentDepthShader) {
      return false;
    }
    result = m_TransparentDepthShader->Initialize(hwnd);
    if (!result) {
      MessageBox(hwnd,
                 L"Could not initialize the transparent depth shader object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  {
    m_ShadowShader =
        (ShadowShaderClass *)_aligned_malloc(sizeof(ShadowShaderClass), 16);
    new (m_ShadowShader) ShadowShaderClass();
    if (!m_ShadowShader) {
      return false;
    }
    result = m_ShadowShader->Initialize(hwnd);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the shadow shader object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  return true;
}

void GraphicsClass::Shutdown() {

  // Release the shadow shader object.
  if (m_ShadowShader) {
    m_ShadowShader->Shutdown();
    m_ShadowShader->~ShadowShaderClass();
    _aligned_free(m_ShadowShader);
    m_ShadowShader = 0;
  }

  // Release the transparent depth shader object.
  if (m_TransparentDepthShader) {
    m_TransparentDepthShader->Shutdown();
    m_TransparentDepthShader->~TransparentDepthShaderClass();
    _aligned_free(m_TransparentDepthShader);
    m_TransparentDepthShader = 0;
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

  // Release the tree object.
  if (m_Tree) {
    m_Tree->Shutdown();
    delete m_Tree;
    m_Tree = 0;
  }

  // Release the ground model object.
  if (m_GroundModel) {
    m_GroundModel->Shutdown();
    delete m_GroundModel;
    m_GroundModel = 0;
  }

  if (light_) {
    light_->~LightClass();
    _aligned_free(light_);
    light_ = nullptr;
    ;
  }
}

void GraphicsClass::Frame(float deltaTime) {

  camera_->SetPosition(pos_x_, pos_y_, pos_z_);
  camera_->SetRotation(rot_x_, rot_y_, rot_z_);
  camera_->Render();

  UpdateLighting();

  // Render the graphics.
  Render();
}

void GraphicsClass::UpdateLighting() {

  static float angle = 270.0f;
  float radians;
  static float offsetX = 9.0f;

  angle -= 0.03f * delta_time_;
  if (angle < 90.0f) {
    angle = 270.0f;
    offsetX = 9.0f;
  }
  radians = angle * 0.0174532925f;
  light_->SetDirection(sinf(radians), cosf(radians), 0.0f);

  offsetX -= 0.003f * delta_time_;
  light_->SetPosition(0.0f + offsetX, 10.0f, 1.0f);
  light_->SetLookAt(0.0f - offsetX, 0.0f, 2.0f);
}

void GraphicsClass::Render() {

  XMMATRIX worldMatrix, viewMatrix, projectionMatrix, lightViewMatrix,
      lightOrthoMatrix;
  XMFLOAT4 ambientColor, diffuseColor;
  float posX, posY, posZ;
  bool result;

  result = RenderSceneToTexture();
  if (!result) {
    return false;
  }

  directx_device_->BeginScene(0.0f, 0.5f, 0.8f, 1.0f);

  camera_->Render();

  light_->GenerateViewMatrix();

  directx_device_->GetWorldMatrix(worldMatrix);
  camera_->GetViewMatrix(viewMatrix);
  directx_device_->GetProjectionMatrix(projectionMatrix);

  light_->GetViewMatrix(lightViewMatrix);
  light_->GetOrthoMatrix(lightOrthoMatrix);

  diffuseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
  ambientColor = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);

  m_GroundModel->GetPosition(posX, posY, posZ);
  worldMatrix = XMMatrixTranslation(posX, posY, posZ);

  m_GroundModel->Render(directx_device_->GetDeviceContext());

  m_ShadowShader->Render(m_GroundModel->GetIndexCount(), worldMatrix,
                         viewMatrix, projectionMatrix, lightViewMatrix,
                         lightOrthoMatrix, m_GroundModel->GetTexture(),
                         render_texture_->GetShaderResourceView(),
                         light_->GetDirection(), ambientColor, diffuseColor);

  directx_device_->GetWorldMatrix(worldMatrix);

  m_Tree->GetPosition(posX, posY, posZ);
  worldMatrix = XMMatrixTranslation(posX, posY, posZ);

  m_Tree->RenderTrunk(directx_device_->GetDeviceContext());

  m_ShadowShader->Render(m_Tree->GetTrunkIndexCount(), worldMatrix, viewMatrix,
                         projectionMatrix, lightViewMatrix, lightOrthoMatrix,
                         m_Tree->GetTrunkTexture(),
                         render_texture_->GetShaderResourceView(),
                         light_->GetDirection(), ambientColor, diffuseColor);

  directx_device_->TurnOnAlphaBlending();

  m_Tree->RenderLeaves(directx_device_->GetDeviceContext());

  m_ShadowShader->Render(m_Tree->GetLeafIndexCount(), worldMatrix, viewMatrix,
                         projectionMatrix, lightViewMatrix, lightOrthoMatrix,
                         m_Tree->GetLeafTexture(),
                         render_texture_->GetShaderResourceView(),
                         light_->GetDirection(), ambientColor, diffuseColor);

  directx_device_->TurnOffAlphaBlending();

  directx_device_->EndScene();
}

bool GraphicsClass::RenderSceneToTexture() {

  XMMATRIX worldMatrix, lightViewMatrix, lightOrthoMatrix;
  float posX, posY, posZ;
  bool result;

  render_texture_->SetRenderTarget(directx_device_->GetDeviceContext());

  render_texture_->ClearRenderTarget(0.0f, 0.0f, 0.0f, 1.0f);

  directx_device_->GetWorldMatrix(worldMatrix);

  light_->GenerateViewMatrix();

  light_->GetViewMatrix(lightViewMatrix);
  light_->GetOrthoMatrix(lightOrthoMatrix);

  m_Tree->GetPosition(posX, posY, posZ);
  worldMatrix = XMMatrixTranslation(posX, posY, posZ);

  m_Tree->RenderTrunk(directx_device_->GetDeviceContext());

  depth_shader_->Render(m_Tree->GetTrunkIndexCount(), worldMatrix,
                        lightViewMatrix, lightOrthoMatrix);

  m_Tree->RenderLeaves(directx_device_->GetDeviceContext());
  result = m_TransparentDepthShader->Render(
      m_Tree->GetLeafIndexCount(), worldMatrix, lightViewMatrix,
      lightOrthoMatrix, m_Tree->GetLeafTexture());
  if (!result) {
    return false;
  }

  directx_device_->GetWorldMatrix(worldMatrix);

  m_GroundModel->GetPosition(posX, posY, posZ);
  worldMatrix = XMMatrixTranslation(posX, posY, posZ);

  m_GroundModel->Render(directx_device_->GetDeviceContext());
  depth_shader_->Render(m_GroundModel->GetIndexCount(), worldMatrix,
                        lightViewMatrix, lightOrthoMatrix);

  directx_device_->SetBackBufferRenderTarget();

  directx_device_->ResetViewport();

  return true;
}