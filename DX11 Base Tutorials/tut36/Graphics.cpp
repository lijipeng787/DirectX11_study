#include "Graphics.h"

#include <new>

#include "../CommonFramework/TypeDefine.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/Camera.h"

#include "modelclass.h"
#include "horizontalblurshaderclass.h"
#include "verticalblurshaderclass.h"
#include "rendertextureclass.h"
#include "orthowindowclass.h"
#include "textureshaderclass.h"

GraphicsClass::GraphicsClass() {}

GraphicsClass::~GraphicsClass() {}

bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd) {

	bool result;
	int downSampleWidth, downSampleHeight;

	// Set the size to sample down to.
	downSampleWidth = screenWidth / 2;
	downSampleHeight = screenHeight / 2;

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
	}

	{
		m_Model = new ModelClass();
		if (!m_Model) {
			return false;
		}
		result = m_Model->Initialize(m_D3D->GetDevice(), "../../tut36/data/cube.txt", L"../../tut36/data/seafloor.dds");
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
		result = m_TextureShader->Initialize(m_D3D->GetDevice(), hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the texture shader object.", L"Error", MB_OK);
			return false;
		}

	}

	{
		m_HorizontalBlurShader = (HorizontalBlurShaderClass*)_aligned_malloc(sizeof(HorizontalBlurShaderClass), 16);
		new (m_HorizontalBlurShader)HorizontalBlurShaderClass();
		if (!m_HorizontalBlurShader) {
			return false;
		}
		result = m_HorizontalBlurShader->Initialize(m_D3D->GetDevice(), hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the horizontal blur shader object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_VerticalBlurShader = (VerticalBlurShaderClass*)_aligned_malloc(sizeof(VerticalBlurShaderClass), 16);
		new (m_VerticalBlurShader)VerticalBlurShaderClass();
		if (!m_VerticalBlurShader) {
			return false;
		}
		result = m_VerticalBlurShader->Initialize(m_D3D->GetDevice(), hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the vertical blur shader object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_RenderTexture = new RenderTextureClass();
		if (!m_RenderTexture) {
			return false;
		}
		result = m_RenderTexture->Initialize(m_D3D->GetDevice(), screenWidth, screenHeight, SCREEN_DEPTH, SCREEN_NEAR);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the render to texture object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_DownSampleTexure = new RenderTextureClass();
		if (!m_DownSampleTexure) {
			return false;
		}
		result = m_DownSampleTexure->Initialize(m_D3D->GetDevice(), downSampleWidth, downSampleHeight, SCREEN_DEPTH, SCREEN_NEAR);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the down sample render to texture object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_HorizontalBlurTexture = new RenderTextureClass();
		if (!m_HorizontalBlurTexture) {
			return false;
		}
		result = m_HorizontalBlurTexture->Initialize(m_D3D->GetDevice(), downSampleWidth, downSampleHeight, SCREEN_DEPTH, SCREEN_NEAR);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the horizontal blur render to texture object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_VerticalBlurTexture = new RenderTextureClass();
		if (!m_VerticalBlurTexture) {
			return false;
		}
		result = m_VerticalBlurTexture->Initialize(m_D3D->GetDevice(), downSampleWidth, downSampleHeight, SCREEN_DEPTH, SCREEN_NEAR);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the vertical blur render to texture object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_UpSampleTexure = new RenderTextureClass();
		if (!m_UpSampleTexure) {
			return false;
		}
		result = m_UpSampleTexure->Initialize(m_D3D->GetDevice(), screenWidth, screenHeight, SCREEN_DEPTH, SCREEN_NEAR);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the up sample render to texture object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_SmallWindow = new OrthoWindowClass();
		if (!m_SmallWindow) {
			return false;
		}
		result = m_SmallWindow->Initialize(m_D3D->GetDevice(), downSampleWidth, downSampleHeight);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the small ortho window object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_FullScreenWindow = new OrthoWindowClass();
		if (!m_FullScreenWindow) {
			return false;
		}
		result = m_FullScreenWindow->Initialize(m_D3D->GetDevice(), screenWidth, screenHeight);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the full screen ortho window object.", L"Error", MB_OK);
			return false;
		}
	}

	return true;
}

