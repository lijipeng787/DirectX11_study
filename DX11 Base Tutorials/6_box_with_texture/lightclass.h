#pragma once

#include <DirectXMath.h>

class LightClass {
public:
	LightClass() {}

	LightClass(const LightClass& rhs) = delete;

	LightClass& operator=(const LightClass& rhs) = delete;

	~LightClass() {}
public:
	inline void LightClass::SetDiffuseColor(float red, float green, float blue, float alpha) {
		diffuse_color_ = DirectX::XMFLOAT4(red, green, blue, alpha);
	}

	inline void LightClass::SetDirection(float x, float y, float z) {
		light_direction_ = DirectX::XMFLOAT3(x, y, z);
	}
public:
	inline DirectX::XMFLOAT4 GetDiffuseColor()const { return diffuse_color_; }

	inline DirectX::XMFLOAT3 GetDirection()const { return light_direction_; }
private:
	DirectX::XMFLOAT4 diffuse_color_ = {};

	DirectX::XMFLOAT3 light_direction_ = {};
};
