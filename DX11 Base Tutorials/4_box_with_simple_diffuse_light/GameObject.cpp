#include "GameObject.h"

GameObject::GameObject(std::shared_ptr<ModelClass> model)
    : model_(std::move(model)), position_(0.0f, 0.0f, 0.0f),
      rotation_(0.0f, 0.0f, 0.0f), scale_(1.0f, 1.0f, 1.0f) {}

void GameObject::SetPosition(const DirectX::XMFLOAT3 &position) {
  position_ = position;
}

void GameObject::SetRotation(const DirectX::XMFLOAT3 &rotation) {
  rotation_ = rotation;
}

void GameObject::SetScale(const DirectX::XMFLOAT3 &scale) { scale_ = scale; }

void GameObject::Update(float deltaTime) {
  // time-based updates
  // For example, automatic rotation:
  rotation_.y +=
      DirectX::XM_PI * 0.05f * deltaTime; // Rotate 9 degrees per second
}

DirectX::XMMATRIX GameObject::GetWorldMatrix() const {
  DirectX::XMMATRIX translationMatrix =
      DirectX::XMMatrixTranslation(position_.x, position_.y, position_.z);
  DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(
      rotation_.x, rotation_.y, rotation_.z);
  DirectX::XMMATRIX scaleMatrix =
      DirectX::XMMatrixScaling(scale_.x, scale_.y, scale_.z);

  // The order matters: Scale, then Rotate, then Translate
  return scaleMatrix * rotationMatrix * translationMatrix;
}

std::shared_ptr<ModelClass> GameObject::GetModel() const { return model_; }