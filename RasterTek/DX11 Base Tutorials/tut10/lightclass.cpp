#include "lightclass.h"

Light::Light(){
}

Light::Light(const Light& other){
}

Light::~Light(){
}

void Light::SetAmbientColor(float red, float green, float blue, float alpha){
	ambient_color_ = DirectX::XMFLOAT4(red, green, blue, alpha);
}

void Light::SetDiffuseColor(float red, float green, float blue, float alpha){
	diffuse_color_ = DirectX::XMFLOAT4(red, green, blue, alpha);
}

void Light::SetDirection(float x, float y, float z){
	direction_ = DirectX::XMFLOAT3(x, y, z);
}

void Light::SetSpecularColor(float red, float green, float blue, float alpha){
	specular_color_ = DirectX::XMFLOAT4(red, green, blue, alpha);
}

void Light::SetSpecularPower(float power){
	specular_power_ = power;
}

DirectX::XMFLOAT4 Light::GetSpecularColor() const{
	return specular_color_;
}

float Light::GetSpecularPower() const{
	return specular_power_;
}

DirectX::XMFLOAT4 Light::GetAmbientColor()const{
	return ambient_color_;
}

DirectX::XMFLOAT4 Light::GetDiffuseColor()const {
	return diffuse_color_;
}

DirectX::XMFLOAT3 Light::GetDirection()const {
	return direction_;
}
