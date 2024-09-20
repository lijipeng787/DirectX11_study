#include "stdafx.h"

#include "Timer.h"

bool Timer::Initialize() {

	// Check to see if this system supports high performance timers.
	QueryPerformanceFrequency((LARGE_INTEGER*)&frequency_);
	if (frequency_ == 0) {
		return false;
	}

	// Find out how many times the frequency counter ticks every millisecond.
	ticks_per_ms_ = static_cast<float>(frequency_ / 1000);

	QueryPerformanceCounter((LARGE_INTEGER*)&start_time_);

	return true;
}

void Timer::Update() {

	INT64 currentTime = 0;

	QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);

	float timeDifference = static_cast<float>(currentTime - start_time_);

	frame_time_ = timeDifference / ticks_per_ms_;

	start_time_ = currentTime;
}

Timer::Timer() {
	Reset();
}

void Timer::Reset() {
	start_time_steady_clock_ = std::chrono::steady_clock::now();
	last_frame_time_steady_clock_ = start_time_steady_clock_;
}

float Timer::GetElapsedSeconds() {
	auto now = std::chrono::steady_clock::now();
	float elapsed = std::chrono::duration<float>(now - last_frame_time_steady_clock_).count();
	last_frame_time_steady_clock_ = now;
	return elapsed;
}

float Timer::GetTotalSeconds() {
	auto now = std::chrono::steady_clock::now();
	return std::chrono::duration<float>(now - start_time_steady_clock_).count();
}
