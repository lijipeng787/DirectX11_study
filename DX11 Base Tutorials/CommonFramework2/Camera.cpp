#include "stdafx.h"

#include "Camera.h"

using namespace DirectX;

void Camera::SetPosition(float x, float y, float z) {
  position_ = XMFLOAT3(x, y, z);
}

void Camera::SetPosition(const XMFLOAT3 &position) { position_ = position; }

void Camera::SetRotation(float x, float y, float z) {
  rotation_ = XMFLOAT3(x, y, z);
}

void Camera::SetRotation(const XMFLOAT3 &rotation) { rotation_ = rotation; }

XMFLOAT3 Camera::GetPosition() const { return position_; }

XMFLOAT3 Camera::GetRotation() const { return rotation_; }

XMVECTOR Camera::ComputeUpVector() const {
  // Create up vector
  XMVECTOR up;
  up.m128_f32[0] = 0.0f;
  up.m128_f32[1] = 1.0f;
  up.m128_f32[2] = 0.0f;
  up.m128_f32[3] = 1.0f;
  return up;
}

XMVECTOR Camera::ComputePositionVector() const {
  // Convert position to vector
  XMVECTOR position;
  position.m128_f32[0] = position_.x;
  position.m128_f32[1] = position_.y;
  position.m128_f32[2] = position_.z;
  position.m128_f32[3] = 1.0f;
  return position;
}

XMVECTOR Camera::ComputeLookAtVector() const {
  // Default look at point
  XMVECTOR lookAt;
  lookAt.m128_f32[0] = 0.0f;
  lookAt.m128_f32[1] = 0.0f;
  lookAt.m128_f32[2] = 1.0f;
  lookAt.m128_f32[3] = 1.0f;
  return lookAt;
}

void Camera::ComputeViewMatrix(const XMVECTOR &position, const XMVECTOR &lookAt,
                               const XMVECTOR &up) {
  view_matrix_ = XMMatrixLookAtLH(position, lookAt, up);
}

void Camera::Render() {
  // Get base vectors
  XMVECTOR up = ComputeUpVector();
  XMVECTOR position = ComputePositionVector();
  XMVECTOR lookAt = ComputeLookAtVector();

  // Create rotation matrix
  float pitch = rotation_.x * HALF_PI;
  float yaw = rotation_.y * HALF_PI;
  float roll = rotation_.z * HALF_PI;
  XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

  // Transform vectors
  lookAt = XMVector3TransformCoord(lookAt, rotationMatrix);
  up = XMVector3TransformCoord(up, rotationMatrix);
  lookAt = position + lookAt;

  // Create view matrix
  ComputeViewMatrix(position, lookAt, up);
}

void Camera::RenderReflection(float height) {
  // Create reflected position
  XMVECTOR position = ComputePositionVector();
  position.m128_f32[1] = -position_.y + (height * 2.0f);

  // Setup look direction
  float radians = rotation_.y * HALF_PI;
  XMVECTOR lookAt;
  lookAt.m128_f32[0] = sinf(radians) + position_.x;
  lookAt.m128_f32[1] = position.m128_f32[1];
  lookAt.m128_f32[2] = cosf(radians) + position_.z;
  lookAt.m128_f32[3] = 1.0f;

  // Create reflection view matrix
  reflection_view_matrix_ =
      XMMatrixLookAtLH(position, lookAt, ComputeUpVector());
}

void Camera::RenderBaseViewMatrix() {
  // Setup lookAt based on rotation
  float radians = rotation_.y * HALF_PI;
  XMVECTOR lookAt;
  lookAt.m128_f32[0] = sinf(radians) + position_.x;
  lookAt.m128_f32[1] = position_.y;
  lookAt.m128_f32[2] = cosf(radians) + position_.z;

  // Create base view matrix
  base_view_matrix_ =
      XMMatrixLookAtLH(ComputePositionVector(), lookAt, ComputeUpVector());
}

