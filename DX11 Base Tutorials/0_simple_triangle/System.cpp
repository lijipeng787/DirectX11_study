#include "System.h"
#include "Graphics.h"
#include <memory>

System *ApplicationInstance = nullptr;

System::System() {}

System::~System() {}

bool System::Initialize() {

  ApplicationInstance = this;

  SetWindProc(WndProc);

  SystemBase::Initialize();

  unsigned int screenWidth = 800, screenHeight = 600;

  // GetScreenWidthAndHeight(screenWidth, screenHeight);

  graphics_ = std::make_unique<GraphicsClass>();
  bool result =
      graphics_->Initialize(screenWidth, screenHeight, GetApplicationHandle());
  if (!result) {
    graphics_.reset();
    return false;
  }

  return true;
}

void System::Shutdown() {

  if (graphics_) {
    graphics_->Shutdown();
    graphics_.reset();
  }
}

bool System::Frame() {

  SystemBase::Frame();

  graphics_->Frame(0.0f);

  return true;
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
