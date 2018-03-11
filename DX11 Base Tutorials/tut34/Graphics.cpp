#include "Graphics.h"

#include <new>

#include "../CommonFramework/TypeDefine.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/Camera.h"

#include "modelclass.h"
#include "textureshaderclass.h"

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
		floor_model_ = new ModelClass();
		if (!floor_model_) {
			return false;
		}
		result = floor_model_->Initialize("../../tut34/data/floor.txt", L"../../tut34/data/grid01.dds");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the floor model object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_BillboardModel = new ModelClass();
		if (!m_BillboardModel) {
			return false;
		}
		result = m_BillboardModel->Initialize("../../tut34/data/square.txt", L"../../tut34/data/seafloor.dds");
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the billboard model object.", L"Error", MB_OK);
			return false;
		}
	}

	return true;
}

void GraphicsClass::Shutdown() {

	// Release the billboard model object.
	if (m_BillboardModel)
	{
		m_BillboardModel->Shutdown();
		delete m_BillboardModel;
		m_BillboardModel = 0;
	}

	// Release the floor model object.
	if (floor_model_)
	{
		floor_model_->Shutdown();
		delete floor_model_;
		floor_model_ = 0;
	}

	
	if (texture_shader_)
	{
		texture_shader_->Shutdown();
		texture_shader_->~TextureShaderClass();
		_aligned_free(texture_shader_);
		texture_shader_ = 0;
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

	// Update the position of the camera.
	camera_->SetPosition(x_, y_, z_);

	// Render the graphics scene.
	result = Render();
	if (!result) {
		return false;
	}

	return true;
}

bool GraphicsClass::Render() {

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, translateMatrix;
	bool result;
	XMFLOAT3 cameraPosition, modelPosition;
	double angle;
	float rotation_;

	directx_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetWorldMatrix(worldMatrix);
	directx_device_->GetProjectionMatrix(projectionMatrix);

	floor_model_->Render(directx_device_->GetDeviceContext());

	result = texture_shader_->Render(directx_device_->GetDeviceContext(), floor_model_->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
		floor_model_->GetTexture());
	if (!result) {
		return false;
	}

	cameraPosition = camera_->GetPosition();

	modelPosition.x = 0.0f;
	modelPosition.y = 1.5f;
	modelPosition.z = 0.0f;

	angle = atan2(modelPosition.x - cameraPosition.x, modelPosition.z - cameraPosition.z) * (180.0 / XM_PI);

	rotation_ = (float)angle * 0.0174532925f;

	worldMatrix = XMMatrixRotationY(rotation_);

	translateMatrix = XMMatrixTranslation(modelPosition.x, modelPosition.y, modelPosition.z);

	worldMatrix = XMMatrixMultiply(worldMatrix, translateMatrix);

	m_BillboardModel->Render(directx_device_->GetDeviceContext());

	result = texture_shader_->Render(directx_device_->GetDeviceContext(), m_BillboardModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
		m_BillboardModel->GetTexture());
	if (!result) {
		return false;
	}

	directx_device_->EndScene();

	return true;
}

void GraphicsClass::SetPosition(float x, float y, float z) {
	x_ = x;
	y_ = y;
	z_ = z;
}
