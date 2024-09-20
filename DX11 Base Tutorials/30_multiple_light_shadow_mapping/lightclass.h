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
	
	void SetLookAt(float, float, float);
public:
	XMFLOAT4 GetAmbientColor();
	
	XMFLOAT4 GetDiffuseColor();
	
	XMFLOAT3 GetPosition();

	void GenerateViewMatrix();

	void GenerateProjectionMatrix(float, float);

	void GetViewMatrix(XMMATRIX&);

	void GetProjectionMatrix(XMMATRIX&);
private:
	XMFLOAT4 ambient_color_;

	XMFLOAT4 diffuse_color_;
	
	XMFLOAT3 light_position_;
	
	XMFLOAT3 light_look_at_;
	
	XMMATRIX light_viewMatrix_;
	
	XMMATRIX projection_matrix_;
};
