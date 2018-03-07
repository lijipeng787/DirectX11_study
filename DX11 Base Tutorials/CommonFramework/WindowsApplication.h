#pragma once

#include "SystemBase.h"

class WindowsApplication :implements SystemBase {
public:
	WindowsApplication() {}

	virtual ~WindowsApplication() {}
public:
	inline unsigned int GetScreenWidth()const { return screen_width_; }

	inline unsigned int GetScreenHeight()const { return screen_height_; }

	inline LPCWSTR GetApplicationName()const { return application_name_; }

	inline HINSTANCE GetApplicationInstance()const { return hinstance_; }

	inline HWND GetApplicationHandle()const { return hwnd_; }
public:

	void InitializeWindows();

	void ShutdownWindows();
public:
	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);
private:
	unsigned int screen_width_ = 0, screen_height_ = 0;

	LPCWSTR application_name_ = {};

	HINSTANCE hinstance_ = {};

	HWND hwnd_ = {};

	LRESULT(CALLBACK *windd_proc_)(HWND, UINT, WPARAM, LPARAM);
};