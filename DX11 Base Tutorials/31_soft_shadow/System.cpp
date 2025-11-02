#include "System.h"

#include "../CommonFramework2/Input.h"
#include "../CommonFramework2/Timer.h"

#include "Graphics.h"
#include "position.h"

using namespace std;

bool System::Initialize() {

  SetWindProc(WndProc);

  auto result = SystemBase::Initialize();
  if (!result) {
    return false;
  }

  // Store System instance pointer in window's user data
  // This allows WndProc to get System instance via window handle, avoiding global variables
  SetWindowLongPtr(GetApplicationHandle(), GWLP_USERDATA,
                   reinterpret_cast<LONG_PTR>(this));

  unsigned int screenWidth = 0, screenHeight = 0;

  GetScreenWidthAndHeight(screenWidth, screenHeight);

  {
    graphics_ = make_unique<Graphics>();
    if (!graphics_) {
      return false;
    }
    bool result = graphics_->Initialize(screenWidth, screenHeight,
                                        GetApplicationHandle());
    if (!result) {
      return false;
    }
  }

  {
    position_ = make_unique<Position>();
    if (!position_) {
      return false;
    }
    // Set the initial position of the viewer to the same as the initial camera
    // position.
    position_->SetPosition(0.0f, 2.0f, -10.0f);
  }

  return true;
}

bool System::Frame() {

  auto result = SystemBase::Frame();
  if (result == false) {
    return false;
  }

  result = GetInputComponent().Frame();
  if (result == false) {
    return false;
  }

  if (GetInputComponent().IsEscapePressed() == true) {
    return true;
  }

  result = HandleInput(GetTimerComponent().GetTime());
  if (!result) {
    return false;
  }

  float posX, posY, posZ;
  float rotX, rotY, rotZ;
  position_->GetPosition(posX, posY, posZ);
  position_->GetRotation(rotX, rotY, rotZ);

  graphics_->SetPosition(posX, posY, posZ);
  // Convert degrees to Camera's normalized rotation format
  // Camera expects: rotation * HALF_PI = actual angle in radians
  // Position stores: degrees (0-360)
  // Conversion: Camera rotation = Position rotation (degrees) / 90.0f
  const float DEGREES_TO_CAMERA_UNIT = 1.0f / 90.0f;
  graphics_->SetRotation(rotX * DEGREES_TO_CAMERA_UNIT,
                         rotY * DEGREES_TO_CAMERA_UNIT,
                         rotZ * DEGREES_TO_CAMERA_UNIT);

  // GetTime() returns milliseconds, convert to seconds for consistent time units
  // This matches the implementation in project 5
  float delta_time = GetTimerComponent().GetTime() / 1000.0f;

  graphics_->Frame(delta_time);

  graphics_->Render();

  return true;
}

bool System::HandleInput(float frameTime) {

  bool keyDown;

  position_->SetFrameTime(frameTime);

  keyDown = GetInputComponent().IsLeftPressed();
  position_->TurnLeft(keyDown);

  keyDown = GetInputComponent().IsRightPressed();
  position_->TurnRight(keyDown);

  keyDown = GetInputComponent().IsUpPressed();
  position_->MoveForward(keyDown);

  keyDown = GetInputComponent().IsDownPressed();
  position_->MoveBackward(keyDown);

  keyDown = GetInputComponent().IsAPressed();
  position_->MoveUpward(keyDown);

  keyDown = GetInputComponent().IsZPressed();
  position_->MoveDownward(keyDown);

  keyDown = GetInputComponent().IsPgUpPressed();
  position_->LookUpward(keyDown);

  keyDown = GetInputComponent().IsPgDownPressed();
  position_->LookDownward(keyDown);

  return true;
}

void System::Shutdown() {
  if (graphics_) {
    graphics_->Shutdown();
    graphics_.reset();
  }

  if (position_) {
    position_.reset();
  }

  SystemBase::Shutdown();
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam,
                         LPARAM lparam) {

  switch (umessage) {
  case WM_DESTROY: {
    PostQuitMessage(0);
    return 0;
  }

  case WM_CLOSE: {
    PostQuitMessage(0);
    return 0;
  }

  default: {
    // Get System instance pointer from window user data, avoiding global variables
    System *system =
        reinterpret_cast<System *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (system) {
      return system->MessageHandler(hwnd, umessage, wparam, lparam);
    }
    return DefWindowProc(hwnd, umessage, wparam, lparam);
  }
  }
}