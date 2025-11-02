#include "Frustum.h"

#include "BoundingVolume.h"
#include <algorithm>

using namespace DirectX;

void FrustumClass::ConstructFrustum(float screenDepth,
                                    const XMMATRIX &projectionMatrix,
                                    const XMMATRIX &viewMatrix) {

  XMMATRIX projectionMatrixCopy = projectionMatrix;

  // Calculate the minimum Z distance in the frustum.
  float zMinimum = -projectionMatrixCopy.r[3].m128_f32[2] /
                   projectionMatrixCopy.r[2].m128_f32[2];
  float r = screenDepth / (screenDepth - zMinimum);
  projectionMatrixCopy.r[2].m128_f32[2] = r;
  projectionMatrixCopy.r[3].m128_f32[2] = -r * zMinimum;

  // Create the frustum matrix from the view matrix and updated projection
  // matrix.
  auto matrix = XMMatrixMultiply(viewMatrix, projectionMatrixCopy);

  // Calculate near plane of frustum.
  planes_[0].m128_f32[0] = matrix.r[0].m128_f32[3] + matrix.r[0].m128_f32[2];
  planes_[0].m128_f32[1] = matrix.r[1].m128_f32[3] + matrix.r[1].m128_f32[2];
  planes_[0].m128_f32[2] = matrix.r[2].m128_f32[3] + matrix.r[2].m128_f32[2];
  planes_[0].m128_f32[3] = matrix.r[3].m128_f32[3] + matrix.r[3].m128_f32[2];
  planes_[0] = XMPlaneNormalize(planes_[0]);

  // Calculate far plane of frustum.
  planes_[1].m128_f32[0] = matrix.r[0].m128_f32[3] - matrix.r[0].m128_f32[2];
  planes_[1].m128_f32[1] = matrix.r[1].m128_f32[3] - matrix.r[1].m128_f32[2];
  planes_[1].m128_f32[2] = matrix.r[2].m128_f32[3] - matrix.r[2].m128_f32[2];
  planes_[1].m128_f32[3] = matrix.r[3].m128_f32[3] - matrix.r[3].m128_f32[2];
  planes_[1] = XMPlaneNormalize(planes_[1]);

  // Calculate left plane of frustum.
  planes_[2].m128_f32[0] = matrix.r[0].m128_f32[3] + matrix.r[0].m128_f32[0];
  planes_[2].m128_f32[1] = matrix.r[1].m128_f32[3] + matrix.r[1].m128_f32[0];
  planes_[2].m128_f32[2] = matrix.r[2].m128_f32[3] + matrix.r[2].m128_f32[0];
  planes_[2].m128_f32[3] = matrix.r[3].m128_f32[3] + matrix.r[3].m128_f32[0];
  planes_[2] = XMPlaneNormalize(planes_[2]);

  // Calculate right plane of frustum.
  planes_[3].m128_f32[0] = matrix.r[0].m128_f32[3] - matrix.r[0].m128_f32[0];
  planes_[3].m128_f32[1] = matrix.r[1].m128_f32[3] - matrix.r[1].m128_f32[0];
  planes_[3].m128_f32[2] = matrix.r[2].m128_f32[3] - matrix.r[2].m128_f32[0];
  planes_[3].m128_f32[3] = matrix.r[3].m128_f32[3] - matrix.r[3].m128_f32[0];
  planes_[3] = XMPlaneNormalize(planes_[3]);

  // Calculate top plane of frustum.
  planes_[4].m128_f32[0] = matrix.r[0].m128_f32[3] - matrix.r[0].m128_f32[1];
  planes_[4].m128_f32[1] = matrix.r[1].m128_f32[3] - matrix.r[1].m128_f32[1];
  planes_[4].m128_f32[2] = matrix.r[2].m128_f32[3] - matrix.r[2].m128_f32[1];
  planes_[4].m128_f32[3] = matrix.r[3].m128_f32[3] - matrix.r[3].m128_f32[1];
  planes_[4] = XMPlaneNormalize(planes_[4]);

  // Calculate bottom plane of frustum.
  planes_[5].m128_f32[0] = matrix.r[0].m128_f32[3] + matrix.r[0].m128_f32[1];
  planes_[5].m128_f32[1] = matrix.r[1].m128_f32[3] + matrix.r[1].m128_f32[1];
  planes_[5].m128_f32[2] = matrix.r[2].m128_f32[3] + matrix.r[2].m128_f32[1];
  planes_[5].m128_f32[3] = matrix.r[3].m128_f32[3] + matrix.r[3].m128_f32[1];
  planes_[5] = XMPlaneNormalize(planes_[5]);
}

