#include "Graphics.h"

#include <new>

#include "../CommonFramework/TypeDefine.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/Camera.h"

#include "modelclass.h"
#include "lightclass.h"
#include "lightshaderclass.h"

GraphicsClass::GraphicsClass() {}

GraphicsClass::~GraphicsClass() {}

float GraphicsClass::rotation = 0.0f;

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
	}

	{
		model_ = new ModelClass();
		if (!model_) {
			return false;
		}
		result = model_->Initialize(directx_device_->GetDevice(), "../../tut07/data/cube.txt", L"../../tut05/data/seafloor.dds");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_LightShader = (LightShaderClass*)_aligned_malloc(sizeof(LightShaderClass), 16);
		new (m_LightShader)LightShaderClass();
		if (!m_LightShader) {
			return false;
		}

		result = m_LightShader->Initialize(directx_device_->GetDevice(), hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the light shader object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_Light = new LightClass();
		if (!m_Light) {
			return false;
		}
		m_Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
		m_Light->SetDirection(0.0f, 0.0f, 1.0f);
	}

	return true;
}

void GraphicsClass::Shutdown() {

	if (m_Light) {
		delete m_Light;
		m_Light = nullptr;
	}

	if (m_LightShader) {
		m_LightShader->Shutdown();
		m_LightShader->~LightShaderClass();
		_aligned_free(m_LightShader);
		m_LightShader = nullptr;
	}

	if (model_) {
		model_->Shutdown();
		delete model_;
		model_ = nullptr;
	}

	if (camera_) {
		camera_->~Camera();
		_aligned_free(camera_);
		camera_ = nullptr;
	}

	if (directx_device_) {
		directx_device_->Shutdown();
		directx_device_->~DirectX11Device();
		delete directx_device_;
		directx_device_ = nullptr;
	}
}

bool GraphicsClass::Frame() {

	rotation += (float)XM_PI * 0.01f;
	if (rotation > 360.0f) {
		rotation -= 360.0f;
	}

	bool result = Render();
	if (!result) {
		return false;
	}

	return true;
}

bool GraphicsClass::Render() {

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;

	directx_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetWorldMatrix(worldMatrix);
	directx_device_->GetProjectionMatrix(projectionMatrix);

	worldMatrix = XMMatrixRotationY(rotation);

	model_->Render(directx_device_->GetDeviceContext());

	result = m_LightShader->Render(directx_device_->GetDeviceContext(), model_->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
		model_->GetTexture(), m_Light->GetDirection(), m_Light->GetDiffuseColor());

	directx_device_->EndScene();

	return true;
}
