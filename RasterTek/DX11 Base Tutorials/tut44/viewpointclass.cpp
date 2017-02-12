////////////////////////////////////////////////////////////////////////////////
// Filename: viewpointclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "viewpointclass.h"


ViewPointClass::ViewPointClass()
{
}


ViewPointClass::ViewPointClass(const ViewPointClass& other)
{
}


ViewPointClass::~ViewPointClass()
{
}


void ViewPointClass::SetPosition(float x, float y, float z)
{
	m_position = XMFLOAT3(x, y, z);
	return;
}


void ViewPointClass::SetLookAt(float x, float y, float z)
{
	m_lookAt = XMFLOAT3(x, y, z);
	return;
}


void ViewPointClass::SetProjectionParameters(float fieldOfView, float aspectRatio, float nearPlane, float farPlane)
{
	m_fieldOfView = fieldOfView;
	m_aspectRatio = aspectRatio;
	m_nearPlane = nearPlane;
	m_farPlane = farPlane;
	return;
}


void ViewPointClass::GenerateViewMatrix()
{
	XMVECTOR up;
	XMVECTOR position;
	XMVECTOR lookAt;

	// Setup the vector that points upwards.
	up.m128_f32[ 0 ] = 0.0f;
	up.m128_f32[ 1 ] = 1.0f;
	up.m128_f32[ 2 ] = 0.0f;
	up.m128_f32[ 3 ] = 1.0f;

	position.m128_f32[ 0 ] = m_position.x;
	position.m128_f32[ 1 ] = m_position.y;
	position.m128_f32[ 2 ] = m_position.z;
	position.m128_f32[ 3 ] = 1.0f;

	lookAt.m128_f32[ 0 ] = m_lookAt.x;
	lookAt.m128_f32[ 1 ] = m_lookAt.y;
	lookAt.m128_f32[ 2 ] = m_lookAt.z;
	lookAt.m128_f32[ 3 ] = 1.0f;

	// Create the view matrix from the three vectors.
	m_viewMatrix = XMMatrixLookAtLH( position, lookAt, up );

	return;
}


void ViewPointClass::GenerateProjectionMatrix()
{
	// Create the projection matrix for the view point.
	m_projectionMatrix = XMMatrixPerspectiveFovLH( m_fieldOfView, m_aspectRatio, m_nearPlane, m_farPlane);

	return;
}


void ViewPointClass::GetViewMatrix( XMMATRIX& viewMatrix)
{
	viewMatrix = m_viewMatrix;
	return;
}


void ViewPointClass::GetProjectionMatrix( XMMATRIX& projectionMatrix)
{
	projectionMatrix = m_projectionMatrix;
	return;
}