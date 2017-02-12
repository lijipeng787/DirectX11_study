#include "light.h"

using namespace DirectX;

Light::Light(){
}

Light::Light(const Light & copy){
}

Light::~Light(){
}

void Light::set_ambient_color(float red, float green, float blue, float alpha){
	ambient_color_ = XMFLOAT4(red, green, blue, alpha);
}

void Light::set_diffuse_color(float red, float green, float blue, float alpha){
	diffuse_color_ = XMFLOAT4(red, green, blue, alpha);
}

void Light::set_direction(float x, float y, float z){
	direction_ = XMFLOAT3(x, y, z);
}

void Light::set_position(float x, float y, float z){
	position_ = XMFLOAT3(x, y, z);
}
