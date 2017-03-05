#include "stdafx.h"
#include "Input.h"

Input::Input(){}

Input::~Input(){}

bool Input::Initialize(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight) {

	HRESULT result;

	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	m_mouseX = 0;
	m_mouseY = 0;

	result = DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_directInput, NULL);
	if (FAILED(result)) {
		return false;
	}

	result = m_directInput->CreateDevice(GUID_SysKeyboard, &m_keyboard, NULL);
	if (FAILED(result)) {
		return false;
	}

	result = m_keyboard->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(result)) {
		return false;
	}

	result = m_keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
	if (FAILED(result)) {
		return false;
	}

	result = m_keyboard->Acquire();
	if (FAILED(result)) {
		return false;
	}

	result = m_directInput->CreateDevice(GUID_SysMouse, &m_mouse, NULL);
	if (FAILED(result)) {
		return false;
	}

	result = m_mouse->SetDataFormat(&c_dfDIMouse);
	if (FAILED(result)) {
		return false;
	}

	result = m_mouse->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(result)) {
		return false;
	}

	result = m_mouse->Acquire();
	if (FAILED(result)) {
		return false;
	}

	return true;
}

void Input::Shutdown() {

	if (m_mouse) {
		m_mouse->Unacquire();
		m_mouse->Release();
		m_mouse = nullptr;
	}

	if (m_keyboard) {
		m_keyboard->Unacquire();
		m_keyboard->Release();
		m_keyboard = nullptr;
	}

	if (m_directInput) {
		m_directInput->Release();
		m_directInput = nullptr;
	}
}

bool Input::Frame() {

	bool result;

	result = ReadKeyboard();
	if (!result) {
		return false;
	}

	result = ReadMouse();
	if (!result) {
		return false;
	}

	ProcessInput();

	return true;
}

bool Input::ReadKeyboard() {

	HRESULT result;

	result = m_keyboard->GetDeviceState(sizeof(m_keyboardState), (LPVOID)&m_keyboardState);
	if (FAILED(result)) {
		
		if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED)) {
			m_keyboard->Acquire();
		}
		else {
			return false;
		}
	}

	return true;
}

bool Input::ReadMouse() {

	HRESULT result;

	result = m_mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&m_mouseState);
	if (FAILED(result)) {
		
		if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED)) {
			m_mouse->Acquire();
		}
		else {
			return false;
		}
	}

	return true;
}

void Input::ProcessInput() {

	m_mouseX += m_mouseState.lX;
	m_mouseY += m_mouseState.lY;

	if (m_mouseX < 0) { m_mouseX = 0; }
	if (m_mouseY < 0) { m_mouseY = 0; }

	if (m_mouseX > m_screenWidth) { m_mouseX = m_screenWidth; }
	if (m_mouseY > m_screenHeight) { m_mouseY = m_screenHeight; }
}

bool Input::IsEscapePressed() {

	if (m_keyboardState[DIK_ESCAPE] & 0x80) {
		return true;
	}

	return false;
}

void Input::GetMouseLocation(int& mouseX, int& mouseY) {

	mouseX = m_mouseX;
	mouseY = m_mouseY;
}

void Input::Initialize() {

	int i;
	for (i = 0; i < 256; i++) {
		keys_[i] = false;
	}
}

void Input::KeyDown(unsigned int input) {
	keys_[input] = true;
}

void Input::KeyUp(unsigned int input) {
	keys_[input] = false;
}

bool Input::IsKeyDown(unsigned int key)const {
	return keys_[key];
}

bool Input::IsLeftArrowPressed() {

	if (m_keyboardState[DIK_LEFT] & 0x80) {
		return true;
	}

	return false;
}

bool Input::IsRightArrowPressed() {

	if (m_keyboardState[DIK_RIGHT] & 0x80) {
		return true;
	}

	return false;
}

bool Input::IsLeftPressed() {

	if (m_keyboardState[DIK_LEFT] & 0x80) {
		return true;
	}

	return false;
}

bool Input::IsRightPressed() {

	if (m_keyboardState[DIK_RIGHT] & 0x80) {
		return true;
	}

	return false;
}

bool Input::IsUpPressed() {

	if (m_keyboardState[DIK_UP] & 0x80) {
		return true;
	}

	return false;
}

bool Input::IsDownPressed() {

	if (m_keyboardState[DIK_DOWN] & 0x80) {
		return true;
	}

	return false;
}

bool Input::IsAPressed() {

	if (m_keyboardState[DIK_A] & 0x80) {
		return true;
	}

	return false;
}

bool Input::IsZPressed() {

	if (m_keyboardState[DIK_Z] & 0x80) {
		return true;
	}

	return false;
}

bool Input::IsPgUpPressed() {

	if (m_keyboardState[DIK_PGUP] & 0x80) {
		return true;
	}

	return false;
}

bool Input::IsPgDownPressed() {

	if (m_keyboardState[DIK_PGDN] & 0x80) {
		return true;
	}

	return false;
}
