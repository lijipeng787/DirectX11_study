#pragma once

#include "../CommonFramework2/SystemBase.h"
#include "Graphics.h"
#include "Position.h"

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
  std::unique_ptr<Graphics> graphics_;

  std::unique_ptr<Position> position_;
};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
