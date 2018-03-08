#include "Graphics.h"

#include <new>

#include "../CommonFramework/TypeDefine.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/Camera.h"

#include "textclass.h"

using namespace DirectX;

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

	XMMATRIX baseViewMatrix = {};
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
		m_Text = (TextClass*)_aligned_malloc(sizeof(TextClass), 16);
		new (m_Text)TextClass();
		if (!m_Text) {
			return false;
		}
		
		result = m_Text->Initialize(directx_device_->GetDevice(), directx_device_->GetDeviceContext(), hwnd, screenWidth, screenHeight, baseViewMatrix);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the text object.", L"Error", MB_OK);
			return false;
		}
	}

	return true;
}

void GraphicsClass::Shutdown() {

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

	bool result = Render();
	if (!result) {
		return false;
	}

	return true;
}

bool GraphicsClass::Render() {

	XMMATRIX worldMatrix, viewMatrix, orthoMatrix, projectionMatrix;
	bool result;

	directx_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetWorldMatrix(worldMatrix);
	directx_device_->GetOrthoMatrix(orthoMatrix);

	directx_device_->TurnZBufferOff();

	directx_device_->TurnOnAlphaBlending();

	result = m_Text->Render(directx_device_->GetDeviceContext(), worldMatrix, orthoMatrix);
	if (!result){
		return false;
	}

	directx_device_->TurnOffAlphaBlending();

	directx_device_->TurnZBufferOn();

	directx_device_->EndScene();

	return true;
}
