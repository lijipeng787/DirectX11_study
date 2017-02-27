#include "stdafx.h"
#include "SystemBase.h"
#include "TypeDefine.h"
#include "Input.h"
#include "GraphicsBase.h"

SystemBase::SystemBase() {}

SystemBase::~SystemBase(){}

bool SystemBase::Initialize() {

	int screenWidth, screenHeight;

	screenWidth = 0;
	screenHeight = 0;

	InitializeWindows(screenWidth, screenHeight);

	{
		m_Input = new Input();
		if (!m_Input) {
			return false;
		}
		m_Input->Initialize();
	}

	return true;
}

void SystemBase::Shutdown() {

	if (m_Input) {
		delete m_Input;
		m_Input = 0;
	}

	ShutdownWindows();

	return;
}

void SystemBase::Run() {

	MSG msg;
	bool done, result;

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
				done = true;
			}
		}

	}
}

bool SystemBase::Frame() {

	if (m_Input->IsKeyDown(VK_ESCAPE)) {
		return false;
	}

	return true;
}

void SystemBase::SetWindProc(LRESULT(CALLBACK *WindProc)(HWND, UINT, WPARAM, LPARAM)){
	windd_proc_ = WindProc;
}

void SystemBase::GetScreenWidthAndHeight(unsigned int & width, unsigned int & height) const{
	width = screen_width_;
	height = screen_height_;
}

LPCWSTR SystemBase::GetApplicationName() const{
	return m_applicationName;
}

HINSTANCE SystemBase::GetApplicationInstance() const{
	return m_hinstance;
}

HWND SystemBase::GetApplicationHandle() const{
	return m_hwnd;
}

const Input & SystemBase::GetInputComponent() const{
	return *m_Input;
}

LRESULT CALLBACK SystemBase::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) {

	switch (umsg)
	{
	case WM_KEYDOWN:
	{
		m_Input->KeyDown((unsigned int)wparam);
		return 0;
	}

	case WM_KEYUP:
	{
		m_Input->KeyUp((unsigned int)wparam);
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

	m_hinstance = GetModuleHandle(NULL);

	m_applicationName = L"Engine";

	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = *windd_proc_;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = m_applicationName;
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

	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName,
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
		posX, posY, screenWidth, screenHeight, NULL, NULL, m_hinstance, NULL);

	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);

	ShowCursor(false);

	screen_height_ = screenHeight;
	screen_width_ = screenWidth;
}

void SystemBase::ShutdownWindows() {

	ShowCursor(true);

	if (FULL_SCREEN) {
		ChangeDisplaySettings(NULL, 0);
	}

	DestroyWindow(m_hwnd);
	m_hwnd = NULL;

	UnregisterClass(m_applicationName, m_hinstance);
	m_hinstance = NULL;
}
