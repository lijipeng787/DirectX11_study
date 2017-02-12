#pragma once

#ifndef GRAPHICS_HEADER_LIGHT_H

#define GRAPHICS_HEADER_LIGHT_H

#include "common.h"

class Light {
public:
	Light();

	Light(const Light& copy);

	~Light();

public:
	void set_ambient_color(float red, float green, float blue, float alpha);

	const DirectX::XMFLOAT4& get_ambinet_color() { return this->ambient_color_; }

	void set_diffuse_color(float red, float green, float blue, float alpha);

	const DirectX::XMFLOAT4& get_diffuse_color() { return this->diffuse_color_; }

	void set_direction(float x, float y, float z);

	const DirectX::XMFLOAT3& get_direction() { return this->direction_; }

	void set_position(float x, float y, float z);

	const DirectX::XMFLOAT3& get_position() { return this->position_; }

private:
	DirectX::XMFLOAT4 ambient_color_;

	DirectX::XMFLOAT4 diffuse_color_;

	DirectX::XMFLOAT3 direction_;

	DirectX::XMFLOAT3 position_;
};

#endif // !GRAPHICS_HEADER_LIGHT_H
