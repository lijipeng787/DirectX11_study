
// Filename: timerclass.h

#ifndef _TIMERCLASS_H_
#define _TIMERCLASS_H_





#include <windows.h>



// Class name: TimerClass

class TimerClass
{
public:
	TimerClass();
	TimerClass(const TimerClass&);
	~TimerClass();

	bool Initialize();
	void Frame();

	float GetTime();

private:
	INT64 m_frequency;
	float m_ticksPerMs;
	INT64 m_startTime;
	float frame_time_;
};

#endif