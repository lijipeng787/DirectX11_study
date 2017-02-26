#ifndef _SYSTEMCLASS_H_
#define _SYSTEMCLASS_H_

#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#include "inputclass.h"
#include "graphicsclass.h"

class System{
public:
	System();

	System(const System& rhs) = delete;

	System& operator=(const System& rhs) = delete;
	
	~System();
public:
	bool Initialize();
	
	void Shutdown();
	
	void Run();

	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);
private:
	bool Frame();
	
	void InitializeWindows(int&, int&);
	
	void ShutdownWindows();
private:
	LPCWSTR m_applicationName = {};

	HINSTANCE m_hinstance = {};

	HWND m_hwnd = {};

	Input* m_Input = nullptr;

	Graphics* m_Graphics = nullptr;
};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

static System* ApplicationHandle = 0;

#endif