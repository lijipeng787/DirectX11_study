#include "Graphics.h"

#include <new>

#include "../CommonFramework/TypeDefine.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/Camera.h"

#include "modelclass.h"
#include "lightclass.h"
#include "rendertextureclass.h"
#include "depthshaderclass.h"
#include "shadowshaderclass.h"

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
		camera_->SetPosition(0.0f, -1.0f, -10.0f);
		camera_->Render();
	}

	{
		m_CubeModel = new ModelClass();
		if (!m_CubeModel) {
			return false;
		}
		result = m_CubeModel->Initialize("../../tut41/data/cube.txt", L"../../tut41/data/wall01.dds");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the cube model object.", L"Error", MB_OK);
			return false;
		}

		m_CubeModel->SetPosition(-2.0f, 2.0f, 0.0f);
	}

	{
		m_SphereModel = new ModelClass();
		if (!m_SphereModel) {
			return false;
		}
		result = m_SphereModel->Initialize("../../tut41/data/sphere.txt", L"../../tut41/data/ice.dds");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the sphere model object.", L"Error", MB_OK);
			return false;
		}

		m_SphereModel->SetPosition(2.0f, 2.0f, 0.0f);
	}

	{
		m_GroundModel = new ModelClass();
		if (!m_GroundModel) {
			return false;
		}
		result = m_GroundModel->Initialize("../../tut41/data/plane01.txt", L"../../tut41/data/metal001.dds");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the ground model object.", L"Error", MB_OK);
			return false;
		}

		m_GroundModel->SetPosition(0.0f, 1.0f, 0.0f);
	}

	{
		light_ = (LightClass*)_aligned_malloc(sizeof(LightClass), 16);
		new (light_)LightClass();
		if (!light_) {
			return false;
		}

		light_->SetAmbientColor(0.15f, 0.15f, 0.15f, 1.0f);
		light_->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
		light_->SetLookAt(0.0f, 0.0f, 0.0f);
		light_->GenerateProjectionMatrix(SCREEN_DEPTH, SCREEN_NEAR);
	}

	{
		m_RenderTexture = (RenderTextureClass*)_aligned_malloc(sizeof(RenderTextureClass), 16);
		new (m_RenderTexture)RenderTextureClass();
		if (!m_RenderTexture) {
			return false;
		}

		result = m_RenderTexture->Initialize(SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, SCREEN_DEPTH, SCREEN_NEAR);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the render to texture object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_DepthShader = (DepthShaderClass*)_aligned_malloc(sizeof(DepthShaderClass), 16);
		new (m_DepthShader)DepthShaderClass();
		if (!m_DepthShader) {
			return false;
		}

		result = m_DepthShader->Initialize(hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the depth shader object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_ShadowShader = (ShadowShaderClass*)_aligned_malloc(sizeof(ShadowShaderClass), 16);
		new (m_ShadowShader)ShadowShaderClass();
		if (!m_ShadowShader) {
			return false;
		}
		result = m_ShadowShader->Initialize(hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the shadow shader object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_Light2 = (LightClass*)_aligned_malloc(sizeof(LightClass), 16);
		new (m_Light2)LightClass();
		if (!m_Light2) {
			return false;
		}

		m_Light2->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
		m_Light2->SetLookAt(0.0f, 0.0f, 0.0f);
		m_Light2->GenerateProjectionMatrix(SCREEN_DEPTH, SCREEN_NEAR);
	}

	{
		m_RenderTexture2 = (RenderTextureClass*)_aligned_malloc(sizeof(RenderTextureClass), 16);
		new (m_RenderTexture2)RenderTextureClass();
		if (!m_RenderTexture2) {
			return false;
		}
		result = m_RenderTexture2->Initialize(SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, SCREEN_DEPTH, SCREEN_NEAR);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the second render to texture object.", L"Error", MB_OK);
			return false;
		}
	}

	return true;
}

