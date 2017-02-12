#ifndef _TIMERCLASS_H_
#define _TIMERCLASS_H_

#include <windows.h>

class TimerClass {

public:
	TimerClass() {}

	explicit TimerClass(const TimerClass& copy) {}

	~TimerClass() {}

public:
	bool Initialize();

	void Update();

	const float GetTime()const { return frame_time_; }

private:
	INT64 frequency_ = 0;

	float ticks_per_ms_ = 0.0f;

	INT64 start_time_ = 0;

	float frame_time_ = 0.0f;
};

#endif