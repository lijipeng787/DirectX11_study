#include "stdafx.h"
#include "Input.h"

Input::Input(){}

Input::~Input(){}

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


bool Input::IsKeyDown(unsigned int key) {
	return keys_[key];
}