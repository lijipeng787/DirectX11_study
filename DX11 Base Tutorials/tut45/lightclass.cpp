


#include "lightclass.h"


LightClass::LightClass()
{
}


LightClass::LightClass(const LightClass& other)
{
}


LightClass::~LightClass()
{
}


void LightClass::SetAmbientColor(float red, float green, float blue, float alpha)
{
	ambient_color_ = XMFLOAT4(red, green, blue, alpha);
	
}


void LightClass::SetDiffuseColor(float red, float green, float blue, float alpha)
{
	diffuse_color_ = XMFLOAT4(red, green, blue, alpha);
	
}


void LightClass::SetDirection(float x, float y, float z)
{
	direction_ = XMFLOAT3(x, y, z);
	
}


void LightClass::SetSpecularColor(float red, float green, float blue, float alpha)
{
	specular_color_ = XMFLOAT4(red, green, blue, alpha);
	
}


void LightClass::SetSpecularPower(float power)
{
	specular_power_ = power;
	
}


XMFLOAT4 LightClass::GetAmbientColor()
{
	return ambient_color_;
}


XMFLOAT4 LightClass::GetDiffuseColor()
{
	return diffuse_color_;
}


XMFLOAT3 LightClass::GetDirection()
{
	return direction_;
}


XMFLOAT4 LightClass::GetSpecularColor()
{
	return specular_color_;
}


float LightClass::GetSpecularPower()
{
	return specular_power_;
}