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
			m_Camera->SetPosition(0.0f, -1.0f, -10.0f);
			m_Camera->Render();
			m_Camera->RenderBaseViewMatrix();
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
		result = m_GroundModel->Initialize(m_D3D->GetDevice(), "../../tut49/data/plane01.txt", L"../../tut49/data/dirt.dds", 2.0f);
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
		result = m_Tree->Initialize(m_D3D->GetDevice(), "../../tut49/data/trees/trunk001.txt", L"../../tut49/data/trees/trunk001.dds", "../../tut49/data/trees/leaf001.txt", L"../../tut49/data/trees/leaf001.dds", 0.1f);
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
		result = m_RenderTexture->Initialize(m_D3D->GetDevice(), SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, SHADOWMAP_DEPTH, SHADOWMAP_NEAR);
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
		result = m_DepthShader->Initialize(m_D3D->GetDevice(), hwnd);
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
		result = m_TransparentDepthShader->Initialize(m_D3D->GetDevice(), hwnd);
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
		result = m_ShadowShader->Initialize(m_D3D->GetDevice(), hwnd);
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

	m_Camera->SetPosition(pos_x_, pos_y_, pos_z_);
	m_Camera->SetRotation(rot_x_, rot_y_, rot_z_);
	m_Camera->Render();

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

	m_D3D->BeginScene(0.0f, 0.5f, 0.8f, 1.0f);

	m_Camera->Render();

	m_Light->GenerateViewMatrix();

	m_D3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);

	m_Light->GetViewMatrix(lightViewMatrix);
	m_Light->GetOrthoMatrix(lightOrthoMatrix);

	diffuseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	ambientColor = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);

	m_GroundModel->GetPosition(posX, posY, posZ);
	worldMatrix = XMMatrixTranslation(posX, posY, posZ);

	m_GroundModel->Render(m_D3D->GetDeviceContext());

	m_ShadowShader->Render(
		m_D3D->GetDeviceContext(),
		m_GroundModel->GetIndexCount(), 
		worldMatrix, viewMatrix, projectionMatrix, lightViewMatrix,
		lightOrthoMatrix, m_GroundModel->GetTexture(),
		m_RenderTexture->GetShaderResourceView(),
		m_Light->GetDirection(),ambientColor, diffuseColor
	);

	m_D3D->GetWorldMatrix(worldMatrix);

	m_Tree->GetPosition(posX, posY, posZ);
	worldMatrix = XMMatrixTranslation(posX, posY, posZ);

	m_Tree->RenderTrunk(m_D3D->GetDeviceContext());

	m_ShadowShader->Render(m_D3D->GetDeviceContext(), 
		m_Tree->GetTrunkIndexCount(), 
		worldMatrix, viewMatrix, projectionMatrix, lightViewMatrix,
		lightOrthoMatrix, m_Tree->GetTrunkTexture(),
		m_RenderTexture->GetShaderResourceView(), m_Light->GetDirection(),ambientColor, diffuseColor
	);

	m_D3D->TurnOnAlphaBlending();

	m_Tree->RenderLeaves(m_D3D->GetDeviceContext());

	m_ShadowShader->Render(m_D3D->GetDeviceContext(),
		m_Tree->GetLeafIndexCount(), 
		worldMatrix, viewMatrix, projectionMatrix, lightViewMatrix,
		lightOrthoMatrix, m_Tree->GetLeafTexture(), 
		m_RenderTexture->GetShaderResourceView(), 
		m_Light->GetDirection(),ambientColor, diffuseColor
	);

	m_D3D->TurnOffAlphaBlending();

	m_D3D->EndScene();

	return true;
}

bool GraphicsClass::RenderSceneToTexture() {

	XMMATRIX worldMatrix, lightViewMatrix, lightOrthoMatrix;
	float posX, posY, posZ;
	bool result;

	m_RenderTexture->SetRenderTarget(m_D3D->GetDeviceContext());

	m_RenderTexture->ClearRenderTarget(m_D3D->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	m_D3D->GetWorldMatrix(worldMatrix);

	m_Light->GenerateViewMatrix();

	m_Light->GetViewMatrix(lightViewMatrix);
	m_Light->GetOrthoMatrix(lightOrthoMatrix);

	m_Tree->GetPosition(posX, posY, posZ);
	worldMatrix = XMMatrixTranslation(posX, posY, posZ);

	m_Tree->RenderTrunk(m_D3D->GetDeviceContext());
	
	m_DepthShader->Render(
		m_D3D->GetDeviceContext(), 
		m_Tree->GetTrunkIndexCount(), 
		worldMatrix, lightViewMatrix, lightOrthoMatrix
	);

	m_Tree->RenderLeaves(m_D3D->GetDeviceContext());
	result = m_TransparentDepthShader->Render(
		m_D3D->GetDeviceContext(), 
		m_Tree->GetLeafIndexCount(), 
		worldMatrix, lightViewMatrix, lightOrthoMatrix, 
		m_Tree->GetLeafTexture()
	);
	if (!result) {
		return false;
	}

	m_D3D->GetWorldMatrix(worldMatrix);

	m_GroundModel->GetPosition(posX, posY, posZ);
	worldMatrix = XMMatrixTranslation(posX, posY, posZ);

	m_GroundModel->Render(m_D3D->GetDeviceContext());
	m_DepthShader->Render(m_D3D->GetDeviceContext(), m_GroundModel->GetIndexCount(), worldMatrix, lightViewMatrix, lightOrthoMatrix);

	m_D3D->SetBackBufferRenderTarget();

	m_D3D->ResetViewport();

	return true;
}