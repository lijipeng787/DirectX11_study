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
		result = model_->Initialize(directx_device_->GetDevice(), L"../../tut28/data/seafloor.dds", "../../tut28/data/cube.txt");
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
		result = m_TextureShader->Initialize(directx_device_->GetDevice(), hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the texture shader object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_RenderTexture = new RenderTextureClass();
		if (!m_RenderTexture) {
			return false;
		}
		result = m_RenderTexture->Initialize(directx_device_->GetDevice(), screenWidth, screenHeight);
		if (!result) {
			return false;
		}
	}

	{
		m_Bitmap = new SimpleMoveableSurface();
		if (!m_Bitmap) {
			return false;
		}
		result = m_Bitmap->Initialize(directx_device_->GetDevice(), screenWidth, screenHeight, screenWidth, screenHeight);
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
		result = m_FadeShader->Initialize(directx_device_->GetDevice(), hwnd);
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

	
	if (m_RenderTexture)
	{
		m_RenderTexture->Shutdown();
		delete m_RenderTexture;
		m_RenderTexture = 0;
	}

	
	if (m_TextureShader)
	{
		m_TextureShader->Shutdown();
		m_TextureShader->~TextureShaderClass();
		_aligned_free(m_TextureShader);
		m_TextureShader = 0;
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
	static float rotation = 0.0f;

	rotation += (float)XM_PI * 0.005f;
	if (rotation > 360.0f) {
		rotation -= 360.0f;
	}

	if (m_fadeDone) {

		RenderNormalScene(rotation);
	}
	else {

		result = RenderToTexture(rotation);
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

bool GraphicsClass::RenderToTexture(float rotation) {

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;

	m_RenderTexture->SetRenderTarget(directx_device_->GetDeviceContext(), directx_device_->GetDepthStencilView());

	m_RenderTexture->ClearRenderTarget(directx_device_->GetDeviceContext(), directx_device_->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	directx_device_->GetWorldMatrix(worldMatrix);
	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetProjectionMatrix(projectionMatrix);

	worldMatrix = XMMatrixRotationY(rotation);

	model_->Render(directx_device_->GetDeviceContext());

	result = m_TextureShader->Render(directx_device_->GetDeviceContext(), model_->GetIndexCount(), worldMatrix, viewMatrix,
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
		m_RenderTexture->GetShaderResourceView(), m_fadePercentage);
	if (!result) {
		return false;
	}

	directx_device_->TurnZBufferOn();

	directx_device_->EndScene();

	return true;
}

bool GraphicsClass::RenderNormalScene(float rotation) {

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;

	directx_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	directx_device_->GetWorldMatrix(worldMatrix);
	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetProjectionMatrix(projectionMatrix);

	worldMatrix = XMMatrixRotationY(rotation);

	model_->Render(directx_device_->GetDeviceContext());

	result = m_TextureShader->Render(directx_device_->GetDeviceContext(), model_->GetIndexCount(), worldMatrix, viewMatrix,
		projectionMatrix, model_->GetTexture());
	if (!result) {
		return false;
	}

	directx_device_->EndScene();

	return true;
}