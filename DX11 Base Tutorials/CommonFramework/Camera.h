#pragma once

#include <DirectXMath.h>

class alignas(16) Camera {
public:
  Camera() = default;

  Camera(const Camera &) = delete;

  Camera &operator=(const Camera &) = delete;

  Camera(Camera &&) = default;

  Camera &operator=(Camera &&) = default;

  ~Camera() = default;

public:
  void GetWorldPosition(DirectX::XMMATRIX &wordMatrix);

  void SetMoveRate(int rate);

  void SetPosition(float, float, float);

  void SetRotation(float, float, float);

  void SetPosition(const DirectX::XMFLOAT3 &position);

  void SetRotation(const DirectX::XMFLOAT3 &rotation);

  DirectX::XMFLOAT3 GetPosition() const;

  DirectX::XMFLOAT3 GetRotation() const;

  void Render();

  void RenderReflection(float);

  DirectX::XMMATRIX GetReflectionViewMatrix();

  void GetViewMatrix(DirectX::XMMATRIX &viewMatrix) const;

  void RenderBaseViewMatrix();

  void GetBaseViewMatrix(DirectX::XMMATRIX &outViewMatrix) const;

  static constexpr float PI = 3.14159265358979323846f;
  static constexpr float HALF_PI = 3.14159265358979323846f / 2.0f;

private:
  float position_x_ = 0.0f, position_y_ = 0.0f, position_z_ = 0.0f;
  float rotation_x_ = 0.0f, rotation_y_ = 0.0f, rotation_z_ = 0.0f;

  DirectX::XMFLOAT3 position_{0.0f, 0.0f, 0.0f};
  DirectX::XMFLOAT3 rotation_{0.0f, 0.0f, 0.0f};

  DirectX::XMMATRIX m_reflectionViewMatrix;

  DirectX::XMMATRIX base_view_matrix_;

  DirectX::XMMATRIX view_matrix_ = {};
};
