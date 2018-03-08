#include "Graphics.h"

#include <new>

#include "../CommonFramework/TypeDefine.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/Camera.h"

#include "modelclass.h"
#include "fogshaderclass.h"

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
		camera_->SetPosition(0.0f, 0.0f, -1.0f);
		camera_->Render();
		camera_->GetViewMatrix(baseViewMatrix);
	}

	{
		model_ = new ModelClass();
		if (!model_) {
			return false;
		}

		result = model_->Initialize(
			directx_device_->GetDevice(),
			L"../../tut23/data/seafloor.dds",
			"../../tut23/data/cube.txt");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_FogShader = (FogShaderClass*)_aligned_malloc(sizeof(FogShaderClass), 16);
		new (m_FogShader)FogShaderClass();
		if (!m_FogShader) {
			return false;
		}

		result = m_FogShader->Initialize(hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the fog shader object.", L"Error", MB_OK);
			return false;
		}
	}

	return true;
}

void GraphicsClass::Shutdown() {

	// Release the fog shader object.
	if (m_FogShader)
	{
		m_FogShader->Shutdown();
		m_FogShader->~FogShaderClass();
		_aligned_free(m_FogShader);
		m_FogShader = 0;
	}


	if (model_)
	{
		model_->Shutdown();
		delete model_;
		model_ = 0;
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

	result = Render();
	if (!result) {
		return false;
	}

	return true;
}

bool GraphicsClass::Render() {

	float fogColor, fogStart, fogEnd;
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;
	static float rotation = 0.0f;

	camera_->SetPosition(0.0f, 0.0f, -10.0f);


	fogColor = 0.5f;

	fogStart = 0.0f;
	fogEnd = 10.0f;

	directx_device_->BeginScene(fogColor, fogColor, fogColor, 1.0f);

	camera_->Render();
	directx_device_->GetWorldMatrix(worldMatrix);
	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetProjectionMatrix(projectionMatrix);
	rotation += (float)XM_PI * 0.005f;
	if (rotation > 360.0f) {
		rotation -= 360.0f;
	}

	worldMatrix = XMMatrixRotationY(rotation);

	model_->Render(directx_device_->GetDeviceContext());
	result = m_FogShader->Render(directx_device_->GetDeviceContext(), model_->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
		model_->GetTexture(), fogStart, fogEnd);
	if (!result) {
		return false;
	}

	directx_device_->EndScene();

	return true;
}