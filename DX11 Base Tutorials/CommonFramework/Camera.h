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
	void GetWorldPosition(DirectX::XMMATRIX& wordMatrix);

	void SetMoveStep(int step);

	void MoveTowardLeft2D(int units);

	void MoveTowardRight2D(int units);

	void MoveTowardTop2D(int units);

	void MoveTowardBottom2D(int units);

	void SetPosition(float, float, float);
	
	void SetRotation(float, float, float);

	DirectX::XMFLOAT3 GetPosition();
	
	DirectX::XMFLOAT3 GetRotation();

	void Render();
	
	void RenderReflection(float);

	DirectX::XMMATRIX GetReflectionViewMatrix();

	void GetViewMatrix(DirectX::XMMATRIX& );

	void RenderBaseViewMatrix();

	void GetBaseViewMatrix(DirectX::XMMATRIX&);
private:
	float position_x_ = 0.0f, position_y_ = 0.0f, position_z_ = 0.0f;
	
	float rotation_x_ = 0.0f, rotation_y_ = 0.0f, rotation_z_ = 0.0f;
	
	DirectX::XMMATRIX m_reflectionViewMatrix;

	DirectX::XMMATRIX m_baseViewMatrix;

	DirectX::XMMATRIX view_matrix_ = {};
};

#endif