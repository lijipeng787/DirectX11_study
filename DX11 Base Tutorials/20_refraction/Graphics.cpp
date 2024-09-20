#include "Graphics.h"

#include <new>

#include "../CommonFramework/TypeDefine.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/Camera.h"

#include "modelclass.h"
#include "rendertextureclass.h"
#include "lightshaderclass.h"
#include "lightclass.h"
#include "refractionshaderclass.h"
#include "watershaderclass.h"

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
		camera_->SetPosition(0.0f, 0.0f, -10.0f);
		camera_->Render();
		camera_->GetViewMatrix(baseViewMatrix);
	}

	{
		ground_model_ = new ModelClass();
		if (!ground_model_) {
			return false;
		}
		result = ground_model_->Initialize(L"data/ground01.dds", "data/ground.txt");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the ground model object.", L"Error", MB_OK);
			return false;
		}

		wall_model_ = new ModelClass();
		if (!wall_model_) {
			return false;
		}
		result = wall_model_->Initialize(L"data/wall01.dds", "data/wall.txt");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the wall model object.", L"Error", MB_OK);
			return false;
		}

		bath_model_ = new ModelClass();
		if (!bath_model_) {
			return false;
		}
		result = bath_model_->Initialize(L"data/marble01.dds", "data/bath.txt");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the bath model object.", L"Error", MB_OK);
			return false;
		}

		water_model_ = new ModelClass();
		if (!water_model_) {
			return false;
		}
		result = water_model_->Initialize(L"data/water01.dds", "data/water.txt");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the water model object.", L"Error", MB_OK);
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
		light_->SetDirection(0.0f, -1.0f, 0.5f);

	}

	{
		refraction_texture_ = new RenderTextureClass();
		if (!refraction_texture_) {
			return false;
		}
		result = refraction_texture_->Initialize(screenWidth, screenHeight);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the refraction render to texture object.", L"Error", MB_OK);
			return false;
		}

	}

	{
		reflection_texture_ = new RenderTextureClass();
		if (!reflection_texture_) {
			return false;
		}
		result = reflection_texture_->Initialize(screenWidth, screenHeight);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the reflection render to texture object.", L"Error", MB_OK);
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
		refraction_shader_ = (RefractionShaderClass*)_aligned_malloc(sizeof(RefractionShaderClass), 16);
		new (refraction_shader_)RefractionShaderClass();
		if (!refraction_shader_) {
			return false;
		}
		result = refraction_shader_->Initialize(hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the refraction shader object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		water_shader_ = (WaterShaderClass*)_aligned_malloc(sizeof(WaterShaderClass), 16);
		new (water_shader_)WaterShaderClass();
		if (!water_shader_) {
			return false;
		}
		result = water_shader_->Initialize(hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the water shader object.", L"Error", MB_OK);
			return false;
		}

		water_height_ = 2.75f;

		water_translation_ = 0.0f;

	}
	return true;
}

void GraphicsClass::Shutdown() {

	if (water_shader_) {
		water_shader_->Shutdown();
		water_shader_->~WaterShaderClass();
		_aligned_free(water_shader_);
		water_shader_ = 0;
	}

	if (refraction_shader_) {
		refraction_shader_->Shutdown();
		refraction_shader_->~RefractionShaderClass();
		_aligned_free(refraction_shader_);
		refraction_shader_ = 0;
	}

	if (light_shader_) {
		light_shader_->Shutdown();
		light_shader_->~LightShaderClass();
		_aligned_free(light_shader_);
		light_shader_ = nullptr;;
	}

	if (reflection_texture_) {
		reflection_texture_->Shutdown();
		delete reflection_texture_;
		reflection_texture_ = 0;
	}

	if (refraction_texture_) {
		refraction_texture_->Shutdown();
		delete refraction_texture_;
		refraction_texture_ = 0;
	}

	if (light_) {
		delete light_;
		light_ = nullptr;;
	}

	if (water_model_) {
		water_model_->Shutdown();
		delete water_model_;
		water_model_ = 0;
	}

	if (bath_model_) {
		bath_model_->Shutdown();
		delete bath_model_;
		bath_model_ = 0;
	}

	if (wall_model_) {
		wall_model_->Shutdown();
		delete wall_model_;
		wall_model_ = 0;
	}

	if (ground_model_) {
		ground_model_->Shutdown();
		delete ground_model_;
		ground_model_ = 0;
	}

	if (camera_) {
		camera_->~Camera();
		_aligned_free(camera_);
		camera_ = nullptr;
	}
}

bool GraphicsClass::Frame() {

	// Update the position of the water to simulate motion.
	water_translation_ += 0.001f;
	if (water_translation_ > 1.0f) {
		water_translation_ -= 1.0f;
	}

	// Set the position and rotation_ of the camera.
	camera_->SetPosition(-10.0f, 6.0f, -10.0f);
	camera_->SetRotation(0.0f, 45.0f, 0.0f);

	return true;
}

bool GraphicsClass::Render() {

	bool result;

	result = RenderRefractionToTexture();
	if (!result) {
		return false;
	}

	result = RenderReflectionToTexture();
	if (!result) {
		return false;
	}

	result = RenderScene();
	if (!result) {
		return false;
	}

	return true;
}

