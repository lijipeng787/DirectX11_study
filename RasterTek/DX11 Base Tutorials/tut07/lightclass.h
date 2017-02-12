#ifndef _LIGHTCLASS_H_
#define _LIGHTCLASS_H_

#include <DirectXMath.h>

class Light{
public:
	Light();
	
	explicit Light(const Light& copy);

	~Light();

public:
	void SetDiffuseColor(float red, float green, float blue, float alpha);
	
	void SetDirection(float x, float y, float z);

	DirectX::XMFLOAT4 GetDiffuseColor();
	
	DirectX::XMFLOAT3 GetDirection();

private:
	DirectX::XMFLOAT4 diffuse_color_;
	
	DirectX::XMFLOAT3 direction_;
};

#endif