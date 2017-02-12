#pragma once

#ifndef GRAPHICS_HEADER_ZONE_H

#define GRAPHICS_HEADER_ZONE_H

#include "d3d11device.h"
#include "input.h"
#include "shader_manager.h"
#include "texture_manager.h"
#include "timer.h"
#include "user_interface.h"
#include "camera.h"
#include "light.h"
#include "position.h"
#include "skybox.h"
#include "terrain.h"

class Zone {
public:
	Zone();

	Zone(const Zone& copy);

	~Zone();

public:
	bool Initialize(HWND hwnd, int screen_width, int screen_height, float screen_depth);

	void Shutdown();

	bool Frame(
		Input *input_object,
		ShaderManager *shader_manager_object,
		TextureManager *texture_manager_object,
		float frame_time, int fps
		);

private:
	void HandleMovementInput(Input *input_object,float frame_tiem);

	bool Render(
		ShaderManager *shader_manager_object, 
		TextureManager *texture_manager_object
		);

private:
	UserInterface *user_interface_;

	Camera *camera_;

	Light *light_;

	Position *position_;

	SkyBox *skybox_;

	Terrain *terrain_;

	bool display_ui_ = true;

	bool wire_frame_ = false;
	
	bool cell_lines_ = true;
};

#endif // !GRAPHICS_HEADER_ZONE_H
