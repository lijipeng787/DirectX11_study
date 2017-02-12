#include "camera.h"

Camera::Camera() :position_x_(0.0f), position_y_(0.0f), position_z_(0.0f),
rotation_x_(0.0f), rotation_y_(0.0f), rotation_z_(0.0f) {
}

Camera::Camera(const Camera & copy){
}

Camera::~Camera(){
}

void Camera::SetPosition(float x, float y, float z){
	position_x_ = x;
	position_y_ = y;
	position_z_ = z;
}

void Camera::SetRotation(float x, float y, float z){
	rotation_x_ = x;
	rotation_y_ = y;
	rotation_z_ = z;
}

DirectX::XMFLOAT3 Camera::GetPosition(){
	return DirectX::XMFLOAT3(position_x_,position_y_,position_z_);
}

DirectX::XMFLOAT3 Camera::GetRotation(){
	return DirectX::XMFLOAT3(rotation_x_,rotation_y_,rotation_z_);
}

void Camera::Render(){
	
	using namespace DirectX;
	
	XMFLOAT3 up;
	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;

	auto up_vector = XMLoadFloat3(&up);

	XMFLOAT3 position;
	position.x = this->position_x_;
	position.y = this->position_y_;
	position.z = this->position_z_;

	auto position_vector = XMLoadFloat3(&position);

	XMFLOAT3 look_at;
	look_at.x = 0.0f;
	look_at.y = 0.0f;
	look_at.z = 1.0f;

	auto look_at_vector = XMLoadFloat3(&look_at);

	auto pitch = rotation_x_*0.0174532925f;
	auto yaw = rotation_y_*0.0174532925f;
	auto roll = rotation_z_*0.0174532925f;

	auto rotation_matrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	look_at_vector = XMVector3TransformCoord(look_at_vector, rotation_matrix);
	
	up_vector = XMVector3TransformCoord(up_vector, rotation_matrix);

	look_at_vector = XMVectorAdd(position_vector, look_at_vector);

	view_matrix_ = XMMatrixLookAtLH(position_vector, look_at_vector, up_vector);
}

void Camera::RenderBaseViewMatrix(){
	
	using namespace DirectX;

	XMFLOAT3 up;
	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;

	auto up_vector = XMLoadFloat3(&up);

	XMFLOAT3 position;
	position.x = this->position_x_;
	position.y = this->position_y_;
	position.z = this->position_z_;

	auto position_vector = XMLoadFloat3(&position);

	XMFLOAT3 look_at;
	look_at.x = 0.0f;
	look_at.y = 0.0f;
	look_at.z = 1.0f;

	auto look_at_vector = XMLoadFloat3(&look_at);

	auto pitch = rotation_x_*0.0174532925f;
	auto yaw = rotation_y_*0.0174532925f;
	auto roll = rotation_z_*0.0174532925f;

	auto rotation_matrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	look_at_vector = XMVector3TransformCoord(look_at_vector, rotation_matrix);

	up_vector = XMVector3TransformCoord(up_vector, rotation_matrix);

	look_at_vector = XMVectorAdd(position_vector, look_at_vector);

	base_view_matrix_ = XMMatrixLookAtLH(position_vector, look_at_vector, up_vector);
}

void Camera::GetViewMatrix(DirectX::XMMATRIX & view_matrix){
	view_matrix = this->view_matrix_;
}

void Camera::GetBaseViewMatrix(DirectX::XMMATRIX & base_view_matrix){
	base_view_matrix = this->base_view_matrix_;
}
