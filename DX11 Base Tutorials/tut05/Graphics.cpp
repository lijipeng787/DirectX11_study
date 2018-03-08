#include "Graphics.h"

#include <new>

#include "../CommonFramework/TypeDefine.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/Camera.h"

#include "modelclass.h"
#include "textureshaderclass.h"

GraphicsClass::GraphicsClass() {}

GraphicsClass::~GraphicsClass() {}

bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd){

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
		result = model_->Initialize(L"../../tut05/data/seafloor.dds");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_TextureShader = (TextureShaderClass*)_aligned_malloc(sizeof(TextureShaderClass), 16);
		new (m_TextureShader)TextureShaderClass();
		if (!m_TextureShader) {
			return false;
		}
		result = m_TextureShader->Initialize(hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the color shader object.", L"Error", MB_OK);
			return false;
		}
	}

	return true;
}

void GraphicsClass::Shutdown() {

	if (m_TextureShader) {
		m_TextureShader->Shutdown();
		m_TextureShader->~TextureShaderClass();
		_aligned_free(m_TextureShader);
		m_TextureShader = 0;
	}

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

bool GraphicsClass::Render() {

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;

	directx_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetWorldMatrix(worldMatrix);
	directx_device_->GetProjectionMatrix(projectionMatrix);

	model_->Render(directx_device_->GetDeviceContext());

	result = m_TextureShader->Render(directx_device_->GetDeviceContext(), model_->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
		model_->GetTexture());

	directx_device_->EndScene();

	return true;
}
