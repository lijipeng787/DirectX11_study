#pragma once

#include <DirectXMath.h>

class LightClass {
public:
	LightClass() {}

	LightClass(const LightClass&) = delete;

	~LightClass() {}
public:
	inline void SetAmbientColor(float red, float green, float blue, float alpha) {
		ambient_color_ = DirectX::XMFLOAT4(red, green, blue, alpha);
	}

	inline void SetDiffuseColor(float red, float green, float blue, float alpha) {
		diffuse_color_ = DirectX::XMFLOAT4(red, green, blue, alpha);
	}

	inline void SetDirection(float x, float y, float z) {
		direction_ = DirectX::XMFLOAT3(x, y, z);
	}
public:
	inline DirectX::XMFLOAT4 GetAmbientColor()const {
		return ambient_color_;
	}

	inline DirectX::XMFLOAT4 GetDiffuseColor()const {
		return diffuse_color_;
	}

	inline DirectX::XMFLOAT3 GetDirection()const {
		return direction_;
	}
private:
	DirectX::XMFLOAT4 ambient_color_{};

	DirectX::XMFLOAT4 diffuse_color_{};

	DirectX::XMFLOAT3 direction_{};
};