void GraphicsClass::Shutdown(){

	// Release the full screen ortho window object.
	if(m_FullScreenWindow)
	{
		m_FullScreenWindow->Shutdown();
		delete m_FullScreenWindow;
		m_FullScreenWindow = 0;
	}

	// Release the small ortho window object.
	if(m_SmallWindow)
	{
		m_SmallWindow->Shutdown();
		delete m_SmallWindow;
		m_SmallWindow = 0;
	}
	
	// Release the up sample render to texture object.
	if(m_UpSampleTexure)
	{
		m_UpSampleTexure->Shutdown();
		delete m_UpSampleTexure;
		m_UpSampleTexure = 0;
	}

	// Release the vertical blur render to texture object.
	if(m_VerticalBlurTexture)
	{
		m_VerticalBlurTexture->Shutdown();
		delete m_VerticalBlurTexture;
		m_VerticalBlurTexture = 0;
	}

	// Release the horizontal blur render to texture object.
	if(m_HorizontalBlurTexture)
	{
		m_HorizontalBlurTexture->Shutdown();
		delete m_HorizontalBlurTexture;
		m_HorizontalBlurTexture = 0;
	}

	// Release the down sample render to texture object.
	if(m_DownSampleTexure)
	{
		m_DownSampleTexure->Shutdown();
		delete m_DownSampleTexure;
		m_DownSampleTexure = 0;
	}
	
	// Release the render to texture object.
	if(m_RenderTexture)
	{
		m_RenderTexture->Shutdown();
		delete m_RenderTexture;
		m_RenderTexture = 0;
	}
	
	// Release the vertical blur shader object.
	if(m_VerticalBlurShader)
	{
		m_VerticalBlurShader->Shutdown();
		m_VerticalBlurShader->~VerticalBlurShaderClass();
		_aligned_free( m_VerticalBlurShader );
		m_VerticalBlurShader = 0;
	}

	// Release the horizontal blur shader object.
	if(m_HorizontalBlurShader)
	{
		m_HorizontalBlurShader->Shutdown();
		m_HorizontalBlurShader->~HorizontalBlurShaderClass();
		_aligned_free( m_HorizontalBlurShader );
		m_HorizontalBlurShader = 0;
	}

	// Release the texture shader object.
	if(m_TextureShader)
	{
		m_TextureShader->Shutdown();
		m_TextureShader->~TextureShaderClass();
		_aligned_free( m_TextureShader );
		m_TextureShader = 0;
	}

	// Release the model object.
	if(m_Model)
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
	static float rotation = 0.0f;

	rotation += (float)XM_PI * 0.005f;
	if (rotation > 360.0f) {
		rotation -= 360.0f;
	}

	result = Render();
	if (!result) {
		return false;
	}

	return true;
}

bool GraphicsClass::Render(){

	bool result;

	result = RenderSceneToTexture(rotation_);
	if(!result){
		return false;
	}

	result = DownSampleTexture();
	if(!result){
		return false;
	}


	result = RenderHorizontalBlurToTexture();
	if(!result){
		return false;
	}

	result = RenderVerticalBlurToTexture();
	if(!result){
		return false;
	}

	result = UpSampleTexture();
	if(!result){
		return false;
	}

	result = Render2DTextureScene();
	if(!result){
		return false;
	}

	return true;
}

bool GraphicsClass::RenderSceneToTexture(float rotation) {

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;

	m_RenderTexture->SetRenderTarget(m_D3D->GetDeviceContext());

	m_RenderTexture->ClearRenderTarget(m_D3D->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	m_Camera->Render();

	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);

	worldMatrix = XMMatrixRotationY(rotation);

	m_Model->Render(m_D3D->GetDeviceContext());

	result = m_TextureShader->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
		m_Model->GetTexture());
	if (!result) {
		return false;
	}

	m_D3D->SetBackBufferRenderTarget();

	m_D3D->ResetViewport();

	return true;
}

bool GraphicsClass::DownSampleTexture() {

	XMMATRIX worldMatrix, viewMatrix, orthoMatrix;
	bool result;

	m_DownSampleTexure->SetRenderTarget(m_D3D->GetDeviceContext());

	m_DownSampleTexure->ClearRenderTarget(m_D3D->GetDeviceContext(), 0.0f, 1.0f, 0.0f, 1.0f);

	m_Camera->Render();

	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);

	m_DownSampleTexure->GetOrthoMatrix(orthoMatrix);

	m_D3D->TurnZBufferOff();

	m_SmallWindow->Render(m_D3D->GetDeviceContext());

	result = m_TextureShader->Render(m_D3D->GetDeviceContext(), m_SmallWindow->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix,
		m_RenderTexture->GetShaderResourceView());
	if (!result) {
		return false;
	}

	m_D3D->TurnZBufferOn();

	m_D3D->SetBackBufferRenderTarget();

	m_D3D->ResetViewport();

	return true;
}

