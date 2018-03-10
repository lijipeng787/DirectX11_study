#include "Graphics.h"

#include <new>

#include "../CommonFramework/TypeDefine.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/Camera.h"

#include "modelclass.h"
#include "rendertextureclass.h"
#include "textureshaderclass.h"
#include "glassshaderclass.h"

GraphicsClass::GraphicsClass() {}

GraphicsClass::~GraphicsClass() {}

bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd) {

	bool result;
	XMMATRIX baseViewMatrix;

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
		camera_->GetViewMatrix(baseViewMatrix);
	}

	{
		model_ = new ModelClass();
		if (!model_) {
			return false;
		}
		result = model_->Initialize("../../tut32/data/cube.txt", L"../../tut32/data/seafloor.dds", L"../../tut32/data/bump03.dds");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
			return false;
		}

		m_WindowModel = new ModelClass();
		if (!m_WindowModel) {
			return false;
		}
		result = m_WindowModel->Initialize("../../tut32/data/square.txt", L"../../tut32/data/glass01.dds", L"../../tut32/data/bump03.dds");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the window model object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_RenderTexture = new RenderTextureClass();
		if (!m_RenderTexture) {
			return false;
		}
		result = m_RenderTexture->Initialize(screenWidth, screenHeight);
		if (!result) {
			return false;
		}
	}

	{
		texture_shader_ = (TextureShaderClass*)_aligned_malloc(sizeof(TextureShaderClass), 16);
		new (texture_shader_)TextureShaderClass();
		if (!texture_shader_) {
			return false;
		}
		result = texture_shader_->Initialize(hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the texture shader object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_GlassShader = (GlassShaderClass*)_aligned_malloc(sizeof(GlassShaderClass), 16);
		new (m_GlassShader)GlassShaderClass();
		if (!m_GlassShader) {
			return false;
		}
		result = m_GlassShader->Initialize(hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the glass shader object.", L"Error", MB_OK);
			return false;
		}
	}

	return true;
}

void GraphicsClass::Shutdown() {

	// Release the glass shader object.
	if (m_GlassShader)
	{
		m_GlassShader->Shutdown();
		m_GlassShader->~GlassShaderClass();
		_aligned_free(m_GlassShader);
		m_GlassShader = 0;
	}

	
	if (texture_shader_)
	{
		texture_shader_->Shutdown();
		texture_shader_->~TextureShaderClass();
		_aligned_free(texture_shader_);
		texture_shader_ = 0;
	}

	
	if (m_RenderTexture)
	{
		m_RenderTexture->Shutdown();
		delete m_RenderTexture;
		m_RenderTexture = 0;
	}

	// Release the window model object.
	if (m_WindowModel)
	{
		m_WindowModel->Shutdown();
		delete m_WindowModel;
		m_WindowModel = 0;
	}


	if (model_)
	{
		model_->Shutdown();
		delete model_;
		model_ = nullptr;
	}

	if (camera_) {
		camera_->~Camera();
		_aligned_free(camera_);
		camera_ = nullptr;
	}

	
		
		
		
	}
}

bool GraphicsClass::Frame() {

	static float rotation_ = 0.0f;
	bool result;


	// Update the rotation_ variable each frame.
	rotation_ += (float)XM_PI * 0.005f;
	if (rotation_ > 360.0f) {
		rotation_ -= 360.0f;
	}
	rotation_ = rotation_;

	// Set the position of the camera.
	camera_->SetPosition(0.0f, 0.0f, -10.0f);

	// Render the scene to texture first.
	result = RenderToTexture(rotation_);
	if (!result) {
		return false;
	}

	// Render the scene.
	result = Render();
	if (!result) {
		return false;
	}

	return true;
}

bool GraphicsClass::RenderToTexture(float rotation_) {

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;

	m_RenderTexture->SetRenderTarget(directx_device_->GetDeviceContext(), directx_device_->GetDepthStencilView());

	m_RenderTexture->ClearRenderTarget(directx_device_->GetDeviceContext(), directx_device_->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	directx_device_->GetWorldMatrix(worldMatrix);
	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetProjectionMatrix(projectionMatrix);

	worldMatrix = XMMatrixRotationY(rotation_);

	model_->Render(directx_device_->GetDeviceContext());

	result = texture_shader_->Render(directx_device_->GetDeviceContext(), model_->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, model_->GetTexture());
	if (!result) {
		return false;
	}

	directx_device_->SetBackBufferRenderTarget();

	return true;
}

bool GraphicsClass::Render() {

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	float refractionScale;
	bool result;

	refractionScale = 0.01f;

	directx_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	directx_device_->GetWorldMatrix(worldMatrix);
	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetProjectionMatrix(projectionMatrix);

	worldMatrix = XMMatrixRotationY(rotation_);

	model_->Render(directx_device_->GetDeviceContext());

	result = texture_shader_->Render(directx_device_->GetDeviceContext(), model_->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
		model_->GetTexture());
	if (!result) {
		return false;
	}

	directx_device_->GetWorldMatrix(worldMatrix);

	worldMatrix = XMMatrixTranslation(0.0f, 0.0f, -1.5f);

	m_WindowModel->Render(directx_device_->GetDeviceContext());

	result = m_GlassShader->Render(directx_device_->GetDeviceContext(), m_WindowModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
		m_WindowModel->GetTexture(), m_WindowModel->GetNormalMap(), m_RenderTexture->GetShaderResourceView(),
		refractionScale);
	if (!result) {
		return false;
	}

	directx_device_->EndScene();

	return true;
}