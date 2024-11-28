

#include <DirectXMath.h>
using namespace DirectX;

class LightClass {
public:
  LightClass();
  LightClass(const LightClass &);
  ~LightClass();

  void SetDirection(float, float, float);
  void SetPosition(float, float, float);
  void SetLookAt(float, float, float);

  XMFLOAT3 GetDirection();
  XMFLOAT3 GetPosition();

  void GenerateViewMatrix();
  void GenerateProjectionMatrix(float, float);
  void GenerateOrthoMatrix(float, float, float, float);

  void GetViewMatrix(XMMATRIX &);
  void GetProjectionMatrix(XMMATRIX &);
  void GetOrthoMatrix(XMMATRIX &);

private:
  XMFLOAT3 direction_, light_position_, light_look_at_;
  XMMATRIX light_viewMatrix_, projection_matrix_, ortho_matrix_;
};

#endif