#pragma once

#include <DirectXMath.h>

// Bounding volume data: contains AABB (Axis-Aligned Bounding Box) and bounding sphere
struct BoundingVolume {
  // AABB (Axis-Aligned Bounding Box)
  DirectX::XMFLOAT3 aabb_min;  // Minimum point
  DirectX::XMFLOAT3 aabb_max;  // Maximum point

  // Bounding Sphere
  DirectX::XMFLOAT3 sphere_center;  // Sphere center
  float sphere_radius;                // Radius

  BoundingVolume()
      : aabb_min(0.0f, 0.0f, 0.0f), aabb_max(0.0f, 0.0f, 0.0f),
        sphere_center(0.0f, 0.0f, 0.0f), sphere_radius(0.0f) {}

  // Calculate bounding volume from vertex array
  void CalculateFromVertices(const DirectX::XMFLOAT3 *vertices, size_t count);

  // Merge two bounding volumes
  void Merge(const BoundingVolume &other);

  // Transform bounding volume (apply world matrix)
  BoundingVolume Transform(const DirectX::XMMATRIX &worldMatrix) const;

  // Get AABB center and size
  DirectX::XMFLOAT3 GetAABBCenter() const;
  DirectX::XMFLOAT3 GetAABBSize() const;
};

