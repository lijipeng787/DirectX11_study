


#include "lightclass.h"


LightClass::LightClass()
{
}


LightClass::LightClass(const LightClass& other)
{
}


LightClass::~LightClass()
{
}


void LightClass::SetAmbientColor(float red, float green, float blue, float alpha)
{
	ambient_color_ = XMFLOAT4(red, green, blue, alpha);
	
}


void LightClass::SetDiffuseColor(float red, float green, float blue, float alpha)
{
	diffuse_color_ = XMFLOAT4(red, green, blue, alpha);
	
}


void LightClass::SetPosition(float x, float y, float z)
{
	m_position = XMFLOAT3(x, y, z);
	
}


void LightClass::SetLookAt(float x, float y, float z)
{
	m_lookAt.x = x;
	m_lookAt.y = y;
	m_lookAt.z = z;
	
}


XMFLOAT4 LightClass::GetAmbientColor()
{
	return ambient_color_;
}


XMFLOAT4 LightClass::GetDiffuseColor()
{
	return diffuse_color_;
}


XMFLOAT3 LightClass::GetPosition()
{
	return m_position;
}


void LightClass::GenerateViewMatrix()
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

	
}


void LightClass::GetViewMatrix( XMMATRIX& viewMatrix)
{
	viewMatrix = m_viewMatrix;
	
}


void LightClass::GenerateOrthoMatrix(float width, float depthPlane, float nearPlane)
{
	// Create the orthographic matrix for the light.
	ortho_matrix_ =  XMMatrixOrthographicLH( width, width, nearPlane, depthPlane);

	
}


void LightClass::GetOrthoMatrix( XMMATRIX& orthoMatrix)
{
	orthoMatrix = ortho_matrix_;
	
}


void LightClass::SetDirection(float x, float y, float z)
{
	direction_ = XMFLOAT3(x, y, z);
	
}


XMFLOAT3 LightClass::GetDirection()
{
	return direction_;
}