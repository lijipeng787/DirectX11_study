#pragma once

#include <DirectXMath.h>

class Camera {
public:
  // Constructors
  Camera() = default;

  Camera(const Camera &) = delete;

  Camera &operator=(const Camera &) = delete;

  Camera(Camera &&) = default;

  Camera &operator=(Camera &&) = default;

  ~Camera() = default;

public:
  // Position & Rotation
  void SetPosition(float x, float y, float z);

  void SetPosition(const DirectX::XMFLOAT3 &position);

  void SetRotation(float x, float y, float z);

  void SetRotation(const DirectX::XMFLOAT3 &rotation);

  DirectX::XMFLOAT3 GetPosition() const;

  DirectX::XMFLOAT3 GetRotation() const;

  // Camera Matrix Operations
  void Render();

  void RenderReflection(float height);

  void RenderBaseViewMatrix();

  void GetViewMatrix(DirectX::XMMATRIX &viewMatrix) const;

  DirectX::XMMATRIX GetReflectionViewMatrix() const;

  void GetBaseViewMatrix(DirectX::XMMATRIX &outViewMatrix) const;

  // Frustum Operations
  void ConstructFrustum(float screenDepth,
                        const DirectX::XMMATRIX &projectionMatrix,
                        const DirectX::XMMATRIX &viewMatrix);

  // Frustum Tests
  bool CheckPoint(float x, float y, float z) const;

  bool CheckSphere(float xCenter, float yCenter, float zCenter,
                   float radius) const;

  bool CheckCube(float xCenter, float yCenter, float zCenter,
                 float radius) const;

  bool CheckRectangle(float xCenter, float yCenter, float zCenter, float xSize,
                      float ySize, float zSize) const;

private:
  // Constants
  static constexpr float PI = 3.14159265358979323846f;
  static constexpr float HALF_PI = PI / 2.0f;
  // Position and rotation

  DirectX::XMFLOAT3 position_{0.0f, 0.0f, 0.0f};
  DirectX::XMFLOAT3 rotation_{0.0f, 0.0f, 0.0f};

  // View matrices
  DirectX::XMMATRIX view_matrix_{};
  DirectX::XMMATRIX reflection_view_matrix_{};
  DirectX::XMMATRIX base_view_matrix_{};

  // Frustum planes - stored in order: near, far, left, right, top, bottom
  DirectX::XMVECTOR frustum_planes_[6]{};

  // Helper methods
  DirectX::XMVECTOR ComputeUpVector() const;
  DirectX::XMVECTOR ComputePositionVector() const;
  DirectX::XMVECTOR ComputeLookAtVector() const;
  void ComputeViewMatrix(const DirectX::XMVECTOR &position,
                         const DirectX::XMVECTOR &lookAt,
                         const DirectX::XMVECTOR &up);
};
