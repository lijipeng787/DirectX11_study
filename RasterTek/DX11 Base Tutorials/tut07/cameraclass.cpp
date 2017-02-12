#include "cameraclass.h"

Camera::Camera()
{
	position_X_ = 0.0f;
	position_y_ = 0.0f;
	position_Z_ = 0.0f;

	rotation_X_ = 0.0f;
	rotation_Y_ = 0.0f;
	rotation_Z_ = 0.0f;
}


Camera::Camera(const Camera& other)
{
}


Camera::~Camera()
{
}


void Camera::SetPosition(float x, float y, float z)
{
	position_X_ = x;
	position_y_ = y;
	position_Z_ = z;
	return;
}


void Camera::SetRotation(float x, float y, float z)
{
	rotation_X_ = x;
	rotation_Y_ = y;
	rotation_Z_ = z;
	return;
}


XMFLOAT3 Camera::GetPosition()
{
	return XMFLOAT3( position_X_, position_y_, position_Z_ );
}


XMFLOAT3 Camera::GetRotation()
{
	return XMFLOAT3( rotation_X_, rotation_Y_, rotation_Z_ );
}


void Camera::Update()
{
	XMVECTOR up, position, lookAt;
	float yaw, pitch, roll;
	XMMATRIX rotationMatrix;

	// Setup the vector that points upwards.
	up.m128_f32[ 0 ] = 0.0f;
	up.m128_f32[ 1 ] = 1.0f;
	up.m128_f32[ 2 ] = 0.0f;
	up.m128_f32[ 3 ] = 1.0f;

	// Setup the position of the camera in the world.
	position.m128_f32[ 0 ] = position_X_;
	position.m128_f32[ 1 ] = position_y_;
	position.m128_f32[ 2 ] = position_Z_;
	position.m128_f32[ 3 ] = 1.0f;

	// Setup where the camera is looking by default.
	lookAt.m128_f32[ 0 ] = 0.0f;
	lookAt.m128_f32[ 1 ] = 0.0f;
	lookAt.m128_f32[ 2 ] = 1.0f;
	lookAt.m128_f32[ 3 ] = 1.0f;

	// Set the yaw (Y axis), pitch (X axis), and roll (Z axis) rotations in radians.
	pitch = rotation_X_ * 0.0174532925f;
	yaw   = rotation_Y_ * 0.0174532925f;
	roll  = rotation_Z_ * 0.0174532925f;

	// Create the rotation matrix from the yaw, pitch, and roll values.
	rotationMatrix = XMMatrixRotationRollPitchYaw( pitch, yaw, roll );

	// Transform the lookAt and up vector by the rotation matrix so the view is correctly rotated at the origin.
	lookAt = XMVector3TransformCoord( lookAt, rotationMatrix );
	up = XMVector3TransformCoord( up, rotationMatrix );

	// Translate the rotated camera position to the location of the viewer.
	lookAt = position + lookAt;

	// Finally create the view matrix from the three updated vectors.
	view_matrix_ = XMMatrixLookAtLH( position, lookAt, up );

	return;
}


void Camera::GetViewMatrix( XMMATRIX& viewMatrix )
{
	viewMatrix = view_matrix_;
	return;
}