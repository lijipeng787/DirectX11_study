#pragma once

#ifndef APPLICATION_HEADER_SYSTEM_H

#define APPLICATION_HEADER_SYSTEM_H

#include "application_common.h"
#include "application.h"

class System
{
public:
	System();
	
	System(const System& copy);
	
	~System();

public:
	bool Initialize();
	
	void Shutdown();
	
	void Run();

	LRESULT CALLBACK MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);

private:
	bool Frame();
	
	void InitializeWindows(int& screen_width, int& screen_height);
	
	void ShutdownWindows();

private:

	LPCWSTR application_name_;

	HINSTANCE hinstance_;
	
	HWND hwnd_;
	
	Application *application_;
};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

static class System *g_system = nullptr;

#endif // !APPLICATION_HEADER_SYSTEM_H
