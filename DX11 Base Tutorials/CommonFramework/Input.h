#ifndef _INPUTCLASS_H_
#define _INPUTCLASS_H_

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

class Input{
public:
	Input();
	
	Input(const Input& rhs) = delete;

	Input& operator=(const Input& rhs) = delete;
	
	~Input();
public:
	bool Initialize(HINSTANCE, HWND, int, int);

	void Shutdown();

	bool Frame();

	void GetMouseLocation(int&, int&);

	bool IsEscapePressed();

	bool IsLeftArrowPressed();
	
	bool IsRightArrowPressed();

	bool IsLeftPressed();
	
	bool IsRightPressed();
	
	bool IsUpPressed();
	
	bool IsDownPressed();
	
	bool IsAPressed();
	
	bool IsZPressed();
	
	bool IsPgUpPressed();
	
	bool IsPgDownPressed();
private:
	bool ReadKeyboard();

	bool ReadMouse();

	void ProcessInput();
// old interface
public:
	void Initialize();

	void KeyDown(unsigned int);
	
	void KeyUp(unsigned int);

	bool IsKeyDown(unsigned int)const;
private:
	IDirectInput8* m_directInput;

	IDirectInputDevice8* m_keyboard;
	
	IDirectInputDevice8* m_mouse;

	unsigned char m_keyboardState[256];
	
	DIMOUSESTATE m_mouseState;

	int m_screenWidth, m_screenHeight;
	
	int m_mouseX, m_mouseY;

	bool keys_[256] = {};
};

#endif