#pragma once

#include <chrono>

class Timer {
public:
	Timer();

	Timer(const Timer& rhs) = delete;

	Timer& operator=(const Timer& rhs) = delete;

	~Timer() = default;

public:
	// old interface
	bool Initialize();

	void Update();

	const float GetTime()const { return frame_time_; }
	// new interface
    void Reset();

    float GetElapsedSeconds();

    float GetTotalSeconds();

private:
	INT64 frequency_ = 0;

	float ticks_per_ms_ = 0.0f;

	INT64 start_time_ = 0;

	float frame_time_ = 0.0f;

    std::chrono::steady_clock::time_point start_time_steady_clock_;
    std::chrono::steady_clock::time_point last_frame_time_steady_clock_;
};
