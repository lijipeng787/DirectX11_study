#pragma once

#include "../CommonFramework/SystemBase.h"

class GraphicsModule;

class System : public SystemBase {
public:
  System() {}

  System(const System &rhs) = delete;

  System &operator=(const System &rhs) = delete;

  virtual ~System() {}

public:
  virtual bool Initialize() override;

  virtual void Shutdown() override;

  virtual bool Frame() override;

private:
  GraphicsModule *Graphics_;
};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

static System *ApplicationInstance = nullptr;
