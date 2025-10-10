#pragma once

#include "../CommonFramework/SystemBase.h"
#include <memory>

class GraphicsClass;

class System : public SystemBase {
public:
  System();

  System(const System &rhs) = delete;

  System &operator=(const System &rhs) = delete;

  virtual ~System();

public:
  virtual bool Initialize() override;

  virtual void Shutdown() override;

  virtual bool Frame() override;

private:
  std::unique_ptr<GraphicsClass> graphics_;
};

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

extern System *ApplicationInstance;
