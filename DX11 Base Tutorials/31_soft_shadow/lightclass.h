#pragma once

#include <DirectXMath.h>

class LightClass {
public:
  LightClass() {}

  LightClass(const LightClass &) {}

  ~LightClass() = default;

public:
  inline void SetAmbientColor(float red, float green, float blue, float alpha) {
    ambient_color_ = DirectX::XMFLOAT4(red, green, blue, alpha);
  }

  inline void SetDiffuseColor(float red, float green, float blue, float alpha) {
    diffuse_color_ = DirectX::XMFLOAT4(red, green, blue, alpha);
  }

  inline void SetPosition(float x, float y, float z) {
    light_position_ = DirectX::XMFLOAT3(x, y, z);
  }

  inline void SetLookAt(float x, float y, float z) {
    light_look_at_.x = x;
    light_look_at_.y = y;
    light_look_at_.z = z;
  }

  void SetDirection(float x, float y, float z) {
    light_direction_ = DirectX::XMFLOAT3(x, y, z);
  }

  inline DirectX::XMFLOAT4 GetAmbientColor() const { return ambient_color_; }

  inline DirectX::XMFLOAT4 GetDiffuseColor() const { return diffuse_color_; }

  inline DirectX::XMFLOAT3 GetPosition() const { return light_position_; }

  inline DirectX::XMFLOAT3 GetDirection() const { return light_direction_; }

  void GenerateViewMatrix();

  void GenerateProjectionMatrix(float, float);

  inline void GetViewMatrix(DirectX::XMMATRIX &viewMatrix) const {
    viewMatrix = light_viewMatrix_;
  }

  inline void GetProjectionMatrix(DirectX::XMMATRIX &projectionMatrix) const {
    projectionMatrix = projection_matrix_;
  }

private:
  DirectX::XMFLOAT4 ambient_color_;
  DirectX::XMFLOAT4 diffuse_color_;
  DirectX::XMFLOAT3 light_position_;
  DirectX::XMFLOAT3 light_direction_;
  DirectX::XMFLOAT3 light_look_at_;
  DirectX::XMMATRIX light_viewMatrix_;
  DirectX::XMMATRIX projection_matrix_;
};