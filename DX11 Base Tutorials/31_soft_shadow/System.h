#pragma once

#include "../CommonFramework2/SystemBase.h"

class GraphicsClass;
class PositionClass;

class System : public SystemBase {
public:
  System() = default;

  System(const System &rhs) = delete;

  System &operator=(const System &rhs) = delete;

  virtual ~System() = default;

public:
  virtual bool Initialize() override;

  virtual void Shutdown() override;

  virtual bool Frame() override;

private:
  bool HandleInput(float frame_time);

private:
  GraphicsClass *graphics_;

  PositionClass *position_;
};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

static System *ApplicationInstance = nullptr;
