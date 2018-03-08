#include "Graphics.h"

#include <new>

#include "../CommonFramework/TypeDefine.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/Camera.h"

#include "modelclass.h"
#include "lightclass.h"
#include "rendertextureclass.h"
#include "depthshaderclass.h"
#include "shadowshaderclass.h"

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
		camera_->SetPosition(0.0f, -1.0f, -10.0f);
		camera_->Render();
		camera_->RenderBaseViewMatrix();
	}

	{
		m_CubeModel = new ModelClass();
		if (!m_CubeModel) {
			return false;
		}
		result = m_CubeModel->Initialize(directx_device_->GetDevice(), "../../tut48/data/cube.txt", L"../../tut48/data/wall01.dds");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the cube model object.", L"Error", MB_OK);
			return false;
		}
		m_CubeModel->SetPosition(-2.0f, 2.0f, 0.0f);
	}

	{
		m_SphereModel = new ModelClass();
		if (!m_SphereModel) {
			return false;
		}
		result = m_SphereModel->Initialize(directx_device_->GetDevice(), "../../tut48/data/sphere.txt", L"../../tut48/data/ice.dds");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the sphere model object.", L"Error", MB_OK);
			return false;
		}
		m_SphereModel->SetPosition(2.0f, 2.0f, 0.0f);
	}

	{
		m_GroundModel = new ModelClass();
		if (!m_GroundModel) {
			return false;
		}
		result = m_GroundModel->Initialize(directx_device_->GetDevice(), "../../tut48/data/plane01.txt", L"../../tut48/data/metal001.dds");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the ground model object.", L"Error", MB_OK);
			return false;
		}
		m_GroundModel->SetPosition(0.0f, 1.0f, 0.0f);
	}

	{
		m_Light = (LightClass*)_aligned_malloc(sizeof(LightClass), 16);
		new (m_Light)LightClass();
		if (!m_Light) {
			return false;
		}
		m_Light->SetAmbientColor(0.15f, 0.15f, 0.15f, 1.0f);
		m_Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
		m_Light->GenerateOrthoMatrix(20.0f, SHADOWMAP_DEPTH, SHADOWMAP_NEAR);
	}

	{
		m_RenderTexture = (RenderTextureClass*)_aligned_malloc(sizeof(RenderTextureClass), 16);
		new (m_RenderTexture)RenderTextureClass();
		if (!m_RenderTexture) {
			return false;
		}
		result = m_RenderTexture->Initialize(directx_device_->GetDevice(), SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, SHADOWMAP_DEPTH, SHADOWMAP_NEAR);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the render to texture object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_DepthShader = (DepthShaderClass*)_aligned_malloc(sizeof(DepthShaderClass), 16);
		new (m_DepthShader)DepthShaderClass();
		if (!m_DepthShader) {
			return false;
		}
		result = m_DepthShader->Initialize(directx_device_->GetDevice(), hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the depth shader object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_ShadowShader = (ShadowShaderClass*)_aligned_malloc(sizeof(ShadowShaderClass), 16);
		new (m_ShadowShader)ShadowShaderClass();
		if (!m_ShadowShader) {
			return false;
		}
		result = m_ShadowShader->Initialize(directx_device_->GetDevice(), hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the shadow shader object.", L"Error", MB_OK);
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

	// Release the depth shader object.
	if (m_DepthShader) {
		m_DepthShader->Shutdown();
		m_DepthShader->~DepthShaderClass();
		_aligned_free(m_DepthShader);
		m_DepthShader = 0;
	}

	// Release the render to texture object.
	if (m_RenderTexture) {
		m_RenderTexture->Shutdown();
		m_RenderTexture->~RenderTextureClass();
		_aligned_free(m_RenderTexture);
		m_RenderTexture = 0;
	}

	// Release the light object.
	if (m_Light) {
		m_Light->~LightClass();
		_aligned_free(m_Light);
		m_Light = 0;
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
		camera_ = 0;
	}

	if (directx_device_) {
		directx_device_->Shutdown();
		delete directx_device_;
		directx_device_ = 0;
	}
}

bool GraphicsClass::Frame() {

	bool result;
	static float lightAngle = 270.0f;
	float radians;
	static float lightPosX = 9.0f;

	// Set the position of the camera.
	camera_->SetPosition(pos_x_, pos_y_, pos_z_);
	camera_->SetRotation(rot_x_, rot_y_, rot_z_);

	// Update the position of the light each frame.
	lightPosX -= 0.003f * frame_time_;

	// Update the angle of the light each frame.
	lightAngle -= 0.03f * frame_time_;
	if (lightAngle < 90.0f) {
		lightAngle = 270.0f;

		// Reset the light position also.
		lightPosX = 9.0f;
	}
	radians = lightAngle * 0.0174532925f;

	// Update the direction of the light.
	m_Light->SetDirection(sinf(radians), cosf(radians), 0.0f);

	// Set the position and lookat for the light.
	m_Light->SetPosition(lightPosX, 8.0f, -0.1f);
	m_Light->SetLookAt(-lightPosX, 0.0f, 0.0f);

	// Render the graphics scene.
	result = Render();
	if (!result) {
		return false;
	}

	return true;
}

bool GraphicsClass::RenderSceneToTexture() {

	XMMATRIX worldMatrix, lightViewMatrix, lightOrthoMatrix;
	float posX, posY, posZ;
	bool result;

	m_RenderTexture->SetRenderTarget(directx_device_->GetDeviceContext());

	m_RenderTexture->ClearRenderTarget(directx_device_->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	m_Light->GenerateViewMatrix();

	directx_device_->GetWorldMatrix(worldMatrix);

	m_Light->GetViewMatrix(lightViewMatrix);
	m_Light->GetOrthoMatrix(lightOrthoMatrix);

	m_CubeModel->GetPosition(posX, posY, posZ);
	worldMatrix = XMMatrixTranslation(posX, posY, posZ);

	m_CubeModel->Render(directx_device_->GetDeviceContext());
	result = m_DepthShader->Render(directx_device_->GetDeviceContext(), m_CubeModel->GetIndexCount(), worldMatrix, lightViewMatrix, lightOrthoMatrix);
	if (!result) {
		return false;
	}

	directx_device_->GetWorldMatrix(worldMatrix);

	m_SphereModel->GetPosition(posX, posY, posZ);
	worldMatrix = XMMatrixTranslation(posX, posY, posZ);

	m_SphereModel->Render(directx_device_->GetDeviceContext());
	result = m_DepthShader->Render(directx_device_->GetDeviceContext(), m_SphereModel->GetIndexCount(), worldMatrix, lightViewMatrix, lightOrthoMatrix);
	if (!result) {
		return false;
	}

	directx_device_->GetWorldMatrix(worldMatrix);

	m_GroundModel->GetPosition(posX, posY, posZ);
	worldMatrix = XMMatrixTranslation(posX, posY, posZ);

	m_GroundModel->Render(directx_device_->GetDeviceContext());
	result = m_DepthShader->Render(directx_device_->GetDeviceContext(), m_GroundModel->GetIndexCount(), worldMatrix, lightViewMatrix, lightOrthoMatrix);
	if (!result) {
		return false;
	}

	directx_device_->SetBackBufferRenderTarget();

	directx_device_->ResetViewport();

	return true;
}

bool GraphicsClass::Render() {

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	XMMATRIX lightViewMatrix, lightOrthoMatrix;
	bool result;
	float posX, posY, posZ;

	result = RenderSceneToTexture();
	if (!result) {
		return false;
	}

	directx_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	m_Light->GenerateViewMatrix();

	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetWorldMatrix(worldMatrix);
	directx_device_->GetProjectionMatrix(projectionMatrix);

	m_Light->GetViewMatrix(lightViewMatrix);
	m_Light->GetOrthoMatrix(lightOrthoMatrix);

	m_CubeModel->GetPosition(posX, posY, posZ);
	worldMatrix = XMMatrixTranslation(posX, posY, posZ);

	m_CubeModel->Render(directx_device_->GetDeviceContext());

	result = m_ShadowShader->Render(directx_device_->GetDeviceContext(), m_CubeModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, lightViewMatrix,
		lightOrthoMatrix, m_CubeModel->GetTexture(), m_RenderTexture->GetShaderResourceView(), m_Light->GetDirection(),
		m_Light->GetAmbientColor(), m_Light->GetDiffuseColor());
	if (!result) {
		return false;
	}

	directx_device_->GetWorldMatrix(worldMatrix);

	m_SphereModel->GetPosition(posX, posY, posZ);
	worldMatrix = XMMatrixTranslation(posX, posY, posZ);

	m_SphereModel->Render(directx_device_->GetDeviceContext());
	result = m_ShadowShader->Render(directx_device_->GetDeviceContext(), m_SphereModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, lightViewMatrix,
		lightOrthoMatrix, m_SphereModel->GetTexture(), m_RenderTexture->GetShaderResourceView(), m_Light->GetDirection(),
		m_Light->GetAmbientColor(), m_Light->GetDiffuseColor());
	if (!result) {
		return false;
	}

	directx_device_->GetWorldMatrix(worldMatrix);

	m_GroundModel->GetPosition(posX, posY, posZ);
	worldMatrix = XMMatrixTranslation(posX, posY, posZ);

	m_GroundModel->Render(directx_device_->GetDeviceContext());
	result = m_ShadowShader->Render(directx_device_->GetDeviceContext(), m_GroundModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, lightViewMatrix,
		lightOrthoMatrix, m_GroundModel->GetTexture(), m_RenderTexture->GetShaderResourceView(), m_Light->GetDirection(),
		m_Light->GetAmbientColor(), m_Light->GetDiffuseColor());
	if (!result) {
		return false;
	}

	directx_device_->EndScene();

	return true;
}
