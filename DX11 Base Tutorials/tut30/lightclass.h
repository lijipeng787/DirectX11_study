
// Filename: lightclass.h

#ifndef _LIGHTCLASS_H_
#define _LIGHTCLASS_H_





#include <DirectXMath.h>
using namespace DirectX;



// Class name: LightClass

class LightClass
{
public:
	LightClass();
	LightClass(const LightClass&);
	~LightClass();

	void SetDiffuseColor(float, float, float, float);
	void SetPosition(float, float, float);

	XMFLOAT4 GetDiffuseColor();
	XMFLOAT4 GetPosition();

private:
	XMFLOAT4 m_diffuseColor;
	XMFLOAT4 m_position;
};

#endif