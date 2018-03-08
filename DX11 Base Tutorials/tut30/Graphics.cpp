#include "Graphics.h"

#include <new>

#include "../CommonFramework/TypeDefine.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/Camera.h"

#include "modelclass.h"
#include "lightclass.h"
#include "lightshaderclass.h"

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
		result = model_->Initialize(L"../../tut30/data/stone01.dds", "../../tut30/data/plane01.txt");
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
		m_Light1 = new LightClass();
		if (!m_Light1) {
			return false;
		}
		m_Light1->SetDiffuseColor(1.0f, 0.0f, 0.0f, 1.0f);
		m_Light1->SetPosition(-3.0f, 1.0f, 3.0f);

		m_Light2 = new LightClass();
		if (!m_Light2) {
			return false;
		}
		m_Light2->SetDiffuseColor(0.0f, 1.0f, 0.0f, 1.0f);
		m_Light2->SetPosition(3.0f, 1.0f, 3.0f);

		m_Light3 = new LightClass();
		if (!m_Light3) {
			return false;
		}
		m_Light3->SetDiffuseColor(0.0f, 0.0f, 1.0f, 1.0f);
		m_Light3->SetPosition(-3.0f, 1.0f, -3.0f);

		m_Light4 = new LightClass();
		if (!m_Light4) {
			return false;
		}
		m_Light4->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
		m_Light4->SetPosition(3.0f, 1.0f, -3.0f);
	}

	return true;
}

void GraphicsClass::Shutdown() {

	if (m_Light1)
	{
		delete m_Light1;
		m_Light1 = 0;
	}

	if (m_Light2)
	{
		delete m_Light2;
		m_Light2 = 0;
	}

	if (m_Light3)
	{
		delete m_Light3;
		m_Light3 = 0;
	}

	if (m_Light4)
	{
		delete m_Light4;
		m_Light4 = 0;
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

	// Set the position of the camera.
	camera_->SetPosition(0.0f, 2.0f, -12.0f);

	// Render the scene.
	result = Render();
	if (!result) {
		return false;
	}

	return true;
}

void GraphicsClass::SetFameTime(float frame_time) {
	frame_time_ = frame_time;
}

bool GraphicsClass::Render() {

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	XMFLOAT4 diffuseColor[4];
	XMFLOAT4 lightPosition[4];
	bool result;

	diffuseColor[0] = m_Light1->GetDiffuseColor();
	diffuseColor[1] = m_Light2->GetDiffuseColor();
	diffuseColor[2] = m_Light3->GetDiffuseColor();
	diffuseColor[3] = m_Light4->GetDiffuseColor();

	lightPosition[0] = m_Light1->GetPosition();
	lightPosition[1] = m_Light2->GetPosition();
	lightPosition[2] = m_Light3->GetPosition();
	lightPosition[3] = m_Light4->GetPosition();

	directx_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	directx_device_->GetWorldMatrix(worldMatrix);
	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetProjectionMatrix(projectionMatrix);

	model_->Render(directx_device_->GetDeviceContext());

	result = light_shader_->Render(directx_device_->GetDeviceContext(), model_->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
		model_->GetTexture(), diffuseColor, lightPosition);
	if (!result) {
		return false;
	}

	directx_device_->EndScene();

	return true;
}