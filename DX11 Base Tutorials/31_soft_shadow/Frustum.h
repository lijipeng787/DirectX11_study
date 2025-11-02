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

  // 优化的AABB检测（使用8个顶点快速检测）
  bool CheckAABB(const DirectX::XMFLOAT3 &min, const DirectX::XMFLOAT3 &max) const;

  // 使用包围体检测（优先使用AABB，更精确）
  bool CheckBoundingVolume(const BoundingVolume &bounds) const;

private:
  DirectX::XMVECTOR planes_[6];
};
