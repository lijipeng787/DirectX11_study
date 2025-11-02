#pragma once

#include <DirectXMath.h>

// 包围体数据：包含AABB（轴对齐包围盒）和包围球
struct BoundingVolume {
  // AABB (Axis-Aligned Bounding Box)
  DirectX::XMFLOAT3 aabb_min;  // 最小点
  DirectX::XMFLOAT3 aabb_max;  // 最大点

  // Bounding Sphere（包围球）
  DirectX::XMFLOAT3 sphere_center;  // 球心
  float sphere_radius;                // 半径

  BoundingVolume()
      : aabb_min(0.0f, 0.0f, 0.0f), aabb_max(0.0f, 0.0f, 0.0f),
        sphere_center(0.0f, 0.0f, 0.0f), sphere_radius(0.0f) {}

  // 从顶点数组计算包围体
  void CalculateFromVertices(const DirectX::XMFLOAT3 *vertices, size_t count);

  // 合并两个包围体
  void Merge(const BoundingVolume &other);

  // 变换包围体（应用世界矩阵）
  BoundingVolume Transform(const DirectX::XMMATRIX &worldMatrix) const;

  // 获取AABB的中心和尺寸
  DirectX::XMFLOAT3 GetAABBCenter() const;
  DirectX::XMFLOAT3 GetAABBSize() const;
};

