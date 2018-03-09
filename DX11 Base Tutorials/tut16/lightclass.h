









#include <DirectXMath.h>
using namespace DirectX;





class LightClass
{
public:
	LightClass();
	LightClass(const LightClass&);
	~LightClass();

	void SetAmbientColor(float, float, float, float);
	void SetDiffuseColor(float, float, float, float);
	void SetSpecularColor(float, float, float, float);
	void SetSpecularPower(float);
	void SetDirection(float, float, float);

	XMFLOAT4 GetAmbientColor();
	XMFLOAT4 GetDiffuseColor();
	XMFLOAT4 GetSpecularColor();
	float GetSpecularPower();
	XMFLOAT3 GetDirection();

private:
	XMFLOAT4 ambient_color_;
	XMFLOAT4 diffuse_color_;
	XMFLOAT4 specular_color_;
	float specular_power_;
	XMFLOAT3 direction_;
};

#endif