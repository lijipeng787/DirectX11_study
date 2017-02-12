///////////////////////////////////////////////////////////////////////////////
// Filename: fpsclass.cpp
///////////////////////////////////////////////////////////////////////////////
#include "fpsclass.h"


FpsClass::FpsClass()
{
}


FpsClass::FpsClass(const FpsClass& other)
{
}


FpsClass::~FpsClass()
{
}


void FpsClass::Initialize()
{
	m_fps = 0;
	m_count = 0;
	start_time_ = timeGetTime();
	return;
}


void FpsClass::Frame()
{
	m_count++;

	if(timeGetTime() >= (start_time_ + 1000))
	{
		m_fps = m_count;
		m_count = 0;
		
		start_time_ = timeGetTime();
	}
}


int FpsClass::GetFps()
{
	return m_fps;
}