void Camera::GetViewMatrix(XMMATRIX &viewMatrix) const {
  viewMatrix = view_matrix_;
}

XMMATRIX Camera::GetReflectionViewMatrix() const {
  return reflection_view_matrix_;
}

void Camera::GetBaseViewMatrix(XMMATRIX &outViewMatrix) const {
  outViewMatrix = base_view_matrix_;
}

void Camera::ConstructFrustum(float screenDepth,
                              const XMMATRIX &projectionMatrix,
                              const XMMATRIX &viewMatrix) {
  // Create frustum matrix
  XMMATRIX projectionCopy = projectionMatrix;

  // Calculate minimum Z distance
  float zMin =
      -projectionCopy.r[3].m128_f32[2] / projectionCopy.r[2].m128_f32[2];
  float r = screenDepth / (screenDepth - zMin);
  projectionCopy.r[2].m128_f32[2] = r;
  projectionCopy.r[3].m128_f32[2] = -r * zMin;

  // Create frustum matrix
  XMMATRIX matrix = XMMatrixMultiply(viewMatrix, projectionCopy);

  // Calculate frustum planes
  // Near plane
  frustum_planes_[0].m128_f32[0] =
      matrix.r[0].m128_f32[3] + matrix.r[0].m128_f32[2];
  frustum_planes_[0].m128_f32[1] =
      matrix.r[1].m128_f32[3] + matrix.r[1].m128_f32[2];
  frustum_planes_[0].m128_f32[2] =
      matrix.r[2].m128_f32[3] + matrix.r[2].m128_f32[2];
  frustum_planes_[0].m128_f32[3] =
      matrix.r[3].m128_f32[3] + matrix.r[3].m128_f32[2];
  frustum_planes_[0] = XMPlaneNormalize(frustum_planes_[0]);

  // Far plane
  frustum_planes_[1].m128_f32[0] =
      matrix.r[0].m128_f32[3] - matrix.r[0].m128_f32[2];
  frustum_planes_[1].m128_f32[1] =
      matrix.r[1].m128_f32[3] - matrix.r[1].m128_f32[2];
  frustum_planes_[1].m128_f32[2] =
      matrix.r[2].m128_f32[3] - matrix.r[2].m128_f32[2];
  frustum_planes_[1].m128_f32[3] =
      matrix.r[3].m128_f32[3] - matrix.r[3].m128_f32[2];
  frustum_planes_[1] = XMPlaneNormalize(frustum_planes_[1]);

  // Calculate left plane of frustum.
  frustum_planes_[2].m128_f32[0] =
      matrix.r[0].m128_f32[3] + matrix.r[0].m128_f32[0];
  frustum_planes_[2].m128_f32[1] =
      matrix.r[1].m128_f32[3] + matrix.r[1].m128_f32[0];
  frustum_planes_[2].m128_f32[2] =
      matrix.r[2].m128_f32[3] + matrix.r[2].m128_f32[0];
  frustum_planes_[2].m128_f32[3] =
      matrix.r[3].m128_f32[3] + matrix.r[3].m128_f32[0];
  frustum_planes_[2] = XMPlaneNormalize(frustum_planes_[2]);

  // Calculate right plane of frustum.
  frustum_planes_[3].m128_f32[0] =
      matrix.r[0].m128_f32[3] - matrix.r[0].m128_f32[0];
  frustum_planes_[3].m128_f32[1] =
      matrix.r[1].m128_f32[3] - matrix.r[1].m128_f32[0];
  frustum_planes_[3].m128_f32[2] =
      matrix.r[2].m128_f32[3] - matrix.r[2].m128_f32[0];
  frustum_planes_[3].m128_f32[3] =
      matrix.r[3].m128_f32[3] - matrix.r[3].m128_f32[0];
  frustum_planes_[3] = XMPlaneNormalize(frustum_planes_[3]);

  // Calculate top plane of frustum.
  frustum_planes_[4].m128_f32[0] =
      matrix.r[0].m128_f32[3] - matrix.r[0].m128_f32[1];
  frustum_planes_[4].m128_f32[1] =
      matrix.r[1].m128_f32[3] - matrix.r[1].m128_f32[1];
  frustum_planes_[4].m128_f32[2] =
      matrix.r[2].m128_f32[3] - matrix.r[2].m128_f32[1];
  frustum_planes_[4].m128_f32[3] =
      matrix.r[3].m128_f32[3] - matrix.r[3].m128_f32[1];
  frustum_planes_[4] = XMPlaneNormalize(frustum_planes_[4]);

  // Calculate bottom plane of frustum.
  frustum_planes_[5].m128_f32[0] =
      matrix.r[0].m128_f32[3] + matrix.r[0].m128_f32[1];
  frustum_planes_[5].m128_f32[1] =
      matrix.r[1].m128_f32[3] + matrix.r[1].m128_f32[1];
  frustum_planes_[5].m128_f32[2] =
      matrix.r[2].m128_f32[3] + matrix.r[2].m128_f32[1];
  frustum_planes_[5].m128_f32[3] =
      matrix.r[3].m128_f32[3] + matrix.r[3].m128_f32[1];
  frustum_planes_[5] = XMPlaneNormalize(frustum_planes_[5]);
}

