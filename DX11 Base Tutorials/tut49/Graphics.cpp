#include "Graphics.h"

#include <new>

#include "../CommonFramework/TypeDefine.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/Camera.h"

#include "lightclass.h"
#include "modelclass.h"
#include "treeclass.h"
#include "rendertextureclass.h"
#include "depthshaderclass.h"
#include "transparentdepthshaderclass.h"
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
			camera_->RenderBaseViewMatrix();
	}

	{
		m_Light = (LightClass*)_aligned_malloc(sizeof(LightClass), 16);
		new (m_Light)LightClass();
		if (!m_Light) {
			return false;
		}
		m_Light->GenerateOrthoMatrix(15.0f, 15.0f, SHADOWMAP_DEPTH, SHADOWMAP_NEAR);
	}

	{
		m_GroundModel = new ModelClass;
		if (!m_GroundModel) {
			return false;
		}
		result = m_GroundModel->Initialize(directx_device_->GetDevice(), "../../tut49/data/plane01.txt", L"../../tut49/data/dirt.dds", 2.0f);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the ground model object.", L"Error", MB_OK);
			return false;
		}
		m_GroundModel->SetPosition(0.0f, 1.0f, 0.0f);
	}

	{
		m_Tree = new TreeClass;
		if (!m_Tree) {
			return false;
		}
		result = m_Tree->Initialize(directx_device_->GetDevice(), "../../tut49/data/trees/trunk001.txt", L"../../tut49/data/trees/trunk001.dds", "../../tut49/data/trees/leaf001.txt", L"../../tut49/data/trees/leaf001.dds", 0.1f);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the tree object.", L"Error", MB_OK);
			return false;
		}
		m_Tree->SetPosition(0.0f, 1.0f, 0.0f);
	}

	{
		m_RenderTexture = (RenderTextureClass*)_aligned_malloc(sizeof(RenderTextureClass), 16);
		new (m_RenderTexture)RenderTextureClass();
		if (!m_RenderTexture) {
			return false;
		}
		result = m_RenderTexture->Initialize(directx_device_->GetDevice(), SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, SHADOWMAP_DEPTH, SHADOWMAP_NEAR);
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
		result = m_DepthShader->Initialize(directx_device_->GetDevice(), hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the depth shader object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_TransparentDepthShader = (TransparentDepthShaderClass*)_aligned_malloc(sizeof(TransparentDepthShaderClass), 16);
		new (m_TransparentDepthShader)TransparentDepthShaderClass();
		if (!m_TransparentDepthShader) {
			return false;
		}
		result = m_TransparentDepthShader->Initialize(directx_device_->GetDevice(), hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the transparent depth shader object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_ShadowShader = (ShadowShaderClass*)_aligned_malloc(sizeof(ShadowShaderClass), 16);
		new (m_ShadowShader)ShadowShaderClass();
		if (!m_ShadowShader) {
			return false;
		}
		result = m_ShadowShader->Initialize(directx_device_->GetDevice(), hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the shadow shader object.", L"Error", MB_OK);
			return false;
		}
	}

	return true;
}

void GraphicsClass::Shutdown() {

	// Release the shadow shader object.
	if (m_ShadowShader) {
		m_ShadowShader->Shutdown();
		m_ShadowShader->~ShadowShaderClass();
		_aligned_free(m_ShadowShader);
		m_ShadowShader = 0;
	}

	// Release the transparent depth shader object.
	if (m_TransparentDepthShader) {
		m_TransparentDepthShader->Shutdown();
		m_TransparentDepthShader->~TransparentDepthShaderClass();
		_aligned_free(m_TransparentDepthShader);
		m_TransparentDepthShader = 0;
	}

	// Release the depth shader object.
	if (m_DepthShader) {
		m_DepthShader->Shutdown();
		m_DepthShader->~DepthShaderClass();
		_aligned_free(m_DepthShader);
		m_DepthShader = 0;
	}

	// Release the render to texture object.
	if (m_RenderTexture) {
		m_RenderTexture->Shutdown();
		m_RenderTexture->~RenderTextureClass();
		_aligned_free(m_RenderTexture);
		m_RenderTexture = 0;
	}

	// Release the tree object.
	if (m_Tree) {
		m_Tree->Shutdown();
		delete m_Tree;
		m_Tree = 0;
	}

	// Release the ground model object.
	if (m_GroundModel) {
		m_GroundModel->Shutdown();
		delete m_GroundModel;
		m_GroundModel = 0;
	}

	// Release the light object.
	if (m_Light) {
		m_Light->~LightClass();
		_aligned_free(m_Light);
		m_Light = 0;
	}
}

bool GraphicsClass::Frame() {

	camera_->SetPosition(pos_x_, pos_y_, pos_z_);
	camera_->SetRotation(rot_x_, rot_y_, rot_z_);
	camera_->Render();

	UpdateLighting();

	// Render the graphics.
	auto result = Render();
	if (!result) {
		return false;
	}

	return true;
}

void GraphicsClass::UpdateLighting() {

	static float angle = 270.0f;
	float radians;
	static float offsetX = 9.0f;

	angle -= 0.03f * delta_time_;
	if (angle < 90.0f) {
		angle = 270.0f;
		offsetX = 9.0f;
	}
	radians = angle * 0.0174532925f;
	m_Light->SetDirection(sinf(radians), cosf(radians), 0.0f);

	offsetX -= 0.003f * delta_time_;
	m_Light->SetPosition(0.0f + offsetX, 10.0f, 1.0f);
	m_Light->SetLookAt(0.0f - offsetX, 0.0f, 2.0f);
}

bool GraphicsClass::Render() {

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, lightViewMatrix, lightOrthoMatrix;
	XMFLOAT4 ambientColor, diffuseColor;
	float posX, posY, posZ;
	bool result;

	result = RenderSceneToTexture();
	if (!result) {
		return false;
	}

	directx_device_->BeginScene(0.0f, 0.5f, 0.8f, 1.0f);

	camera_->Render();

	m_Light->GenerateViewMatrix();

	directx_device_->GetWorldMatrix(worldMatrix);
	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetProjectionMatrix(projectionMatrix);

	m_Light->GetViewMatrix(lightViewMatrix);
	m_Light->GetOrthoMatrix(lightOrthoMatrix);

	diffuseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	ambientColor = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);

	m_GroundModel->GetPosition(posX, posY, posZ);
	worldMatrix = XMMatrixTranslation(posX, posY, posZ);

	m_GroundModel->Render(directx_device_->GetDeviceContext());

	m_ShadowShader->Render(
		directx_device_->GetDeviceContext(),
		m_GroundModel->GetIndexCount(), 
		worldMatrix, viewMatrix, projectionMatrix, lightViewMatrix,
		lightOrthoMatrix, m_GroundModel->GetTexture(),
		m_RenderTexture->GetShaderResourceView(),
		m_Light->GetDirection(),ambientColor, diffuseColor
	);

	directx_device_->GetWorldMatrix(worldMatrix);

	m_Tree->GetPosition(posX, posY, posZ);
	worldMatrix = XMMatrixTranslation(posX, posY, posZ);

	m_Tree->RenderTrunk(directx_device_->GetDeviceContext());

	m_ShadowShader->Render(directx_device_->GetDeviceContext(), 
		m_Tree->GetTrunkIndexCount(), 
		worldMatrix, viewMatrix, projectionMatrix, lightViewMatrix,
		lightOrthoMatrix, m_Tree->GetTrunkTexture(),
		m_RenderTexture->GetShaderResourceView(), m_Light->GetDirection(),ambientColor, diffuseColor
	);

	directx_device_->TurnOnAlphaBlending();

	m_Tree->RenderLeaves(directx_device_->GetDeviceContext());

	m_ShadowShader->Render(directx_device_->GetDeviceContext(),
		m_Tree->GetLeafIndexCount(), 
		worldMatrix, viewMatrix, projectionMatrix, lightViewMatrix,
		lightOrthoMatrix, m_Tree->GetLeafTexture(), 
		m_RenderTexture->GetShaderResourceView(), 
		m_Light->GetDirection(),ambientColor, diffuseColor
	);

	directx_device_->TurnOffAlphaBlending();

	directx_device_->EndScene();

	return true;
}

