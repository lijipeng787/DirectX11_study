#pragma once

#ifndef APPLICATION_HEADER_TIMER_H
#define APPLICATION_HEADER_TIMER_H

#include "application_common.h"

class Timer {
public:
	Timer();
	Timer(const Timer& copy);
	~Timer();

public:
	bool initialize();

	void frame();

	float get_time();

	void start_timer();

	void stop_timer();

	int get_timing();

private:
	float frequency;

	INT64 start_time;

	float frame_time;

	INT64 begin_time;

	INT64 end_time;
};


#endif // !APPLICATION_HEADER_TIMER_H
