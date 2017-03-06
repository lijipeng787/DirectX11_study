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
		m_Camera->GetViewMatrix(baseViewMatrix);
	}

	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	{
		m_Model = new ModelClass();
		if (!m_Model) {
			return false;
		}
		result = m_Model->Initialize(m_D3D->GetDevice(), "../../tut47/data/sphere.txt", L"../../tut47/data/blue.dds");
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
		m_LightShader = (LightShaderClass*)_aligned_malloc(sizeof(LightShaderClass), 16);
		new (m_LightShader)LightShaderClass();
		if (!m_LightShader) {
			return false;
		}
		result = m_LightShader->Initialize(m_D3D->GetDevice(), hwnd);
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
		result = m_Text->Initialize(m_D3D->GetDevice(), m_D3D->GetDeviceContext(), hwnd, screenWidth, screenHeight, baseViewMatrix);
		if (!result) {
			MessageBox(hwnd, L"Could not initialize the text object.", L"Error", MB_OK);
			return false;
		}
	}

	{
		m_Bitmap = new BitmapClass();
		if (!m_Bitmap) {
			return false;
		}
		result = m_Bitmap->Initialize(m_D3D->GetDevice(), screenWidth, screenHeight, L"../../tut47/data/mouse.dds", 32, 32);
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
	if (m_Model) {
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
	m_D3D->GetProjectionMatrix(projectionMatrix);
	pointX = pointX / projectionMatrix.r[0].m128_f32[0];
	pointY = pointY / projectionMatrix.r[1].m128_f32[1];

	// Get the inverse of the view matrix.
	m_Camera->GetViewMatrix(viewMatrix);
	inverseViewMatrix = XMMatrixInverse(NULL, viewMatrix);

	// Calculate the direction of the picking ray in view space.
	direction.m128_f32[0] = 
		(pointX * inverseViewMatrix.r[0].m128_f32[0]) + (pointY * inverseViewMatrix.r[1].m128_f32[0]) + inverseViewMatrix.r[2].m128_f32[0];
	direction.m128_f32[1] = 
		(pointX * inverseViewMatrix.r[0].m128_f32[1]) + (pointY * inverseViewMatrix.r[1].m128_f32[1]) + inverseViewMatrix.r[2].m128_f32[1];
	direction.m128_f32[2] = 
		(pointX * inverseViewMatrix.r[0].m128_f32[2]) + (pointY * inverseViewMatrix.r[1].m128_f32[2]) + inverseViewMatrix.r[2].m128_f32[2];

	// Get the origin of the picking ray which is the position of the camera.
	XMFLOAT3 originCopy = m_Camera->GetPosition();
	origin.m128_f32[0] = originCopy.x;
	origin.m128_f32[1] = originCopy.y;
	origin.m128_f32[2] = originCopy.z;

	// Get the world matrix and translate to the location of the sphere.
	m_D3D->GetWorldMatrix(worldMatrix);
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
		result = m_Text->SetIntersection(true, m_D3D->GetDeviceContext());
	}
	else {
		// If not then set the intersection to "No".
		result = m_Text->SetIntersection(false, m_D3D->GetDeviceContext());
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

	m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	m_Camera->Render();

	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);
	m_D3D->GetOrthoMatrix(orthoMatrix);

	translateMatrix = XMMatrixTranslation(-5.0f, 1.0f, 5.0f);
	worldMatrix = XMMatrixMultiply(worldMatrix, translateMatrix);

	m_Model->Render(m_D3D->GetDeviceContext());
	result = m_LightShader->Render(
		m_D3D->GetDeviceContext(), 
		m_Model->GetIndexCount(), 
		worldMatrix, viewMatrix, projectionMatrix, 
		m_Model->GetTexture(), m_Light->GetDirection()
	);
	if (!result) {
		return false;
	}

	m_D3D->GetWorldMatrix(worldMatrix);

	m_D3D->TurnZBufferOff();

	m_D3D->TurnOnAlphaBlending();

	result = m_Bitmap->Render(m_D3D->GetDeviceContext(), mouse_x_, mouse_y_);  if (!result) { return false; }
	result = m_TextureShader->Render(
		m_D3D->GetDeviceContext(), 
		m_Bitmap->GetIndexCount(), 
		worldMatrix, viewMatrix, orthoMatrix, 
		m_Bitmap->GetTexture()
	);

	result = m_Text->Render(m_D3D->GetDeviceContext(), worldMatrix, orthoMatrix);
	if (!result) {
		return false;
	}

	m_D3D->TurnOffAlphaBlending();

	m_D3D->TurnZBufferOn();

	m_D3D->EndScene();

	return true;
}
