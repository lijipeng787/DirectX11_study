#include "Graphics.h"

#include <new>

#include "../CommonFramework/TypeDefine.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/Camera.h"

#include "textureshaderclass.h"
#include "bitmapclass.h"
#include "rendertextureclass.h"
#include "orthowindowclass.h"
#include "horizontalblurshaderclass.h"
#include "verticalblurshaderclass.h"
#include "glowmapshaderclass.h"
#include "glowshaderclass.h"

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
		camera_->SetPosition(0.0f, 0.0f, -10.0f);
		camera_->Render();
	}

	{
		m_TextureShader = (TextureShaderClass*)_aligned_malloc(sizeof(TextureShaderClass), 16);
		new (m_TextureShader)TextureShaderClass();
		if (!m_TextureShader) {
			return false;
		}
		result = m_TextureShader->Initialize(hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the texture shader object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_Bitmap = new SimpleMoveableSurface();
		if (!m_Bitmap) {
			return false;
		}
		result = m_Bitmap->Initialize(screenWidth, screenHeight, L"../../tut46/data/test.dds", L"../../tut46/data/glowmap.dds", 256, 32);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the bitmap object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_RenderTexture = (RenderTextureClass*)_aligned_malloc(sizeof(RenderTextureClass), 16);
		new (m_RenderTexture)RenderTextureClass();
		if (!m_RenderTexture) {
			return false;
		}
		result = m_RenderTexture->Initialize(screenWidth, screenHeight, SCREEN_DEPTH, SCREEN_NEAR);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the render to texture object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_DownSampleTexure = (RenderTextureClass*)_aligned_malloc(sizeof(RenderTextureClass), 16);
		new (m_DownSampleTexure)RenderTextureClass();
		if (!m_DownSampleTexure) {
			return false;
		}
		result = m_DownSampleTexure->Initialize((screenWidth / 2), (screenHeight / 2), SCREEN_DEPTH, SCREEN_NEAR);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the down sample render to texture object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_SmallWindow = new OrthoWindowClass();
		if (!m_SmallWindow) {
			return false;
		}
		result = m_SmallWindow->Initialize((screenWidth / 2), (screenHeight / 2));
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the small ortho window object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_HorizontalBlurTexture = (RenderTextureClass*)_aligned_malloc(sizeof(RenderTextureClass), 16);
		new (m_HorizontalBlurTexture)RenderTextureClass();
		if (!m_HorizontalBlurTexture) {
			return false;
		}
		result = m_HorizontalBlurTexture->Initialize((screenWidth / 2), (screenHeight / 2), SCREEN_DEPTH, SCREEN_NEAR);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the horizontal blur render to texture object.", L"Error", MB_OK);
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
		m_VerticalBlurTexture = (RenderTextureClass*)_aligned_malloc(sizeof(RenderTextureClass), 16);
		new (m_VerticalBlurTexture)RenderTextureClass();
		if (!m_VerticalBlurTexture) {
			return false;
		}
		result = m_VerticalBlurTexture->Initialize((screenWidth / 2), (screenHeight / 2), SCREEN_DEPTH, SCREEN_NEAR);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the vertical blur render to texture object.", L"Error", MB_OK);
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
		m_UpSampleTexure = (RenderTextureClass*)_aligned_malloc(sizeof(RenderTextureClass), 16);
		new (m_UpSampleTexure)RenderTextureClass();
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

	{
		m_GlowMapShader = (GlowMapShaderClass*)_aligned_malloc(sizeof(GlowMapShaderClass), 16);
		new (m_GlowMapShader)GlowMapShaderClass();
		if (!m_GlowMapShader) {
			return false;
		}
		result = m_GlowMapShader->Initialize(hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the glow map shader object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_GlowShader = (GlowShaderClass*)_aligned_malloc(sizeof(GlowShaderClass), 16);
		new (m_GlowShader)GlowShaderClass();
		if (!m_GlowShader) {
			return false;
		}
		result = m_GlowShader->Initialize(hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the glow shader object.", L"Error", MB_OK);
			return false;
		}
	}

	return true;
}

void GraphicsClass::Shutdown() {

	// Release the glow shader object.
	if (m_GlowShader) {
		m_GlowShader->Shutdown();
		m_GlowShader->~GlowShaderClass();
		_aligned_free(m_GlowShader);
		m_GlowShader = 0;
	}

	// Release the glow map shader object.
	if (m_GlowMapShader) {
		m_GlowMapShader->Shutdown();
		m_GlowMapShader->~GlowMapShaderClass();
		_aligned_free(m_GlowMapShader);
		m_GlowMapShader = 0;
	}

	buffer_number
	if (m_FullScreenWindow) {
		m_FullScreenWindow->Shutdown();
		delete m_FullScreenWindow;
		m_FullScreenWindow = 0;
	}


	if (m_UpSampleTexure) {
		m_UpSampleTexure->Shutdown();
		m_UpSampleTexure->~RenderTextureClass();
		_aligned_free(m_UpSampleTexure);
		m_UpSampleTexure = 0;
	}

	
	if (m_VerticalBlurShader) {
		m_VerticalBlurShader->Shutdown();
		m_VerticalBlurShader->~VerticalBlurShaderClass();
		_aligned_free(m_VerticalBlurShader);
		m_VerticalBlurShader = 0;
	}

	
	if (m_VerticalBlurTexture) {
		m_VerticalBlurTexture->Shutdown();
		m_VerticalBlurTexture->~RenderTextureClass();
		_aligned_free(m_VerticalBlurTexture);
		m_VerticalBlurTexture = 0;
	}

	
	if (m_HorizontalBlurShader) {
		m_HorizontalBlurShader->Shutdown();
		m_HorizontalBlurShader->~HorizontalBlurShaderClass();
		_aligned_free(m_HorizontalBlurShader);
		m_HorizontalBlurShader = 0;
	}

	
	if (m_HorizontalBlurTexture) {
		m_HorizontalBlurTexture->Shutdown();
		m_HorizontalBlurTexture->~RenderTextureClass();
		_aligned_free(m_HorizontalBlurTexture);
		m_HorizontalBlurTexture = 0;
	}

	
	if (m_SmallWindow) {
		m_SmallWindow->Shutdown();
		delete m_SmallWindow;
		m_SmallWindow = 0;
	}


	if (m_DownSampleTexure) {
		m_DownSampleTexure->Shutdown();
		m_DownSampleTexure->~RenderTextureClass();
		_aligned_free(m_DownSampleTexure);
		m_DownSampleTexure = 0;
	}

	
	if (m_RenderTexture) {
		m_RenderTexture->Shutdown();
		m_RenderTexture->~RenderTextureClass();
		_aligned_free(m_RenderTexture);
		m_RenderTexture = 0;
	}

	// Release the bitmap object.
	if (m_Bitmap) {
		m_Bitmap->Shutdown();
		delete m_Bitmap;
		m_Bitmap = 0;
	}

	
	if (m_TextureShader) {
		m_TextureShader->Shutdown();
		m_TextureShader->~TextureShaderClass();
		_aligned_free(m_TextureShader);
		m_TextureShader = 0;
	}
	if (camera_) {
		camera_->~Camera();
		_aligned_free(camera_);
		camera_ = 0;
	}

	
		
		
		directx_device_ = 0;
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

	bool result;

	// First render the glow maps to a render texture.
	result = RenderGlowMapToTexture();
	if (!result) {
		return false;
	}

	// Next down sample the render texture to a smaller sized texture.
	result = DownSampleTexture();
	if (!result) {
		return false;
	}

	// Perform a horizontal blur on the down sampled render texture.
	result = RenderHorizontalBlurToTexture();
	if (!result) {
		return false;
	}

	// Now perform a vertical blur on the horizontal blur render texture.
	result = RenderVerticalBlurToTexture();
	if (!result) {
		return false;
	}

	// Up sample the final blurred render texture to screen size again.
	result = UpSampleTexture();
	if (!result) {
		return false;
	}

	// Render the regular UI elements to a full screen texture.
	result = RenderUIElementsToTexture();
	if (!result) {
		return false;
	}

	// Render the final scene combining the UI elements with the glowing UI elements.
	RenderGlowScene();

	return true;
}

bool GraphicsClass::RenderGlowMapToTexture() {

	XMMATRIX worldMatrix, viewMatrix, orthoMatrix;
	bool result;

	m_RenderTexture->SetRenderTarget(directx_device_->GetDeviceContext());

	m_RenderTexture->ClearRenderTarget(directx_device_->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	directx_device_->GetWorldMatrix(worldMatrix);
	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetOrthoMatrix(orthoMatrix);

	directx_device_->TurnZBufferOff();

	result = m_Bitmap->Render(directx_device_->GetDeviceContext(), 100, 100);
	if (!result) {
		return false;
	}

	m_GlowMapShader->Render(directx_device_->GetDeviceContext(), m_Bitmap->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix,
		m_Bitmap->GetTexture(), m_Bitmap->GetGlowMap());

	directx_device_->TurnZBufferOn();

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

	directx_device_->GetWorldMatrix(worldMatrix);
	camera_->GetViewMatrix(viewMatrix);

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

	m_HorizontalBlurTexture->SetRenderTarget(directx_device_->GetDeviceContext());

	m_HorizontalBlurTexture->ClearRenderTarget(directx_device_->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetWorldMatrix(worldMatrix);

	m_HorizontalBlurTexture->GetOrthoMatrix(orthoMatrix);

	directx_device_->TurnZBufferOff();

	screenSizeX = (float)m_HorizontalBlurTexture->GetTextureWidth();

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

	m_VerticalBlurTexture->SetRenderTarget(directx_device_->GetDeviceContext());

	m_VerticalBlurTexture->ClearRenderTarget(directx_device_->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetWorldMatrix(worldMatrix);

	m_VerticalBlurTexture->GetOrthoMatrix(orthoMatrix);

	directx_device_->TurnZBufferOff();

	screenSizeY = (float)m_VerticalBlurTexture->GetTextureHeight();

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

	directx_device_->GetWorldMatrix(worldMatrix);
	camera_->GetViewMatrix(viewMatrix);

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

bool GraphicsClass::RenderUIElementsToTexture() {

	XMMATRIX worldMatrix, viewMatrix, orthoMatrix;
	bool result;

	m_RenderTexture->SetRenderTarget(directx_device_->GetDeviceContext());

	m_RenderTexture->ClearRenderTarget(directx_device_->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	directx_device_->GetWorldMatrix(worldMatrix);
	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetOrthoMatrix(orthoMatrix);

	directx_device_->TurnZBufferOff();

	result = m_Bitmap->Render(directx_device_->GetDeviceContext(), 100, 100);
	if (!result) {
		return false;
	}

	result = m_TextureShader->Render(directx_device_->GetDeviceContext(), m_Bitmap->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix, m_Bitmap->GetTexture());
	if (!result) {
		return false;
	}

	directx_device_->TurnZBufferOn();

	directx_device_->SetBackBufferRenderTarget();

	directx_device_->ResetViewport();

	return true;
}

bool GraphicsClass::RenderGlowScene() {

	XMMATRIX worldMatrix, viewMatrix, orthoMatrix;

	directx_device_->BeginScene(1.0f, 0.0f, 0.0f, 0.0f);

	camera_->Render();

	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetWorldMatrix(worldMatrix);
	directx_device_->GetOrthoMatrix(orthoMatrix);

	directx_device_->TurnZBufferOff();

	m_FullScreenWindow->Render(directx_device_->GetDeviceContext());

	m_GlowShader->Render(directx_device_->GetDeviceContext(), m_FullScreenWindow->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix,
		m_RenderTexture->GetShaderResourceView(), m_UpSampleTexure->GetShaderResourceView(), 3.0f);

	directx_device_->TurnZBufferOn();

	directx_device_->EndScene();

	return true;
}
