#include "Graphics.h"

#include <new>

#include "../CommonFramework/TypeDefine.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/Camera.h"

#include "modelclass.h"
#include "textureshaderclass.h"
#include "transparentshaderclass.h"

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
		m_Model1 = new ModelClass();
		if (!m_Model1) {
			return false;
		}

		result = m_Model1->Initialize(L"../../tut26/data/dirt01.dds", "../../tut26/data/square.txt");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the first model object.", L"Error", MB_OK);
			return false;
		}

		m_Model2 = new ModelClass();
		if (!m_Model2) {
			return false;
		}

		result = m_Model2->Initialize(L"../../tut26/data/stone01.dds", "../../tut26/data/square.txt");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the second model object.", L"Error", MB_OK);
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

	{
		m_TransparentShader = (TransparentShaderClass*)_aligned_malloc(sizeof(TransparentShaderClass), 16);
		new (m_TransparentShader)TransparentShaderClass();
		if (!m_TransparentShader) {
			return false;
		}

		result = m_TransparentShader->Initialize(hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the transparent shader object.", L"Error", MB_OK);
			return false;
		}
	}

	return true;
}

void GraphicsClass::Shutdown() {
	// Release the transparent shader object.
	if (m_TransparentShader)
	{
		m_TransparentShader->Shutdown();
		m_TransparentShader->~TransparentShaderClass();
		_aligned_free(m_TransparentShader);
		m_TransparentShader = 0;
	}

	
	if (m_TextureShader)
	{
		m_TextureShader->Shutdown();
		m_TextureShader->~TextureShaderClass();
		_aligned_free(m_TextureShader);
		m_TextureShader = 0;
	}

	// Release the second model object.
	if (m_Model2)
	{
		m_Model2->Shutdown();
		delete m_Model2;
		m_Model2 = 0;
	}

	// Release the first model object.
	if (m_Model1)
	{
		m_Model1->Shutdown();
		delete m_Model1;
		m_Model1 = 0;
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

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;
	float blendAmount;


	blendAmount = 0.5f;

	directx_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	directx_device_->GetWorldMatrix(worldMatrix);
	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetProjectionMatrix(projectionMatrix);


	m_Model1->Render(directx_device_->GetDeviceContext());

	result = m_TextureShader->Render(directx_device_->GetDeviceContext(), m_Model1->GetIndexCount(), worldMatrix, viewMatrix,
		projectionMatrix, m_Model1->GetTexture());
	if (!result) {
		return false;
	}

	worldMatrix = XMMatrixTranslation(1.0f, 0.0f, -1.0f);

	directx_device_->TurnOnAlphaBlending();

	m_Model2->Render(directx_device_->GetDeviceContext());

	result = m_TransparentShader->Render(directx_device_->GetDeviceContext(), m_Model2->GetIndexCount(), worldMatrix, viewMatrix,
		projectionMatrix, m_Model2->GetTexture(), blendAmount);
	if (!result) {
		return false;
	}

	directx_device_->TurnOffAlphaBlending();

	directx_device_->EndScene();

	return true;
}