#include "System.h"

#include "../CommonFramework/Input.h"
#include "../CommonFramework/Timer.h"

#include "Graphics.h"
#include "positionclass.h"

System::System() {}

System::~System() {}

bool System::Initialize() {

  ApplicationInstance = this;
  SetWindProc(WndProc);

  SystemBase::Initialize();

  unsigned int screenWidth = 800, screenHeight = 600;

  // GetScreenWidthAndHeight(screenWidth, screenHeight);

  {
    graphics_ = new GraphicsClass();
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
    position_ = new PositionClass();
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

  SystemBase::Frame();

  bool result;

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

  auto delta_time = GetTimerComponent().GetTime();
  graphics_->SetFrameTime(delta_time);

  graphics_->SetPosition(posX, posY, posZ);
  graphics_->SetRotation(rotX, rotY, rotZ);

  graphics_->Frame(0.0f);

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

  SystemBase::Shutdown();

  if (graphics_) {
    graphics_->Shutdown();
    delete graphics_;
    graphics_ = 0;
  }
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
    return ApplicationInstance->MessageHandler(hwnd, umessage, wparam, lparam);
  }
  }
}