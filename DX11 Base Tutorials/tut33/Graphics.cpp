#include "Graphics.h"

#include <new>

#include "../CommonFramework/TypeDefine.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/Camera.h"

#include "modelclass.h"
#include "fireshaderclass.h"

GraphicsClass::GraphicsClass() {}

GraphicsClass::~GraphicsClass() {}

bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd){

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
		result = model_->Initialize(
			
			"../../tut33/data/square.txt", 
			L"../../tut33/data/fire01.dds",
			L"../../tut33/data/noise01.dds", 
			L"../../tut33/data/alpha01.dds"
		);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_FireShader = (FireShaderClass*)_aligned_malloc(sizeof(FireShaderClass), 16);
		new (m_FireShader)FireShaderClass();
		if (!m_FireShader) {
			return false;
		}
		result = m_FireShader->Initialize(hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the fire shader object.", L"Error", MB_OK);
			return false;
		}
	}

	return true;
}

void GraphicsClass::Shutdown() {

	// Release the fire shader object.
	if (m_FireShader)
	{
		m_FireShader->Shutdown();
		m_FireShader->~FireShaderClass();
		m_FireShader = 0;
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

bool GraphicsClass::Frame() {

	bool result;

	// Set the position of the camera.
	camera_->SetPosition(0.0f, 0.0f, -10.0f);

	// Render the scene.
	result = Render();
	if (!result) {
		return false;
	}

	return true;
}

bool GraphicsClass::Render() {

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;
	XMFLOAT3 scrollSpeeds, scales;
	XMFLOAT2 distortion1, distortion2, distortion3;
	float distortionScale, distortionBias;
	static float frameTime = 0.0f;

	frameTime += 0.01f;
	if (frameTime > 1000.0f) {
		frameTime = 0.0f;
	}

	scrollSpeeds = XMFLOAT3(1.3f, 2.1f, 2.3f);

	scales = XMFLOAT3(1.0f, 2.0f, 3.0f);

	distortion1 = XMFLOAT2(0.1f, 0.2f);
	distortion2 = XMFLOAT2(0.1f, 0.3f);
	distortion3 = XMFLOAT2(0.1f, 0.1f);

	distortionScale = 0.8f;
	distortionBias = 0.5f;

	directx_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	directx_device_->GetWorldMatrix(worldMatrix);
	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetProjectionMatrix(projectionMatrix);

	directx_device_->TurnOnAlphaBlending();

	model_->Render(directx_device_->GetDeviceContext());

	result = m_FireShader->Render(directx_device_->GetDeviceContext(), model_->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
		model_->GetTexture1(), model_->GetTexture2(), model_->GetTexture3(), frameTime, scrollSpeeds,
		scales, distortion1, distortion2, distortion3, distortionScale, distortionBias);
	if (!result) {
		return false;
	}

	directx_device_->TurnOffAlphaBlending();

	directx_device_->EndScene();

	return true;
}