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
		result = m_CubeModel->Initialize("../../tut48/data/cube.txt", L"../../tut48/data/wall01.dds");
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
		result = m_SphereModel->Initialize("../../tut48/data/sphere.txt", L"../../tut48/data/ice.dds");
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
		result = m_GroundModel->Initialize("../../tut48/data/plane01.txt", L"../../tut48/data/metal001.dds");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the ground model object.", L"Error", MB_OK);
			return false;
		}
		m_GroundModel->SetPosition(0.0f, 1.0f, 0.0f);
	}

	{
		light_ = (LightClass*)_aligned_malloc(sizeof(LightClass), 16);
		new (light_)LightClass();
		if (!light_) {
			return false;
		}
		light_->SetAmbientColor(0.15f, 0.15f, 0.15f, 1.0f);
		light_->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
		light_->GenerateOrthoMatrix(20.0f, SHADOWMAP_DEPTH, SHADOWMAP_NEAR);
	}

	{
		render_texture_ = (RenderTextureClass*)_aligned_malloc(sizeof(RenderTextureClass), 16);
		new (render_texture_)RenderTextureClass();
		if (!render_texture_) {
			return false;
		}
		result = render_texture_->Initialize(SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, SHADOWMAP_DEPTH, SHADOWMAP_NEAR);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the render to texture object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		depth_shader_ = (DepthShaderClass*)_aligned_malloc(sizeof(DepthShaderClass), 16);
		new (depth_shader_)DepthShaderClass();
		if (!depth_shader_) {
			return false;
		}
		result = depth_shader_->Initialize(hwnd);
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
		result = m_ShadowShader->Initialize(hwnd);
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
		light_ = nullptr;;
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
}

bool GraphicsClass::Frame() {

	bool result;
	static float lightAngle = 270.0f;
	float radians;
	static float lightPosX = 9.0f;


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
	light_->SetDirection(sinf(radians), cosf(radians), 0.0f);

	// Set the position and lookat for the light.
	light_->SetPosition(lightPosX, 8.0f, -0.1f);
	light_->SetLookAt(-lightPosX, 0.0f, 0.0f);

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

	render_texture_->SetRenderTarget(directx_device_->GetDeviceContext());

	render_texture_->ClearRenderTarget(directx_device_->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	light_->GenerateViewMatrix();

	directx_device_->GetWorldMatrix(worldMatrix);

	light_->GetViewMatrix(lightViewMatrix);
	light_->GetOrthoMatrix(lightOrthoMatrix);

	m_CubeModel->GetPosition(posX, posY, posZ);
	worldMatrix = XMMatrixTranslation(posX, posY, posZ);

	m_CubeModel->Render(directx_device_->GetDeviceContext());
	result = depth_shader_->Render(directx_device_->GetDeviceContext(), m_CubeModel->GetIndexCount(), worldMatrix, lightViewMatrix, lightOrthoMatrix);
	if (!result) {
		return false;
	}

	directx_device_->GetWorldMatrix(worldMatrix);

	m_SphereModel->GetPosition(posX, posY, posZ);
	worldMatrix = XMMatrixTranslation(posX, posY, posZ);

	m_SphereModel->Render(directx_device_->GetDeviceContext());
	result = depth_shader_->Render(directx_device_->GetDeviceContext(), m_SphereModel->GetIndexCount(), worldMatrix, lightViewMatrix, lightOrthoMatrix);
	if (!result) {
		return false;
	}

	directx_device_->GetWorldMatrix(worldMatrix);

	m_GroundModel->GetPosition(posX, posY, posZ);
	worldMatrix = XMMatrixTranslation(posX, posY, posZ);

	m_GroundModel->Render(directx_device_->GetDeviceContext());
	result = depth_shader_->Render(directx_device_->GetDeviceContext(), m_GroundModel->GetIndexCount(), worldMatrix, lightViewMatrix, lightOrthoMatrix);
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

	light_->GenerateViewMatrix();

	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetWorldMatrix(worldMatrix);
	directx_device_->GetProjectionMatrix(projectionMatrix);

	light_->GetViewMatrix(lightViewMatrix);
	light_->GetOrthoMatrix(lightOrthoMatrix);

	m_CubeModel->GetPosition(posX, posY, posZ);
	worldMatrix = XMMatrixTranslation(posX, posY, posZ);

	m_CubeModel->Render(directx_device_->GetDeviceContext());

	result = m_ShadowShader->Render(directx_device_->GetDeviceContext(), m_CubeModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, lightViewMatrix,
		lightOrthoMatrix, m_CubeModel->GetTexture(), render_texture_->GetShaderResourceView(), light_->GetDirection(),
		light_->GetAmbientColor(), light_->GetDiffuseColor());
	if (!result) {
		return false;
	}

	directx_device_->GetWorldMatrix(worldMatrix);

	m_SphereModel->GetPosition(posX, posY, posZ);
	worldMatrix = XMMatrixTranslation(posX, posY, posZ);

	m_SphereModel->Render(directx_device_->GetDeviceContext());
	result = m_ShadowShader->Render(directx_device_->GetDeviceContext(), m_SphereModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, lightViewMatrix,
		lightOrthoMatrix, m_SphereModel->GetTexture(), render_texture_->GetShaderResourceView(), light_->GetDirection(),
		light_->GetAmbientColor(), light_->GetDiffuseColor());
	if (!result) {
		return false;
	}

	directx_device_->GetWorldMatrix(worldMatrix);

	m_GroundModel->GetPosition(posX, posY, posZ);
	worldMatrix = XMMatrixTranslation(posX, posY, posZ);

	m_GroundModel->Render(directx_device_->GetDeviceContext());
	result = m_ShadowShader->Render(directx_device_->GetDeviceContext(), m_GroundModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, lightViewMatrix,
		lightOrthoMatrix, m_GroundModel->GetTexture(), render_texture_->GetShaderResourceView(), light_->GetDirection(),
		light_->GetAmbientColor(), light_->GetDiffuseColor());
	if (!result) {
		return false;
	}

	directx_device_->EndScene();

	return true;
}
