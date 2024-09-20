#pragma once

#include <DirectXMath.h>

class LightClass {
public:
	LightClass();

	LightClass(const LightClass& rhs) = delete;

	LightClass& operator=(const LightClass& rhs) = delete;

	~LightClass();

public:
	void SetAmbientColor(float red, float green, float blue, float alpha);

	void SetDiffuseColor(float red, float green, float blue, float alpha);

	void SetDirection(float x, float y, float z);

	void SetSpecularColor(float, float, float, float);

	void SetSpecularPower(float);

	DirectX::XMFLOAT4 GetAmbientColor();

	DirectX::XMFLOAT4 GetDiffuseColor();

	DirectX::XMFLOAT3 GetDirection();

	DirectX::XMFLOAT4 GetSpecularColor();

	float GetSpecularPower();

private:
	DirectX::XMFLOAT4 ambient_color_ = {};

	DirectX::XMFLOAT4 diffuse_color_ = {};

	DirectX::XMFLOAT3 light_direction_ = {};

	DirectX::XMFLOAT4 specular_color_;

	float specular_power_;
};
