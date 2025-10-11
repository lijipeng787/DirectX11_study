#include "System.h"
#include <memory>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline,
                   int iCmdshow) {

  // 使用智能指针管理System生命周期，避免手动new/delete
  auto system = std::make_unique<System>();
  if (!system) {
    return 0;
  }

  bool result = system->Initialize();
  if (result) {
    system->Run();
  }

  system->Shutdown();
  // system会自动析构，无需手动delete

  return 0;
}