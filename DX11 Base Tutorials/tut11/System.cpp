#include "System.h"

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

	m_Graphics = new GraphicsClass();
	if (!m_Graphics) {
		return false;
	}
	bool result = m_Graphics->Initialize(screenWidth, screenHeight, GetApplicationHandle());
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

bool System::Frame() {

	SystemBase::Frame();

	if (GetInputComponent().IsKeyDown(VK_ESCAPE))
	{
		return false;
	}

	bool result;

	result = m_Graphics->Frame();
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
