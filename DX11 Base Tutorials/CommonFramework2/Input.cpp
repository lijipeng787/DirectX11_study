#include "stdafx.h"

#include "Input.h"

Input::Input() {}

Input::~Input() {}

bool Input::Initialize(HINSTANCE hinstance, HWND hwnd, int screenWidth,
                       int screenHeight) {

  HRESULT result;

  screen_width_ = screenWidth;
  screen_height_ = screenHeight;

  mouse_x_ = 0;
  mouse_y_ = 0;

  result = DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8,
                              (void **)&input_device_, NULL);
  if (FAILED(result)) {
    return false;
  }

  result = input_device_->CreateDevice(GUID_SysKeyboard, &keyboard_, NULL);
  if (FAILED(result)) {
    return false;
  }

  result = keyboard_->SetDataFormat(&c_dfDIKeyboard);
  if (FAILED(result)) {
    return false;
  }

  result =
      keyboard_->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
  if (FAILED(result)) {
    return false;
  }

  result = keyboard_->Acquire();
  if (FAILED(result)) {
    return false;
  }

  result = input_device_->CreateDevice(GUID_SysMouse, &mouse_, NULL);
  if (FAILED(result)) {
    return false;
  }

  result = mouse_->SetDataFormat(&c_dfDIMouse);
  if (FAILED(result)) {
    return false;
  }

  result =
      mouse_->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
  if (FAILED(result)) {
    return false;
  }

  result = mouse_->Acquire();
  if (FAILED(result)) {
    return false;
  }

  return true;
}

void Input::Shutdown() {

  if (mouse_) {
    mouse_->Unacquire();
    mouse_->Release();
    mouse_ = nullptr;
  }

  if (keyboard_) {
    keyboard_->Unacquire();
    keyboard_->Release();
    keyboard_ = nullptr;
  }

  if (input_device_) {
    input_device_->Release();
    input_device_ = nullptr;
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

  result = keyboard_->GetDeviceState(sizeof(keyboard_state_),
                                     (LPVOID)&keyboard_state_);
  if (FAILED(result)) {

    if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED)) {
      keyboard_->Acquire();
    } else {
      return false;
    }
  }

  return true;
}

bool Input::ReadMouse() {

  HRESULT result;

  result = mouse_->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&mouse_state_);
  if (FAILED(result)) {

    if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED)) {
      mouse_->Acquire();
    } else {
      return false;
    }
  }

  return true;
}

void Input::ProcessInput() {

  mouse_x_ += mouse_state_.lX;
  mouse_y_ += mouse_state_.lY;

  if (mouse_x_ < 0) {
    mouse_x_ = 0;
  }
  if (mouse_y_ < 0) {
    mouse_y_ = 0;
  }

  if (mouse_x_ > screen_width_) {
    mouse_x_ = screen_width_;
  }
  if (mouse_y_ > screen_height_) {
    mouse_y_ = screen_height_;
  }
}

bool Input::IsEscapePressed() {

  if (keyboard_state_[DIK_ESCAPE] & 0x80) {
    return true;
  }

  return false;
}

void Input::GetMouseLocation(int &mouseX, int &mouseY) {

  mouseX = mouse_x_;
  mouseY = mouse_y_;
}

void Input::Initialize() {

  int i;
  for (i = 0; i < 256; i++) {
    keys_[i] = false;
  }
}

void Input::KeyDown(unsigned int input) { keys_[input] = true; }

void Input::KeyUp(unsigned int input) { keys_[input] = false; }

bool Input::IsKeyDown(unsigned int key) const { return keys_[key]; }

bool Input::IsLeftArrowPressed() {

  if (keyboard_state_[DIK_LEFT] & 0x80) {
    return true;
  }

  return false;
}

bool Input::IsRightArrowPressed() {

  if (keyboard_state_[DIK_RIGHT] & 0x80) {
    return true;
  }

  return false;
}

bool Input::IsLeftPressed() {

  if (keyboard_state_[DIK_LEFT] & 0x80) {
    return true;
  }

  return false;
}

bool Input::IsRightPressed() {

  if (keyboard_state_[DIK_RIGHT] & 0x80) {
    return true;
  }

  return false;
}

bool Input::IsUpPressed() {

  if (keyboard_state_[DIK_UP] & 0x80) {
    return true;
  }

  return false;
}

bool Input::IsDownPressed() {

  if (keyboard_state_[DIK_DOWN] & 0x80) {
    return true;
  }

  return false;
}

bool Input::IsAPressed() {

  if (keyboard_state_[DIK_A] & 0x80) {
    return true;
  }

  return false;
}

bool Input::IsZPressed() {

  if (keyboard_state_[DIK_Z] & 0x80) {
    return true;
  }

  return false;
}

bool Input::IsPgUpPressed() {

  if (keyboard_state_[DIK_PGUP] & 0x80) {
    return true;
  }

  return false;
}

bool Input::IsPgDownPressed() {

  if (keyboard_state_[DIK_PGDN] & 0x80) {
    return true;
  }

  return false;
}

bool Input::IsLeftMouseButtonDown() {

  if (mouse_state_.rgbButtons[0] & 0x80) {
    return true;
  }

  return false;
}
