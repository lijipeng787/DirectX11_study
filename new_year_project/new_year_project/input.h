#pragma once

#ifndef APPLICATION_HEADER_INPUT_H
#define APPLICATION_HEADER_INPUT_H

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

#define DIRECTINPUT_VERSION 0x0800

#include "application_common.h"
#include <dinput.h>

class Input {
public:
	Input();
	Input(const Input& copy);
	~Input();

public:
	bool Initialize(HINSTANCE hinstance, HWND hwnd, int screen_width, int screen_height);

	void Shutdown();

	bool Frame();

	void GetMouseLocation(int &mouse_location_x, int &mouse_location_y);

	bool IsEscapePressed();

	bool IsLeftPressed();

	bool IsRightPressed();

	bool IsUpPressed();

	bool IsDownPressed();

	bool IsAPressed();

	bool IsZPressed();

	bool IsPageUpPressed();

	bool IsPageDownPressed();

	bool IsF1Toggled();

	bool IsF2Toggled();

	bool IsF3Toggled();

private:
	bool ReadKeyboard();

	bool ReadMouse();

	void ProcessInput();

private:
	IDirectInput8 *direct_input_ = nullptr;

	IDirectInputDevice8 *keyboard_ = nullptr;

	IDirectInputDevice8 *mouse_ = nullptr;

	unsigned char keyboard_state_[256] = { 0 };

	DIMOUSESTATE mouse_state_;

	int screen_width_;

	int screen_height_;

	int mouse_location_x_;

	int mouse_location_y_;

	bool F1_released_ ;

	bool F2_released_;

	bool F3_released_;
};

#endif // !APPLICATION_HEADER_INPUT_H
