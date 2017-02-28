#include "lightclass.h"

using namespace DirectX;

LightClass::LightClass(){}

LightClass::~LightClass(){}

void LightClass::SetDiffuseColor(float red, float green, float blue, float alpha) {
	diffuse_color_ = XMFLOAT4(red, green, blue, alpha);
}

void LightClass::SetDirection(float x, float y, float z) {
	light_direction_ = XMFLOAT3(x, y, z);
}

XMFLOAT4 LightClass::GetDiffuseColor() {
	return diffuse_color_;
}

XMFLOAT3 LightClass::GetDirection() {
	return light_direction_;
}
