

#include "lightclass.h"

LightClass::LightClass() {}

LightClass::LightClass(const LightClass &other) {}

LightClass::~LightClass() {}

void LightClass::SetDirection(float x, float y, float z) {
  direction_ = XMFLOAT3(x, y, z);
}

void LightClass::SetPosition(float x, float y, float z) {
  light_position_ = XMFLOAT3(x, y, z);
}

void LightClass::SetLookAt(float x, float y, float z) {
  light_look_at_.x = x;
  light_look_at_.y = y;
  light_look_at_.z = z;
}

XMFLOAT3 LightClass::GetDirection() { return direction_; }

XMFLOAT3 LightClass::GetPosition() { return light_position_; }

void LightClass::GenerateViewMatrix() {
  XMVECTOR up;
  XMVECTOR position;
  XMVECTOR lookAt;

  // Setup the vector that points upwards.
  up.m128_f32[0] = 0.0f;
  up.m128_f32[1] = 1.0f;
  up.m128_f32[2] = 0.0f;
  up.m128_f32[3] = 1.0f;

  position.m128_f32[0] = light_position_.x;
  position.m128_f32[1] = light_position_.y;
  position.m128_f32[2] = light_position_.z;
  position.m128_f32[3] = 1.0f;

  lookAt.m128_f32[0] = light_look_at_.x;
  lookAt.m128_f32[1] = light_look_at_.y;
  lookAt.m128_f32[2] = light_look_at_.z;
  lookAt.m128_f32[3] = 1.0f;

  // Create the view matrix from the three vectors.
  light_viewMatrix_ = XMMatrixLookAtLH(position, lookAt, up);
}

void LightClass::GenerateProjectionMatrix(float screenDepth, float screenNear) {
  float fieldOfView, screenAspect;

  // Setup field of view and screen aspect for a square light source.
  fieldOfView = (float)XM_PI / 2.0f;
  screenAspect = 1.0f;

  // Create the projection matrix for the light.
  projection_matrix_ = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect,
                                                screenNear, screenDepth);
}

void LightClass::GenerateOrthoMatrix(float width, float height,
                                     float screenDepth, float screenNear) {
  // Create the projection matrix for the light.
  ortho_matrix_ =
      XMMatrixOrthographicLH(width, height, screenNear, screenDepth);
}

void LightClass::GetViewMatrix(XMMATRIX &viewMatrix) {
  viewMatrix = light_viewMatrix_;
}

void LightClass::GetProjectionMatrix(XMMATRIX &projectionMatrix) {
  projectionMatrix = projection_matrix_;
}

void LightClass::GetOrthoMatrix(XMMATRIX &orthoMatrix) {
  orthoMatrix = ortho_matrix_;
}