#include "Graphics.h"

#include <new>

#include "../CommonFramework/TypeDefine.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/Camera.h"

#include "modelclass.h"
#include "textureshaderclass.h"
#include "reflectionshaderclass.h"
#include "rendertextureclass.h"
#include "reflectionshaderclass.h"

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

		result = model_->Initialize(directx_device_->GetDevice(), L"../../tut27/data/seafloor.dds", "../../tut27/data/cube.txt");
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
		m_FloorModel = new ModelClass();
		if (!m_FloorModel) {
			return false;
		}
		result = m_FloorModel->Initialize(directx_device_->GetDevice(), L"../../tut27/data/blue01.dds", "../../tut27/data/floor.txt");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the floor model object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_ReflectionShader = (ReflectionShaderClass*)_aligned_malloc(sizeof(ReflectionShaderClass), 16);
		new (m_ReflectionShader)ReflectionShaderClass();
		if (!m_ReflectionShader) {
			return false;
		}
		result = m_ReflectionShader->Initialize(directx_device_->GetDevice(), hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the reflection shader object.", L"Error", MB_OK);
			return false;
		}
	}

	return true;
}

void GraphicsClass::Shutdown() {
	// Release the reflection shader object.
	if (m_ReflectionShader)
	{
		m_ReflectionShader->Shutdown();
		m_ReflectionShader->~ReflectionShaderClass();
		_aligned_free(m_ReflectionShader);
		m_ReflectionShader = 0;
	}

	// Release the floor model object.
	if (m_FloorModel)
	{
		m_FloorModel->Shutdown();
		delete m_FloorModel;
		m_FloorModel = 0;
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

bool GraphicsClass::Frame() {

	bool result;

	result = Render();
	if (!result) {
		return false;
	}

	return true;
}

bool GraphicsClass::Render() {
	bool result;

	result = RenderToTexture();
	if (!result) {
		return false;
	}

	result = RenderScene();
	if (!result) {
		return false;
	}

	return true;
}

bool GraphicsClass::RenderToTexture() {

	XMMATRIX worldMatrix, reflectionViewMatrix, projectionMatrix;
	static float rotation = 0.0f;


	m_RenderTexture->SetRenderTarget(directx_device_->GetDeviceContext(), directx_device_->GetDepthStencilView());

	m_RenderTexture->ClearRenderTarget(directx_device_->GetDeviceContext(), directx_device_->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);

	camera_->RenderReflection(-1.5f);

	reflectionViewMatrix = camera_->GetReflectionViewMatrix();

	directx_device_->GetWorldMatrix(worldMatrix);
	directx_device_->GetProjectionMatrix(projectionMatrix);

	rotation += (float)XM_PI * 0.005f;
	if (rotation > 360.0f) {
		rotation -= 360.0f;
	}
	worldMatrix = XMMatrixRotationY(rotation);

	model_->Render(directx_device_->GetDeviceContext());

	m_TextureShader->Render(directx_device_->GetDeviceContext(), model_->GetIndexCount(), worldMatrix, reflectionViewMatrix,
		projectionMatrix, model_->GetTexture());

	directx_device_->SetBackBufferRenderTarget();

	return true;
}

bool GraphicsClass::RenderScene() {
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, reflectionMatrix;
	bool result;
	static float rotation = 0.0f;

	directx_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	directx_device_->GetWorldMatrix(worldMatrix);
	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetProjectionMatrix(projectionMatrix);

	rotation += (float)XM_PI * 0.005f;
	if (rotation > 360.0f) {
		rotation -= 360.0f;
	}

	worldMatrix = XMMatrixRotationY(rotation);

	model_->Render(directx_device_->GetDeviceContext());

	result = m_TextureShader->Render(directx_device_->GetDeviceContext(), model_->GetIndexCount(), worldMatrix, viewMatrix,
		projectionMatrix, model_->GetTexture());
	if (!result) {
		return false;
	}

	directx_device_->GetWorldMatrix(worldMatrix);
	worldMatrix = XMMatrixTranslation(0.0f, -1.5f, 0.0f);

	reflectionMatrix = camera_->GetReflectionViewMatrix();

	m_FloorModel->Render(directx_device_->GetDeviceContext());

	result = m_ReflectionShader->Render(directx_device_->GetDeviceContext(), m_FloorModel->GetIndexCount(), worldMatrix, viewMatrix,
		projectionMatrix, m_FloorModel->GetTexture(), m_RenderTexture->GetShaderResourceView(),
		reflectionMatrix);

	directx_device_->EndScene();

	return true;
}