#include "Graphics.h"

#include <new>

#include "../CommonFramework/TypeDefine.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/Camera.h"

#include "lightclass.h"
#include "modelclass.h"
#include "orthowindowclass.h"
#include "deferredbuffersclass.h"
#include "deferredshaderclass.h"
#include "lightshaderclass.h"

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
		camera_->RenderBaseViewMatrix();
	}

	{
		light_ = new LightClass;
		if (!light_) {
			return false;
		}
		light_->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
		light_->SetDirection(0.0f, 0.0f, 1.0f);
	}

	{
		model_ = new ModelClass;
		if (!model_) {
			return false;
		}
		result = model_->Initialize("../../tut50/data/cube.txt", L"../../tut50/data/seafloor.dds");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_FullScreenWindow = new OrthoWindowClass;
		if (!m_FullScreenWindow) {
			return false;
		}
		result = m_FullScreenWindow->Initialize(screenWidth, screenHeight);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the full screen ortho window object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_DeferredBuffers = new DeferredBuffersClass;
		if (!m_DeferredBuffers) {
			return false;
		}
		result = m_DeferredBuffers->Initialize(screenWidth, screenHeight, SCREEN_DEPTH, SCREEN_NEAR);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the deferred buffers object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_DeferredShader = (DeferredShaderClass*)_aligned_malloc(sizeof(DeferredShaderClass), 16);
		new (m_DeferredShader)DeferredShaderClass();
		if (!m_DeferredShader) {
			return false;
		}
		result = m_DeferredShader->Initialize(hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the deferred shader object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		light_shader_ = (LightShaderClass*)_aligned_malloc(sizeof(LightShaderClass), 16);
		new (light_shader_)LightShaderClass();
		if (!light_shader_) {
			return false;
		}
		result = light_shader_->Initialize(hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the light shader object.", L"Error", MB_OK);
			return false;
		}
	}

	return true;
}

void GraphicsClass::Shutdown() {

	if (light_shader_) {
		light_shader_->Shutdown();
		light_shader_->~LightShaderClass();
		_aligned_free(light_shader_);
		light_shader_ = 0;
	}

	// Release the deferred shader object.
	if (m_DeferredShader) {
		m_DeferredShader->Shutdown();
		m_DeferredShader->~DeferredShaderClass();
		_aligned_free(m_DeferredShader);
		m_DeferredShader = 0;
	}

	// Release the deferred buffers object.
	if (m_DeferredBuffers) {
		m_DeferredBuffers->Shutdown();
		delete m_DeferredBuffers;
		m_DeferredBuffers = 0;
	}

	buffer_number
	if (m_FullScreenWindow) {
		m_FullScreenWindow->Shutdown();
		delete m_FullScreenWindow;
		m_FullScreenWindow = 0;
	}


	if (model_) {
		model_->Shutdown();
		delete model_;
		model_ = 0;
	}

	// Release the light object.
	if (light_) {
		delete light_;
		light_ = 0;
	}
}

bool GraphicsClass::Frame() {

	auto result = Render();
	if (!result) {
		return false;
	}

	return true;
}

bool GraphicsClass::Render() {

	bool result;
	XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;

	result = RenderSceneToTexture();
	if (!result) {
		return false;
	}

	directx_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	directx_device_->GetWorldMatrix(worldMatrix);
	camera_->GetBaseViewMatrix(baseViewMatrix);
	directx_device_->GetOrthoMatrix(orthoMatrix);

	directx_device_->TurnZBufferOff();

	m_FullScreenWindow->Render(directx_device_->GetDeviceContext());

	light_shader_->Render(directx_device_->GetDeviceContext(), m_FullScreenWindow->GetIndexCount(), worldMatrix, baseViewMatrix, orthoMatrix,
		m_DeferredBuffers->GetShaderResourceView(0), m_DeferredBuffers->GetShaderResourceView(1),
		light_->GetDirection());

	directx_device_->TurnZBufferOn();

	directx_device_->EndScene();

	return true;
}

bool GraphicsClass::RenderSceneToTexture() {

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;

	m_DeferredBuffers->SetRenderTargets(directx_device_->GetDeviceContext());

	m_DeferredBuffers->ClearRenderTargets(directx_device_->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	directx_device_->GetWorldMatrix(worldMatrix);
	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetProjectionMatrix(projectionMatrix);

	static float rotation = 0.0f;
	rotation += (float)XM_PI * 0.01f;
	if (rotation > 360.0f) {
		rotation -= 360.0f;
	}

	worldMatrix = XMMatrixRotationY(rotation);

	model_->Render(directx_device_->GetDeviceContext());

	m_DeferredShader->Render(directx_device_->GetDeviceContext(), model_->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, model_->GetTexture());

	directx_device_->SetBackBufferRenderTarget();

	directx_device_->ResetViewport();

	return true;
}