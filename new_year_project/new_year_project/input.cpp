#include "input.h"

Input::Input() :screen_width_(0), screen_height_(0), mouse_location_x_(0), mouse_location_y_(0),
F1_released_(true), F2_released_(true), F3_released_(true) {
	ZeroMemory(&keyboard_state_, sizeof(keyboard_state_));
	ZeroMemory(&mouse_state_, sizeof(mouse_state_));
}

Input::Input(const Input & copy) {
}

Input::~Input() {
}

bool Input::Initialize(HINSTANCE hinstance, HWND hwnd, int screen_width, int screen_height) {

	this->screen_width_ = screen_width;
	this->screen_height_ = screen_height;
	this->mouse_location_x_ = 0;
	this->mouse_location_y_ = 0;

	if (FAILED(DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)(&direct_input_), nullptr))) {
		return false;
	}

	if (FAILED(direct_input_->CreateDevice(GUID_SysKeyboard, &(keyboard_), nullptr))) {
		return false;
	}

	if (FAILED(keyboard_->SetDataFormat(&c_dfDIKeyboard))) {
		return false;
	}

	if (FAILED(keyboard_->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE))) {
		return false;
	}

	//if (FAILED(keyboard_->Acquire())) {
	//	return false;
	//}

	if (FAILED(direct_input_->CreateDevice(GUID_SysMouse, &(mouse_), nullptr))) {
		return false;
	}

	if (FAILED(mouse_->SetDataFormat(&c_dfDIMouse))) {
		return false;
	}

	if (FAILED(mouse_->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE))) {
		return false;
	}

	//if (FAILED(mouse_->Acquire())) {
	//	return false;
	//}

	return true;
}

void Input::Shutdown() {

	if (mouse_) {
		mouse_->Unacquire();
		mouse_->Release();
		mouse_ = 0;
	}

	if (keyboard_) {
		keyboard_->Unacquire();
		keyboard_->Release();
		keyboard_ = 0;
	}

	if (direct_input_) {
		direct_input_->Release();
		direct_input_ = 0;
	}
}

bool Input::Frame() {

	if (!(ReadKeyboard())) {
		return false;
	}

	if (!(ReadMouse())) {
		return false;
	}

	ProcessInput();

	return true;
}

void Input::GetMouseLocation(int & mouse_location_x, int & mouse_location_y) {

	this->mouse_location_x_ = mouse_location_x;
	this->mouse_location_y_ = mouse_location_y;
}

bool Input::IsEscapePressed()
{
	if (keyboard_state_[DIK_ESCAPE] & 0x80)
		return true;
	return false;
}

bool Input::IsLeftPressed()
{
	if (keyboard_state_[DIK_LEFT] & 0x80)
		return true;
	return false;
}

bool Input::IsRightPressed()
{
	if (keyboard_state_[DIK_RIGHT] & 0x80)
		return true;
	return false;
}

bool Input::IsUpPressed()
{
	if (keyboard_state_[DIK_UP] & 0x80)
		return true;
	return false;
}

bool Input::IsDownPressed()
{
	if (keyboard_state_[DIK_DOWN] & 0x80)
		return true;
	return false;
}

bool Input::IsAPressed()
{
	if (keyboard_state_[DIK_A] & 0x80)
		return true;
	return false;
}

bool Input::IsZPressed()
{
	if (keyboard_state_[DIK_Z] & 0x80)
		return true;
	return false;
}

bool Input::IsPageUpPressed()
{
	if (keyboard_state_[DIK_PGUP] & 0x80)
		return true;
	return false;
}

bool Input::IsPageDownPressed()
{
	if (keyboard_state_[DIK_PGDN] & 0x80)
		return true;
	return false;
}

bool Input::IsF1Toggled()
{
	if (keyboard_state_[DIK_F1] & 0x80) {
		if (F1_released_) {
			F1_released_ = false;
			return true;
		}
	}
	else {
		F1_released_ = true;
	}

	return false;
}

bool Input::IsF2Toggled()
{
	if (keyboard_state_[DIK_F2] & 0x80) {
		if (F2_released_) {
			F2_released_ = false;
			return true;
		}
	}
	else {
		F2_released_ = true;
	}

	return false;
}

bool Input::IsF3Toggled()
{
	if (keyboard_state_[DIK_F3] & 0x80) {
		if (F3_released_) {
			F3_released_ = false;
			return true;
		}
	}
	else {
		F3_released_ = true;
	}

	return false;
}

bool Input::ReadKeyboard() {

	auto return_status = keyboard_->GetDeviceState(sizeof(keyboard_state_), (LPVOID)&keyboard_state_);
	if (FAILED(return_status)) {
		if (DIERR_INPUTLOST == return_status || DIERR_NOTACQUIRED == return_status) {
			keyboard_->Acquire();
		}
		else {
			return false;
		}
	}

	return true;
}

bool Input::ReadMouse() {

	auto return_status = mouse_->GetDeviceState(sizeof(mouse_state_), (LPVOID)&mouse_state_);
	if (FAILED(return_status)) {
		if (DIERR_INPUTLOST == return_status || DIERR_NOTACQUIRED == return_status) {
			mouse_->Acquire();
		}
		else {
			return false;
		}
	}
	return true;
}

void Input::ProcessInput() {

	mouse_location_x_ += mouse_state_.lX;
	mouse_location_y_ += mouse_state_.lY;

	(0 > mouse_location_x_) ? (mouse_location_x_ = 0) : mouse_location_x_;
	(0 > mouse_location_y_) ? (mouse_location_y_ = 0) : mouse_location_y_;
	(screen_width_ < mouse_location_x_) ? (mouse_location_x_ = screen_width_) : mouse_location_x_;
	(screen_height_ < mouse_location_y_) ? (mouse_location_y_ = screen_height_) : mouse_location_y_;
}
