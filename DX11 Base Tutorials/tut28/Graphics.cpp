#include "Graphics.h"

#include <new>

#include "../CommonFramework/TypeDefine.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/Camera.h"

#include "modelclass.h"
#include "textureshaderclass.h"
#include "rendertextureclass.h"
#include "fadeshaderclass.h"
#include "bitmapclass.h"

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
		model_ = new ModelClass();
		if (!model_) {
			return false;
		}
		result = model_->Initialize(L"../../tut28/data/seafloor.dds", "../../tut28/data/cube.txt");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		texture_shader_ = (TextureShaderClass*)_aligned_malloc(sizeof(TextureShaderClass), 16);
		new (texture_shader_)TextureShaderClass();
		if (!texture_shader_) {
			return false;
		}
		result = texture_shader_->Initialize(hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the texture shader object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		render_texture_ = new RenderTextureClass();
		if (!render_texture_) {
			return false;
		}
		result = render_texture_->Initialize(screenWidth, screenHeight);
		if (!result) {
			return false;
		}
	}

	{
		bitmap_ = new SimpleMoveableSurface();
		if (!bitmap_) {
			return false;
		}
		result = bitmap_->Initialize(screenWidth, screenHeight, screenWidth, screenHeight);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the bitmap object.", L"Error", MB_OK);
			return false;
		}

	}

	{
		fadein_time_ = 7000.0f;

		fade_shader_ = (FadeShaderClass*)_aligned_malloc(sizeof(FadeShaderClass), 16);
		new (fade_shader_)FadeShaderClass();
		if (!fade_shader_) {
			return false;
		}
		result = fade_shader_->Initialize(hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the fade shader object.", L"Error", MB_OK);
			return false;
		}
	}

	return true;
}

void GraphicsClass::Shutdown() {

	if (fade_shader_) {
		fade_shader_->Shutdown();
		fade_shader_->~FadeShaderClass();
		_aligned_free(fade_shader_);
		fade_shader_ = 0;
	}

	if (bitmap_) {
		bitmap_->Shutdown();
		delete bitmap_;
		bitmap_ = 0;
	}

	if (render_texture_) {
		render_texture_->Shutdown();
		delete render_texture_;
		render_texture_ = 0;
	}

	if (texture_shader_) {
		texture_shader_->Shutdown();
		texture_shader_->~TextureShaderClass();
		_aligned_free(texture_shader_);
		texture_shader_ = 0;
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

void GraphicsClass::SetFameTime(float frame_time) {
	frame_time_ = frame_time;

}

bool GraphicsClass::Frame() {

	if (!is_fade_done_) {
		// Update the accumulated time with the extra frame time addition.
		accumulated_time_ += frame_time_;

		// While the time goes on increase the fade in amount by the time that is passing each frame.
		if (accumulated_time_ < fadein_time_) {
			// Calculate the percentage that the screen should be faded in based on the accumulated time.
			fade_percentage_ = accumulated_time_ / fadein_time_;
		}
		else {
			// If the fade in time is complete then turn off the fade effect and render the scene normally.
			is_fade_done_ = true;

			// Set the percentage to 100%.
			fade_percentage_ = 1.0f;
		}
	}

	camera_->SetPosition(0.0f, 0.0f, -10.0f);

	return true;
}

bool GraphicsClass::Render() {

	static float rotation_ = 0.0f;

	rotation_ += (float)XM_PI * 0.005f;
	if (rotation_ > 360.0f) {
		rotation_ -= 360.0f;
	}

	if (is_fade_done_) {

		RenderNormalScene(rotation_);
	}
	else {

		auto result = RenderToTexture(rotation_);
		if (!result) {
			return false;
		}

		result = RenderFadingScene();
		if (!result) {
			return false;
		}
	}

	return true;
}

bool GraphicsClass::RenderToTexture(float rotation_) {

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;

	auto directx_device = DirectX11Device::GetD3d11DeviceInstance();

	render_texture_->SetRenderTarget(directx_device->GetDepthStencilView());

	render_texture_->ClearRenderTarget(directx_device->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	directx_device->GetWorldMatrix(worldMatrix);
	camera_->GetViewMatrix(viewMatrix);
	directx_device->GetProjectionMatrix(projectionMatrix);

	worldMatrix = XMMatrixRotationY(rotation_);

	model_->Render();

	auto result = texture_shader_->Render(model_->GetIndexCount(), worldMatrix, viewMatrix,
										  projectionMatrix, model_->GetTexture());
	if (!result) {
		return false;
	}

	directx_device->SetBackBufferRenderTarget();

	return true;
}

bool GraphicsClass::RenderFadingScene() {

	XMMATRIX worldMatrix, viewMatrix, orthoMatrix;

	auto directx_device = DirectX11Device::GetD3d11DeviceInstance();

	directx_device->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	directx_device->GetWorldMatrix(worldMatrix);
	camera_->GetViewMatrix(viewMatrix);
	directx_device->GetOrthoMatrix(orthoMatrix);

	directx_device->TurnZBufferOff();

	auto result = bitmap_->Render(0, 0);
	if (!result) {
		return false;
	}

	result = fade_shader_->Render(bitmap_->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix,
								  render_texture_->GetShaderResourceView(), fade_percentage_);
	if (!result) {
		return false;
	}

	directx_device->TurnZBufferOn();

	directx_device->EndScene();

	return true;
}

bool GraphicsClass::RenderNormalScene(float rotation_) {

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;

	auto directx_device = DirectX11Device::GetD3d11DeviceInstance();

	directx_device->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	directx_device->GetWorldMatrix(worldMatrix);
	camera_->GetViewMatrix(viewMatrix);
	directx_device->GetProjectionMatrix(projectionMatrix);

	worldMatrix = XMMatrixRotationY(rotation_);

	model_->Render();

	auto result = texture_shader_->Render(model_->GetIndexCount(), worldMatrix, viewMatrix,
										  projectionMatrix, model_->GetTexture());
	if (!result) {
		return false;
	}

	directx_device->EndScene();

	return true;
}