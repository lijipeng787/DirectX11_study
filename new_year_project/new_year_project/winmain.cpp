
#include "system.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	System *system = new System;
	if (!system)
		return false;

	if (system->Initialize()) {
		system->Run();
	}

	system->Shutdown();
	delete system;
	system = nullptr;

	return 0;
}