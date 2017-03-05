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
		if (!m_Position){
			return false;
		}
		// Set the initial position of the viewer to the same as the initial camera position.
		m_Position->SetPosition(0.0f, 2.0f, -10.0f);
	}

	return true;
}

bool System::Frame() {

	SystemBase::Frame();

	bool result;

	result = GetInputComponent().Frame();
	if (result == false) {
		return false;
	}

	if (GetInputComponent().IsEscapePressed() == true){
		return true;
	}

	result = HandleInput(GetTimerComponent().GetTime());
	if (!result){
		return false;
	}

	float posX, posY, posZ;
	float rotX, rotY, rotZ;
	m_Position->GetPosition(posX, posY, posZ);
	m_Position->GetRotation(rotX, rotY, rotZ);

	m_Graphics->SetPosition(posX, posY, posZ);
	m_Graphics->SetRotation(rotX, rotY, rotZ);

	result = m_Graphics->Frame();
	if (!result) {
		return false;
	}

	auto delta_time = GetTimerComponent().GetTime();

	result = m_Graphics->Render();
	if (!result) {
		return false;
	}

	return true;
}

bool System::HandleInput(float frameTime) {

	bool keyDown;

	m_Position->SetFrameTime(frameTime);

	keyDown = GetInputComponent().IsLeftPressed();
	m_Position->TurnLeft(keyDown);

	keyDown = GetInputComponent().IsRightPressed();
	m_Position->TurnRight(keyDown);

	keyDown = GetInputComponent().IsUpPressed();
	m_Position->MoveForward(keyDown);

	keyDown = GetInputComponent().IsDownPressed();
	m_Position->MoveBackward(keyDown);

	keyDown = GetInputComponent().IsAPressed();
	m_Position->MoveUpward(keyDown);

	keyDown = GetInputComponent().IsZPressed();
	m_Position->MoveDownward(keyDown);

	keyDown = GetInputComponent().IsPgUpPressed();
	m_Position->LookUpward(keyDown);

	keyDown = GetInputComponent().IsPgDownPressed();
	m_Position->LookDownward(keyDown);

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