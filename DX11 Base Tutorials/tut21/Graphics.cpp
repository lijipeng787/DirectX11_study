#include "Graphics.h"

#include <new>

#include "../CommonFramework/TypeDefine.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/Camera.h"

#include "modelclass.h"
#include "texturearrayclass.h"
#include "lightclass.h"
#include "specmapshaderclass.h"

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
		
		result = model_->Initialize(
			"../../tut21/data/cube.txt",
			L"../../tut21/data/stone02.dds",
			L"../../tut21/data/bump02.dds",
			L"../../tut21/data/spec02.dds"
		);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		specularmap_shader_ = (SpecMapShaderClass*)_aligned_malloc(sizeof(SpecMapShaderClass), 16);
		new (specularmap_shader_)SpecMapShaderClass();
		if (!specularmap_shader_) {
			return false;
		}

		result = specularmap_shader_->Initialize(hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the specular map shader object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		light_ = new LightClass();
		if (!light_) {
			return false;
		}

		light_->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
		light_->SetDirection(0.0f, 0.0f, 1.0f);
		light_->SetSpecularColor(1.0f, 1.0f, 1.0f, 1.0f);
		light_->SetSpecularPower(16.0f);
	}

	return true;
}

void GraphicsClass::Shutdown()
{

	if (light_)
	{
		delete light_;
		light_ = nullptr;;
	}

	if (specularmap_shader_)
	{
		specularmap_shader_->Shutdown();
		specularmap_shader_->~SpecMapShaderClass();
		_aligned_free(specularmap_shader_);
		specularmap_shader_ = 0;
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

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix;
	static float rotation_ = 0.0f;

	camera_->SetPosition(0.0f, 0.0f, -5.0f);

	auto directx_device_ = DirectX11Device::GetD3d11DeviceInstance();

	directx_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	directx_device_->GetWorldMatrix(worldMatrix);
	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetProjectionMatrix(projectionMatrix);
	directx_device_->GetOrthoMatrix(orthoMatrix);

	rotation_ += (float)XM_PI * 0.0025f;
	if (rotation_ > 360.0f) {
		rotation_ -= 360.0f;
	}

	worldMatrix = XMMatrixRotationY(rotation_);

	model_->Render();

	specularmap_shader_->Render(
		model_->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
		model_->GetTextureArray(), light_->GetDirection(), light_->GetDiffuseColor(),
		camera_->GetPosition(), light_->GetSpecularColor(), light_->GetSpecularPower());
	directx_device_->EndScene();

	return true;
}