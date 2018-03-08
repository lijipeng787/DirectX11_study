#include "Graphics.h"

#include <new>

#include "../CommonFramework/TypeDefine.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/Camera.h"

#include "shadermanagerclass.h"
#include "lightclass.h"
#include "modelclass.h"
#include "bumpmodelclass.h"

GraphicsClass::GraphicsClass() {}

GraphicsClass::~GraphicsClass() {}

bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd) {

	bool result;

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
	}

	{
		m_ShaderManager = new ShaderManagerClass();
		if (!m_ShaderManager) {
			return false;
		}
		result = m_ShaderManager->Initialize(directx_device_->GetDevice(), hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the shader manager object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_Light = (LightClass*)_aligned_malloc(sizeof(LightClass), 16);
		new (m_Light)LightClass();
		if (!m_Light) {
			return false;
		}
		m_Light->SetAmbientColor(0.15f, 0.15f, 0.15f, 1.0f);
		m_Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
		m_Light->SetDirection(0.0f, 0.0f, 1.0f);
		m_Light->SetSpecularColor(1.0f, 1.0f, 1.0f, 1.0f);
		m_Light->SetSpecularPower(64.0f);
	}

	{
		m_Model1 = new ModelClass();
		if (!m_Model1) {
			return false;
		}
		result = m_Model1->Initialize(directx_device_->GetDevice(), "../../tut45/data/cube.txt", L"../../tut45/data/marble.dds");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the first model object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_Model2 = new ModelClass();
		if (!m_Model2) {
			return false;
		}
		result = m_Model2->Initialize(directx_device_->GetDevice(), "../../tut45/data/cube.txt", L"../../tut45/data/metal.dds");
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
		result = m_Model3->Initialize(directx_device_->GetDevice(), "../../tut45/data/cube.txt", L"../../tut45/data/stone.dds",
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
	if (m_Model1) {
		m_Model1->Shutdown();
		delete m_Model1;
		m_Model1 = 0;
	}

	if (m_Model2) {
		m_Model2->Shutdown();
		delete m_Model2;
		m_Model2 = 0;
	}

	if (m_Model3) {
		m_Model3->Shutdown();
		delete m_Model3;
		m_Model3 = 0;
	}

	// Release the light object.
	if (m_Light) {
		m_Light->~LightClass();
		_aligned_free(m_Light);
		m_Light = 0;
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
	static float rotation = 0.0f;

	rotation += (float)XM_PI * 0.005f;
	if (rotation > 360.0f) {
		rotation -= 360.0f;
	}
	rotation_ = rotation;

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

	m_Model1->Render(directx_device_->GetDeviceContext());
	result = m_ShaderManager->RenderTextureShader(directx_device_->GetDeviceContext(), m_Model1->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
		m_Model1->GetTexture());
	if (!result) {
		return false;
	}

	directx_device_->GetWorldMatrix(worldMatrix);
	worldMatrix = XMMatrixRotationY(rotation_);
	translateMatrix = XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	worldMatrix = XMMatrixMultiply(worldMatrix, translateMatrix);

	m_Model2->Render(directx_device_->GetDeviceContext());
	result = m_ShaderManager->RenderLightShader(directx_device_->GetDeviceContext(), m_Model2->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
		m_Model2->GetTexture(), m_Light->GetDirection(), m_Light->GetAmbientColor(), m_Light->GetDiffuseColor(),
		camera_->GetPosition(), m_Light->GetSpecularColor(), m_Light->GetSpecularPower());
	if (!result) {
		return false;
	}

	directx_device_->GetWorldMatrix(worldMatrix);
	worldMatrix = XMMatrixRotationY(rotation_);
	translateMatrix = XMMatrixTranslation(3.5f, 0.0f, 0.0f);
	worldMatrix = XMMatrixMultiply(worldMatrix, translateMatrix);

	m_Model3->Render(directx_device_->GetDeviceContext());
	result = m_ShaderManager->RenderBumpMapShader(directx_device_->GetDeviceContext(), m_Model3->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
		m_Model3->GetColorTexture(), m_Model3->GetNormalMapTexture(), m_Light->GetDirection(),
		m_Light->GetDiffuseColor());
	if (!result) {
		return false;
	}

	directx_device_->EndScene();

	return true;
}