bool Camera::CheckPoint(float x, float y, float z) const {
  XMFLOAT3 point(x, y, z);
  for (int i = 0; i < 6; i++) {
    if (XMPlaneDotCoord(frustum_planes_[i], XMLoadFloat3(&point)).m128_f32[0] <
        0.0f) {
      return false;
    }
  }
  return true;
}

bool Camera::CheckCube(float xCenter, float yCenter, float zCenter,
                       float radius) const {

  XMVECTOR tempVector{};
  XMFLOAT3 temp{};

  // Check if any one point of the cube is in the view frustum.
  for (int i = 0; i < 6; i++) {

    temp.x = xCenter - radius;
    temp.y = yCenter - radius;
    temp.z = zCenter - radius;
    tempVector = XMPlaneDotCoord(frustum_planes_[i], XMLoadFloat3(&temp));

    if (tempVector.m128_f32[0] >= 0.0f) {
      continue;
    }

    temp.x = xCenter + radius;
    temp.y = yCenter - radius;
    temp.z = zCenter - radius;
    tempVector = XMPlaneDotCoord(frustum_planes_[i], XMLoadFloat3(&temp));

    if (tempVector.m128_f32[0] >= 0.0f) {
      continue;
    }

    temp.x = xCenter - radius;
    temp.y = yCenter + radius;
    temp.z = zCenter - radius;
    tempVector = XMPlaneDotCoord(frustum_planes_[i], XMLoadFloat3(&temp));

    if (tempVector.m128_f32[0] >= 0.0f) {
      continue;
    }

    temp.x = xCenter + radius;
    temp.y = yCenter + radius;
    temp.z = zCenter - radius;
    tempVector = XMPlaneDotCoord(frustum_planes_[i], XMLoadFloat3(&temp));

    if (tempVector.m128_f32[0] >= 0.0f) {
      continue;
    }

    temp.x = xCenter - radius;
    temp.y = yCenter - radius;
    temp.z = zCenter + radius;
    tempVector = XMPlaneDotCoord(frustum_planes_[i], XMLoadFloat3(&temp));

    if (tempVector.m128_f32[0] >= 0.0f) {
      continue;
    }

    temp.x = xCenter + radius;
    temp.y = yCenter - radius;
    temp.z = zCenter + radius;
    tempVector = XMPlaneDotCoord(frustum_planes_[i], XMLoadFloat3(&temp));

    if (tempVector.m128_f32[0] >= 0.0f) {
      continue;
    }

    temp.x = xCenter - radius;
    temp.y = yCenter + radius;
    temp.z = zCenter + radius;
    tempVector = XMPlaneDotCoord(frustum_planes_[i], XMLoadFloat3(&temp));

    if (tempVector.m128_f32[0] >= 0.0f) {
      continue;
    }

    temp.x = xCenter + radius;
    temp.y = yCenter + radius;
    temp.z = zCenter + radius;
    tempVector = XMPlaneDotCoord(frustum_planes_[i], XMLoadFloat3(&temp));

    if (tempVector.m128_f32[0] >= 0.0f) {
      continue;
    }

    return false;
  }

  return true;
}

