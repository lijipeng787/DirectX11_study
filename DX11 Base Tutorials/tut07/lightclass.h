


#include <DirectXMath.h>

class LightClass {
public:
	LightClass();

	LightClass(const LightClass& rhs) = delete;

	LightClass& operator=(const LightClass& rhs) = delete;

	~LightClass();
public:
	void SetDiffuseColor(float red, float green, float blue, float alpha);

	void SetDirection(float x, float y, float z);

	DirectX::XMFLOAT4 GetDiffuseColor();

	DirectX::XMFLOAT3 GetDirection();
private:
	DirectX::XMFLOAT4 diffuse_color_ = {};

	DirectX::XMFLOAT3 light_direction_ = {};
};

#endif