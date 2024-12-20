#pragma once

#include <DirectXMath.h>

class LightClass {
public:
  LightClass() {}

  LightClass(const LightClass &) = delete;

  ~LightClass() {}

public:
  void LightClass::SetAmbientColor(float red, float green, float blue,
                                   float alpha) {
    ambient_color_ = DirectX::XMFLOAT4(red, green, blue, alpha);
  }

  void LightClass::SetDiffuseColor(float red, float green, float blue,
                                   float alpha) {
    diffuse_color_ = DirectX::XMFLOAT4(red, green, blue, alpha);
  }

  void LightClass::SetDirection(float x, float y, float z) {
    direction_ = DirectX::XMFLOAT3(x, y, z);
  }

  void LightClass::SetSpecularColor(float red, float green, float blue,
                                    float alpha) {
    specular_color_ = DirectX::XMFLOAT4(red, green, blue, alpha);
  }

  void LightClass::SetSpecularPower(float power) { specular_power_ = power; }

public:
  inline DirectX::XMFLOAT4 GetAmbientColor() const { return ambient_color_; }

  inline DirectX::XMFLOAT4 GetDiffuseColor() const { return diffuse_color_; }

  inline DirectX::XMFLOAT3 GetDirection() const { return direction_; }

  inline DirectX::XMFLOAT4 GetSpecularColor() const { return specular_color_; }

  inline float GetSpecularPower() const { return specular_power_; }

private:
  DirectX::XMFLOAT4 ambient_color_{}, diffuse_color_{}, specular_color_{};

  DirectX::XMFLOAT3 direction_{};

  float specular_power_ = 0.0f;
};
