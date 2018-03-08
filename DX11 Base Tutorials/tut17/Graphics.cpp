#include "Graphics.h"

#include <new>

#include "../CommonFramework/TypeDefine.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/Camera.h"

#include "modelclass.h"
#include "multitextureshaderclass.h"

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
	}

	{
		model_ = new ModelClass();
		if (!model_) {
			return false;
		}
		result = model_->Initialize(
			directx_device_->GetDevice(),
			"../../tut17/data/square.txt",
			L"../../tut17/data/stone01.dds",
			L"../../tut17/data/dirt01.dds");

		if (!result) {
			MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_MultiTextureShader = (MultiTextureShaderClass*)_aligned_malloc(sizeof(MultiTextureShaderClass), 16);
		new (m_MultiTextureShader)MultiTextureShaderClass();
		if (!m_MultiTextureShader)
		{
			return false;
		}

		result = m_MultiTextureShader->Initialize(hwnd);
		if (!result)
		{
			MessageBox(hwnd, L"Could not initialize the multitexture shader object.", L"Error", MB_OK);
			return false;
		}
	}

	return true;
}

void GraphicsClass::Shutdown() {

	if (model_) {
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
		directx_device_->~DirectX11Device();
		_aligned_free(directx_device_);
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

bool GraphicsClass::Render()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix;

	camera_->SetPosition(0.0f, 0.0f, -5.0f);

	directx_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	directx_device_->GetWorldMatrix(worldMatrix);
	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetProjectionMatrix(projectionMatrix);
	directx_device_->GetOrthoMatrix(orthoMatrix);

	model_->Render(directx_device_->GetDeviceContext());

	m_MultiTextureShader->Render(directx_device_->GetDeviceContext(), model_->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
								 model_->GetTextureArray());

	directx_device_->EndScene();

	return true;
}
