#ifndef _SYSTEMCLASS_H_
#define _SYSTEMCLASS_H_

#include "../Common/SystemBase.h"

class System:public SystemBase{
public:
	System();
	
	System(const System& rhs) = delete;

	System& operator=(const System& rhs) = delete;
	
	virtual ~System();
public:
	virtual bool Initialize()override;

	virtual void Shutdown()override;
	
	virtual void Run()override;

	virtual bool Frame()override;
};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

static System* ApplicationHandle = 0;

#endif