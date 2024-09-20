#pragma once

#include <DirectXMath.h>

using namespace DirectX;

class LightClass
{
public:
	LightClass();

	LightClass(const LightClass&);
	
	~LightClass();
public:
	void SetAmbientColor(float, float, float, float);
	
	void SetDiffuseColor(float, float, float, float);
	
	void SetPosition(float, float, float);
	
	void SetLookAt(float, float, float);
public:
	XMFLOAT4 GetAmbientColor();
	
	XMFLOAT4 GetDiffuseColor();
	
	XMFLOAT3 GetPosition();
public:
	void GenerateViewMatrix();
	
	void GetViewMatrix(XMMATRIX&);

	void GenerateOrthoMatrix(float, float, float);
	
	void GetOrthoMatrix(XMMATRIX&);

	void SetDirection(float, float, float);
	
	XMFLOAT3 GetDirection();
private:
	XMFLOAT4 ambient_color_;

	XMFLOAT4 diffuse_color_;
	
	XMFLOAT3 light_position_;
	
	XMFLOAT3 light_look_at_;
	
	XMMATRIX light_viewMatrix_;
	
	XMMATRIX ortho_matrix_;
	
	XMFLOAT3 direction_;
};