bool GraphicsClass::RenderRefractionToTexture() {

	bool result;

	auto clipPlane = XMFLOAT4(0.0f, -1.0f, 0.0f, water_height_ + 0.1f);

	auto directx_device = DirectX11Device::GetD3d11DeviceInstance();

	refraction_texture_->SetRenderTarget(
		directx_device->GetDepthStencilView()
	);

	refraction_texture_->ClearRenderTarget(
		directx_device->GetDepthStencilView(),
		0.0f, 0.0f, 0.0f, 1.0f
	);

	camera_->Render();

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;

	directx_device->GetWorldMatrix(worldMatrix);
	camera_->GetViewMatrix(viewMatrix);
	directx_device->GetProjectionMatrix(projectionMatrix);

	worldMatrix = XMMatrixTranslation(0.0f, 2.0f, 0.0f);

	bath_model_->Render();

	result = refraction_shader_->Render(
		bath_model_->GetIndexCount(),
		worldMatrix, viewMatrix, projectionMatrix,
		bath_model_->GetTexture(),
		light_->GetDirection(), light_->GetAmbientColor(), light_->GetDiffuseColor(),
		clipPlane
	);
	if (!result) {
		return false;
	}

	directx_device->SetBackBufferRenderTarget();

	return true;
}

bool GraphicsClass::RenderReflectionToTexture() {

	auto directx_device = DirectX11Device::GetD3d11DeviceInstance();

	reflection_texture_->SetRenderTarget(
		directx_device->GetDepthStencilView()
	);

	reflection_texture_->ClearRenderTarget(
		directx_device->GetDepthStencilView(),
		0.0f, 0.0f, 0.0f, 1.0f
	);

	camera_->RenderReflection(water_height_);

	XMMATRIX reflectionViewMatrix, worldMatrix, projectionMatrix;

	reflectionViewMatrix = camera_->GetReflectionViewMatrix();
	directx_device->GetWorldMatrix(worldMatrix);
	directx_device->GetProjectionMatrix(projectionMatrix);

	worldMatrix = XMMatrixTranslation(0.0f, 6.0f, 8.0f);

	wall_model_->Render();

	auto result = light_shader_->Render(
		wall_model_->GetIndexCount(),
		worldMatrix, reflectionViewMatrix, projectionMatrix,
		wall_model_->GetTexture(),
		light_->GetDirection(), light_->GetAmbientColor(), light_->GetDiffuseColor()
	);
	if (!result) {
		return false;
	}

	directx_device->SetBackBufferRenderTarget();

	return true;
}

bool GraphicsClass::RenderScene() {

	auto directx_device = DirectX11Device::GetD3d11DeviceInstance();

	directx_device->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, reflectionMatrix;

	directx_device->GetWorldMatrix(worldMatrix);
	camera_->GetViewMatrix(viewMatrix);
	directx_device->GetProjectionMatrix(projectionMatrix);

	worldMatrix = XMMatrixTranslation(0.0f, 1.0f, 0.0f);

	ground_model_->Render();

	auto result = light_shader_->Render(
		ground_model_->GetIndexCount(),
		worldMatrix, viewMatrix, projectionMatrix,
		ground_model_->GetTexture(),
		light_->GetDirection(), light_->GetAmbientColor(), light_->GetDiffuseColor()
	);
	if (!result) {
		return false;
	}

	directx_device->GetWorldMatrix(worldMatrix);

	worldMatrix = XMMatrixTranslation(0.0f, 6.0f, 8.0f);

	wall_model_->Render();

	result = light_shader_->Render(
		wall_model_->GetIndexCount(),
		worldMatrix, viewMatrix, projectionMatrix,
		wall_model_->GetTexture(),
		light_->GetDirection(), light_->GetAmbientColor(), light_->GetDiffuseColor()
	);
	if (!result) {
		return false;
	}

	directx_device->GetWorldMatrix(worldMatrix);

	worldMatrix = XMMatrixTranslation(0.0f, 2.0f, 0.0f);

	bath_model_->Render();

	result = light_shader_->Render(
		bath_model_->GetIndexCount(),
		worldMatrix, viewMatrix, projectionMatrix,
		bath_model_->GetTexture(),
		light_->GetDirection(), light_->GetAmbientColor(), light_->GetDiffuseColor()
	);
	if (!result) {
		return false;
	}

	directx_device->GetWorldMatrix(worldMatrix);

	reflectionMatrix = camera_->GetReflectionViewMatrix();

	worldMatrix = XMMatrixTranslation(0.0f, water_height_, 0.0f);

	water_model_->Render();

	result = water_shader_->Render(
		water_model_->GetIndexCount(),
		worldMatrix, viewMatrix, projectionMatrix, reflectionMatrix,
		reflection_texture_->GetShaderResourceView(),
		refraction_texture_->GetShaderResourceView(),
		water_model_->GetTexture(),
		water_translation_,
		0.01f
	);
	if (!result) {
		return false;
	}

	directx_device->EndScene();

	return true;
}