#include "System.h"
#include "ShaderParameterContainerTests.h"
#include <iostream>
#include <memory>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline,
                   int iCmdshow) {

#ifdef _DEBUG
  // Allocate console window in Debug mode
  AllocConsole();
  FILE *pCout = nullptr;
  freopen_s(&pCout, "CONOUT$", "w", stdout);
  freopen_s(&pCout, "CONOUT$", "w", stderr);
  std::cout.clear();
  std::cerr.clear();
  std::cout << "=== Debug Console Initialized ===" << std::endl;
#endif

  if (!RunShaderParameterContainerTests()) {
    std::cerr << "ShaderParameterContainer tests failed. Aborting startup." << std::endl;
#ifdef _DEBUG
    FreeConsole();
#endif
    return 1;
  }

  // Use smart pointer to manage System lifetime, avoid manual new/delete
  auto system = std::make_unique<System>();
  if (!system) {
    return 0;
  }

  bool result = system->Initialize();
  if (result) {
    system->Run();
  }

  system->Shutdown();
  // system will automatically destruct, no need to manually delete

#ifdef _DEBUG
  // Clean up console
  FreeConsole();
#endif

  return 0;
}