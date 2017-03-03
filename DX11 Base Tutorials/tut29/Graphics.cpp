#include "Graphics.h"

#include <new>

#include "../CommonFramework/TypeDefine.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/Camera.h"

#include "modelclass.h"
#include "rendertextureclass.h"
#include "lightshaderclass.h"
#include "lightclass.h"
#include "refractionshaderclass.h"
#include "watershaderclass.h"

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
		m_Camera->SetPosition(0.0f, 0.0f, -10.0f);
		m_Camera->Render();
		m_Camera->GetViewMatrix(baseViewMatrix);
	}

	{
		m_GroundModel = new ModelClass();
		if (!m_GroundModel) {
			return false;
		}
		result = m_GroundModel->Initialize(m_D3D->GetDevice(), L"../../tut29/data/ground01.dds", "../../tut29/data/ground.txt");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the ground model object.", L"Error", MB_OK);
			return false;
		}

		m_WallModel = new ModelClass();
		if (!m_WallModel) {
			return false;
		}
		result = m_WallModel->Initialize(m_D3D->GetDevice(), L"../../tut29/data/wall01.dds", "../../tut29/data/wall.txt");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the wall model object.", L"Error", MB_OK);
			return false;
		}

		m_BathModel = new ModelClass();
		if (!m_BathModel) {
			return false;
		}
		result = m_BathModel->Initialize(m_D3D->GetDevice(), L"../../tut29/data/marble01.dds", "../../tut29/data/bath.txt");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the bath model object.", L"Error", MB_OK);
			return false;
		}

		m_WaterModel = new ModelClass();
		if (!m_WaterModel) {
			return false;
		}
		result = m_WaterModel->Initialize(m_D3D->GetDevice(), L"../../tut29/data/water01.dds", "../../tut29/data/water.txt");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the water model object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_Light = new LightClass();
		if (!m_Light) {
			return false;
		}
		m_Light->SetAmbientColor(0.15f, 0.15f, 0.15f, 1.0f);
		m_Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
		m_Light->SetDirection(0.0f, -1.0f, 0.5f);

	}

	{
		m_RefractionTexture = new RenderTextureClass();
		if (!m_RefractionTexture) {
			return false;
		}
		result = m_RefractionTexture->Initialize(m_D3D->GetDevice(), screenWidth, screenHeight);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the refraction render to texture object.", L"Error", MB_OK);
			return false;
		}

	}

	{
		m_ReflectionTexture = new RenderTextureClass();
		if (!m_ReflectionTexture) {
			return false;
		}
		result = m_ReflectionTexture->Initialize(m_D3D->GetDevice(), screenWidth, screenHeight);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the reflection render to texture object.", L"Error", MB_OK);
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
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the light shader object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_RefractionShader = (RefractionShaderClass*)_aligned_malloc(sizeof(RefractionShaderClass), 16);
		new (m_RefractionShader)RefractionShaderClass();
		if (!m_RefractionShader) {
			return false;
		}
		result = m_RefractionShader->Initialize(m_D3D->GetDevice(), hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the refraction shader object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_WaterShader = (WaterShaderClass*)_aligned_malloc(sizeof(WaterShaderClass), 16);
		new (m_WaterShader)WaterShaderClass();
		if (!m_WaterShader) {
			return false;
		}
		result = m_WaterShader->Initialize(m_D3D->GetDevice(), hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the water shader object.", L"Error", MB_OK);
			return false;
		}

		m_waterHeight = 2.75f;

		m_waterTranslation = 0.0f;

	}
	return true;
}

void GraphicsClass::Shutdown() {

	// Release the water shader object.
	if (m_WaterShader)
	{
		m_WaterShader->Shutdown();
		m_WaterShader->~WaterShaderClass();
		_aligned_free(m_WaterShader);
		m_WaterShader = 0;
	}

	// Release the refraction shader object.
	if (m_RefractionShader)
	{
		m_RefractionShader->Shutdown();
		m_RefractionShader->~RefractionShaderClass();
		_aligned_free(m_RefractionShader);
		m_RefractionShader = 0;
	}

	// Release the light shader object.
	if (m_LightShader)
	{
		m_LightShader->Shutdown();
		m_LightShader->~LightShaderClass();
		_aligned_free(m_LightShader);
		m_LightShader = 0;
	}

	// Release the reflection render to texture object.
	if (m_ReflectionTexture)
	{
		m_ReflectionTexture->Shutdown();
		delete m_ReflectionTexture;
		m_ReflectionTexture = 0;
	}

	// Release the refraction render to texture object.
	if (m_RefractionTexture)
	{
		m_RefractionTexture->Shutdown();
		delete m_RefractionTexture;
		m_RefractionTexture = 0;
	}

	// Release the light object.
	if (m_Light)
	{
		delete m_Light;
		m_Light = 0;
	}

	// Release the water model object.
	if (m_WaterModel)
	{
		m_WaterModel->Shutdown();
		delete m_WaterModel;
		m_WaterModel = 0;
	}

	// Release the bath model object.
	if (m_BathModel)
	{
		m_BathModel->Shutdown();
		delete m_BathModel;
		m_BathModel = 0;
	}

	// Release the wall model object.
	if (m_WallModel)
	{
		m_WallModel->Shutdown();
		delete m_WallModel;
		m_WallModel = 0;
	}

	// Release the ground model object.
	if (m_GroundModel)
	{
		m_GroundModel->Shutdown();
		delete m_GroundModel;
		m_GroundModel = 0;
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

	// Update the position of the water to simulate motion.
	m_waterTranslation += 0.001f;
	if (m_waterTranslation > 1.0f) {
		m_waterTranslation -= 1.0f;
	}

	// Set the position and rotation of the camera.
	m_Camera->SetPosition(-10.0f, 6.0f, -10.0f);
	m_Camera->SetRotation(0.0f, 45.0f, 0.0f);

	return true;
}

bool GraphicsClass::Render() {

	bool result;

	// Render the refraction of the scene to a texture.
	result = RenderRefractionToTexture();
	if (!result) {
		return false;
	}

	// Render the reflection of the scene to a texture.
	result = RenderReflectionToTexture();
	if (!result) {
		return false;
	}

	// Render the scene as normal to the back buffer.
	result = RenderScene();
	if (!result) {
		return false;
	}

	return true;
}

bool GraphicsClass::RenderRefractionToTexture() {

	XMFLOAT4 clipPlane;
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;

	clipPlane = XMFLOAT4(0.0f, -1.0f, 0.0f, m_waterHeight + 0.1f);

	m_RefractionTexture->SetRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView());

	m_RefractionTexture->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);

	m_Camera->Render();

	m_D3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);

	worldMatrix = XMMatrixTranslation(0.0f, 2.0f, 0.0f);

	m_BathModel->Render(m_D3D->GetDeviceContext());

	result = m_RefractionShader->Render(m_D3D->GetDeviceContext(), m_BathModel->GetIndexCount(), worldMatrix, viewMatrix,
		projectionMatrix, m_BathModel->GetTexture(), m_Light->GetDirection(),
		m_Light->GetAmbientColor(), m_Light->GetDiffuseColor(), clipPlane);
	if (!result) {
		return false;
	}

	m_D3D->SetBackBufferRenderTarget();

	return true;
}

