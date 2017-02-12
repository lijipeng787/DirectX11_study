#pragma once

#ifndef GRAPHICS_HEADER_USER_INTERFACE_H

#define GRAPHICS_HEADER_USER_INTERFACE_H

#include "text.h"
#include "rendeable.h"

class UserInterface :public Renderable {
public:
	UserInterface();

	UserInterface(const UserInterface& copy);

	~UserInterface();

public:
	bool Initialize(int screen_width, int screen_height);

	void Shutdown();

	bool Frame(
		int fps,
		float pos_x, float pos_y, float pos_z,
		float rot_x, float rot_y, float rot_z
		);

	bool Render(
		ShaderManager *shader_manager_object,
		DirectX::XMMATRIX world, 
		DirectX::XMMATRIX view, 
		DirectX::XMMATRIX orhthgonality
		);

private:
	bool UpdateFpsString(int fps_value);

	bool UpdatePosisitonString(
		float pos_x, float pos_y, float pos_z,
		float rot_x, float rot_y, float rot_z
		);

private:
	Font *font_object_;

	Text *fps_string_, *video_string_, *position_string_;

	int previous_fps_;

	int previous_position_[6] = { -1,-1,-1,-1,-1,-1 };
};

#endif // !GRAPHICS_HEADER_USER_INTERFACE_H
