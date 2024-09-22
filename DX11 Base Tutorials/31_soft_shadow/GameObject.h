#pragma once

#include "modelclass.h"
#include <DirectXMath.h>
#include <memory>

class GameObject {
public:
  GameObject(std::shared_ptr<ModelClass> model);

public:
  void SetPosition(const DirectX::XMFLOAT3 &position);

  void SetRotation(const DirectX::XMFLOAT3 &rotation);

  void SetScale(const DirectX::XMFLOAT3 &scale);

  void Update(float deltaTime);

  DirectX::XMMATRIX GetWorldMatrix() const;

  std::shared_ptr<ModelClass> GetModel() const;

private:
  std::shared_ptr<ModelClass> model_;
  DirectX::XMFLOAT3 position_;
  DirectX::XMFLOAT3 rotation_;
  DirectX::XMFLOAT3 scale_;
};