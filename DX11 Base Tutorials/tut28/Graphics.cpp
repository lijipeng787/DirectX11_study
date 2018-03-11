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
		m_Bitmap = new SimpleMoveableSurface();
		if (!m_Bitmap) {
			return false;
		}
		result = m_Bitmap->Initialize(screenWidth, screenHeight, screenWidth, screenHeight);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the bitmap object.", L"Error", MB_OK);
			return false;
		}

	}

	{
		m_fadeInTime = 7000.0f;

		m_FadeShader = (FadeShaderClass*)_aligned_malloc(sizeof(FadeShaderClass), 16);
		new (m_FadeShader)FadeShaderClass();
		if (!m_FadeShader) {
			return false;
		}
		result = m_FadeShader->Initialize(hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the fade shader object.", L"Error", MB_OK);
			return false;
		}
	}

	return true;
}

void GraphicsClass::Shutdown() {
	// Release the fade shader object.
	if (m_FadeShader)
	{
		m_FadeShader->Shutdown();
		m_FadeShader->~FadeShaderClass();
		_aligned_free(m_FadeShader);
		m_FadeShader = 0;
	}

	// Release the bitmap object.
	if (m_Bitmap)
	{
		m_Bitmap->Shutdown();
		delete m_Bitmap;
		m_Bitmap = 0;
	}

	
	if (render_texture_)
	{
		render_texture_->Shutdown();
		delete render_texture_;
		render_texture_ = 0;
	}

	
	if (texture_shader_)
	{
		texture_shader_->Shutdown();
		texture_shader_->~TextureShaderClass();
		_aligned_free(texture_shader_);
		texture_shader_ = 0;
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
}

void GraphicsClass::SetFameTime(float frame_time) {
	frame_time_ = frame_time;

}

bool GraphicsClass::Frame() {

	if (!m_fadeDone) {
		// Update the accumulated time with the extra frame time addition.
		m_accumulatedTime += frame_time_;

		// While the time goes on increase the fade in amount by the time that is passing each frame.
		if (m_accumulatedTime < m_fadeInTime) {
			// Calculate the percentage that the screen should be faded in based on the accumulated time.
			m_fadePercentage = m_accumulatedTime / m_fadeInTime;
		}
		else {
			// If the fade in time is complete then turn off the fade effect and render the scene normally.
			m_fadeDone = true;

			// Set the percentage to 100%.
			m_fadePercentage = 1.0f;
		}
	}

	// Set the position of the camera.
	camera_->SetPosition(0.0f, 0.0f, -10.0f);

	return true;
}

bool GraphicsClass::Render() {

	bool result;
	static float rotation_ = 0.0f;

	rotation_ += (float)XM_PI * 0.005f;
	if (rotation_ > 360.0f) {
		rotation_ -= 360.0f;
	}

	if (m_fadeDone) {

		RenderNormalScene(rotation_);
	}
	else {

		result = RenderToTexture(rotation_);
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
	bool result;

	render_texture_->SetRenderTarget(directx_device_->GetDeviceContext(), directx_device_->GetDepthStencilView());

	render_texture_->ClearRenderTarget(directx_device_->GetDeviceContext(), directx_device_->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	directx_device_->GetWorldMatrix(worldMatrix);
	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetProjectionMatrix(projectionMatrix);

	worldMatrix = XMMatrixRotationY(rotation_);

	model_->Render(directx_device_->GetDeviceContext());

	result = texture_shader_->Render(directx_device_->GetDeviceContext(), model_->GetIndexCount(), worldMatrix, viewMatrix,
		projectionMatrix, model_->GetTexture());
	if (!result) {
		return false;
	}

	directx_device_->SetBackBufferRenderTarget();

	return true;
}

bool GraphicsClass::RenderFadingScene() {

	XMMATRIX worldMatrix, viewMatrix, orthoMatrix;
	bool result;

	directx_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	directx_device_->GetWorldMatrix(worldMatrix);
	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetOrthoMatrix(orthoMatrix);

	directx_device_->TurnZBufferOff();

	result = m_Bitmap->Render(directx_device_->GetDeviceContext(), 0, 0);
	if (!result) {
		return false;
	}

	result = m_FadeShader->Render(directx_device_->GetDeviceContext(), m_Bitmap->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix,
		render_texture_->GetShaderResourceView(), m_fadePercentage);
	if (!result) {
		return false;
	}

	directx_device_->TurnZBufferOn();

	directx_device_->EndScene();

	return true;
}

bool GraphicsClass::RenderNormalScene(float rotation_) {

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;

	directx_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	directx_device_->GetWorldMatrix(worldMatrix);
	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetProjectionMatrix(projectionMatrix);

	worldMatrix = XMMatrixRotationY(rotation_);

	model_->Render(directx_device_->GetDeviceContext());

	result = texture_shader_->Render(directx_device_->GetDeviceContext(), model_->GetIndexCount(), worldMatrix, viewMatrix,
		projectionMatrix, model_->GetTexture());
	if (!result) {
		return false;
	}

	directx_device_->EndScene();

	return true;
}