bool GraphicsClass::RenderReflectionToTexture() {

	XMMATRIX reflectionViewMatrix, worldMatrix, projectionMatrix;
	bool result;


	m_ReflectionTexture->SetRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView());

	m_ReflectionTexture->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);

	m_Camera->RenderReflection(m_waterHeight);

	reflectionViewMatrix = m_Camera->GetReflectionViewMatrix();
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);

	worldMatrix = XMMatrixTranslation(0.0f, 6.0f, 8.0f);

	m_WallModel->Render(m_D3D->GetDeviceContext());

	result = m_LightShader->Render(m_D3D->GetDeviceContext(), m_WallModel->GetIndexCount(), worldMatrix, reflectionViewMatrix,
		projectionMatrix, m_WallModel->GetTexture(), m_Light->GetDirection(),
		m_Light->GetAmbientColor(), m_Light->GetDiffuseColor());
	if (!result) {
		return false;
	}

	m_D3D->SetBackBufferRenderTarget();

	return true;
}

bool GraphicsClass::RenderScene() {

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, reflectionMatrix;
	bool result;

	m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	m_Camera->Render();

	m_D3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);

	worldMatrix = XMMatrixTranslation(0.0f, 1.0f, 0.0f);

	m_GroundModel->Render(m_D3D->GetDeviceContext());

	result = m_LightShader->Render(m_D3D->GetDeviceContext(), m_GroundModel->GetIndexCount(), worldMatrix, viewMatrix,
		projectionMatrix, m_GroundModel->GetTexture(), m_Light->GetDirection(),
		m_Light->GetAmbientColor(), m_Light->GetDiffuseColor());
	if (!result) {
		return false;
	}

	m_D3D->GetWorldMatrix(worldMatrix);

	worldMatrix = XMMatrixTranslation(0.0f, 6.0f, 8.0f);

	m_WallModel->Render(m_D3D->GetDeviceContext());

	result = m_LightShader->Render(m_D3D->GetDeviceContext(), m_WallModel->GetIndexCount(), worldMatrix, viewMatrix,
		projectionMatrix, m_WallModel->GetTexture(), m_Light->GetDirection(),
		m_Light->GetAmbientColor(), m_Light->GetDiffuseColor());
	if (!result) {
		return false;
	}

	m_D3D->GetWorldMatrix(worldMatrix);

	worldMatrix = XMMatrixTranslation(0.0f, 2.0f, 0.0f);

	m_BathModel->Render(m_D3D->GetDeviceContext());

	result = m_LightShader->Render(m_D3D->GetDeviceContext(), m_BathModel->GetIndexCount(), worldMatrix, viewMatrix,
		projectionMatrix, m_BathModel->GetTexture(), m_Light->GetDirection(),
		m_Light->GetAmbientColor(), m_Light->GetDiffuseColor());
	if (!result) {
		return false;
	}

	m_D3D->GetWorldMatrix(worldMatrix);

	reflectionMatrix = m_Camera->GetReflectionViewMatrix();

	worldMatrix = XMMatrixTranslation(0.0f, m_waterHeight, 0.0f);

	m_WaterModel->Render(m_D3D->GetDeviceContext());

	result = m_WaterShader->Render(m_D3D->GetDeviceContext(), m_WaterModel->GetIndexCount(), worldMatrix, viewMatrix,
		projectionMatrix, reflectionMatrix, m_ReflectionTexture->GetShaderResourceView(),
		m_RefractionTexture->GetShaderResourceView(), m_WaterModel->GetTexture(),
		m_waterTranslation, 0.01f);
	if (!result) {
		return false;
	}

	m_D3D->EndScene();

	return true;
}