#pragma once

#define DIRECTINPUT_VERSION 0x0800

#include <dinput.h>

class Input {
public:
  Input();

  Input(const Input &rhs) = delete;

  Input &operator=(const Input &rhs) = delete;

  ~Input();

public:
  bool Initialize(HINSTANCE, HWND, int, int);

  void Shutdown();

  bool Frame();

  void GetMouseLocation(int &, int &);

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

  bool IsLeftMouseButtonDown();

private:
  bool ReadKeyboard();

  bool ReadMouse();

  void ProcessInput();
  // old interface

public:
  void Initialize();

  void KeyDown(unsigned int);

  void KeyUp(unsigned int);

  bool IsKeyDown(unsigned int) const;

private:
  IDirectInput8 *input_device_;

  IDirectInputDevice8 *keyboard_;

  IDirectInputDevice8 *mouse_;

  unsigned char keyboard_state_[256];

  DIMOUSESTATE mouse_state_;

  int screen_width_, screen_height_;

  int mouse_x_, mouse_y_;

  bool keys_[256] = {};
};
