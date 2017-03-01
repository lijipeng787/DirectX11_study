#ifndef _FRUSTUMCLASS_H_
#define _FRUSTUMCLASS_H_

#include <DirectXMath.h>

class FrustumClass{
public:
	FrustumClass();

	FrustumClass(const FrustumClass& rhs) = delete;

	FrustumClass& operator=(const FrustumClass& rhs) = delete;
	
	~FrustumClass();
public:
	void ConstructFrustum(float, const DirectX::XMMATRIX&, const DirectX::XMMATRIX& );

	bool CheckPoint(float, float, float);
	
	bool CheckCube(float, float, float, float);
	
	bool CheckSphere(float, float, float, float);
	
	bool CheckRectangle(float, float, float, float, float, float);
private:
	DirectX::XMVECTOR m_planes[6];
};

#endif