bool GraphicsClass::RenderHorizontalBlurToTexture() {

	XMMATRIX worldMatrix, viewMatrix, orthoMatrix;
	float screenSizeX;
	bool result;

	screenSizeX = (float)m_HorizontalBlurTexture->GetTextureWidth();

	m_HorizontalBlurTexture->SetRenderTarget(m_D3D->GetDeviceContext());

	m_HorizontalBlurTexture->ClearRenderTarget(m_D3D->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	m_Camera->Render();

	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);

	m_HorizontalBlurTexture->GetOrthoMatrix(orthoMatrix);

	m_D3D->TurnZBufferOff();

	m_SmallWindow->Render(m_D3D->GetDeviceContext());

	result = m_HorizontalBlurShader->Render(m_D3D->GetDeviceContext(), m_SmallWindow->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix,
		m_DownSampleTexure->GetShaderResourceView(), screenSizeX);
	if (!result) {
		return false;
	}

	m_D3D->TurnZBufferOn();

	m_D3D->SetBackBufferRenderTarget();

	m_D3D->ResetViewport();

	return true;
}

bool GraphicsClass::RenderVerticalBlurToTexture() {

	XMMATRIX worldMatrix, viewMatrix, orthoMatrix;
	float screenSizeY;
	bool result;

	screenSizeY = (float)m_VerticalBlurTexture->GetTextureHeight();

	m_VerticalBlurTexture->SetRenderTarget(m_D3D->GetDeviceContext());

	m_VerticalBlurTexture->ClearRenderTarget(m_D3D->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	m_Camera->Render();

	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);

	m_VerticalBlurTexture->GetOrthoMatrix(orthoMatrix);

	m_D3D->TurnZBufferOff();

	m_SmallWindow->Render(m_D3D->GetDeviceContext());

	result = m_VerticalBlurShader->Render(m_D3D->GetDeviceContext(), m_SmallWindow->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix,
		m_HorizontalBlurTexture->GetShaderResourceView(), screenSizeY);
	if (!result) {
		return false;
	}

	m_D3D->TurnZBufferOn();

	m_D3D->SetBackBufferRenderTarget();

	m_D3D->ResetViewport();

	return true;
}

bool GraphicsClass::UpSampleTexture() {

	XMMATRIX worldMatrix, viewMatrix, orthoMatrix;
	bool result;

	m_UpSampleTexure->SetRenderTarget(m_D3D->GetDeviceContext());

	m_UpSampleTexure->ClearRenderTarget(m_D3D->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	m_Camera->Render();

	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);

	m_UpSampleTexure->GetOrthoMatrix(orthoMatrix);

	m_D3D->TurnZBufferOff();

	m_FullScreenWindow->Render(m_D3D->GetDeviceContext());

	result = m_TextureShader->Render(m_D3D->GetDeviceContext(), m_FullScreenWindow->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix,
		m_VerticalBlurTexture->GetShaderResourceView());
	if (!result) {
		return false;
	}

	m_D3D->TurnZBufferOn();

	m_D3D->SetBackBufferRenderTarget();

	m_D3D->ResetViewport();

	return true;
}

bool GraphicsClass::Render2DTextureScene() {

	XMMATRIX worldMatrix, viewMatrix, orthoMatrix;
	bool result;

	m_D3D->BeginScene(1.0f, 0.0f, 0.0f, 0.0f);

	m_Camera->Render();

	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetOrthoMatrix(orthoMatrix);

	m_D3D->TurnZBufferOff();

	m_FullScreenWindow->Render(m_D3D->GetDeviceContext());

	result = m_TextureShader->Render(m_D3D->GetDeviceContext(), m_FullScreenWindow->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix,
		m_UpSampleTexure->GetShaderResourceView());
	if (!result) {
		return false;
	}

	m_D3D->TurnZBufferOn();

	m_D3D->EndScene();

	return true;
}