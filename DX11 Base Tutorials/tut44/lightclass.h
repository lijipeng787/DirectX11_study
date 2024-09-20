#pragma once

#include <DirectXMath.h>

using namespace DirectX;

class LightClass {
public:
	LightClass();

	LightClass(const LightClass&);
	
	~LightClass();
public:
	void SetAmbientColor(float, float, float, float);

	void SetDiffuseColor(float, float, float, float);
	
	void SetPosition(float, float, float);
public:
	XMFLOAT4 GetAmbientColor();
	
	XMFLOAT4 GetDiffuseColor();
	
	XMFLOAT3 GetPosition();
private:
	XMFLOAT4 ambient_color_;

	XMFLOAT4 diffuse_color_;
	
	XMFLOAT3 light_position_;
};
