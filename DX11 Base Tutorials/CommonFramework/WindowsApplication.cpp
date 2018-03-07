#include "stdafx.h"
#include "WindowsApplication.h"

void WindowsApplication::InitializeWindows() {

	hinstance_ = GetModuleHandle(NULL);

	application_name_ = L"Engine";

	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));

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

	screen_width_ = GetSystemMetrics(SM_CXSCREEN);
	screen_height_ = GetSystemMetrics(SM_CYSCREEN);

	int posX, posY = 0;

	if (FULL_SCREEN) {

		DEVMODE dmScreenSettings;
		ZeroMemory(&dmScreenSettings, sizeof(DEVMODE));

		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = static_cast<unsigned long>(screen_width_);
		dmScreenSettings.dmPelsHeight = static_cast<unsigned long>(screen_height_);
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		posX = posY = 0;
	}
	else {
		screen_width_ = 800;
		screen_height_ = 600;

		posX = (GetSystemMetrics(SM_CXSCREEN) - screen_width_) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - screen_height_) / 2;
	}

	hwnd_ = CreateWindowEx(
		WS_EX_APPWINDOW, application_name_, application_name_,
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
		posX, posY, screen_width_, screen_height_, NULL,
		NULL, hinstance_, NULL
	);

	ShowWindow(hwnd_, SW_SHOW);
	SetForegroundWindow(hwnd_);
	SetFocus(hwnd_);

	ShowCursor(false);
}

void WindowsApplication::ShutdownWindows() {

	ShowCursor(true);

	if (FULL_SCREEN) {
		ChangeDisplaySettings(NULL, 0);
	}

	DestroyWindow(hwnd_);
	hwnd_ = NULL;

	UnregisterClass(application_name_, hinstance_);
	hinstance_ = NULL;
}
