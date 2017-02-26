#ifndef _SYSTEMCLASS_BASE_H_
#define _SYSTEMCLASS_BASE_H_

#define WIN32_LEAN_AND_MEAN

#include <windows.h>

class GraphicsBase;
class Input;

class SystemBase{
public:
	SystemBase();

	SystemBase(const SystemBase& rhs) = delete;

	SystemBase& operator=(const SystemBase& rhs) = delete;
	
	virtual ~SystemBase();
public:
	virtual bool Initialize();
	
	virtual void Shutdown();
	
	virtual void Run();

	virtual bool Frame();

	void InitializeWindows(int&, int&);
	
	void ShutdownWindows();

	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);
private:
	LPCSTR m_applicationName = {};

	HINSTANCE m_hinstance = {};

	HWND m_hwnd = {};

	Input* m_Input = nullptr;

	GraphicsBase* m_Graphics = nullptr;
};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

static SystemBase* ApplicationHandle = 0;

#endif