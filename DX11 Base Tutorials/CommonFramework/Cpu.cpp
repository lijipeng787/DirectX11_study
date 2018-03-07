#include "stdafx.h"

#include "Cpu.h"

void Cpu::Initialize() {
	PDH_STATUS status;

	// Initialize the flag indicating whether this object can read the system cpu usage or not.
	can_read_cpu_ = true;

	// Create a query object to poll cpu usage.
	status = PdhOpenQuery(NULL, 0, &query_handle_);
	if (status != ERROR_SUCCESS) {
		can_read_cpu_ = false;
	}

	// Set query object to poll all cpus in the system.
	status = PdhAddCounter(query_handle_, TEXT("\\Processor(_Total)\\% processor time"), 0, &counter_handle_);
	if (status != ERROR_SUCCESS) {
		can_read_cpu_ = false;
	}

	last_sample_time_ = GetTickCount();

	cpu_usage_ = 0;
}

void Cpu::Shutdown() {
	if (can_read_cpu_) {
		PdhCloseQuery(query_handle_);
	}
}

void Cpu::Frame() {

	PDH_FMT_COUNTERVALUE value;

	if (can_read_cpu_) {
		if ((last_sample_time_ + 1000) < GetTickCount()) {
			last_sample_time_ = GetTickCount();

			PdhCollectQueryData(query_handle_);

			PdhGetFormattedCounterValue(counter_handle_, PDH_FMT_LONG, NULL, &value);

			cpu_usage_ = value.longValue;
		}
	}
}

int Cpu::GetCpuPercentage() {
	int usage;

	if (can_read_cpu_) {
		usage = (int)cpu_usage_;
	}
	else {
		usage = 0;
	}

	return usage;
}