bool FrustumClass::CheckPoint(float x, float y, float z) const {

  XMVECTOR tempVector{};
  XMFLOAT3 temp(x, y, z);

  // Check if the point is inside all six planes of the view frustum.
  for (int i = 0; i < 6; i++) {
    tempVector = XMPlaneDotCoord(planes_[i], XMLoadFloat3(&temp));

    if (tempVector.m128_f32[0] < 0.0f) {
      return false;
    }
  }

  return true;
}

bool FrustumClass::CheckCube(float xCenter, float yCenter, float zCenter,
                             float radius) const {

  XMVECTOR tempVector{};
  XMFLOAT3 temp{};

  // Check if any one point of the cube is in the view frustum.
  for (int i = 0; i < 6; i++) {

    temp.x = xCenter - radius;
    temp.y = yCenter - radius;
    temp.z = zCenter - radius;
    tempVector = XMPlaneDotCoord(planes_[i], XMLoadFloat3(&temp));

    if (tempVector.m128_f32[0] >= 0.0f) {
      continue;
    }

    temp.x = xCenter + radius;
    temp.y = yCenter - radius;
    temp.z = zCenter - radius;
    tempVector = XMPlaneDotCoord(planes_[i], XMLoadFloat3(&temp));

    if (tempVector.m128_f32[0] >= 0.0f) {
      continue;
    }

    temp.x = xCenter - radius;
    temp.y = yCenter + radius;
    temp.z = zCenter - radius;
    tempVector = XMPlaneDotCoord(planes_[i], XMLoadFloat3(&temp));

    if (tempVector.m128_f32[0] >= 0.0f) {
      continue;
    }

    temp.x = xCenter + radius;
    temp.y = yCenter + radius;
    temp.z = zCenter - radius;
    tempVector = XMPlaneDotCoord(planes_[i], XMLoadFloat3(&temp));

    if (tempVector.m128_f32[0] >= 0.0f) {
      continue;
    }

    temp.x = xCenter - radius;
    temp.y = yCenter - radius;
    temp.z = zCenter + radius;
    tempVector = XMPlaneDotCoord(planes_[i], XMLoadFloat3(&temp));

    if (tempVector.m128_f32[0] >= 0.0f) {
      continue;
    }

    temp.x = xCenter + radius;
    temp.y = yCenter - radius;
    temp.z = zCenter + radius;
    tempVector = XMPlaneDotCoord(planes_[i], XMLoadFloat3(&temp));

    if (tempVector.m128_f32[0] >= 0.0f) {
      continue;
    }

    temp.x = xCenter - radius;
    temp.y = yCenter + radius;
    temp.z = zCenter + radius;
    tempVector = XMPlaneDotCoord(planes_[i], XMLoadFloat3(&temp));

    if (tempVector.m128_f32[0] >= 0.0f) {
      continue;
    }

    temp.x = xCenter + radius;
    temp.y = yCenter + radius;
    temp.z = zCenter + radius;
    tempVector = XMPlaneDotCoord(planes_[i], XMLoadFloat3(&temp));

    if (tempVector.m128_f32[0] >= 0.0f) {
      continue;
    }

    return false;
  }

  return true;
}

bool FrustumClass::CheckSphere(float xCenter, float yCenter, float zCenter,
                               float radius) const {

  XMVECTOR tempVector{};
  XMFLOAT3 temp(xCenter, yCenter, zCenter);

  // Check if the radius of the sphere is inside the view frustum.
  for (int i = 0; i < 6; i++) {
    tempVector = XMPlaneDotCoord(planes_[i], XMLoadFloat3(&temp));

    if (tempVector.m128_f32[0] < -radius) {
      return false;
    }
  }

  return true;
}

