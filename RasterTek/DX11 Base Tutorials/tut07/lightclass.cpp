#include "lightclass.h"

Light::Light(){
}

Light::Light(const Light& other){
}

Light::~Light(){
}

void Light::SetDiffuseColor(float red, float green, float blue, float alpha){
	diffuse_color_ = DirectX::XMFLOAT4(red, green, blue, alpha);
}

void Light::SetDirection(float x, float y, float z){
	direction_ = DirectX::XMFLOAT3(x, y, z);
}

DirectX::XMFLOAT4 Light::GetDiffuseColor(){
	return diffuse_color_;
}

DirectX::XMFLOAT3 Light::GetDirection(){
	return direction_;
}
