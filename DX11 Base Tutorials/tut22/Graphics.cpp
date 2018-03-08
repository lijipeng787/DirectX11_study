#include "Graphics.h"

#include <new>

#include "../CommonFramework/TypeDefine.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/Camera.h"

#include "modelclass.h"
#include "debugwindowclass.h"
#include "lightclass.h"
#include "rendertextureclass.h"
#include "lightshaderclass.h"
#include "textureshaderclass.h"

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

		result = model_->Initialize(
			directx_device_->GetDevice(),
			L"../../tut22/data/seafloor.dds",
			"../../tut22/data/cube.txt"
		);
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

		light_->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
		light_->SetDirection(0.0f, 0.0f, 1.0f);
	}

	{
		m_RenderTexture = new RenderTextureClass();
		if (!m_RenderTexture) {
			return false;
		}

		result = m_RenderTexture->Initialize(screenWidth, screenHeight);
		if (!result) {
			return false;
		}
	}

	{
		m_DebugWindow = new DebugWindowClass();
		if (!m_DebugWindow) {
			return false;
		}

		result = m_DebugWindow->Initialize(screenWidth, screenHeight, 100, 100);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the debug window object.", L"Error", MB_OK);
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
			MessageBox(hwnd, L"Could not initialize the texture shader object.", L"Error", MB_OK);
			return false;
		}
	}

	return true;
}

void GraphicsClass::Shutdown() {

	
	if (m_TextureShader)
	{
		m_TextureShader->Shutdown();
		m_TextureShader->~TextureShaderClass();
		_aligned_free(m_TextureShader);
		m_TextureShader = 0;
	}

	// Release the debug window object.
	if (m_DebugWindow)
	{
		m_DebugWindow->Shutdown();
		delete m_DebugWindow;
		m_DebugWindow = 0;
	}

	
	if (m_RenderTexture)
	{
		m_RenderTexture->Shutdown();
		delete m_RenderTexture;
		m_RenderTexture = 0;
	}

	// Release the light object.
	if (light_)
	{
		delete light_;
		light_ = 0;
	}

	// Release the light shader object.
	if (light_shader_)
	{
		light_shader_->Shutdown();
		light_shader_->~LightShaderClass();
		_aligned_free(light_shader_);
		light_shader_ = 0;
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

	XMMATRIX worldMatrix, viewMatrix, orthoMatrix;
	bool result;

	camera_->SetPosition(0.0f, 0.0f, -5.0f);

	result = RenderToTexture();
	if (!result) {
		return false;
	}

	directx_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	result = RenderScene();
	if (!result) {
		return false;
	}

	directx_device_->TurnZBufferOff();

	directx_device_->GetWorldMatrix(worldMatrix);
	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetOrthoMatrix(orthoMatrix);

	result = m_DebugWindow->Render(directx_device_->GetDeviceContext(), 50, 50);
	if (!result) {
		return false;
	}

	result = m_TextureShader->Render(directx_device_->GetDeviceContext(), m_DebugWindow->GetIndexCount(), worldMatrix, viewMatrix,
		orthoMatrix, m_RenderTexture->GetShaderResourceView());
	if (!result) {
		return false;
	}

	directx_device_->TurnZBufferOn();

	directx_device_->EndScene();

	return true;
}


bool GraphicsClass::RenderToTexture() {

	bool result;

	m_RenderTexture->SetRenderTarget(directx_device_->GetDeviceContext(), directx_device_->GetDepthStencilView());

	m_RenderTexture->ClearRenderTarget(directx_device_->GetDeviceContext(), directx_device_->GetDepthStencilView(), 0.0f, 0.0f, 1.0f, 1.0f);

	result = RenderScene();
	if (!result) {
		return false;
	}

	directx_device_->SetBackBufferRenderTarget();

	return true;
}


bool GraphicsClass::RenderScene() {

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;
	static float rotation = 0.0f;

	camera_->Render();

	directx_device_->GetWorldMatrix(worldMatrix);
	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetProjectionMatrix(projectionMatrix);

	rotation += (float)XM_PI * 0.005f;
	if (rotation > 360.0f)
	{
		rotation -= 360.0f;
	}

	worldMatrix = XMMatrixRotationY(rotation);

	model_->Render(directx_device_->GetDeviceContext());
	result = light_shader_->Render(directx_device_->GetDeviceContext(), model_->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
		model_->GetTexture(), light_->GetDirection(), light_->GetDiffuseColor());
	if (!result) {
		return false;
	}

	return true;
}