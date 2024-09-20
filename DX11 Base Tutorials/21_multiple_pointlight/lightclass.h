#pragma once

#include <DirectXMath.h>

using namespace DirectX;

class LightClass {
public:
	LightClass();

	LightClass(const LightClass&);

	~LightClass();
public:
	void SetDiffuseColor(float, float, float, float);

	void SetPosition(float, float, float);

	XMFLOAT4 GetDiffuseColor();

	XMFLOAT4 GetPosition();
private:
	XMFLOAT4 diffuse_color_;

	XMFLOAT4 light_position_;
};
