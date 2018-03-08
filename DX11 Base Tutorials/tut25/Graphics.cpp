#include "Graphics.h"

#include <new>

#include "../CommonFramework/TypeDefine.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/Camera.h"

#include "modelclass.h"
#include "translateshaderclass.h"

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

		result = model_->Initialize(L"../../tut25/data/seafloor.dds", "../../tut25/data/triangle.txt");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_TranslateShader = (TranslateShaderClass*)_aligned_malloc(sizeof(TranslateShaderClass), 16);
		new (m_TranslateShader)TranslateShaderClass();
		if (!m_TranslateShader) {
			return false;
		}
		result = m_TranslateShader->Initialize(hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the texture translation shader object.", L"Error", MB_OK);
			return false;
		}
	}

	return true;
}

void GraphicsClass::Shutdown() {
	// Release the texture translation shader object.
	if (m_TranslateShader)
	{
		m_TranslateShader->Shutdown();
		m_TranslateShader->~TranslateShaderClass();
		_aligned_free(m_TranslateShader);
		m_TranslateShader = 0;
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
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;
	static float textureTranslation = 0.0f;


	textureTranslation += 0.01f;
	if (textureTranslation > 1.0f) {
		textureTranslation -= 1.0f;
	}

	directx_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	camera_->SetPosition(0.0f, 0.0f, -10.0f);
	camera_->Render();
	
	directx_device_->GetWorldMatrix(worldMatrix);
	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetProjectionMatrix(projectionMatrix);

	model_->Render(directx_device_->GetDeviceContext());

	result = m_TranslateShader->Render(directx_device_->GetDeviceContext(), model_->GetIndexCount(), worldMatrix, viewMatrix,
		projectionMatrix, model_->GetTexture(), textureTranslation);
	if (!result) {
		return false;
	}

	directx_device_->EndScene();

	return true;
}