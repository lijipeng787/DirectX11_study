#include "stdafx.h"

#include "Camera.h"

using namespace DirectX;

void Camera::GetWorldPosition(DirectX::XMMATRIX& wordMatrix) {
}

void Camera::SetMoveRate(int rate) {
}

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

void Camera::SetPosition(const XMFLOAT3& position) {
	position_ = position;
}

void Camera::SetRotation(const XMFLOAT3& rotation) {
	rotation_ = rotation;
}

XMFLOAT3 Camera::GetPosition() const {
	return XMFLOAT3(position_x_, position_y_, position_z_);
}

XMFLOAT3 Camera::GetRotation() const {
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
	pitch = rotation_x_ * HALF_PI;
	yaw = rotation_y_ * HALF_PI;
	roll = rotation_z_ * HALF_PI;

	// Create the rotation_ matrix from the yaw, pitch, and roll values.
	rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	// Transform the lookAt and up vector by the rotation_ matrix so the view is correctly rotated at the origin.
	lookAt = XMVector3TransformCoord(lookAt, rotationMatrix);
	up = XMVector3TransformCoord(up, rotationMatrix);

	// Translate the rotated camera position to the location of the viewer.
	lookAt = position + lookAt;

	// Finally create the view matrix from the three updated vectors.
	view_matrix_ = XMMatrixLookAtLH(position, lookAt, up);
}

void Camera::GetViewMatrix(XMMATRIX& viewMatrix) const {
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

	// Calculate the rotation_ in radians.
	radians = rotation_y_ * HALF_PI;

	// Setup where the camera is looking by default.
	lookAt.m128_f32[0] = sinf(radians) + position_x_;
	lookAt.m128_f32[1] = position.m128_f32[1];
	lookAt.m128_f32[2] = cosf(radians) + position_z_;
	lookAt.m128_f32[3] = 1.0f;

	// Create the view matrix from the three vectors.
	m_reflectionViewMatrix = XMMatrixLookAtLH(position, lookAt, up);
}

XMMATRIX Camera::GetReflectionViewMatrix() {
	return m_reflectionViewMatrix;
}

void Camera::RenderBaseViewMatrix() {

	XMVECTOR up, position, lookAt;
	float radians;

	// Setup the vector that points upwards.
	up.m128_f32[0] = 0.0f;
	up.m128_f32[1] = 1.0f;
	up.m128_f32[2] = 0.0f;

	// Setup the position of the camera in the world.
	position.m128_f32[0] = position_x_;
	position.m128_f32[1] = position_y_;
	position.m128_f32[2] = position_z_;

	// Calculate the rotation_ in radians.
	radians = rotation_y_ * HALF_PI;

	// Setup where the camera is looking.
	lookAt.m128_f32[0] = sinf(radians) + position_x_;
	lookAt.m128_f32[1] = position_y_;
	lookAt.m128_f32[2] = cosf(radians) + position_z_;

	// Create the base view matrix from the three vectors.
	base_view_matrix_ = XMMatrixLookAtLH(position, lookAt, up);
}

void Camera::GetBaseViewMatrix(XMMATRIX& outViewMatrix) const {
	outViewMatrix = base_view_matrix_;
}