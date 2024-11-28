#pragma once

#include <DirectXMath.h>

class LightClass {
public:
  LightClass() {}

  LightClass(const LightClass &) = delete;

  ~LightClass() {}

public:
  void LightClass::SetDiffuseColor(float red, float green, float blue,
                                   float alpha) {
    diffuse_color_ = DirectX::XMFLOAT4(red, green, blue, alpha);
  }

  void LightClass::SetDirection(float x, float y, float z) {
    direction_ = DirectX::XMFLOAT3(x, y, z);
  }

  inline DirectX::XMFLOAT4 LightClass::GetDiffuseColor() const {
    return diffuse_color_;
  }

  inline DirectX::XMFLOAT3 LightClass::GetDirection() const {
    return direction_;
  }

private:
  DirectX::XMFLOAT4 diffuse_color_{};

  DirectX::XMFLOAT3 direction_{};
};
