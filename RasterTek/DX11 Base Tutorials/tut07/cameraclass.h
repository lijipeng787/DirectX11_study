#ifndef _CAMERACLASS_H_
#define _CAMERACLASS_H_

#include <DirectXMath.h>
using namespace DirectX;

class Camera
{
public:
	Camera();

	explicit Camera(const Camera& copy);
	
	~Camera();

public:
	void SetPosition(float, float, float);
	
	void SetRotation(float, float, float);

	XMFLOAT3 GetPosition();
	
	XMFLOAT3 GetRotation();

	void Update();
	
	void GetViewMatrix(XMMATRIX& view);

private:
	float position_X_, position_y_, position_Z_;

	float rotation_X_, rotation_Y_, rotation_Z_;
	
	XMMATRIX view_matrix_;
};

#endif