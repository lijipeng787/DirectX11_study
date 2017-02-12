#include "timer.h"

Timer::Timer() :frequency(0.0f), start_time(0),
frame_time(0.0f), begin_time(0), end_time(0) {
}

Timer::Timer(const Timer & copy){
}

Timer::~Timer(){
}

bool Timer::initialize(){

	INT64 frequency = 0;
	QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
	if (0 == frequency) {
		return false;
	}
	this->frequency = static_cast<float>(frequency);

	QueryPerformanceCounter((LARGE_INTEGER*)&start_time);

	return true;
}

void Timer::frame(){

	INT64 current_time = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&current_time);

	auto elapsed_ticks = current_time - start_time;

	frame_time = static_cast<float>(elapsed_ticks) / frequency;

	start_time = current_time;
}

float Timer::get_time(){
	return frame_time;
}

void Timer::start_timer(){
	QueryPerformanceCounter((LARGE_INTEGER*)&begin_time);
}

void Timer::stop_timer(){
	QueryPerformanceCounter((LARGE_INTEGER*)&end_time);
}

int Timer::get_timing()
{
	auto elapsed_ticks = static_cast<float>(end_time - begin_time);

	auto millseconds = (elapsed_ticks / static_cast<float>(this->frequency)*1000.0f);

	return static_cast<int>(millseconds);
}
