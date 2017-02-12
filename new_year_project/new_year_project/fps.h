#pragma once

#ifndef APPLICATION_HEADER_FPS_H
#define APPLICATION_HEADER_FPS_H

#pragma comment(lib,"winmm.lib")

#include "application_common.h"

class Fps {
public:
	Fps();
	Fps(const Fps& copy);
	~Fps();

public:
	void Initialize();

	void Frame();

	int GetFps();

private:
	int fps_;
	
	int count_;

	unsigned long start_time_;
};

#endif // !APPLICATION_HEADER_FPS_H
