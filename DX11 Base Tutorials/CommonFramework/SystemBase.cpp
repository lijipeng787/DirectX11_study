#include "stdafx.h"
#include "SystemBase.h"
#include "TypeDefine.h"
#include "Input.h"
#include "GraphicsBase.h"
#include "Timer.h"

SystemBase::SystemBase() {}

SystemBase::~SystemBase(){}

bool SystemBase::PreInitialize(){
	return true;
}

bool SystemBase::Initialize() {

	int screenWidth = 0, screenHeight = 0;

	InitializeWindows(screenWidth, screenHeight);

	{
		input_ = new Input();
		if (!input_) {
			return false;
		}
		input_->Initialize(hinstance_, hwnd_, screenWidth, screenHeight);
	}

	{
		timer_ = new Timer();
		if (!timer_) {
			return false;
		}
		bool result = timer_->Initialize();
		if (!result)
		{
			MessageBox(hwnd_, L"Could not initialize the Timer object.", L"Error", MB_OK);
			return false;
		}
	}

	return true;
}

bool SystemBase::PostInitialize(){
	return false;
}

void SystemBase::Shutdown() {

	if (input_) {
		delete input_;
		input_ = nullptr;
	}

	if (timer_) {
		delete timer_;
		timer_ = nullptr;
	}

	ShutdownWindows();
}

void SystemBase::Run() {

	bool done, result;
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	done = false;
	while (!done) {

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT) {
			done = true;
		}
		else {
			result = Frame();
			if (!result) {
				MessageBox(hwnd_, L"Frame Processing Failed", L"Error", MB_OK);
				done = true;
			}
		}

		if (GetInputComponent().IsEscapePressed() == true) {
			done = true;
		}

	}
}

bool SystemBase::Frame() {

	if (input_->IsKeyDown(VK_ESCAPE)) {
		return true;
	}

	timer_->Update();

	return true;
}

void SystemBase::SetWindProc(LRESULT(CALLBACK *WindProc)(HWND, UINT, WPARAM, LPARAM)){
	windd_proc_ = WindProc;
}

void SystemBase::GetScreenWidthAndHeight(unsigned int & width, unsigned int & height) const{
	width = screen_width_;
	height = screen_height_;
}

LPCWSTR SystemBase::GetApplicationName() {
	return application_name_;
}

HINSTANCE SystemBase::GetApplicationInstance() {
	return hinstance_;
}

HWND SystemBase::GetApplicationHandle() {
	return hwnd_;
}

Input& SystemBase::GetInputComponent()const {
	return *input_;
}

Timer& SystemBase::GetTimerComponent()const {
	return *timer_;
}

LRESULT CALLBACK SystemBase::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) {

	switch (umsg)
	{
	case WM_KEYDOWN:
	{
		input_->KeyDown((unsigned int)wparam);
		return 0;
	}

	case WM_KEYUP:
	{
		input_->KeyUp((unsigned int)wparam);
		return 0;
	}

	default:
	{
		return DefWindowProc(hwnd, umsg, wparam, lparam);
	}
	}
}

void SystemBase::InitializeWindows(int& screenWidth, int& screenHeight) {

	WNDCLASSEX wc;
	DEVMODE dmScreenSettings;
	int posX, posY;

	hinstance_ = GetModuleHandle(NULL);

	application_name_ = L"Engine";

	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = *windd_proc_;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hinstance_;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = application_name_;
	wc.cbSize = sizeof(WNDCLASSEX);

	RegisterClassEx(&wc);

	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	if (FULL_SCREEN) {

		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = (unsigned long)screenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		posX = posY = 0;
	}
	else {
		screenWidth = 800;
		screenHeight = 600;

		posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
	}

	hwnd_ = CreateWindowEx(WS_EX_APPWINDOW, application_name_, application_name_,
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
		posX, posY, screenWidth, screenHeight, NULL, NULL, hinstance_, NULL);

	ShowWindow(hwnd_, SW_SHOW);
	SetForegroundWindow(hwnd_);
	SetFocus(hwnd_);

	ShowCursor(false);

	screen_height_ = screenHeight;
	screen_width_ = screenWidth;
}

void SystemBase::ShutdownWindows() {

	ShowCursor(true);

	if (FULL_SCREEN) {
		ChangeDisplaySettings(NULL, 0);
	}

	DestroyWindow(hwnd_);
	hwnd_ = NULL;

	UnregisterClass(application_name_, hinstance_);
	hinstance_ = NULL;
}
