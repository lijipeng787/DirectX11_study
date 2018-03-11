#include <new>

#include "../CommonFramework/TypeDefine.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/Camera.h"

#include "Graphics.h"
#include "shadermanagerclass.h"
#include "lightclass.h"
#include "modelclass.h"
#include "bumpmodelclass.h"

GraphicsClass::GraphicsClass() {}

GraphicsClass::~GraphicsClass() {}

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
		camera_->Render();
	}

	{
		m_ShaderManager = new ShaderManagerClass();
		if (!m_ShaderManager) {
			return false;
		}
		result = m_ShaderManager->Initialize(hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the shader manager object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		light_ = (LightClass*)_aligned_malloc(sizeof(LightClass), 16);
		new (light_)LightClass();
		if (!light_) {
			return false;
		}
		light_->SetAmbientColor(0.15f, 0.15f, 0.15f, 1.0f);
		light_->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
		light_->SetDirection(0.0f, 0.0f, 1.0f);
		light_->SetSpecularColor(1.0f, 1.0f, 1.0f, 1.0f);
		light_->SetSpecularPower(64.0f);
	}

	{
		model_1_ = new ModelClass();
		if (!model_1_) {
			return false;
		}
		result = model_1_->Initialize("../../tut45/data/cube.txt", L"../../tut45/data/marble.dds");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the first model object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		model_2_ = new ModelClass();
		if (!model_2_) {
			return false;
		}
		result = model_2_->Initialize("../../tut45/data/cube.txt", L"../../tut45/data/metal.dds");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the second model object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_Model3 = new BumpModelClass();
		if (!m_Model3) {
			return false;
		}
		result = m_Model3->Initialize("../../tut45/data/cube.txt", L"../../tut45/data/stone.dds",
			L"../../tut45/data/normal.dds");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the third model object.", L"Error", MB_OK);
			return false;
		}
	}

	return true;
}

void GraphicsClass::Shutdown() {

	// Release the model objects.
	if (model_1_) {
		model_1_->Shutdown();
		delete model_1_;
		model_1_ = 0;
	}

	if (model_2_) {
		model_2_->Shutdown();
		delete model_2_;
		model_2_ = 0;
	}

	if (m_Model3) {
		m_Model3->Shutdown();
		delete m_Model3;
		m_Model3 = 0;
	}


	if (light_) {
		light_->~LightClass();
		_aligned_free(light_);
		light_ = nullptr;;
	}

	// Release the shader manager object.
	if (m_ShaderManager) {
		m_ShaderManager->Shutdown();
		delete m_ShaderManager;
		m_ShaderManager = 0;
	}

	if (camera_) {
		camera_->~Camera();
		_aligned_free(camera_);
		camera_ = nullptr;
	}

	
		
		
		
	}
}

bool GraphicsClass::Frame() {

	bool result;
	static float rotation_ = 0.0f;

	rotation_ += (float)XM_PI * 0.005f;
	if (rotation_ > 360.0f) {
		rotation_ -= 360.0f;
	}
	rotation_ = rotation_;

	result = Render();
	if (!result) {
		return false;
	}

	return true;
}

bool GraphicsClass::Render() {

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, translateMatrix;
	bool result;

	directx_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	directx_device_->GetWorldMatrix(worldMatrix);
	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetProjectionMatrix(projectionMatrix);

	worldMatrix = XMMatrixRotationY(rotation_);
	translateMatrix = XMMatrixTranslation(-3.5f, 0.0f, 0.0f);
	worldMatrix = XMMatrixMultiply(worldMatrix, translateMatrix);

	model_1_->Render(directx_device_->GetDeviceContext());
	result = m_ShaderManager->RenderTextureShader(directx_device_->GetDeviceContext(), model_1_->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
		model_1_->GetTexture());
	if (!result) {
		return false;
	}

	directx_device_->GetWorldMatrix(worldMatrix);
	worldMatrix = XMMatrixRotationY(rotation_);
	translateMatrix = XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	worldMatrix = XMMatrixMultiply(worldMatrix, translateMatrix);

	model_2_->Render(directx_device_->GetDeviceContext());
	result = m_ShaderManager->RenderLightShader(directx_device_->GetDeviceContext(), model_2_->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
		model_2_->GetTexture(), light_->GetDirection(), light_->GetAmbientColor(), light_->GetDiffuseColor(),
		camera_->GetPosition(), light_->GetSpecularColor(), light_->GetSpecularPower());
	if (!result) {
		return false;
	}

	directx_device_->GetWorldMatrix(worldMatrix);
	worldMatrix = XMMatrixRotationY(rotation_);
	translateMatrix = XMMatrixTranslation(3.5f, 0.0f, 0.0f);
	worldMatrix = XMMatrixMultiply(worldMatrix, translateMatrix);

	m_Model3->Render(directx_device_->GetDeviceContext());
	result = m_ShaderManager->RenderBumpMapShader(directx_device_->GetDeviceContext(), m_Model3->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
		m_Model3->GetColorTexture(), m_Model3->GetNormalMapTexture(), light_->GetDirection(),
		light_->GetDiffuseColor());
	if (!result) {
		return false;
	}

	directx_device_->EndScene();

	return true;
}
