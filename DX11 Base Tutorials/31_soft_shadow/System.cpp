#include "System.h"

#include "../CommonFramework2/Input.h"
#include "../CommonFramework2/Timer.h"

#include "Graphics.h"
#include "position.h"

using namespace std;

bool System::Initialize() {

  SetWindProc(WndProc);

  SystemBase::Initialize();

  // 将System实例指针存储到窗口的用户数据中
  // 这样WndProc可以通过窗口句柄获取System实例，而不需要全局变量
  SetWindowLongPtr(GetApplicationHandle(), GWLP_USERDATA,
                   reinterpret_cast<LONG_PTR>(this));

  unsigned int screenWidth = 0, screenHeight = 0;

  GetScreenWidthAndHeight(screenWidth, screenHeight);

  {
    graphics_ = make_unique<GraphicsClass>();
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

  SystemBase::Frame();

  auto result = GetInputComponent().Frame();
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
  graphics_->SetRotation(rotX, rotY, rotZ);

  auto delta_time = GetTimerComponent().GetTime();

  graphics_->Frame(delta_time);

  graphics_->Render();
  if (!result) {
    return false;
  }

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

void System::Shutdown() { SystemBase::Shutdown(); }

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
    // 从窗口用户数据中获取System实例指针，避免全局变量
    System *system =
        reinterpret_cast<System *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (system) {
      return system->MessageHandler(hwnd, umessage, wparam, lparam);
    }
    return DefWindowProc(hwnd, umessage, wparam, lparam);
  }
  }
}