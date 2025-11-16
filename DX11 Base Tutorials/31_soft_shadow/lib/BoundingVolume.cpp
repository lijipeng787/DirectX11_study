#include "BoundingVolume.h"

#include <algorithm>
#include <cmath>

using namespace DirectX;

void BoundingVolume::CalculateFromVertices(const XMFLOAT3 *vertices,
                                           size_t count) {
  if (count == 0) {
    aabb_min = XMFLOAT3(0.0f, 0.0f, 0.0f);
    aabb_max = XMFLOAT3(0.0f, 0.0f, 0.0f);
    sphere_center = XMFLOAT3(0.0f, 0.0f, 0.0f);
    sphere_radius = 0.0f;
    return;
  }

  // Calculate AABB
  aabb_min = vertices[0];
  aabb_max = vertices[0];

  for (size_t i = 1; i < count; ++i) {
    aabb_min.x = std::min(aabb_min.x, vertices[i].x);
    aabb_min.y = std::min(aabb_min.y, vertices[i].y);
    aabb_min.z = std::min(aabb_min.z, vertices[i].z);

    aabb_max.x = std::max(aabb_max.x, vertices[i].x);
    aabb_max.y = std::max(aabb_max.y, vertices[i].y);
    aabb_max.z = std::max(aabb_max.z, vertices[i].z);
  }

  // Calculate bounding sphere from AABB
  // Sphere center = AABB center
  sphere_center.x = (aabb_min.x + aabb_max.x) * 0.5f;
  sphere_center.y = (aabb_min.y + aabb_max.y) * 0.5f;
  sphere_center.z = (aabb_min.z + aabb_max.z) * 0.5f;

  // Radius = distance from center to farthest AABB vertex
  XMVECTOR center = XMLoadFloat3(&sphere_center);
  XMVECTOR maxCorner = XMLoadFloat3(&aabb_max);
  XMVECTOR distance = XMVector3Length(XMVectorSubtract(maxCorner, center));
  XMStoreFloat(&sphere_radius, distance);
}

void BoundingVolume::Merge(const BoundingVolume &other) {
  // Merge AABB
  aabb_min.x = std::min(aabb_min.x, other.aabb_min.x);
  aabb_min.y = std::min(aabb_min.y, other.aabb_min.y);
  aabb_min.z = std::min(aabb_min.z, other.aabb_min.z);

  aabb_max.x = std::max(aabb_max.x, other.aabb_max.x);
  aabb_max.y = std::max(aabb_max.y, other.aabb_max.y);
  aabb_max.z = std::max(aabb_max.z, other.aabb_max.z);

  // Recalculate bounding sphere
  sphere_center.x = (aabb_min.x + aabb_max.x) * 0.5f;
  sphere_center.y = (aabb_min.y + aabb_max.y) * 0.5f;
  sphere_center.z = (aabb_min.z + aabb_max.z) * 0.5f;

  XMVECTOR center = XMLoadFloat3(&sphere_center);
  XMVECTOR maxCorner = XMLoadFloat3(&aabb_max);
  XMVECTOR distance = XMVector3Length(XMVectorSubtract(maxCorner, center));
  XMStoreFloat(&sphere_radius, distance);
}

BoundingVolume BoundingVolume::Transform(const XMMATRIX &worldMatrix) const {
  BoundingVolume transformed;

  // Transform AABB's 8 vertices
  XMFLOAT3 corners[8];
  corners[0] = XMFLOAT3(aabb_min.x, aabb_min.y, aabb_min.z);
  corners[1] = XMFLOAT3(aabb_max.x, aabb_min.y, aabb_min.z);
  corners[2] = XMFLOAT3(aabb_min.x, aabb_max.y, aabb_min.z);
  corners[3] = XMFLOAT3(aabb_max.x, aabb_max.y, aabb_min.z);
  corners[4] = XMFLOAT3(aabb_min.x, aabb_min.y, aabb_max.z);
  corners[5] = XMFLOAT3(aabb_max.x, aabb_min.y, aabb_max.z);
  corners[6] = XMFLOAT3(aabb_min.x, aabb_max.y, aabb_max.z);
  corners[7] = XMFLOAT3(aabb_max.x, aabb_max.y, aabb_max.z);

  // Transform all vertices
  XMFLOAT3 transformedCorners[8];
  for (int i = 0; i < 8; ++i) {
    XMVECTOR corner = XMLoadFloat3(&corners[i]);
    XMVECTOR transformed = XMVector3Transform(corner, worldMatrix);
    XMStoreFloat3(&transformedCorners[i], transformed);
  }

  // Recalculate AABB from transformed vertices
  transformed.CalculateFromVertices(transformedCorners, 8);

  return transformed;
}

XMFLOAT3 BoundingVolume::GetAABBCenter() const {
  return XMFLOAT3((aabb_min.x + aabb_max.x) * 0.5f,
                  (aabb_min.y + aabb_max.y) * 0.5f,
                  (aabb_min.z + aabb_max.z) * 0.5f);
}

XMFLOAT3 BoundingVolume::GetAABBSize() const {
  return XMFLOAT3(aabb_max.x - aabb_min.x, aabb_max.y - aabb_min.y,
                  aabb_max.z - aabb_min.z);
}
