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
		m_Graphics = new GraphicsClass();
		if (!m_Graphics) {
			return false;
		}
		bool result = m_Graphics->Initialize(screenWidth, screenHeight, GetApplicationHandle());
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
		m_Position->SetPosition(0.0f, 3.0f, -10.0f);
	}

	return true;
}

bool System::Frame() {

	SystemBase::Frame();

	bool keyDown, result;

	GetInputComponent().Frame();

	float time = GetTimerComponent().GetTime();
	
	m_Position->SetFrameTime(time);

	keyDown = GetInputComponent().IsLeftArrowPressed();
	m_Position->MoveLeft(keyDown);

	keyDown = GetInputComponent().IsRightArrowPressed();
	m_Position->MoveRight(keyDown);

	float x, y, z;
	m_Position->GetPosition(x, y, z);

	m_Graphics->SetPosition(x, y, z);

	// Do the frame processing for the graphics object.
	result = m_Graphics->Frame();
	if (!result) {
		return false;
	}

	// Finally render the graphics to the screen.
	result = m_Graphics->Render();
	if (!result) {
		return false;
	}

	return true;
}

void System::Shutdown() {

	SystemBase::Shutdown();

	if (m_Graphics) {
		m_Graphics->Shutdown();
		delete m_Graphics;
		m_Graphics = 0;
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
