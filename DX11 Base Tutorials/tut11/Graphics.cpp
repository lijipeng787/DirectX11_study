#include "Graphics.h"

#include <new>

#include "../CommonFramework/TypeDefine.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/Camera.h"

#include "textureshaderclass.h"
#include "bitmapclass.h"

GraphicsClass::GraphicsClass() {}

GraphicsClass::~GraphicsClass() {}

bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd) {

	bool result;
	auto directx11_device_ = DirectX11Device::GetD3d11DeviceInstance();

	{
		result = directx11_device_->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
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
	}

	{
		texture_shader_ = (TextureShader*)_aligned_malloc(sizeof(TextureShader), 16);
		new (texture_shader_)TextureShader();
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
		bitmap_ = new SimpleMoveableBitmap();
		if (!bitmap_) {
			return false;
		}

		result = bitmap_->Initialize(screenWidth, screenHeight, 256, 256);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the bitmap object.", L"Error", MB_OK);
			return false;
		}

		result = bitmap_->InitializeVertexAndIndexBuffers();
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the bitmap buffers.", L"Error", MB_OK);
			return false;
		}

		result = bitmap_->LoadTextureFromFile(L"../../tut11/data/seafloor.dds");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the bitmap texture.", L"Error", MB_OK);
			return false;
		}
	}

	return true;
}

void GraphicsClass::Shutdown() {

	if (bitmap_) {
		bitmap_->Release();
		delete bitmap_;
		bitmap_ = 0;
	}

	if (texture_shader_) {
		texture_shader_->Shutdown();
		texture_shader_->~TextureShader();
		_aligned_free(texture_shader_);
		texture_shader_ = 0;
	}

	if (camera_) {
		camera_->~Camera();
		_aligned_free(camera_);
		camera_ = nullptr;
	}
}

bool GraphicsClass::Frame() {

	bool result = Render();
	if (!result) {
		return false;
	}

	return true;
}

bool GraphicsClass::Render() {

	auto directx11_device_ = DirectX11Device::GetD3d11DeviceInstance();

	directx11_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	XMMATRIX worldMatrix, viewMatrix, orthoMatrix;

	camera_->GetViewMatrix(viewMatrix);
	directx11_device_->GetWorldMatrix(worldMatrix);
	directx11_device_->GetOrthoMatrix(orthoMatrix);

	directx11_device_->TurnZBufferOff();

	auto result = bitmap_->Render(100, 100);
	if (!result) {
		return false;
	}

	result = texture_shader_->Render(bitmap_->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix, bitmap_->GetTexture());
	if (!result) {
		return false;
	}

	directx11_device_->TurnZBufferOn();

	directx11_device_->EndScene();

	return true;
}
