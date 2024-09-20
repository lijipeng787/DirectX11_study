#pragma once

#include <DirectXMath.h>

class LightClass {
public:
    LightClass() = default;

    LightClass(const LightClass&) = delete;

    LightClass& operator=(const LightClass&) = delete;

    LightClass(LightClass&&) noexcept = default;

    LightClass& operator=(LightClass&&) noexcept = default;

    ~LightClass() = default;

public:
    void SetDiffuseColor(float red, float green, float blue, float alpha) {
        diffuse_color_ = DirectX::XMFLOAT4(red, green, blue, alpha);
    }

    void SetDirection(float x, float y, float z) {
        light_direction_ = DirectX::XMFLOAT3(x, y, z);
    }

    [[nodiscard]] DirectX::XMFLOAT4 GetDiffuseColor() const { return diffuse_color_; }
    [[nodiscard]] DirectX::XMFLOAT3 GetDirection() const { return light_direction_; }

private:
    DirectX::XMFLOAT4 diffuse_color_{};
    DirectX::XMFLOAT3 light_direction_{};
};