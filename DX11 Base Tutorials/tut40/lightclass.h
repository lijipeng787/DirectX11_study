









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
	void SetPosition(float, float, float);
	void SetLookAt(float, float, float);

	XMFLOAT4 GetAmbientColor();
	XMFLOAT4 GetDiffuseColor();
	XMFLOAT3 GetPosition();

	void GenerateViewMatrix();
	void GenerateProjectionMatrix(float, float);

	void GetViewMatrix( XMMATRIX& );
	void GetProjectionMatrix( XMMATRIX& );

private:
	XMFLOAT4 ambient_color_;
	XMFLOAT4 diffuse_color_;
	XMFLOAT3 m_position;
	XMFLOAT3 m_lookAt;
	XMMATRIX m_viewMatrix;
	XMMATRIX projection_matrix_;
};

#endif