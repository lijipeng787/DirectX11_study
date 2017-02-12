#ifndef _LIGHTCLASS_H_
#define _LIGHTCLASS_H_

#include <DirectXMath.h>

class Light{
public:
	Light();
	
	explicit Light(const Light& copy);

	~Light();

public:
	void SetAmbientColor(float red, float green, float blue, float alpha);

	void SetDiffuseColor(float red, float green, float blue, float alpha);
	
	void SetDirection(float x, float y, float z);

	void SetSpecularColor(float red, float green, float blue, float alpha);

	void SetSpecularPower(float power);

	DirectX::XMFLOAT4 GetAmbientColor()const;

	DirectX::XMFLOAT4 GetDiffuseColor()const;
	
	DirectX::XMFLOAT3 GetDirection()const;

	DirectX::XMFLOAT4 GetSpecularColor()const;

	float GetSpecularPower()const;

private:
	DirectX::XMFLOAT4 ambient_color_ = {};

	DirectX::XMFLOAT4 diffuse_color_ = {};
	
	DirectX::XMFLOAT3 direction_ = {};

	DirectX::XMFLOAT4 specular_color_ = {};

	float specular_power_ = 0.0f;
};

#endif