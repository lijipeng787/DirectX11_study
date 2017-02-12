#pragma once

#ifndef APPLIACTION_HEADER_APPLICATION_H

#define APPLIACTION_HEADER_APPLICATION_H

#include "application_common.h"
#include "input.h"
#include "d3d11device.h"
#include "shader_manager.h"
#include "texture_manager.h"
#include "timer.h"
#include "fps.h"
#include "zone.h"

class Application {
public:
	Application();

	Application(const Application& copy) = delete;

	~Application();

public:
	bool Initialize(HINSTANCE hinstance, HWND hwnd, int screen_width, int screen_height);

	bool Frame();

private:
	std::shared_ptr<Input> input_ = nullptr;

	D3d11Device *d3d11_device_ = nullptr;

	std::shared_ptr<ShaderManager> shader_manager_ = nullptr;
	
	std::shared_ptr<TextureManager> texture_manager_ = nullptr;
	
	std::shared_ptr<Timer> timer_ = nullptr;
	
	std::shared_ptr<Fps> fps_ = nullptr;
	
	std::shared_ptr<Zone> zone_ = nullptr;
};

#endif // !APPLIACTION_HEADER_APPLICATION_H
