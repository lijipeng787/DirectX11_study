#include "fps.h"

Fps::Fps() :fps_(0), count_(0), start_time_() {
}

Fps::Fps(const Fps & copy){
}

Fps::~Fps(){
}

void Fps::Initialize(){
	start_time_ = timeGetTime();
}

void Fps::Frame(){
	
	++count_;
	if ((start_time_ + 1000) <= timeGetTime()) {
		fps_ = count_;
		count_ = 0;
		start_time_ = timeGetTime();
	}
}

int Fps::GetFps()
{
	return fps_;
}
