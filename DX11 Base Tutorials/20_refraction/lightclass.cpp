#include "lightclass.h"

using namespace DirectX;

LightClass::LightClass() {}

LightClass::~LightClass() {}

void LightClass::SetAmbientColor(float red, float green, float blue,
                                 float alpha) {
  ambient_color_ = XMFLOAT4(red, green, blue, alpha);
}

XMFLOAT4 LightClass::GetAmbientColor() { return ambient_color_; }

void LightClass::SetSpecularColor(float red, float green, float blue,
                                  float alpha) {
  specular_color_ = XMFLOAT4(red, green, blue, alpha);
}

void LightClass::SetSpecularPower(float power) { specular_power_ = power; }

void LightClass::SetDiffuseColor(float red, float green, float blue,
                                 float alpha) {
  diffuse_color_ = XMFLOAT4(red, green, blue, alpha);
}

void LightClass::SetDirection(float x, float y, float z) {
  light_direction_ = XMFLOAT3(x, y, z);
}

XMFLOAT4 LightClass::GetDiffuseColor() { return diffuse_color_; }

XMFLOAT3 LightClass::GetDirection() { return light_direction_; }

XMFLOAT4 LightClass::GetSpecularColor() { return specular_color_; }

float LightClass::GetSpecularPower() { return specular_power_; }
