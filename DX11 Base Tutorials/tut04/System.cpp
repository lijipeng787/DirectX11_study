#include "System.h"
#include "Graphics.h"

System::System() {}

System::~System(){}

bool System::Initialize() {

	ApplicationInstance = this;

	SetWindProc(WndProc);

	SystemBase::Initialize();

	unsigned int screenWidth = 800, screenHeight = 600;

	//GetScreenWidthAndHeight(screenWidth, screenHeight);
	
	graphics_ = new GraphicsClass();
	if (!graphics_) {
		return false;
	}
	bool result = graphics_->Initialize(screenWidth, screenHeight, GetApplicationHandle());
	if (!result) {
		return false;
	}

	return true;
}

void System::Shutdown() {

	if (graphics_) {
		graphics_->Shutdown();
		delete graphics_;
		graphics_ = 0;
	}
}

bool System::Frame() {

	SystemBase::Frame();

	bool result;

	result = graphics_->Frame();
	if (!result) {
		return false;
	}

	return true;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam) {

	switch (umessage)
	{
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}

	case WM_CLOSE:
	{
		PostQuitMessage(0);
		return 0;
	}

	default:
	{
		return ApplicationInstance->MessageHandler(hwnd, umessage, wparam, lparam);
	}
	}
}
