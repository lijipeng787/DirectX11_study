#include "System.h"

#include "../CommonFramework/Timer.h"
#include "../CommonFramework/Input.h"
#include "Graphics.h"
#include "positionclass.h"

System::System() {}

System::~System() {}

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
		m_Position = new PositionClass();
		if (!m_Position)
		{
			return false;
		}
	}

	return true;
}

bool System::Frame() {

	SystemBase::Frame();

	bool keyDown, result;
	float rotationY = 0.0f;;

	GetInputComponent().Frame();

	float time = GetTimerComponent().GetTime();
	// Set the frame time for calculating the updated position.
	m_Position->SetFrameTime(time);

	// Check if the left or right arrow key has been pressed, if so rotate the camera accordingly.
	keyDown = GetInputComponent().IsLeftArrowPressed();
	m_Position->TurnLeft(keyDown);

	keyDown = GetInputComponent().IsRightArrowPressed();
	m_Position->TurnRight(keyDown);

	// Get the current view point rotation_.
	m_Position->GetRotation(rotationY);

	graphics_->SetRotation(rotationY);

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
