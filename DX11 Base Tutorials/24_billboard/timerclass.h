#pragma once

#include <windows.h>

class TimerClass {
public:
  TimerClass() {}

  TimerClass(const TimerClass &rhs) = delete;

  ~TimerClass() {}

public:
  bool Initialize();

  void Frame();

  float GetTime();

private:
  INT64 frequency_;

  float ticks_per_ms_, frame_time_;

  INT64 start_time_;
};
