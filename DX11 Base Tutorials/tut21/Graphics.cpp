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
		m_SpecMapShader = (SpecMapShaderClass*)_aligned_malloc(sizeof(SpecMapShaderClass), 16);
		new (m_SpecMapShader)SpecMapShaderClass();
		if (!m_SpecMapShader) {
			return false;
		}

		result = m_SpecMapShader->Initialize(directx_device_->GetDevice(), hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the specular map shader object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_Light = new LightClass();
		if (!m_Light) {
			return false;
		}

		m_Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
		m_Light->SetDirection(0.0f, 0.0f, 1.0f);
		m_Light->SetSpecularColor(1.0f, 1.0f, 1.0f, 1.0f);
		m_Light->SetSpecularPower(16.0f);
	}

	return true;
}

void GraphicsClass::Shutdown()
{
	// Release the light object.
	if (m_Light)
	{
		delete m_Light;
		m_Light = 0;
	}

	// Release the specular map shader object.
	if (m_SpecMapShader)
	{
		m_SpecMapShader->Shutdown();
		m_SpecMapShader->~SpecMapShaderClass();
		_aligned_free(m_SpecMapShader);
		m_SpecMapShader = 0;
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

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix;
	static float rotation = 0.0f;

	camera_->SetPosition(0.0f, 0.0f, -5.0f);

	directx_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	directx_device_->GetWorldMatrix(worldMatrix);
	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetProjectionMatrix(projectionMatrix);
	directx_device_->GetOrthoMatrix(orthoMatrix);

	rotation += (float)XM_PI * 0.0025f;
	if (rotation > 360.0f) {
		rotation -= 360.0f;
	}

	worldMatrix = XMMatrixRotationY(rotation);

	model_->Render(directx_device_->GetDeviceContext());
	m_SpecMapShader->Render(directx_device_->GetDeviceContext(), model_->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
		model_->GetTextureArray(), m_Light->GetDirection(), m_Light->GetDiffuseColor(),
		camera_->GetPosition(), m_Light->GetSpecularColor(), m_Light->GetSpecularPower());
	directx_device_->EndScene();

	return true;
}