bool GraphicsClass::RenderSceneToTexture() {

	XMMATRIX worldMatrix, lightViewMatrix, lightOrthoMatrix;
	float posX, posY, posZ;
	bool result;

	m_RenderTexture->SetRenderTarget(directx_device_->GetDeviceContext());

	m_RenderTexture->ClearRenderTarget(directx_device_->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	directx_device_->GetWorldMatrix(worldMatrix);

	m_Light->GenerateViewMatrix();

	m_Light->GetViewMatrix(lightViewMatrix);
	m_Light->GetOrthoMatrix(lightOrthoMatrix);

	m_Tree->GetPosition(posX, posY, posZ);
	worldMatrix = XMMatrixTranslation(posX, posY, posZ);

	m_Tree->RenderTrunk(directx_device_->GetDeviceContext());
	
	m_DepthShader->Render(
		directx_device_->GetDeviceContext(), 
		m_Tree->GetTrunkIndexCount(), 
		worldMatrix, lightViewMatrix, lightOrthoMatrix
	);

	m_Tree->RenderLeaves(directx_device_->GetDeviceContext());
	result = m_TransparentDepthShader->Render(
		directx_device_->GetDeviceContext(), 
		m_Tree->GetLeafIndexCount(), 
		worldMatrix, lightViewMatrix, lightOrthoMatrix, 
		m_Tree->GetLeafTexture()
	);
	if (!result) {
		return false;
	}

	directx_device_->GetWorldMatrix(worldMatrix);

	m_GroundModel->GetPosition(posX, posY, posZ);
	worldMatrix = XMMatrixTranslation(posX, posY, posZ);

	m_GroundModel->Render(directx_device_->GetDeviceContext());
	m_DepthShader->Render(directx_device_->GetDeviceContext(), m_GroundModel->GetIndexCount(), worldMatrix, lightViewMatrix, lightOrthoMatrix);

	directx_device_->SetBackBufferRenderTarget();

	directx_device_->ResetViewport();

	return true;
}