#include "system.h"

#define OBJECT_INITIALIZE_AND_TEST(obj_name,class_name)	\
		(obj_name)=new (class_name);					\
		if(!obj_name){return false;}					\

#define OBJECT_SEAF_DELTE(object_name)	\
		if(object_name){				\
			object_name->shutdown();	\
			delete object_name;			\
			object_name=nullptr;		\
		}								\

System::System() {
}

System::System(const System& copy){
}

System::~System(){
}

bool System::Initialize(){

	auto screen_height = 0;
	auto screen_width=0;
	InitializeWindows(screen_width, screen_height);

	OBJECT_INITIALIZE_AND_TEST(application_, Application);

	if (!(application_->Initialize(hinstance_, hwnd_, screen_width, screen_height))) {
		return false;
	}

	return true;
}

void System::Shutdown() {
	ShutdownWindows();
}

void System::Run() {

	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	auto done = false;
	while (!done) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT) {
			done = true;
		}
		else {
			if (!(Frame())) {
				done = true;
			}
		}
	}
}


bool System::Frame(){
	if(!(application_->Frame())){
		return false;
	}

	return true;
}


LRESULT CALLBACK System::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam){
	return DefWindowProc(hwnd, umsg, wparam, lparam);
}


void System::InitializeWindows(int& screen_width, int& screen_height){
	g_system = this;

	hinstance_ = GetModuleHandle(NULL);

	application_name_ = L"Engine";

	WNDCLASSEX windows_class;
	ZeroMemory(&windows_class, sizeof(windows_class));

	windows_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	windows_class.lpfnWndProc = WndProc;
	windows_class.cbClsExtra = 0;
	windows_class.cbWndExtra = 0;
	windows_class.hInstance = hinstance_;
	windows_class.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	windows_class.hIconSm = windows_class.hIcon;
	windows_class.hCursor = LoadCursor(NULL, IDC_ARROW);
	windows_class.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	windows_class.lpszMenuName = NULL;
	windows_class.lpszClassName = application_name_;
	windows_class.cbSize = sizeof(WNDCLASSEX);

	RegisterClassEx(&windows_class);

	screen_width = GetSystemMetrics(SM_CXSCREEN);
	screen_height = GetSystemMetrics(SM_CYSCREEN);

	auto pos_x = 0; 
	auto pos_y = 0;
	DEVMODE screen_settings;
	ZeroMemory(&screen_settings, sizeof(screen_settings));

	// todo: manual select the full screen option
	if (false){
		memset(&screen_settings, 0, sizeof(screen_settings));
		screen_settings.dmSize = sizeof(screen_settings);
		screen_settings.dmPelsWidth = (unsigned long)screen_width;
		screen_settings.dmPelsHeight = (unsigned long)screen_height;
		screen_settings.dmBitsPerPel = 32;
		screen_settings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		ChangeDisplaySettings(&screen_settings, CDS_FULLSCREEN);

		pos_x = pos_y = 0;
	}
	else{
		screen_width = 800;
		screen_height = 600;

		pos_x = (GetSystemMetrics(SM_CXSCREEN) - screen_width) / 2;
		pos_y = (GetSystemMetrics(SM_CYSCREEN) - screen_height) / 2;
	}

	hwnd_ = CreateWindowExW(WS_EX_APPWINDOW, application_name_, application_name_, WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
		pos_x, pos_y, screen_width, screen_height, NULL, NULL, hinstance_, NULL);

	ShowWindow(hwnd_, SW_SHOW);
	SetForegroundWindow(hwnd_);
	SetFocus(hwnd_);

	ShowCursor(false);
}


void System::ShutdownWindows(){
	
	ShowCursor(true);
	
	// not enable full screen currently
	if (false){
		ChangeDisplaySettings(NULL, 0);
	}

	DestroyWindow(hwnd_);
	hwnd_ = NULL;

	UnregisterClassW(application_name_, hinstance_);
	hinstance_ = NULL;

	g_system = NULL;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam) {
	switch (umessage) {
	case WM_DESTROY: {
		PostQuitMessage(0);
		return 0;
	}
	case WM_CLOSE: {
		PostQuitMessage(0);
		return 0;
	}

	default: {
		return g_system->MessageHandler(hwnd, umessage, wparam, lparam);
	}
	}
}
