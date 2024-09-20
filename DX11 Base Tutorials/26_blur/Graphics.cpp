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

using namespace DirectX;

GraphicsClass::GraphicsClass() {}

GraphicsClass::~GraphicsClass() {}

bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd) {

	bool result;
	int downSampleWidth, downSampleHeight;

	// Set the size to sample down to.
	downSampleWidth = screenWidth / 2;
	downSampleHeight = screenHeight / 2;

	{
		auto directx11_device_ = DirectX11Device::GetD3d11DeviceInstance();

		auto result = directx11_device_->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
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
		result = model_->Initialize("data/cube.txt", L"data/seafloor.dds");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		texture_shader_ = (TextureShaderClass*)_aligned_malloc(sizeof(TextureShaderClass), 16);
		new (texture_shader_)TextureShaderClass();
		if (!texture_shader_) {
			return false;
		}
		result = texture_shader_->Initialize(hwnd);
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
		result = m_HorizontalBlurShader->Initialize(hwnd);
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
		result = m_VerticalBlurShader->Initialize(hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the vertical blur shader object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		render_texture_ = new RenderTextureClass();
		if (!render_texture_) {
			return false;
		}
		result = render_texture_->Initialize(screenWidth, screenHeight, SCREEN_DEPTH, SCREEN_NEAR);
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
		result = m_DownSampleTexure->Initialize(downSampleWidth, downSampleHeight, SCREEN_DEPTH, SCREEN_NEAR);
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
		result = m_HorizontalBlurTexture->Initialize(downSampleWidth, downSampleHeight, SCREEN_DEPTH, SCREEN_NEAR);
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
		result = m_VerticalBlurTexture->Initialize(downSampleWidth, downSampleHeight, SCREEN_DEPTH, SCREEN_NEAR);
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
		result = m_UpSampleTexure->Initialize(screenWidth, screenHeight, SCREEN_DEPTH, SCREEN_NEAR);
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
		result = m_SmallWindow->Initialize(downSampleWidth, downSampleHeight);
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
		result = m_FullScreenWindow->Initialize(screenWidth, screenHeight);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the full screen ortho window object.", L"Error", MB_OK);
			return false;
		}
	}

	return true;
}

void GraphicsClass::Shutdown(){

	if(m_FullScreenWindow)
	{
		m_FullScreenWindow->Shutdown();
		delete m_FullScreenWindow;
		m_FullScreenWindow = 0;
	}

	if(m_SmallWindow)
	{
		m_SmallWindow->Shutdown();
		delete m_SmallWindow;
		m_SmallWindow = 0;
	}
	
	if(m_UpSampleTexure)
	{
		m_UpSampleTexure->Shutdown();
		delete m_UpSampleTexure;
		m_UpSampleTexure = 0;
	}

	if(m_VerticalBlurTexture)
	{
		m_VerticalBlurTexture->Shutdown();
		delete m_VerticalBlurTexture;
		m_VerticalBlurTexture = 0;
	}

	if(m_HorizontalBlurTexture)
	{
		m_HorizontalBlurTexture->Shutdown();
		delete m_HorizontalBlurTexture;
		m_HorizontalBlurTexture = 0;
	}

	if(m_DownSampleTexure)
	{
		m_DownSampleTexure->Shutdown();
		delete m_DownSampleTexure;
		m_DownSampleTexure = 0;
	}
	
	if(render_texture_)
	{
		render_texture_->Shutdown();
		delete render_texture_;
		render_texture_ = 0;
	}
	
	if(m_VerticalBlurShader)
	{
		m_VerticalBlurShader->Shutdown();
		m_VerticalBlurShader->~VerticalBlurShaderClass();
		_aligned_free( m_VerticalBlurShader );
		m_VerticalBlurShader = 0;
	}
	
	if(m_HorizontalBlurShader)
	{
		m_HorizontalBlurShader->Shutdown();
		m_HorizontalBlurShader->~HorizontalBlurShaderClass();
		_aligned_free( m_HorizontalBlurShader );
		m_HorizontalBlurShader = 0;
	}

	if(texture_shader_)
	{
		texture_shader_->Shutdown();
		texture_shader_->~TextureShaderClass();
		_aligned_free( texture_shader_ );
		texture_shader_ = 0;
	}

	if(model_)
	{
		model_->Shutdown();
		delete model_;
		model_ = nullptr;
	}

	if (camera_) {
		camera_->~Camera();
		_aligned_free(camera_);
		camera_ = nullptr;
	}
}

