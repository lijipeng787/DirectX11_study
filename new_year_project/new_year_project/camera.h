#pragma once

#ifndef APPLICATION_HEADER_CAMERA_H
#define APPLICATION_HEADER_CAMERA_H

#include "application_common.h"

class Camera {
public:
	Camera();

	Camera(const Camera& copy);

	~Camera();

public:
	void SetPosition(float x,float y,float z);

	void SetRotation(float x, float y, float z);

	DirectX::XMFLOAT3 GetPosition();

	DirectX::XMFLOAT3 GetRotation();

	void Render();

	void RenderBaseViewMatrix();

	void GetViewMatrix(DirectX::XMMATRIX& view_matrix);

	void GetBaseViewMatrix(DirectX::XMMATRIX& base_view_matrix);

private:
	float position_x_; 
	
	float position_y_;

	float position_z_;
	
	float rotation_x_;
	
	float rotation_y_;
	
	float rotation_z_;
	
	DirectX::XMMATRIX view_matrix_;
	
	DirectX::XMMATRIX base_view_matrix_;
};

#endif // !APPLICATION_HEADER_CAMERA_H
