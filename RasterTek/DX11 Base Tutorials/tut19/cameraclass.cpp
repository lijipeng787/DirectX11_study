////////////////////////////////////////////////////////////////////////////////
// Filename: cameraclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "cameraclass.h"


CameraClass::CameraClass()
{
	m_positionX = 0.0f;
	m_positionY = 0.0f;
	m_positionZ = 0.0f;

	m_rotationX = 0.0f;
	m_rotationY = 0.0f;
	m_rotationZ = 0.0f;
}


CameraClass::CameraClass(const CameraClass& other)
{
}


CameraClass::~CameraClass()
{
}


void CameraClass::SetPosition(float x, float y, float z)
{
	m_positionX = x;
	m_positionY = y;
	m_positionZ = z;
	return;
}


void CameraClass::SetRotation(float x, float y, float z)
{
	m_rotationX = x;
	m_rotationY = y;
	m_rotationZ = z;
	return;
}


void CameraClass::Render()
{
	XMVECTOR up;
	XMVECTOR position;
	XMVECTOR lookAt;
	float radians;

	// Setup the vector that points upwards.
	up.m128_f32[ 0 ] = 0.0f;
	up.m128_f32[ 1 ] = 1.0f;
	up.m128_f32[ 2 ] = 0.0f;
	up.m128_f32[ 3 ] = 1.0f;

	// Setup the position of the camera in the world.
	position.m128_f32[ 0 ] = m_positionX;
	position.m128_f32[ 1 ] = m_positionY;
	position.m128_f32[ 2 ] = m_positionZ;
	position.m128_f32[ 3 ] = 1.0f;

	// Calculate the rotation in radians.
	radians = m_rotationY * 0.0174532925f;

	// Setup where the camera is looking by default.
	lookAt.m128_f32[ 0 ] = sinf( radians ) + m_positionX;
	lookAt.m128_f32[ 1 ] = m_positionY;
	lookAt.m128_f32[ 2 ] = cosf( radians ) + m_positionZ;
	lookAt.m128_f32[ 3 ] = 1.0f;

	// Create the view matrix from the three vectors.
	m_viewMatrix = XMMatrixLookAtLH( position, lookAt, up );

	return;
}


void CameraClass::GetViewMatrix( XMMATRIX& viewMatrix )
{
	viewMatrix = m_viewMatrix;
	return;
}