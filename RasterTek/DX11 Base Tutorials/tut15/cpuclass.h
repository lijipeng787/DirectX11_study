#ifndef _CPUCLASS_H_
#define _CPUCLASS_H_

#include <pdh.h>

class CpuClass{

public:
	CpuClass() {}
	
	explicit CpuClass(const CpuClass& copy) {}
	
	~CpuClass() {}

public:
	void Initialize();
	
	void Shutdown();
	
	void Frame();
	
	int GetCpuPercentage();

private:
	bool m_canReadCpu;
	
	HQUERY m_queryHandle;
	
	HCOUNTER m_counterHandle;
	
	unsigned long m_lastSampleTime;
	
	long m_cpuUsage;
};

#endif