#ifndef _VIEWPOINTCLASS_H_
#define _VIEWPOINTCLASS_H_

#include <DirectXMath.h>

using namespace DirectX;

class ViewPointClass {
public:
	ViewPointClass();

	ViewPointClass(const ViewPointClass&);

	~ViewPointClass();
public:
	void SetPosition(float, float, float);

	void SetLookAt(float, float, float);
	
	void SetProjectionParameters(float, float, float, float);

	void GenerateViewMatrix();
	
	void GenerateProjectionMatrix();

	void GetViewMatrix(XMMATRIX&);
	
	void GetProjectionMatrix(XMMATRIX&);
private:
	XMFLOAT3 m_position, m_lookAt;

	XMMATRIX m_viewMatrix, projection_matrix_;

	float m_fieldOfView, m_aspectRatio, m_nearPlane, m_farPlane;
};

#endif