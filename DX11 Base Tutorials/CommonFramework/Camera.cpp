#include "stdafx.h"
#include "Camera.h"

using namespace DirectX;

Camera::Camera() {}

Camera::~Camera(){}

void Camera::SetPosition(float x, float y, float z) {
	
	position_x_ = x;
	position_y_ = y;
	position_z_ = z;
}

void Camera::SetRotation(float x, float y, float z) {

	rotation_x_ = x;
	rotation_y_ = y;
	rotation_z_ = z;
}

XMFLOAT3 Camera::GetPosition() {
	return XMFLOAT3(position_x_, position_y_, position_z_);
}

XMFLOAT3 Camera::GetRotation() {
	return XMFLOAT3(rotation_x_, rotation_y_, rotation_z_);
}

void Camera::Render() {

	XMVECTOR up, position, lookAt;
	float yaw, pitch, roll;
	XMMATRIX rotationMatrix;

	// Setup the vector that points upwards.
	up.m128_f32[0] = 0.0f;
	up.m128_f32[1] = 1.0f;
	up.m128_f32[2] = 0.0f;
	up.m128_f32[3] = 1.0f;

	// Setup the position of the camera in the world.
	position.m128_f32[0] = position_x_;
	position.m128_f32[1] = position_y_;
	position.m128_f32[2] = position_z_;
	position.m128_f32[3] = 1.0f;

	// Setup where the camera is looking by default.
	lookAt.m128_f32[0] = 0.0f;
	lookAt.m128_f32[1] = 0.0f;
	lookAt.m128_f32[2] = 1.0f;
	lookAt.m128_f32[3] = 1.0f;

	// Set the yaw (Y axis), pitch (X axis), and roll (Z axis) rotations in radians.
	pitch = rotation_x_ * 0.0174532925f;
	yaw = rotation_y_ * 0.0174532925f;
	roll = rotation_z_ * 0.0174532925f;

	// Create the rotation matrix from the yaw, pitch, and roll values.
	rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	// Transform the lookAt and up vector by the rotation matrix so the view is correctly rotated at the origin.
	lookAt = XMVector3TransformCoord(lookAt, rotationMatrix);
	up = XMVector3TransformCoord(up, rotationMatrix);

	// Translate the rotated camera position to the location of the viewer.
	lookAt = position + lookAt;

	// Finally create the view matrix from the three updated vectors.
	view_matrix_ = XMMatrixLookAtLH(position, lookAt, up);
}

void Camera::GetViewMatrix(XMMATRIX& viewMatrix) {
	viewMatrix = view_matrix_;
}

void Camera::RenderReflection(float height) {

	XMVECTOR up;
	XMVECTOR position;
	XMVECTOR lookAt;
	float radians;

	// Setup the vector that points upwards.
	up.m128_f32[0] = 0.0f;
	up.m128_f32[1] = 1.0f;
	up.m128_f32[2] = 0.0f;
	up.m128_f32[3] = 1.0f;

	// Setup the position of the camera in the world.
	// For planar reflection invert the Y position of the camera.
	position.m128_f32[0] = position_x_;
	position.m128_f32[1] = -position_y_ + (height * 2.0f);
	position.m128_f32[2] = position_z_;
	position.m128_f32[3] = 1.0f;

	// Calculate the rotation in radians.
	radians = rotation_y_ * 0.0174532925f;

	// Setup where the camera is looking by default.
	lookAt.m128_f32[0] = sinf(radians) + position_x_;
	lookAt.m128_f32[1] = position.m128_f32[1];
	lookAt.m128_f32[2] = cosf(radians) + position_z_;
	lookAt.m128_f32[3] = 1.0f;

	// Create the view matrix from the three vectors.
	m_reflectionViewMatrix = XMMatrixLookAtLH(position, lookAt, up);

	return;
}


XMMATRIX Camera::GetReflectionViewMatrix() {
	return m_reflectionViewMatrix;
}
