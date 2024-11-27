#pragma once

#include <pdh.h>

class Cpu {
public:
  Cpu() {}

  Cpu(const Cpu &rhs) = delete;

  Cpu &operator=(const Cpu &rhs) = delete;

  ~Cpu() {}

public:
  void Initialize();

  void Shutdown();

  void Frame();

  int GetCpuPercentage();

private:
  bool can_read_cpu_ = false;

  HQUERY query_handle_{};

  HCOUNTER counter_handle_{};

  unsigned long last_sample_time_ = 0;

  long cpu_usage_ = 0;
};
