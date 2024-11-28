#include "Graphics.h"

#include <new>

#include "../CommonFramework/Camera.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/TypeDefine.h"

#include "lightclass.h"
#include "modelclass.h"
#include "projectionshaderclass.h"
#include "textureclass.h"
#include "viewpointclass.h"

GraphicsClass::GraphicsClass() {}

GraphicsClass::~GraphicsClass() {}

bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd) {
  bool result;

  {
    auto directx_device_ = DirectX11Device::GetD3d11DeviceInstance();
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
    camera_->SetPosition(0.0f, 0.0f, -10.0f);
    camera_->Render();
    camera_->SetPosition(0.0f, 7.0f, -10.0f);
    camera_->SetRotation(35.0f, 0.0f, 0.0f);
  }

  {
    m_GroundModel = new ModelClass();
    if (!m_GroundModel) {
      return false;
    }
    result = m_GroundModel->Initialize("data/floor.txt", L"data/stone.dds");
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the ground model object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  {
    m_CubeModel = new ModelClass();
    if (!m_CubeModel) {
      return false;
    }
    result = m_CubeModel->Initialize("data/cube.txt", L"data/seafloor.dds");
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the cube model object.", L"Error",
                 MB_OK);
      return false;
    }
  }

  {
    light_ = (LightClass *)_aligned_malloc(sizeof(LightClass), 16);
    new (light_) LightClass();
    if (!light_) {
      return false;
    }
    light_->SetAmbientColor(0.15f, 0.15f, 0.15f, 1.0f);
    light_->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
    light_->SetPosition(2.0f, 5.0f, -2.0f);
  }

  {
    m_ProjectionShader = (ProjectionShaderClass *)_aligned_malloc(
        sizeof(ProjectionShaderClass), 16);
    new (m_ProjectionShader) ProjectionShaderClass();
    if (!m_ProjectionShader) {
      return false;
    }
    result = m_ProjectionShader->Initialize(hwnd);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the projection shader object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  {
    m_ProjectionTexture =
        (TextureClass *)_aligned_malloc(sizeof(TextureClass), 16);
    new (m_ProjectionTexture) TextureClass();
    if (!m_ProjectionTexture) {
      return false;
    }
    result = m_ProjectionTexture->Initialize(L"data/grate.dds");
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the projection texture object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  {
    m_ViewPoint = (ViewPointClass *)_aligned_malloc(sizeof(ViewPointClass), 16);
    new (m_ViewPoint) ViewPointClass();
    if (!m_ViewPoint) {
      return false;
    }
    m_ViewPoint->SetPosition(2.0f, 5.0f, -2.0f);
    m_ViewPoint->SetLookAt(0.0f, 0.0f, 0.0f);
    m_ViewPoint->SetProjectionParameters((float)(XM_PI / 2.0f), 1.0f, 0.1f,
                                         100.0f);
    m_ViewPoint->GenerateViewMatrix();
    m_ViewPoint->GenerateProjectionMatrix();
  }

  return true;
}

void GraphicsClass::Shutdown() {

  // Release the view point object.
  if (m_ViewPoint) {
    m_ViewPoint->~ViewPointClass();
    _aligned_free(m_ViewPoint);
    m_ViewPoint = 0;
  }

  // Release the projection texture object.
  if (m_ProjectionTexture) {
    m_ProjectionTexture->Shutdown();
    m_ProjectionTexture->~TextureClass();
    _aligned_free(m_ProjectionTexture);
    m_ProjectionTexture = 0;
  }

  // Release the projection shader object.
  if (m_ProjectionShader) {
    m_ProjectionShader->Shutdown();
    m_ProjectionShader->~ProjectionShaderClass();
    _aligned_free(m_ProjectionShader);
    m_ProjectionShader = 0;
  }

  if (light_) {
    light_->~LightClass();
    _aligned_free(light_);
    light_ = nullptr;
    ;
  }

  // Release the cube model object.
  if (m_CubeModel) {
    m_CubeModel->Shutdown();
    delete m_CubeModel;
    m_CubeModel = 0;
  }

  // Release the ground model object.
  if (m_GroundModel) {
    m_GroundModel->Shutdown();
    delete m_GroundModel;
    m_GroundModel = 0;
  }

  if (camera_) {
    camera_->~Camera();
    _aligned_free(camera_);
    camera_ = nullptr;
  }
}

bool GraphicsClass::Frame() {
  bool result;

  result = Render();
  if (!result) {
    return false;
  }

  return true;
}

bool GraphicsClass::Render() {

  XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
  XMMATRIX viewMatrix2, projectionMatrix2;
  bool result;

  DirectX11Device::GetD3d11DeviceInstance()->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

  camera_->Render();

  DirectX11Device::GetD3d11DeviceInstance()->GetWorldMatrix(worldMatrix);
  camera_->GetViewMatrix(viewMatrix);
  DirectX11Device::GetD3d11DeviceInstance()->GetProjectionMatrix(
      projectionMatrix);

  m_ViewPoint->GetViewMatrix(viewMatrix2);
  m_ViewPoint->GetProjectionMatrix(projectionMatrix2);

  worldMatrix = XMMatrixTranslation(0.0f, 1.0f, 0.0f);

  m_GroundModel->Render();
  result = m_ProjectionShader->Render(
      m_GroundModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
      m_GroundModel->GetTexture(), light_->GetAmbientColor(),
      light_->GetDiffuseColor(), light_->GetPosition(), viewMatrix2,
      projectionMatrix2, m_ProjectionTexture->GetTexture());
  if (!result) {
    return false;
  }

  DirectX11Device::GetD3d11DeviceInstance()->GetWorldMatrix(worldMatrix);
  worldMatrix = XMMatrixTranslation(0.0f, 2.0f, 0.0f);

  m_CubeModel->Render();
  result = m_ProjectionShader->Render(
      m_CubeModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
      m_CubeModel->GetTexture(), light_->GetAmbientColor(),
      light_->GetDiffuseColor(), light_->GetPosition(), viewMatrix2,
      projectionMatrix2, m_ProjectionTexture->GetTexture());
  if (!result) {
    return false;
  }

  DirectX11Device::GetD3d11DeviceInstance()->EndScene();

  return true;
}
