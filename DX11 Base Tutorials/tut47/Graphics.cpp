#include "Graphics.h"

#include <new>

#include "../CommonFramework/TypeDefine.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/Camera.h"

#include "modelclass.h"
#include "textureshaderclass.h"
#include "lightshaderclass.h"
#include "lightclass.h"
#include "textclass.h"
#include "bitmapclass.h"

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
		camera_->SetPosition(0.0f, 0.0f, -10.0f);
		camera_->Render();
		camera_->GetViewMatrix(baseViewMatrix);
	}

	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	{
		model_ = new ModelClass();
		if (!model_) {
			return false;
		}
		result = model_->Initialize(directx_device_->GetDevice(), "../../tut47/data/sphere.txt", L"../../tut47/data/blue.dds");
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
		m_LightShader = (LightShaderClass*)_aligned_malloc(sizeof(LightShaderClass), 16);
		new (m_LightShader)LightShaderClass();
		if (!m_LightShader) {
			return false;
		}
		result = m_LightShader->Initialize(directx_device_->GetDevice(), hwnd);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the light shader object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_Light = (LightClass*)_aligned_malloc(sizeof(LightClass), 16);
		new (m_Light)LightClass();
		if (!m_Light) {
			return false;
		}
		m_Light->SetDirection(0.0f, 0.0f, 1.0f);
	}

	{
		m_Text = (TextClass*)_aligned_malloc(sizeof(TextClass), 16);
		new (m_Text)TextClass();
		if (!m_Text) {
			return false;
		}
		result = m_Text->Initialize(directx_device_->GetDevice(), directx_device_->GetDeviceContext(), hwnd, screenWidth, screenHeight, baseViewMatrix);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the text object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_Bitmap = new SimpleMoveableSurface();
		if (!m_Bitmap) {
			return false;
		}
		result = m_Bitmap->Initialize(directx_device_->GetDevice(), screenWidth, screenHeight, L"../../tut47/data/mouse.dds", 32, 32);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the bitmap object.", L"Error", MB_OK);
			return false;
		}
	}

	return true;
}

