#ifndef _INPUTCLASS_H_
#define _INPUTCLASS_H_

#define DIRECTINPUT_VERSION 0x0800

#include <dinput.h>

class Input {

public:
	Input() {}

	explicit Input(const Input& copy) {}

	~Input() {}

public:
	bool Initialize(HINSTANCE, HWND, int, int);

	void Shutdown();

	bool Update();

	bool IsEscapePressed() {
		// Do a bitwise and on the keyboard state to check if the escape key is currently being pressed.
		if (keyboard_state_[DIK_ESCAPE] & 0x80) {
			return true;
		}
		return false;
	}

	void GetMouseLocation(int& mouse_x, int& mouse_y) {
		mouse_x = mouse_x_;
		mouse_y = mouse_y_;
	}

private:
	void ProcessInput();

	bool ReadKeyboard();

	bool ReadMouse();

private:
	IDirectInput8* directInput_ = nullptr;

	IDirectInputDevice8* keyboard_ = nullptr;

	IDirectInputDevice8* mouse_ = nullptr;

	unsigned char keyboard_state_[256] = { 0 };

	DIMOUSESTATE mouse_state_ = {};

	int screen_width_ = 0, screen_height_ = 0;

	int mouse_x_ = 0, mouse_y_ = 0;
};

#endif