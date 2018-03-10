#include "Graphics.h"

#include <new>

#include "../CommonFramework/TypeDefine.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/Camera.h"

#include "modelclass.h"
#include "lightclass.h"
#include "lightshaderclass.h"

using namespace DirectX;

float GraphicsClass::rotation_ = 0.0f;

bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd) {

	bool result;

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
		camera_->SetPosition(0.0f, 0.0f, -10.0f);
	}

	{
		model_ = new ModelClass();
		if (!model_) {
			return false;
		}
		result = model_->Initialize("../../5_box_with_specular_light/data/cube.txt", L"../../5_box_with_specular_light/data/seafloor.dds");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
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

	{
		light_ = new LightClass();
		if (!light_) {
			return false;
		}
		light_->SetAmbientColor(0.15f, 0.15f, 0.15f, 1.0f);
		light_->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
		light_->SetDirection(0.0f, 0.0f, 1.0f);
		light_->SetSpecularColor(1.0f, 1.0f, 1.0f, 1.0f);
		light_->SetSpecularPower(32.0f);
	}

	return true;
}

void GraphicsClass::Shutdown() {

	if (light_) {
		delete light_;
		light_ = nullptr;
	}

	if (light_shader_) {
		light_shader_->Shutdown();
		light_shader_->~LightShaderClass();
		_aligned_free(light_shader_);
		light_shader_ = nullptr;
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
}

bool GraphicsClass::Frame() {

	rotation_ += (float)XM_PI * 0.01f;
	if (rotation_ > 360.0f) {
		rotation_ -= 360.0f;
	}

	bool result = Render();
	if (!result) {
		return false;
	}

	return true;
}

bool GraphicsClass::Render() {

	XMMATRIX worldMatrix{}, viewMatrix{}, projectionMatrix{};

	auto directx_device_ = DirectX11Device::GetD3d11DeviceInstance();
	
	directx_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetWorldMatrix(worldMatrix);
	directx_device_->GetProjectionMatrix(projectionMatrix);

	worldMatrix = XMMatrixRotationY(rotation_);

	model_->Render();

	auto result = light_shader_->Render(model_->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
		model_->GetTexture(), light_->GetDirection(), light_->GetAmbientColor(), light_->GetDiffuseColor(),
		camera_->GetPosition(), light_->GetSpecularColor(), light_->GetSpecularPower());
	if (!result) {
		return false;
	}

	directx_device_->EndScene();

	return true;
}
