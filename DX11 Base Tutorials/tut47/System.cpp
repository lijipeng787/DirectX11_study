#include "System.h"

#include "../CommonFramework/Timer.h"
#include "../CommonFramework/Input.h"
#include "Graphics.h"

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

	return true;
}

bool System::Frame() {

	SystemBase::Frame();

	bool result;

	GetInputComponent().Frame();

	HandleInput();

	result = m_Graphics->Frame();
	if (!result) {
		return false;
	}

	result = m_Graphics->Render();
	if (!result) {
		return false;
	}

	return true;
}

bool System::HandleInput() {

	int mouseX, mouseY;

	GetInputComponent().GetMouseLocation(mouseX, mouseY);
	m_Graphics->SetMousePosition(mouseX, mouseY);

	// Check if the user pressed escape and wants to exit the application.
	if (GetInputComponent().IsEscapePressed() == true) {
		return true;
	}

	// Check if the left mouse button has been pressed.
	if (GetInputComponent().IsLeftMouseButtonDown() == true) {
		// If they have clicked on the screen with the mouse then perform an intersection test.
		if (m_beginCheck == false) {
			m_beginCheck = true;
			m_Graphics->TestIntersection(mouseX, mouseY);
		}
	}

	// Check if the left mouse button has been released.
	if (GetInputComponent().IsLeftMouseButtonDown() == false) {
		m_beginCheck = false;
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