#include "System.h"
#include "../CommonFramework/Input.h"
#include "GraphicsModule.h"

bool System::Initialize() {

	ApplicationInstance = this;

	SetWindProc(WndProc);

	SystemBase::Initialize();

	unsigned int screenWidth = 800, screenHeight = 600;

	//GetScreenWidthAndHeight(screenWidth, screenHeight);

	Graphics_ = new GraphicsModule();
	if (!Graphics_) {
		return false;
	}

	bool result = Graphics_->Initialize(screenWidth, screenHeight, GetApplicationHandle());
	if (!result) {
		return false;
	}

	return true;
}

void System::Shutdown() {

	SystemBase::Shutdown();

	if (Graphics_) {
		Graphics_->Shutdown();
		delete Graphics_;
		Graphics_ = nullptr;
	}
}

bool System::Frame() {

	SystemBase::Frame();

	if (GetInputComponent().IsKeyDown(VK_ESCAPE)) {
		return false;
	}

	auto result = Graphics_->Frame();
	if (!result) {
		return false;
	}

	return true;
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
		return ApplicationInstance->MessageHandler(hwnd, umessage, wparam, lparam);
	}
	}
}
