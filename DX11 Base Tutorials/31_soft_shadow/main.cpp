#include "System.h"
#include <iostream>
#include <memory>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline,
                   int iCmdshow) {

#ifdef _DEBUG
  // 在Debug模式下分配控制台窗口
  AllocConsole();
  FILE *pCout = nullptr;
  freopen_s(&pCout, "CONOUT$", "w", stdout);
  freopen_s(&pCout, "CONOUT$", "w", stderr);
  std::cout.clear();
  std::cerr.clear();
  std::cout << "=== Debug Console Initialized ===" << std::endl;
#endif

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

#ifdef _DEBUG
  // 清理控制台
  FreeConsole();
#endif

  return 0;
}