void GraphicsClass::Shutdown() {

	// Release the bitmap object.
	if (m_Bitmap) {
		m_Bitmap->Shutdown();
		delete m_Bitmap;
		m_Bitmap = 0;
	}

	// Release the text object.
	if (m_Text) {
		m_Text->Shutdown();
		m_Text->~TextClass();
		_aligned_free(m_Text);
		m_Text = 0;
	}

	// Release the light object.
	if (m_Light) {
		m_Light->~LightClass();
		_aligned_free(m_Light);
		m_Light = 0;
	}

	// Release the light shader object.
	if (m_LightShader) {
		m_LightShader->Shutdown();
		m_LightShader->~LightShaderClass();
		_aligned_free(m_LightShader);
		m_LightShader = 0;
	}

	// Release the texture shader object.
	if (m_TextureShader) {
		m_TextureShader->Shutdown();
		m_TextureShader->~TextureShaderClass();
		_aligned_free(m_TextureShader);
		m_TextureShader = 0;
	}

	// Release the model object.
	if (model_) {
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

	result = Render();
	if (!result) {
		return false;
	}

	return true;
}

void GraphicsClass::TestIntersection(int mouseX, int mouseY) {

	float pointX, pointY;
	XMMATRIX projectionMatrix, viewMatrix, inverseViewMatrix, worldMatrix, translateMatrix, inverseWorldMatrix;
	XMVECTOR direction, origin, rayOrigin, rayDirection;
	bool intersect, result;


	// Move the mouse cursor coordinates into the -1 to +1 range.
	pointX = ((2.0f * (float)mouseX) / (float)m_screenWidth) - 1.0f;
	pointY = (((2.0f * (float)mouseY) / (float)m_screenHeight) - 1.0f) * -1.0f;

	// Adjust the points using the projection matrix to account for the aspect ratio of the viewport.
	directx_device_->GetProjectionMatrix(projectionMatrix);
	pointX = pointX / projectionMatrix.r[0].m128_f32[0];
	pointY = pointY / projectionMatrix.r[1].m128_f32[1];

	// Get the inverse of the view matrix.
	camera_->GetViewMatrix(viewMatrix);
	inverseViewMatrix = XMMatrixInverse(NULL, viewMatrix);

	// Calculate the direction of the picking ray in view space.
	direction.m128_f32[0] = 
		(pointX * inverseViewMatrix.r[0].m128_f32[0]) + (pointY * inverseViewMatrix.r[1].m128_f32[0]) + inverseViewMatrix.r[2].m128_f32[0];
	direction.m128_f32[1] = 
		(pointX * inverseViewMatrix.r[0].m128_f32[1]) + (pointY * inverseViewMatrix.r[1].m128_f32[1]) + inverseViewMatrix.r[2].m128_f32[1];
	direction.m128_f32[2] = 
		(pointX * inverseViewMatrix.r[0].m128_f32[2]) + (pointY * inverseViewMatrix.r[1].m128_f32[2]) + inverseViewMatrix.r[2].m128_f32[2];

	// Get the origin of the picking ray which is the position of the camera.
	XMFLOAT3 originCopy = camera_->GetPosition();
	origin.m128_f32[0] = originCopy.x;
	origin.m128_f32[1] = originCopy.y;
	origin.m128_f32[2] = originCopy.z;

	// Get the world matrix and translate to the location of the sphere.
	directx_device_->GetWorldMatrix(worldMatrix);
	translateMatrix = XMMatrixTranslation(-5.0f, 1.0f, 5.0f);
	worldMatrix = XMMatrixMultiply(worldMatrix, translateMatrix);

	// Now get the inverse of the translated world matrix.
	inverseWorldMatrix = XMMatrixInverse(NULL, worldMatrix);

	// Now transform the ray origin and the ray direction from view space to world space.
	rayOrigin = XMVector3TransformCoord(origin, inverseWorldMatrix);
	rayDirection = XMVector3TransformNormal(direction, inverseWorldMatrix);

	// Normalize the ray direction.
	rayDirection = XMVector3Normalize(rayDirection);

	// Now perform the ray-sphere intersection test.
	XMFLOAT3 rayOriginCopy(rayOrigin.m128_f32[0], rayOrigin.m128_f32[1], rayOrigin.m128_f32[2]);
	XMFLOAT3 rayDirectionCopy(rayDirection.m128_f32[0], rayDirection.m128_f32[1], rayDirection.m128_f32[2]);
	intersect = RaySphereIntersect(rayOriginCopy, rayDirectionCopy, 1.0f);

	if (intersect == true) {
		// If it does intersect then set the intersection to "yes" in the text string that is displayed to the screen.
		result = m_Text->SetIntersection(true, directx_device_->GetDeviceContext());
	}
	else {
		// If not then set the intersection to "No".
		result = m_Text->SetIntersection(false, directx_device_->GetDeviceContext());
	}
}


bool GraphicsClass::RaySphereIntersect(const XMFLOAT3& rayOrigin, const XMFLOAT3& rayDirection, float radius) {

	float a, b, c, discriminant;

	// Calculate the a, b, and c coefficients.
	a = (rayDirection.x * rayDirection.x) + (rayDirection.y * rayDirection.y) + (rayDirection.z * rayDirection.z);
	b = ((rayDirection.x * rayOrigin.x) + (rayDirection.y * rayOrigin.y) + (rayDirection.z * rayOrigin.z)) * 2.0f;
	c = ((rayOrigin.x * rayOrigin.x) + (rayOrigin.y * rayOrigin.y) + (rayOrigin.z * rayOrigin.z)) - (radius * radius);

	// Find the discriminant.
	discriminant = (b * b) - (4 * a * c);

	// if discriminant is negative the picking ray missed the sphere, otherwise it intersected the sphere.
	if (discriminant < 0.0f) {
		return false;
	}

	return true;
}

bool GraphicsClass::Render() {

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix, translateMatrix;
	bool result;

	directx_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	camera_->Render();

	camera_->GetViewMatrix(viewMatrix);
	directx_device_->GetWorldMatrix(worldMatrix);
	directx_device_->GetProjectionMatrix(projectionMatrix);
	directx_device_->GetOrthoMatrix(orthoMatrix);

	translateMatrix = XMMatrixTranslation(-5.0f, 1.0f, 5.0f);
	worldMatrix = XMMatrixMultiply(worldMatrix, translateMatrix);

	model_->Render(directx_device_->GetDeviceContext());
	result = m_LightShader->Render(
		directx_device_->GetDeviceContext(), 
		model_->GetIndexCount(), 
		worldMatrix, viewMatrix, projectionMatrix, 
		model_->GetTexture(), m_Light->GetDirection()
	);
	if (!result) {
		return false;
	}

	directx_device_->GetWorldMatrix(worldMatrix);

	directx_device_->TurnZBufferOff();

	directx_device_->TurnOnAlphaBlending();

	result = m_Bitmap->Render(directx_device_->GetDeviceContext(), mouse_x_, mouse_y_);  if (!result) { return false; }
	result = m_TextureShader->Render(
		directx_device_->GetDeviceContext(), 
		m_Bitmap->GetIndexCount(), 
		worldMatrix, viewMatrix, orthoMatrix, 
		m_Bitmap->GetTexture()
	);

	result = m_Text->Render(directx_device_->GetDeviceContext(), worldMatrix, orthoMatrix);
	if (!result) {
		return false;
	}

	directx_device_->TurnOffAlphaBlending();

	directx_device_->TurnZBufferOn();

	directx_device_->EndScene();

	return true;
}