bool FrustumClass::CheckRectangle(float xCenter, float yCenter, float zCenter,
                                  float xSize, float ySize, float zSize) const {

  XMVECTOR tempVector{};
  XMFLOAT3 temp{};

  // Check if any of the 6 planes of the rectangle are inside the view frustum.
  for (int i = 0; i < 6; i++) {

    temp.x = xCenter - xSize;
    temp.y = yCenter - ySize;
    temp.z = zCenter - zSize;
    tempVector = XMPlaneDotCoord(planes_[i], XMLoadFloat3(&temp));

    if (tempVector.m128_f32[0] >= 0.0f) {
      continue;
    }

    temp.x = xCenter + xSize;
    temp.y = yCenter - ySize;
    temp.z = zCenter - zSize;
    tempVector = XMPlaneDotCoord(planes_[i], XMLoadFloat3(&temp));

    if (tempVector.m128_f32[0] >= 0.0f) {
      continue;
    }

    temp.x = xCenter - xSize;
    temp.y = yCenter + ySize;
    temp.z = zCenter - zSize;
    tempVector = XMPlaneDotCoord(planes_[i], XMLoadFloat3(&temp));

    if (tempVector.m128_f32[0] >= 0.0f) {
      continue;
    }

    temp.x = xCenter - xSize;
    temp.y = yCenter - ySize;
    temp.z = zCenter + zSize;
    tempVector = XMPlaneDotCoord(planes_[i], XMLoadFloat3(&temp));

    if (tempVector.m128_f32[0] >= 0.0f) {
      continue;
    }

    temp.x = xCenter + xSize;
    temp.y = yCenter + ySize;
    temp.z = zCenter - zSize;
    tempVector = XMPlaneDotCoord(planes_[i], XMLoadFloat3(&temp));

    if (tempVector.m128_f32[0] >= 0.0f) {
      continue;
    }

    temp.x = xCenter + xSize;
    temp.y = yCenter - ySize;
    temp.z = zCenter + zSize;
    tempVector = XMPlaneDotCoord(planes_[i], XMLoadFloat3(&temp));

    if (tempVector.m128_f32[0] >= 0.0f) {
      continue;
    }

    temp.x = xCenter - xSize;
    temp.y = yCenter + ySize;
    temp.z = zCenter + zSize;
    tempVector = XMPlaneDotCoord(planes_[i], XMLoadFloat3(&temp));

    if (tempVector.m128_f32[0] >= 0.0f) {
      continue;
    }

    temp.x = xCenter + xSize;
    temp.y = yCenter + ySize;
    temp.z = zCenter + zSize;
    tempVector = XMPlaneDotCoord(planes_[i], XMLoadFloat3(&temp));

    if (tempVector.m128_f32[0] >= 0.0f) {
      continue;
    }

    return false;
  }

  return true;
}

bool FrustumClass::CheckAABB(const XMFLOAT3 &min,
                             const XMFLOAT3 &max) const {
  // 优化的AABB检测：检查AABB的8个顶点
  // 如果所有顶点都在视锥体外，则对象不可见
  // 如果至少有一个顶点在视锥体内，或AABB与视锥体相交，则可见

  XMFLOAT3 corners[8];
  corners[0] = XMFLOAT3(min.x, min.y, min.z);
  corners[1] = XMFLOAT3(max.x, min.y, min.z);
  corners[2] = XMFLOAT3(min.x, max.y, min.z);
  corners[3] = XMFLOAT3(max.x, max.y, min.z);
  corners[4] = XMFLOAT3(min.x, min.y, max.z);
  corners[5] = XMFLOAT3(max.x, min.y, max.z);
  corners[6] = XMFLOAT3(min.x, max.y, max.z);
  corners[7] = XMFLOAT3(max.x, max.y, max.z);

  // 对每个视锥体平面进行检查
  for (int i = 0; i < 6; ++i) {
    bool hasInside = false;
    
    // 检查8个顶点，看是否有任何一个在平面内侧
    for (int j = 0; j < 8; ++j) {
      XMVECTOR corner = XMLoadFloat3(&corners[j]);
      XMVECTOR planeDot = XMPlaneDotCoord(planes_[i], corner);
      
      if (planeDot.m128_f32[0] >= 0.0f) {
        hasInside = true;
        break; // 至少有一个顶点在平面内侧
      }
    }
    
    // 如果所有顶点都在平面外侧，则AABB完全在视锥体外
    if (!hasInside) {
      return false;
    }
  }

  return true;
}

bool FrustumClass::CheckBoundingVolume(const BoundingVolume &bounds) const {
  // 优先使用AABB检测（更精确），回退到包围球检测
  // 先快速检查包围球，如果包围球不可见，则AABB也不可见
  if (!CheckSphere(bounds.sphere_center.x, bounds.sphere_center.y,
                   bounds.sphere_center.z, bounds.sphere_radius)) {
    return false;
  }

  // 包围球可见，使用AABB进行精确检测
  return CheckAABB(bounds.aabb_min, bounds.aabb_max);
}
