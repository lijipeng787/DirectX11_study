#include "lightclass.h"

LightClass::LightClass() {}

LightClass::LightClass(const LightClass& other) {}

LightClass::~LightClass() {}

void LightClass::SetDiffuseColor(float red, float green, float blue, float alpha) {
	diffuse_color_ = XMFLOAT4(red, green, blue, alpha);
}

void LightClass::SetPosition(float x, float y, float z) {
	light_position_ = XMFLOAT4(x, y, z, 1.0f);
}

XMFLOAT4 LightClass::GetDiffuseColor() {
	return diffuse_color_;
}

XMFLOAT4 LightClass::GetPosition() {
	return light_position_;
}