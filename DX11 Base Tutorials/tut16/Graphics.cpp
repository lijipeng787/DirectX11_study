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
		m_Text = (TextClass*)_aligned_malloc(sizeof(TextClass), 16);
		new (m_Text)TextClass();
		if (!m_Text)
		{
			return false;
		}

		result = m_Text->Initialize(directx_device_->GetDeviceContext(), hwnd, screenWidth, screenHeight, baseViewMatrix);
		if (!result)
		{
			MessageBox(hwnd, L"Could not initialize the text object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		model_ = new ModelClass();
		if (!model_) {
			return false;
		}

		result = model_->Initialize(L"../../tut16/data/seafloor.dds", "../../tut16/data/sphere.txt");
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
		if (!result)
		{
			MessageBox(hwnd, L"Could not initialize the light shader object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		light_ = new LightClass();
		if (!light_) {
			return false;
		}

		light_->SetDirection(0.0f, 0.0f, 1.0f);

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

	if (light_) {
		delete light_;
		light_ = nullptr;;
	}

	if (light_shader_) {
		light_shader_->Shutdown();
		light_shader_->~LightShaderClass();
		_aligned_free(light_shader_);
		light_shader_ = nullptr;;
	}

	if (model_) {
		model_->Shutdown();
		delete model_;
		model_ = nullptr;
	}

	if (m_Text) {
		m_Text->Shutdown();
		m_Text->~TextClass();
		_aligned_free(m_Text);
		m_Text = 0;
	}

	if (camera_) {
		camera_->~Camera();
		_aligned_free(camera_);
		camera_ = nullptr;
	}

	
		
		
	}

	
}

bool GraphicsClass::Frame(){

	camera_->SetPosition(0.0f, 0.0f, -10.0f);

	camera_->SetRotation(0.0f, rotationY, 0.0f);

	return true;
}


bool GraphicsClass::Render() {

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix;
	int modelCount, renderCount, index;
	float positionX, positionY, positionZ, radius;
	XMFLOAT4 color;
	bool renderModel, result;

	directx_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetWorldMatrix(worldMatrix);
	directx_device_->GetProjectionMatrix(projectionMatrix);
	directx_device_->GetOrthoMatrix(orthoMatrix);

	m_Frustum->ConstructFrustum(SCREEN_DEPTH, projectionMatrix, viewMatrix);

	modelCount = m_ModelList->GetModelCount();

	renderCount = 0;

	for (index = 0; index < modelCount; index++) {
		m_ModelList->GetData(index, positionX, positionY, positionZ, color);

		radius = 1.0f;

		renderModel = m_Frustum->CheckSphere(positionX, positionY, positionZ, radius);

		if (renderModel) {
	
			worldMatrix = XMMatrixTranslation(positionX, positionY, positionZ);

			model_->Render(directx_device_->GetDeviceContext());

			light_shader_->Render(directx_device_->GetDeviceContext(), model_->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
				model_->GetTexture(), light_->GetDirection(), color);

			directx_device_->GetWorldMatrix(worldMatrix);

			renderCount++;
		}
	}

	result = m_Text->SetRenderCount(renderCount, directx_device_->GetDeviceContext());
	if (!result) {
		return false;
	}

	directx_device_->TurnZBufferOff();
	directx_device_->TurnOnAlphaBlending();

	result = m_Text->Render(directx_device_->GetDeviceContext(), worldMatrix, orthoMatrix);
	if (!result) {
		return false;
	}

	directx_device_->TurnOffAlphaBlending();
	directx_device_->TurnZBufferOn();

	directx_device_->EndScene();

	return true;
}

