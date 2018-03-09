#include "Graphics.h"

#include <new>

#include "../CommonFramework/TypeDefine.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/Camera.h"

#include "modelclass.h"
#include "lightclass.h"
#include "texturearrayclass.h"
#include "bumpmapshaderclass.h"

using namespace DirectX;

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
			"../../tut20/data/cube.txt",
			L"../../tut20/data/stone01.dds",
			L"../../tut20/data/bump01.dds"
		);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_BumpMapShader = (BumpMapShaderClass*)_aligned_malloc(sizeof(BumpMapShaderClass), 16);
		new (m_BumpMapShader)BumpMapShaderClass();
		if (!m_BumpMapShader) {
			return false;
		}


		result = m_BumpMapShader->Initialize(hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the bump map shader object.", L"Error", MB_OK);
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
	return true;
}

void GraphicsClass::Shutdown(){

	if(light_)
	{
		delete light_;
		light_ = 0;
	}

	if(m_BumpMapShader)
	{
		m_BumpMapShader->Shutdown();
		m_BumpMapShader->~BumpMapShaderClass();
		_aligned_free( m_BumpMapShader );
		m_BumpMapShader = 0;
	}

	if(model_)
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
	static float rotation_ = 0.0f;

	camera_->SetPosition(0.0f, 0.0f, -5.0f);

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

	model_->Render(directx_device_->GetDeviceContext());

	m_BumpMapShader->Render(directx_device_->GetDeviceContext(), model_->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
		model_->GetTextureArray(), light_->GetDirection(), light_->GetDiffuseColor());

	directx_device_->EndScene();

	return true;
}