bool GraphicsClass::Frame() {

	bool result;
	static float rotation_ = 0.0f;

	rotation_ += (float)XM_PI * 0.005f;
	if (rotation_ > 360.0f) {
		rotation_ -= 360.0f;
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

bool GraphicsClass::RenderSceneToTexture(float rotation_) {

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	
	render_texture_->SetRenderTarget();

	render_texture_->ClearRenderTarget(0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	camera_->GetViewMatrix(viewMatrix);

	auto directx_device = DirectX11Device::GetD3d11DeviceInstance();
	directx_device->GetWorldMatrix(worldMatrix);
	directx_device->GetProjectionMatrix(projectionMatrix);

	worldMatrix = XMMatrixRotationY(rotation_);

	model_->Render();

	auto result = texture_shader_->Render(model_->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
		model_->GetTexture());
	if (!result) {
		return false;
	}

	directx_device->SetBackBufferRenderTarget();

	directx_device->ResetViewport();

	return true;
}

bool GraphicsClass::DownSampleTexture() {

	XMMATRIX worldMatrix, viewMatrix, orthoMatrix;
	
	m_DownSampleTexure->SetRenderTarget();

	m_DownSampleTexure->ClearRenderTarget(0.0f, 1.0f, 0.0f, 1.0f);

	camera_->Render();

	camera_->GetViewMatrix(viewMatrix);
	
	auto directx_device = DirectX11Device::GetD3d11DeviceInstance();
	directx_device->GetWorldMatrix(worldMatrix);

	m_DownSampleTexure->GetOrthoMatrix(orthoMatrix);

	directx_device->TurnZBufferOff();

	m_SmallWindow->Render();

	auto result = texture_shader_->Render(m_SmallWindow->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix,
		render_texture_->GetShaderResourceView());
	if (!result) {
		return false;
	}

	directx_device->TurnZBufferOn();

	directx_device->SetBackBufferRenderTarget();

	directx_device->ResetViewport();

	return true;
}

bool GraphicsClass::RenderHorizontalBlurToTexture() {

	XMMATRIX worldMatrix, viewMatrix, orthoMatrix;
	
	float screenSizeX = (float)m_HorizontalBlurTexture->GetTextureWidth();

	auto directx_device = DirectX11Device::GetD3d11DeviceInstance();
	m_HorizontalBlurTexture->SetRenderTarget();

	m_HorizontalBlurTexture->ClearRenderTarget(0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	camera_->GetViewMatrix(viewMatrix);
	directx_device->GetWorldMatrix(worldMatrix);

	m_HorizontalBlurTexture->GetOrthoMatrix(orthoMatrix);

	directx_device->TurnZBufferOff();

	m_SmallWindow->Render();

	auto result = m_HorizontalBlurShader->Render(m_SmallWindow->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix,
		m_DownSampleTexure->GetShaderResourceView(), screenSizeX);
	if (!result) {
		return false;
	}

	directx_device->TurnZBufferOn();

	directx_device->SetBackBufferRenderTarget();

	directx_device->ResetViewport();

	return true;
}

bool GraphicsClass::RenderVerticalBlurToTexture() {

	XMMATRIX worldMatrix, viewMatrix, orthoMatrix;
	
	float screenSizeY = (float)m_VerticalBlurTexture->GetTextureHeight();

	m_VerticalBlurTexture->SetRenderTarget();

	m_VerticalBlurTexture->ClearRenderTarget(0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	camera_->GetViewMatrix(viewMatrix);

	auto directx_device = DirectX11Device::GetD3d11DeviceInstance();
	directx_device->GetWorldMatrix(worldMatrix);

	m_VerticalBlurTexture->GetOrthoMatrix(orthoMatrix);

	directx_device->TurnZBufferOff();

	m_SmallWindow->Render();

	auto result = m_VerticalBlurShader->Render(m_SmallWindow->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix,
		m_HorizontalBlurTexture->GetShaderResourceView(), screenSizeY);
	if (!result) {
		return false;
	}

	directx_device->TurnZBufferOn();

	directx_device->SetBackBufferRenderTarget();

	directx_device->ResetViewport();

	return true;
}

bool GraphicsClass::UpSampleTexture() {

	XMMATRIX worldMatrix, viewMatrix, orthoMatrix;
	
	m_UpSampleTexure->SetRenderTarget();

	m_UpSampleTexure->ClearRenderTarget(0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	camera_->GetViewMatrix(viewMatrix);

	auto directx_device = DirectX11Device::GetD3d11DeviceInstance();
	directx_device->GetWorldMatrix(worldMatrix);

	m_UpSampleTexure->GetOrthoMatrix(orthoMatrix);

	directx_device->TurnZBufferOff();

	m_FullScreenWindow->Render();

	auto result = texture_shader_->Render(m_FullScreenWindow->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix,
		m_VerticalBlurTexture->GetShaderResourceView());
	if (!result) {
		return false;
	}

	directx_device->TurnZBufferOn();

	directx_device->SetBackBufferRenderTarget();

	directx_device->ResetViewport();

	return true;
}

bool GraphicsClass::Render2DTextureScene() {

	XMMATRIX worldMatrix, viewMatrix, orthoMatrix;

	auto directx_device = DirectX11Device::GetD3d11DeviceInstance();
	directx_device->BeginScene(1.0f, 0.0f, 0.0f, 0.0f);

	camera_->Render();

	camera_->GetViewMatrix(viewMatrix);
	directx_device->GetWorldMatrix(worldMatrix);
	directx_device->GetOrthoMatrix(orthoMatrix);

	directx_device->TurnZBufferOff();

	m_FullScreenWindow->Render();

    auto result = texture_shader_->Render(m_FullScreenWindow->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix,
		m_UpSampleTexure->GetShaderResourceView());
	if (!result) {
		return false;
	}

	directx_device->TurnZBufferOn();

	directx_device->EndScene();

	return true;
}