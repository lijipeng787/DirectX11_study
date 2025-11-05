#include "light.h"

using namespace DirectX;

void Light::GenerateViewMatrix() {
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

void Light::GenerateProjectionMatrix(float screenDepth, float screenNear) {
  float fieldOfView, screenAspect;

  // Setup field of view and screen aspect for a square light source.
  fieldOfView = (float)XM_PI / 2.0f;
  screenAspect = 1.0f;

  // Create the projection matrix for the light.
  projection_matrix_ = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect,
                                                screenNear, screenDepth);
}