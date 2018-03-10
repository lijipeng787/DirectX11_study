#include "Graphics.h"

#include <new>

#include "../CommonFramework/TypeDefine.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/Camera.h"

#include "modelclass.h"
#include "lightclass.h"
#include "projectionshaderclass.h"
#include "textureclass.h"
#include "viewpointclass.h"

GraphicsClass::GraphicsClass() {}

GraphicsClass::~GraphicsClass() {}

bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd) {
	bool result;

	{
		directx_device_ = new DirectX11Device;
		if (!directx_device_) {
			return false;
		}
		result = directx_device_->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
			return false;
		}
	}

	{
		camera_ = (Camera*)_aligned_malloc(sizeof(Camera), 16);
		new (camera_)Camera();
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
		result = m_GroundModel->Initialize("../../tut44/data/floor.txt", L"../../tut44/data/stone.dds");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the ground model object.", L"Error", MB_OK);
			return false;
		}
	}


	{
		m_CubeModel = new ModelClass();
		if (!m_CubeModel) {
			return false;
		}
		result = m_CubeModel->Initialize("../../tut44/data/cube.txt", L"../../tut44/data/seafloor.dds");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the cube model object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		light_ = (LightClass*)_aligned_malloc(sizeof(LightClass), 16);
		new (light_)LightClass();
		if (!light_) {
			return false;
		}
		light_->SetAmbientColor(0.15f, 0.15f, 0.15f, 1.0f);
		light_->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
		light_->SetPosition(2.0f, 5.0f, -2.0f);
	}

	{
		m_ProjectionShader = (ProjectionShaderClass*)_aligned_malloc(sizeof(ProjectionShaderClass), 16);
		new (m_ProjectionShader)ProjectionShaderClass();
		if (!m_ProjectionShader) {
			return false;
		}
		result = m_ProjectionShader->Initialize(hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the projection shader object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_ProjectionTexture = (TextureClass*)_aligned_malloc(sizeof(TextureClass), 16);
		new (m_ProjectionTexture)TextureClass();
		if (!m_ProjectionTexture) {
			return false;
		}
		result = m_ProjectionTexture->Initialize(L"../../tut44/data/grate.dds");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the projection texture object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_ViewPoint = (ViewPointClass*)_aligned_malloc(sizeof(ViewPointClass), 16);
		new (m_ViewPoint)ViewPointClass();
		if (!m_ViewPoint) {
			return false;
		}
		m_ViewPoint->SetPosition(2.0f, 5.0f, -2.0f);
		m_ViewPoint->SetLookAt(0.0f, 0.0f, 0.0f);
		m_ViewPoint->SetProjectionParameters((float)(XM_PI / 2.0f), 1.0f, 0.1f, 100.0f);
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

	// Release the light object.
	if (light_) {
		light_->~LightClass();
		_aligned_free(light_);
		light_ = nullptr;;
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

	directx_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	directx_device_->GetWorldMatrix(worldMatrix);
	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetProjectionMatrix(projectionMatrix);

	m_ViewPoint->GetViewMatrix(viewMatrix2);
	m_ViewPoint->GetProjectionMatrix(projectionMatrix2);

	worldMatrix = XMMatrixTranslation(0.0f, 1.0f, 0.0f);

	m_GroundModel->Render(directx_device_->GetDeviceContext());
	result = m_ProjectionShader->Render(directx_device_->GetDeviceContext(), m_GroundModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
		m_GroundModel->GetTexture(), light_->GetAmbientColor(), light_->GetDiffuseColor(), light_->GetPosition(),
		viewMatrix2, projectionMatrix2, m_ProjectionTexture->GetTexture());
	if (!result) {
		return false;
	}

	directx_device_->GetWorldMatrix(worldMatrix);
	worldMatrix = XMMatrixTranslation(0.0f, 2.0f, 0.0f);

	m_CubeModel->Render(directx_device_->GetDeviceContext());
	result = m_ProjectionShader->Render(directx_device_->GetDeviceContext(), m_CubeModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
		m_CubeModel->GetTexture(), light_->GetAmbientColor(), light_->GetDiffuseColor(), light_->GetPosition(),
		viewMatrix2, projectionMatrix2, m_ProjectionTexture->GetTexture());
	if (!result) {
		return false;
	}

	directx_device_->EndScene();

	return true;
}
