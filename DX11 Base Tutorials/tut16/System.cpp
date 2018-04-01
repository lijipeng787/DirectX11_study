#include "System.h"

#include "../CommonFramework/Timer.h"
#include "../CommonFramework/Input.h"
#include "Graphics.h"
#include "positionclass.h"

bool System::Initialize() {

	ApplicationInstance = this;
	SetWindProc(WndProc);

	SystemBase::Initialize();

	unsigned int screenWidth = 800, screenHeight = 600;

	//GetScreenWidthAndHeight(screenWidth, screenHeight);

	{
		graphics_ = new GraphicsClass();
		if (!graphics_) {
			return false;
		}
		bool result = graphics_->Initialize(screenWidth, screenHeight, GetApplicationHandle());
		if (!result) {
			return false;
		}
	}

	{
		position_ = new PositionClass();
		if (!position_) {
			return false;
		}
	}

	return true;
}

bool System::Frame() {

	SystemBase::Frame();

	bool keyDown, result;
	float rotation_y_ = 0.0f;;

	GetInputComponent().Frame();

	float time = GetTimerComponent().GetTime();
	// Set the frame time for calculating the updated position.
	position_->SetFrameTime(time);

	// Check if the left or right arrow key has been pressed, if so rotate the camera accordingly.
	keyDown = GetInputComponent().IsLeftArrowPressed();
	position_->TurnLeft(keyDown);

	keyDown = GetInputComponent().IsRightArrowPressed();
	position_->TurnRight(keyDown);

	// Get the current view point rotation_.
	position_->GetRotation(rotation_y_);

	graphics_->SetRotation(rotation_y_);

	// Do the frame processing for the graphics object.
	result = graphics_->Frame();
	if (!result) {
		return false;
	}

	// Finally render the graphics to the screen.
	result = graphics_->Render();
	if (!result) {
		return false;
	}

	return true;
}

void System::Shutdown() {

	SystemBase::Shutdown();

	if (graphics_) {
		graphics_->Shutdown();
		delete graphics_;
		graphics_ = 0;
	}
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam) {

	switch (umessage) {
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