void GraphicsClass::Shutdown(){

	// Release the second render to texture object.
	if(m_RenderTexture2)
	{
		m_RenderTexture2->Shutdown();
		m_RenderTexture2->~RenderTextureClass();
		_aligned_free( m_RenderTexture2 );
		m_RenderTexture2 = 0;
	}

	// Release the second light object.
	if(m_Light2)
	{
		m_Light2->~LightClass();
		_aligned_free( m_Light2 );
		m_Light2 = 0;
	}

	// Release the shadow shader object.
	if(m_ShadowShader)
	{
		m_ShadowShader->Shutdown();
		m_ShadowShader->~ShadowShaderClass();
		_aligned_free( m_ShadowShader );
		m_ShadowShader = 0;
	}

	// Release the depth shader object.
	if(m_DepthShader)
	{
		m_DepthShader->Shutdown();
		m_DepthShader->~DepthShaderClass();
		_aligned_free( m_DepthShader );
		m_DepthShader = 0;
	}

	
	if(m_RenderTexture)
	{
		m_RenderTexture->Shutdown();
		m_RenderTexture->~RenderTextureClass();
		_aligned_free( m_RenderTexture );
		m_RenderTexture = 0;
	}

	// Release the light object.
	if(light_)
	{
		light_->~LightClass();
		_aligned_free( light_ );
		light_ = nullptr;;
	}

	// Release the ground model object.
	if(m_GroundModel)
	{
		m_GroundModel->Shutdown();
		delete m_GroundModel;
		m_GroundModel = 0;
	}

	// Release the sphere model object.
	if(m_SphereModel)
	{
		m_SphereModel->Shutdown();
		delete m_SphereModel;
		m_SphereModel = 0;
	}

	// Release the cube model object.
	if(m_CubeModel)
	{
		m_CubeModel->Shutdown();
		delete m_CubeModel;
		m_CubeModel = 0;
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
	camera_->SetPosition(pos_x_, pos_y_, pos_z_);
	camera_->SetRotation(rot_x_, rot_y_, rot_z_);

	// Set the position of the first light.
	light_->SetPosition(5.0f, 8.0f, -5.0f);

	// Set the position of the second light.
	m_Light2->SetPosition(-5.0f, 8.0f, -5.0f);

	// Render the graphics scene.
	result = Render();
	if (!result) {
		return false;
	}

	return true;
}

bool GraphicsClass::RenderSceneToTexture() {

	XMMATRIX worldMatrix, lightViewMatrix, lightProjectionMatrix, translateMatrix;
	float posX, posY, posZ;
	bool result;

	m_RenderTexture->SetRenderTarget(directx_device_->GetDeviceContext());

	m_RenderTexture->ClearRenderTarget(directx_device_->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	light_->GenerateViewMatrix();

	directx_device_->GetWorldMatrix(worldMatrix);

	light_->GetViewMatrix(lightViewMatrix);
	light_->GetProjectionMatrix(lightProjectionMatrix);

	m_CubeModel->GetPosition(posX, posY, posZ);
	worldMatrix = XMMatrixTranslation(posX, posY, posZ);

	m_CubeModel->Render(directx_device_->GetDeviceContext());
	result = m_DepthShader->Render(directx_device_->GetDeviceContext(), m_CubeModel->GetIndexCount(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	if (!result) {
		return false;
	}

	directx_device_->GetWorldMatrix(worldMatrix);

	m_SphereModel->GetPosition(posX, posY, posZ);
	worldMatrix = XMMatrixTranslation(posX, posY, posZ);

	m_SphereModel->Render(directx_device_->GetDeviceContext());
	result = m_DepthShader->Render(directx_device_->GetDeviceContext(), m_SphereModel->GetIndexCount(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	if (!result) {
		return false;
	}

	directx_device_->GetWorldMatrix(worldMatrix);

	m_GroundModel->GetPosition(posX, posY, posZ);
	worldMatrix = XMMatrixTranslation(posX, posY, posZ);

	m_GroundModel->Render(directx_device_->GetDeviceContext());
	result = m_DepthShader->Render(directx_device_->GetDeviceContext(), m_GroundModel->GetIndexCount(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	if (!result) {
		return false;
	}

	directx_device_->SetBackBufferRenderTarget();

	directx_device_->ResetViewport();

	return true;
}

bool GraphicsClass::RenderSceneToTexture2() {

	XMMATRIX worldMatrix, lightViewMatrix, lightProjectionMatrix, translateMatrix;
	float posX, posY, posZ;
	bool result;

	m_RenderTexture2->SetRenderTarget(directx_device_->GetDeviceContext());

	m_RenderTexture2->ClearRenderTarget(directx_device_->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	m_Light2->GenerateViewMatrix();

	directx_device_->GetWorldMatrix(worldMatrix);

	m_Light2->GetViewMatrix(lightViewMatrix);
	m_Light2->GetProjectionMatrix(lightProjectionMatrix);

	m_CubeModel->GetPosition(posX, posY, posZ);
	worldMatrix = XMMatrixTranslation(posX, posY, posZ);

	m_CubeModel->Render(directx_device_->GetDeviceContext());
	result = m_DepthShader->Render(directx_device_->GetDeviceContext(), m_CubeModel->GetIndexCount(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	if (!result) {
		return false;
	}

	directx_device_->GetWorldMatrix(worldMatrix);

	m_SphereModel->GetPosition(posX, posY, posZ);
	worldMatrix = XMMatrixTranslation(posX, posY, posZ);

	m_SphereModel->Render(directx_device_->GetDeviceContext());
	result = m_DepthShader->Render(directx_device_->GetDeviceContext(), m_SphereModel->GetIndexCount(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	if (!result) {
		return false;
	}

	directx_device_->GetWorldMatrix(worldMatrix);

	m_GroundModel->GetPosition(posX, posY, posZ);
	worldMatrix = XMMatrixTranslation(posX, posY, posZ);

	m_GroundModel->Render(directx_device_->GetDeviceContext());
	result = m_DepthShader->Render(directx_device_->GetDeviceContext(), m_GroundModel->GetIndexCount(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	if (!result) {
		return false;
	}

	directx_device_->SetBackBufferRenderTarget();

	directx_device_->ResetViewport();

	return true;
}

bool GraphicsClass::Render() {

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, translateMatrix;
	XMMATRIX lightViewMatrix, lightProjectionMatrix;
	XMMATRIX lightViewMatrix2, lightProjectionMatrix2;
	bool result;
	float posX, posY, posZ;

	result = RenderSceneToTexture();
	if (!result) {
		return false;
	}

	result = RenderSceneToTexture2();
	if (!result) {
		return false;
	}

	directx_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	light_->GenerateViewMatrix();

	m_Light2->GenerateViewMatrix();

	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetWorldMatrix(worldMatrix);
	directx_device_->GetProjectionMatrix(projectionMatrix);

	light_->GetViewMatrix(lightViewMatrix);
	light_->GetProjectionMatrix(lightProjectionMatrix);

	m_Light2->GetViewMatrix(lightViewMatrix2);
	m_Light2->GetProjectionMatrix(lightProjectionMatrix2);

	m_CubeModel->GetPosition(posX, posY, posZ);
	worldMatrix = XMMatrixTranslation(posX, posY, posZ);

	m_CubeModel->Render(directx_device_->GetDeviceContext());

	result = m_ShadowShader->Render(
		directx_device_->GetDeviceContext(),
		m_CubeModel->GetIndexCount(),
		worldMatrix, viewMatrix, projectionMatrix, lightViewMatrix, lightProjectionMatrix,
		m_CubeModel->GetTexture(), m_RenderTexture->GetShaderResourceView(),
		light_->GetPosition(), light_->GetAmbientColor(), light_->GetDiffuseColor(),
		lightViewMatrix2, lightProjectionMatrix2,
		m_RenderTexture2->GetShaderResourceView(),
		m_Light2->GetPosition(), m_Light2->GetDiffuseColor()
	);
	if (!result) {
		return false;
	}

	directx_device_->GetWorldMatrix(worldMatrix);

	m_SphereModel->GetPosition(posX, posY, posZ);
	worldMatrix = XMMatrixTranslation(posX, posY, posZ);

	m_SphereModel->Render(directx_device_->GetDeviceContext());
	result = m_ShadowShader->Render(
		directx_device_->GetDeviceContext(),
		m_SphereModel->GetIndexCount(),
		worldMatrix, viewMatrix, projectionMatrix, lightViewMatrix, lightProjectionMatrix,
		m_SphereModel->GetTexture(), m_RenderTexture->GetShaderResourceView(),
		light_->GetPosition(), light_->GetAmbientColor(), light_->GetDiffuseColor(),
		lightViewMatrix2, lightProjectionMatrix2,
		m_RenderTexture2->GetShaderResourceView(), m_Light2->GetPosition(), m_Light2->GetDiffuseColor()
	);
	if (!result) {
		return false;
	}

	directx_device_->GetWorldMatrix(worldMatrix);

	m_GroundModel->GetPosition(posX, posY, posZ);
	worldMatrix = XMMatrixTranslation(posX, posY, posZ);

	m_GroundModel->Render(directx_device_->GetDeviceContext());
	result = m_ShadowShader->Render(
		directx_device_->GetDeviceContext(),
		m_GroundModel->GetIndexCount(),
		worldMatrix, viewMatrix, projectionMatrix, lightViewMatrix, lightProjectionMatrix,
		m_GroundModel->GetTexture(), m_RenderTexture->GetShaderResourceView(), light_->GetPosition(),
		light_->GetAmbientColor(), light_->GetDiffuseColor(),
		lightViewMatrix2, lightProjectionMatrix2,
		m_RenderTexture2->GetShaderResourceView(), m_Light2->GetPosition(), m_Light2->GetDiffuseColor()
	);
	if (!result) {
		return false;
	}

	directx_device_->EndScene();

	return true;
}