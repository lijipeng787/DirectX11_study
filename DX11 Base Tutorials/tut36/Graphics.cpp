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
	}

	{
		model_ = new ModelClass();
		if (!model_) {
			return false;
		}
		result = model_->Initialize(directx_device_->GetDevice(), "../../tut36/data/cube.txt", L"../../tut36/data/seafloor.dds");
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
		m_HorizontalBlurShader = (HorizontalBlurShaderClass*)_aligned_malloc(sizeof(HorizontalBlurShaderClass), 16);
		new (m_HorizontalBlurShader)HorizontalBlurShaderClass();
		if (!m_HorizontalBlurShader) {
			return false;
		}
		result = m_HorizontalBlurShader->Initialize(directx_device_->GetDevice(), hwnd);
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
		result = m_VerticalBlurShader->Initialize(directx_device_->GetDevice(), hwnd);
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
		result = m_RenderTexture->Initialize(directx_device_->GetDevice(), screenWidth, screenHeight, SCREEN_DEPTH, SCREEN_NEAR);
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
		result = m_DownSampleTexure->Initialize(directx_device_->GetDevice(), downSampleWidth, downSampleHeight, SCREEN_DEPTH, SCREEN_NEAR);
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
		result = m_HorizontalBlurTexture->Initialize(directx_device_->GetDevice(), downSampleWidth, downSampleHeight, SCREEN_DEPTH, SCREEN_NEAR);
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
		result = m_VerticalBlurTexture->Initialize(directx_device_->GetDevice(), downSampleWidth, downSampleHeight, SCREEN_DEPTH, SCREEN_NEAR);
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
		result = m_UpSampleTexure->Initialize(directx_device_->GetDevice(), screenWidth, screenHeight, SCREEN_DEPTH, SCREEN_NEAR);
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
		result = m_SmallWindow->Initialize(directx_device_->GetDevice(), downSampleWidth, downSampleHeight);
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
		result = m_FullScreenWindow->Initialize(directx_device_->GetDevice(), screenWidth, screenHeight);
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
	if(model_)
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

	m_RenderTexture->SetRenderTarget(directx_device_->GetDeviceContext());

	m_RenderTexture->ClearRenderTarget(directx_device_->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetWorldMatrix(worldMatrix);
	directx_device_->GetProjectionMatrix(projectionMatrix);

	worldMatrix = XMMatrixRotationY(rotation);

	model_->Render(directx_device_->GetDeviceContext());

	result = m_TextureShader->Render(directx_device_->GetDeviceContext(), model_->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
		model_->GetTexture());
	if (!result) {
		return false;
	}

	directx_device_->SetBackBufferRenderTarget();

	directx_device_->ResetViewport();

	return true;
}

bool GraphicsClass::DownSampleTexture() {

	XMMATRIX worldMatrix, viewMatrix, orthoMatrix;
	bool result;

	m_DownSampleTexure->SetRenderTarget(directx_device_->GetDeviceContext());

	m_DownSampleTexure->ClearRenderTarget(directx_device_->GetDeviceContext(), 0.0f, 1.0f, 0.0f, 1.0f);

	camera_->Render();

	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetWorldMatrix(worldMatrix);

	m_DownSampleTexure->GetOrthoMatrix(orthoMatrix);

	directx_device_->TurnZBufferOff();

	m_SmallWindow->Render(directx_device_->GetDeviceContext());

	result = m_TextureShader->Render(directx_device_->GetDeviceContext(), m_SmallWindow->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix,
		m_RenderTexture->GetShaderResourceView());
	if (!result) {
		return false;
	}

	directx_device_->TurnZBufferOn();

	directx_device_->SetBackBufferRenderTarget();

	directx_device_->ResetViewport();

	return true;
}

bool GraphicsClass::RenderHorizontalBlurToTexture() {

	XMMATRIX worldMatrix, viewMatrix, orthoMatrix;
	float screenSizeX;
	bool result;

	screenSizeX = (float)m_HorizontalBlurTexture->GetTextureWidth();

	m_HorizontalBlurTexture->SetRenderTarget(directx_device_->GetDeviceContext());

	m_HorizontalBlurTexture->ClearRenderTarget(directx_device_->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetWorldMatrix(worldMatrix);

	m_HorizontalBlurTexture->GetOrthoMatrix(orthoMatrix);

	directx_device_->TurnZBufferOff();

	m_SmallWindow->Render(directx_device_->GetDeviceContext());

	result = m_HorizontalBlurShader->Render(directx_device_->GetDeviceContext(), m_SmallWindow->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix,
		m_DownSampleTexure->GetShaderResourceView(), screenSizeX);
	if (!result) {
		return false;
	}

	directx_device_->TurnZBufferOn();

	directx_device_->SetBackBufferRenderTarget();

	directx_device_->ResetViewport();

	return true;
}

bool GraphicsClass::RenderVerticalBlurToTexture() {

	XMMATRIX worldMatrix, viewMatrix, orthoMatrix;
	float screenSizeY;
	bool result;

	screenSizeY = (float)m_VerticalBlurTexture->GetTextureHeight();

	m_VerticalBlurTexture->SetRenderTarget(directx_device_->GetDeviceContext());

	m_VerticalBlurTexture->ClearRenderTarget(directx_device_->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetWorldMatrix(worldMatrix);

	m_VerticalBlurTexture->GetOrthoMatrix(orthoMatrix);

	directx_device_->TurnZBufferOff();

	m_SmallWindow->Render(directx_device_->GetDeviceContext());

	result = m_VerticalBlurShader->Render(directx_device_->GetDeviceContext(), m_SmallWindow->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix,
		m_HorizontalBlurTexture->GetShaderResourceView(), screenSizeY);
	if (!result) {
		return false;
	}

	directx_device_->TurnZBufferOn();

	directx_device_->SetBackBufferRenderTarget();

	directx_device_->ResetViewport();

	return true;
}

bool GraphicsClass::UpSampleTexture() {

	XMMATRIX worldMatrix, viewMatrix, orthoMatrix;
	bool result;

	m_UpSampleTexure->SetRenderTarget(directx_device_->GetDeviceContext());

	m_UpSampleTexure->ClearRenderTarget(directx_device_->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetWorldMatrix(worldMatrix);

	m_UpSampleTexure->GetOrthoMatrix(orthoMatrix);

	directx_device_->TurnZBufferOff();

	m_FullScreenWindow->Render(directx_device_->GetDeviceContext());

	result = m_TextureShader->Render(directx_device_->GetDeviceContext(), m_FullScreenWindow->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix,
		m_VerticalBlurTexture->GetShaderResourceView());
	if (!result) {
		return false;
	}

	directx_device_->TurnZBufferOn();

	directx_device_->SetBackBufferRenderTarget();

	directx_device_->ResetViewport();

	return true;
}

bool GraphicsClass::Render2DTextureScene() {

	XMMATRIX worldMatrix, viewMatrix, orthoMatrix;
	bool result;

	directx_device_->BeginScene(1.0f, 0.0f, 0.0f, 0.0f);

	camera_->Render();

	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetWorldMatrix(worldMatrix);
	directx_device_->GetOrthoMatrix(orthoMatrix);

	directx_device_->TurnZBufferOff();

	m_FullScreenWindow->Render(directx_device_->GetDeviceContext());

	result = m_TextureShader->Render(directx_device_->GetDeviceContext(), m_FullScreenWindow->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix,
		m_UpSampleTexure->GetShaderResourceView());
	if (!result) {
		return false;
	}

	directx_device_->TurnZBufferOn();

	directx_device_->EndScene();

	return true;
}