bool Camera::CheckSphere(float xCenter, float yCenter, float zCenter,
                         float radius) const {

  XMVECTOR tempVector{};
  XMFLOAT3 temp(xCenter, yCenter, zCenter);

  // Check if the radius of the sphere is inside the view frustum.
  for (int i = 0; i < 6; i++) {
    tempVector = XMPlaneDotCoord(frustum_planes_[i], XMLoadFloat3(&temp));

    if (tempVector.m128_f32[0] < -radius) {
      return false;
    }
  }

  return true;
}

bool Camera::CheckRectangle(float xCenter, float yCenter, float zCenter,
                            float xSize, float ySize, float zSize) const {

  XMVECTOR tempVector{};
  XMFLOAT3 temp{};

  // Check if any of the 6 planes of the rectangle are inside the view frustum.
  for (int i = 0; i < 6; i++) {

    temp.x = xCenter - xSize;
    temp.y = yCenter - ySize;
    temp.z = zCenter - zSize;
    tempVector = XMPlaneDotCoord(frustum_planes_[i], XMLoadFloat3(&temp));

    if (tempVector.m128_f32[0] >= 0.0f) {
      continue;
    }

    temp.x = xCenter + xSize;
    temp.y = yCenter - ySize;
    temp.z = zCenter - zSize;
    tempVector = XMPlaneDotCoord(frustum_planes_[i], XMLoadFloat3(&temp));

    if (tempVector.m128_f32[0] >= 0.0f) {
      continue;
    }

    temp.x = xCenter - xSize;
    temp.y = yCenter + ySize;
    temp.z = zCenter - zSize;
    tempVector = XMPlaneDotCoord(frustum_planes_[i], XMLoadFloat3(&temp));

    if (tempVector.m128_f32[0] >= 0.0f) {
      continue;
    }

    temp.x = xCenter - xSize;
    temp.y = yCenter - ySize;
    temp.z = zCenter + zSize;
    tempVector = XMPlaneDotCoord(frustum_planes_[i], XMLoadFloat3(&temp));

    if (tempVector.m128_f32[0] >= 0.0f) {
      continue;
    }

    temp.x = xCenter + xSize;
    temp.y = yCenter + ySize;
    temp.z = zCenter - zSize;
    tempVector = XMPlaneDotCoord(frustum_planes_[i], XMLoadFloat3(&temp));

    if (tempVector.m128_f32[0] >= 0.0f) {
      continue;
    }

    temp.x = xCenter + xSize;
    temp.y = yCenter - ySize;
    temp.z = zCenter + zSize;
    tempVector = XMPlaneDotCoord(frustum_planes_[i], XMLoadFloat3(&temp));

    if (tempVector.m128_f32[0] >= 0.0f) {
      continue;
    }

    temp.x = xCenter - xSize;
    temp.y = yCenter + ySize;
    temp.z = zCenter + zSize;
    tempVector = XMPlaneDotCoord(frustum_planes_[i], XMLoadFloat3(&temp));

    if (tempVector.m128_f32[0] >= 0.0f) {
      continue;
    }

    temp.x = xCenter + xSize;
    temp.y = yCenter + ySize;
    temp.z = zCenter + zSize;
    tempVector = XMPlaneDotCoord(frustum_planes_[i], XMLoadFloat3(&temp));

    if (tempVector.m128_f32[0] >= 0.0f) {
      continue;
    }

    return false;
  }

  return true;
}