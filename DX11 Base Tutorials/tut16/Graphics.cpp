#include "Graphics.h"

#include <new>

#include "../CommonFramework/TypeDefine.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/Camera.h"

#include "Frustum.h"
#include "modellistclass.h"
#include "modelclass.h"
#include "lightclass.h"
#include "lightshaderclass.h"
#include "textureclass.h"
#include "textclass.h"

using namespace DirectX;

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
		text_ = (TextClass*)_aligned_malloc(sizeof(TextClass), 16);
		new (text_)TextClass();
		if (!text_) {
			return false;
		}

		result = text_->Initialize(hwnd, screenWidth, screenHeight, baseViewMatrix);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the text object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		model_ = new ModelClass();
		if (!model_) {
			return false;
		}
		
		result = model_->Initialize("../../tut16/data/sphere.txt", L"../../tut16/data/seafloor.dds");
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

		light_->SetDirection(0.0f, 0.0f, 1.0f);

	}

	{
		model_list_ = new ModelListClass();
		if (!model_list_) {
			return false;
		}

		result = model_list_->Initialize(25);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the model list object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		frustum_ = (FrustumClass*)_aligned_malloc(sizeof(FrustumClass), 16);
		new (frustum_)FrustumClass();
		if (!frustum_) {
			return false;
		}
	}

	return true;
}

void GraphicsClass::Shutdown() {

	if (frustum_) {
		frustum_->~FrustumClass();
		_aligned_free(frustum_);
		frustum_ = 0;
	}

	if (model_list_) {
		model_list_->Shutdown();
		delete model_list_;
		model_list_ = 0;
	}

	if (light_) {
		delete light_;
		light_ = nullptr;;
	}

	if (light_shader_) {
		light_shader_->Shutdown();
		light_shader_->~LightShaderClass();
		_aligned_free(light_shader_);
		light_shader_ = nullptr;;
	}

	if (model_) {
		model_->Shutdown();
		delete model_;
		model_ = nullptr;
	}

	if (text_) {
		text_->Shutdown();
		text_->~TextClass();
		_aligned_free(text_);
		text_ = 0;
	}

	if (camera_) {
		camera_->~Camera();
		_aligned_free(camera_);
		camera_ = nullptr;
	}
}

bool GraphicsClass::Frame() {

	camera_->SetPosition(0.0f, 0.0f, -10.0f);

	camera_->SetRotation(0.0f, rotation_y_, 0.0f);

	return true;
}

bool GraphicsClass::Render() {

	auto directx_device_ = DirectX11Device::GetD3d11DeviceInstance();

	directx_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix;

	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetWorldMatrix(worldMatrix);
	directx_device_->GetProjectionMatrix(projectionMatrix);
	directx_device_->GetOrthoMatrix(orthoMatrix);

	frustum_->ConstructFrustum(SCREEN_DEPTH, projectionMatrix, viewMatrix);

	int modelCount = model_list_->GetModelCount(), renderCount = 0;
	auto renderModel = false;
	XMFLOAT4 color{};
	float positionX, positionY, positionZ, radius;

	for (int index = 0; index < modelCount; index++) {

		model_list_->GetData(index, positionX, positionY, positionZ, color);

		radius = 1.0f;

		renderModel = frustum_->CheckSphere(positionX, positionY, positionZ, radius);

		if (renderModel) {

			worldMatrix = XMMatrixTranslation(positionX, positionY, positionZ);

			model_->Render();

			light_shader_->Render(model_->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
								  model_->GetTexture(), light_->GetDirection(), color);

			directx_device_->GetWorldMatrix(worldMatrix);

			renderCount++;
		}
	}

	auto result = text_->SetRenderCount(renderCount);
	if (!result) {
		return false;
	}

	directx_device_->TurnZBufferOff();

	directx_device_->TurnOnAlphaBlending();

	result = text_->Render(worldMatrix, orthoMatrix);
	if (!result) {
		return false;
	}

	directx_device_->TurnOffAlphaBlending();

	directx_device_->TurnZBufferOn();

	directx_device_->EndScene();

	return true;
}
