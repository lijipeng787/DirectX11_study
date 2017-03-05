#include "Graphics.h"

#include <new>

#include "../CommonFramework/TypeDefine.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/Camera.h"

#include "Frustum.h"
#include "modellistclass.h"
#include "modelclass.h"
#include "lightclass.h"
#include "lightshaderclass.h"
#include "textureclass.h"
#include "textclass.h"

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
		m_Text = (TextClass*)_aligned_malloc(sizeof(TextClass), 16);
		new (m_Text)TextClass();
		if (!m_Text)
		{
			return false;
		}

		result = m_Text->Initialize(m_D3D->GetDevice(), m_D3D->GetDeviceContext(), hwnd, screenWidth, screenHeight, baseViewMatrix);
		if (!result)
		{
			MessageBox(hwnd, L"Could not initialize the text object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_Model = new ModelClass();
		if (!m_Model) {
			return false;
		}

		result = m_Model->Initialize(m_D3D->GetDevice(), L"../../tut16/data/seafloor.dds", "../../tut16/data/sphere.txt");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_LightShader = (LightShaderClass*)_aligned_malloc(sizeof(LightShaderClass), 16);
		new (m_LightShader)LightShaderClass();
		if (!m_LightShader) {
			return false;
		}

		result = m_LightShader->Initialize(m_D3D->GetDevice(), hwnd);
		if (!result)
		{
			MessageBox(hwnd, L"Could not initialize the light shader object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_Light = new LightClass();
		if (!m_Light) {
			return false;
		}

		m_Light->SetDirection(0.0f, 0.0f, 1.0f);

	}

	{
		m_ModelList = new ModelListClass();
		if (!m_ModelList) {
			return false;
		}

		result = m_ModelList->Initialize(25);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the model list object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_Frustum = (FrustumClass*)_aligned_malloc(sizeof(FrustumClass), 16);
		new (m_Frustum)FrustumClass();
		if (!m_Frustum) {
			return false;
		}
	}

	return true;
}

void GraphicsClass::Shutdown() {

	if (m_Frustum) {
		m_Frustum->~FrustumClass();
		_aligned_free(m_Frustum);
		m_Frustum = 0;
	}

	if (m_ModelList) {
		m_ModelList->Shutdown();
		delete m_ModelList;
		m_ModelList = 0;
	}

	if (m_Light) {
		delete m_Light;
		m_Light = 0;
	}

	if (m_LightShader) {
		m_LightShader->Shutdown();
		m_LightShader->~LightShaderClass();
		_aligned_free(m_LightShader);
		m_LightShader = 0;
	}

	if (m_Model) {
		m_Model->Shutdown();
		delete m_Model;
		m_Model = 0;
	}

	if (m_Text) {
		m_Text->Shutdown();
		m_Text->~TextClass();
		_aligned_free(m_Text);
		m_Text = 0;
	}

	if (m_Camera) {
		m_Camera->~Camera();
		_aligned_free(m_Camera);
		m_Camera = 0;
	}

	if (m_D3D) {
		m_D3D->Shutdown();
		delete m_D3D;
	}

	return;
}

bool GraphicsClass::Frame(){

	m_Camera->SetPosition(0.0f, 0.0f, -10.0f);

	m_Camera->SetRotation(0.0f, rotationY, 0.0f);

	return true;
}


bool GraphicsClass::Render() {

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix;
	int modelCount, renderCount, index;
	float positionX, positionY, positionZ, radius;
	XMFLOAT4 color;
	bool renderModel, result;

	m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	m_Camera->Render();

	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);
	m_D3D->GetOrthoMatrix(orthoMatrix);

	m_Frustum->ConstructFrustum(SCREEN_DEPTH, projectionMatrix, viewMatrix);

	modelCount = m_ModelList->GetModelCount();

	renderCount = 0;

	for (index = 0; index < modelCount; index++) {
		m_ModelList->GetData(index, positionX, positionY, positionZ, color);

		radius = 1.0f;

		renderModel = m_Frustum->CheckSphere(positionX, positionY, positionZ, radius);

		if (renderModel) {
	
			worldMatrix = XMMatrixTranslation(positionX, positionY, positionZ);

			m_Model->Render(m_D3D->GetDeviceContext());

			m_LightShader->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
				m_Model->GetTexture(), m_Light->GetDirection(), color);

			m_D3D->GetWorldMatrix(worldMatrix);

			renderCount++;
		}
	}

	result = m_Text->SetRenderCount(renderCount, m_D3D->GetDeviceContext());
	if (!result) {
		return false;
	}

	m_D3D->TurnZBufferOff();
	m_D3D->TurnOnAlphaBlending();

	result = m_Text->Render(m_D3D->GetDeviceContext(), worldMatrix, orthoMatrix);
	if (!result) {
		return false;
	}

	m_D3D->TurnOffAlphaBlending();
	m_D3D->TurnZBufferOn();

	m_D3D->EndScene();

	return true;
}

