#include "Graphics.h"

#include <new>

#include "../CommonFramework/TypeDefine.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/Camera.h"

#include "modelclass.h"
#include "fogshaderclass.h"

GraphicsClass::GraphicsClass() {}

GraphicsClass::~GraphicsClass() {}

bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd) {
	
	bool result;
	XMMATRIX baseViewMatrix;
	
	{
		m_D3D = new DirectX11Device;
		if (!m_D3D) {
			return false;
		}
		result = m_D3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_Camera = (Camera*)_aligned_malloc(sizeof(Camera), 16);
		new (m_Camera)Camera();
		if (!m_Camera) {
			return false;
		}
		m_Camera->SetPosition(0.0f, 0.0f, -1.0f);
		m_Camera->Render();
		m_Camera->GetViewMatrix(baseViewMatrix);
	}

	{
		m_Model = new ModelClass();
		if (!m_Model) {
			return false;
		}

		result = m_Model->Initialize(
			m_D3D->GetDevice(),
			L"../../tut23/data/seafloor.dds",
			"../../tut23/data/cube.txt");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_FogShader = (FogShaderClass*)_aligned_malloc(sizeof(FogShaderClass), 16);
		new (m_FogShader)FogShaderClass();
		if (!m_FogShader) {
			return false;
		}

		result = m_FogShader->Initialize(m_D3D->GetDevice(), hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the fog shader object.", L"Error", MB_OK);
			return false;
		}
	}

	return true;
}

void GraphicsClass::Shutdown() {

	// Release the fog shader object.
	if (m_FogShader)
	{
		m_FogShader->Shutdown();
		m_FogShader->~FogShaderClass();
		_aligned_free(m_FogShader);
		m_FogShader = 0;
	}

	// Release the model object.
	if (m_Model)
	{
		m_Model->Shutdown();
		delete m_Model;
		m_Model = 0;
	}

	if (m_Camera) {
		m_Camera->~Camera();
		_aligned_free(m_Camera);
		m_Camera = 0;
	}

	if (m_D3D) {
		m_D3D->Shutdown();
		delete m_D3D;
		m_D3D = 0;
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

	float fogColor, fogStart, fogEnd;
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;
	static float rotation = 0.0f;

	m_Camera->SetPosition(0.0f, 0.0f, -10.0f);


	fogColor = 0.5f;

	fogStart = 0.0f;
	fogEnd = 10.0f;

	m_D3D->BeginScene(fogColor, fogColor, fogColor, 1.0f);

	m_Camera->Render();
	m_D3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);
	rotation += (float)XM_PI * 0.005f;
	if (rotation > 360.0f) {
		rotation -= 360.0f;
	}

	worldMatrix = XMMatrixRotationY(rotation);

	m_Model->Render(m_D3D->GetDeviceContext());
	result = m_FogShader->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
		m_Model->GetTexture(), fogStart, fogEnd);
	if (!result) {
		return false;
	}

	m_D3D->EndScene();

	return true;
}