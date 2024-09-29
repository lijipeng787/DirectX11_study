#pragma once

#include <DirectXMath.h>

class LightClass {
public:
  LightClass();

  LightClass(const LightClass &);

  ~LightClass() = default;

public:
  void SetAmbientColor(float, float, float, float);

  void SetDiffuseColor(float, float, float, float);

  void SetPosition(float, float, float);

  void SetLookAt(float, float, float);

  DirectX::XMFLOAT4 GetAmbientColor();

  DirectX::XMFLOAT4 GetDiffuseColor();

  DirectX::XMFLOAT3 GetPosition();

  void GenerateViewMatrix();

  void GenerateProjectionMatrix(float, float);

  void GetViewMatrix(DirectX::XMMATRIX &);

  void GetProjectionMatrix(DirectX::XMMATRIX &);

private:
  DirectX::XMFLOAT4 ambient_color_;
  DirectX::XMFLOAT4 diffuse_color_;
  DirectX::XMFLOAT3 light_position_;
  DirectX::XMFLOAT3 light_look_at_;
  DirectX::XMMATRIX light_viewMatrix_;
  DirectX::XMMATRIX projection_matrix_;
};