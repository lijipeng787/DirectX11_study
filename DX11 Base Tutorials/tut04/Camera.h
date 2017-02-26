#ifndef _CAMERACLASS_H_
#define _CAMERACLASS_H_

#include <DirectXMath.h>

class Camera{
public:
	Camera();

	Camera(const Camera& rhs) = delete;

	Camera& operator=(const Camera& rhs) = delete;
	
	~Camera();
public:
	void SetPosition(float, float, float);
	
	void SetRotation(float, float, float);

	DirectX::XMFLOAT3 GetPosition();
	
	DirectX::XMFLOAT3 GetRotation();

	void Render();
	
	void GetViewMatrix(DirectX::XMMATRIX&);
private:
	float m_positionX = 0.0f, m_positionY = 0.0f, m_positionZ = 0.0f;
	
	float m_rotationX = 0.0f, m_rotationY = 0.0f, m_rotationZ = 0.0f;
	
	DirectX::XMMATRIX m_viewMatrix;
};

#endif