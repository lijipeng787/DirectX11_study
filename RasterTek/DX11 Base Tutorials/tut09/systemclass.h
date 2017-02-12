#ifndef HEADER_SYSTEMCLASS_H_
#define HEADER_SYSTEMCLASS_H_

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include "inputclass.h"
#include "graphicsclass.h"

class System{
public:
	System();

	explicit System(const System& copy)=delete;
	
	~System();

public:
	bool Initialize();
	
	void Shutdown();
	
	void Run();

	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

private:
	bool Frame();
	
	void InitializeWindows(int screen_width, int screen_height);
	
	void ShutdownWindows();

private:
	LPCWSTR application_name_;

	HINSTANCE hinstance_;
	
	HWND hwnd_;

	Input* input_;

	Graphics* graphics_;
};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

static System* g_system_instance = 0;

#endif //!HEADER_SYSTEMCLASS_H