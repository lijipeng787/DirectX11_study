#ifndef _SYSTEMCLASS_BASE_H_
#define _SYSTEMCLASS_BASE_H_

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

class GraphicsBase;
class Input;
class Timer;

class SystemBase{
public:
	SystemBase();

	SystemBase(const SystemBase& rhs) = delete;

	SystemBase& operator=(const SystemBase& rhs) = delete;
	
	virtual ~SystemBase();
public:
	virtual bool PreInitialize();

	virtual bool Initialize();

	virtual bool PostInitialize();
	
	virtual void Shutdown();
	
	virtual void Run();

	virtual bool Frame();

	virtual void SetWindProc(LRESULT(CALLBACK *WindProc)(HWND, UINT, WPARAM, LPARAM));
public:
	void GetScreenWidthAndHeight(unsigned int& width, unsigned int& height)const;

	LPCWSTR GetApplicationName();

	HINSTANCE GetApplicationInstance();

	HWND GetApplicationHandle();

	Input& GetInputComponent()const;

	Timer& GetTimerComponent()const;

	void InitializeWindows(int& output_width, int& output_height);
	
	void ShutdownWindows();

	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);
private:
	unsigned int screen_width_ = 0, screen_height_ = 0;

	LPCWSTR application_name_ = {};

	HINSTANCE hinstance_ = {};

	HWND hwnd_ = {};

	Input *input_ = nullptr;

	Timer *timer_ = nullptr;

	LRESULT(CALLBACK *windd_proc_)(HWND, UINT, WPARAM, LPARAM);
};

#endif
