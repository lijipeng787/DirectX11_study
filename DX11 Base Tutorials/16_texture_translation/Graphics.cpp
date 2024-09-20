#include "Graphics.h"

#include <new>

#include "../CommonFramework/TypeDefine.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/Camera.h"

#include "modelclass.h"
#include "translateshaderclass.h"

using namespace DirectX;

GraphicsClass::GraphicsClass() {}

GraphicsClass::~GraphicsClass() {}

bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd) {

	bool result;
	XMMATRIX baseViewMatrix;

	{
		auto directx11_device_ = DirectX11Device::GetD3d11DeviceInstance();

		auto result = directx11_device_->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
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

		result = model_->Initialize(L"data/seafloor.dds", "data/triangle.txt");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		translation_shader_ = (TranslateShaderClass*)_aligned_malloc(sizeof(TranslateShaderClass), 16);
		new (translation_shader_)TranslateShaderClass();
		if (!translation_shader_) {
			return false;
		}
		result = translation_shader_->Initialize(hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the texture translation shader object.", L"Error", MB_OK);
			return false;
		}
	}

	return true;
}

void GraphicsClass::Shutdown() {

	if (translation_shader_)
	{
		translation_shader_->Shutdown();
		translation_shader_->~TranslateShaderClass();
		_aligned_free(translation_shader_);
		translation_shader_ = nullptr;
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

	static float textureTranslation = 0.0f;

	textureTranslation += 0.01f;
	if (textureTranslation > 1.0f) {
		textureTranslation -= 1.0f;
	}

	auto directx_device_ = DirectX11Device::GetD3d11DeviceInstance();

	directx_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	camera_->SetPosition(0.0f, 0.0f, -10.0f);
	camera_->Render();
	
	directx_device_->GetWorldMatrix(worldMatrix);
	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetProjectionMatrix(projectionMatrix);

	model_->Render();

	auto result = translation_shader_->Render(model_->GetIndexCount(), worldMatrix, viewMatrix,
		projectionMatrix, model_->GetTexture(), textureTranslation);
	if (!result) {
		return false;
	}

	directx_device_->EndScene();

	return true;
}