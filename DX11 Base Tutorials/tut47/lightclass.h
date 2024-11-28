

#include <DirectXMath.h>
using namespace DirectX;

class LightClass {
public:
  LightClass();
  LightClass(const LightClass &);
  ~LightClass();

  void SetDiffuseColor(float, float, float, float);
  void SetDirection(float, float, float);

  XMFLOAT4 GetDiffuseColor();
  XMFLOAT3 GetDirection();

private:
  XMFLOAT4 diffuse_color_;
  XMFLOAT3 direction_;
};

#endif