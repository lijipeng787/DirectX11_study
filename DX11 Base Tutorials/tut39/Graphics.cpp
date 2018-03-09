#include "Graphics.h"

#include <new>

#include "../CommonFramework/TypeDefine.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/Camera.h"

#include "particleshaderclass.h"
#include "particlesystemclass.h"

GraphicsClass::GraphicsClass() {}

GraphicsClass::~GraphicsClass() {}

bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd){

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
		camera_->SetPosition(0.0f, -1.0f, -10.0f);
		camera_->Render();
	}

	// Create the particle shader object.
	m_ParticleShader = ( ParticleShaderClass* )_aligned_malloc( sizeof( ParticleShaderClass ), 16 );
	new ( m_ParticleShader )ParticleShaderClass();
	if(!m_ParticleShader)
	{
		return false;
	}

	// Initialize the particle shader object.
	result = m_ParticleShader->Initialize(hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the particle shader object.", L"Error", MB_OK);
		return false;
	}

	// Create the particle system object.
	m_ParticleSystem = ( ParticleSystemClass* )_aligned_malloc( sizeof( ParticleSystemClass ), 16 );
	new ( m_ParticleSystem )ParticleSystemClass();
	if(!m_ParticleSystem)
	{
		return false;
	}

	// Initialize the particle system object.
	result = m_ParticleSystem->Initialize(L"../../tut39/data/star.dds");
	if(!result)
	{
		return false;
	}

	return true;
}

void GraphicsClass::Shutdown() {

	// Release the particle system object.
	if (m_ParticleSystem)
	{
		m_ParticleSystem->Shutdown();
		m_ParticleSystem->~ParticleSystemClass();
		_aligned_free(m_ParticleSystem);
		m_ParticleSystem = 0;
	}

	// Release the particle shader object.
	if (m_ParticleShader)
	{
		m_ParticleShader->Shutdown();
		m_ParticleShader->~ParticleShaderClass();
		_aligned_free(m_ParticleShader);
		m_ParticleShader = 0;
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

	m_ParticleSystem->Frame(frame_time_, directx_device_->GetDeviceContext());

	result = Render();
	if (!result) {
		return false;
	}

	return true;
}

bool GraphicsClass::Render() {

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;

	directx_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetWorldMatrix(worldMatrix);
	directx_device_->GetProjectionMatrix(projectionMatrix);

	directx_device_->TurnOnAlphaBlending();

	m_ParticleSystem->Render(directx_device_->GetDeviceContext());

	result = m_ParticleShader->Render(directx_device_->GetDeviceContext(), m_ParticleSystem->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
		m_ParticleSystem->GetTexture());
	if (!result) {
		return false;
	}

	directx_device_->TurnOffAlphaBlending();

	directx_device_->EndScene();

	return true;
}