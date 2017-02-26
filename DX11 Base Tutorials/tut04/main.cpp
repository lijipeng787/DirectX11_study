#include "systemclass.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow) {

	System *System = new System();
	if (!System) {
		return 0;
	}
	bool result = System->Initialize();
	if (result) {
		System->Run();
	}

	System->Shutdown();
	delete System;
	System = 0;

	return 0;
}