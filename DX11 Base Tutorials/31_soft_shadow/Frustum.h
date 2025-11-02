#pragma once

#include <DirectXMath.h>

struct BoundingVolume;

class FrustumClass {
public:
  FrustumClass() {}

  FrustumClass(const FrustumClass &rhs) = delete;

  FrustumClass &operator=(const FrustumClass &rhs) = delete;

  ~FrustumClass() {}

public:
  void ConstructFrustum(float, const DirectX::XMMATRIX &,
                        const DirectX::XMMATRIX &);

  bool CheckPoint(float, float, float) const;

  bool CheckCube(float, float, float, float) const;

  bool CheckSphere(float, float, float, float) const;

  bool CheckRectangle(float, float, float, float, float, float) const;

  // Optimized AABB test (using 8 vertices for fast detection)
  bool CheckAABB(const DirectX::XMFLOAT3 &min,
                 const DirectX::XMFLOAT3 &max) const;

  // Test using bounding volume (prefer AABB for more precision)
  bool CheckBoundingVolume(const BoundingVolume &bounds) const;

private:
  DirectX::XMVECTOR planes_[6];
};
