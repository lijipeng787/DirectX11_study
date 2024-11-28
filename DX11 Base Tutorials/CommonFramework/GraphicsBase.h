#pragma once

#include <Windows.h>

class GraphicsBase {
public:
  GraphicsBase();

  GraphicsBase(const GraphicsBase &rhs) = delete;

  GraphicsBase &operator=(const GraphicsBase &rhs) = delete;

  virtual ~GraphicsBase();

public:
  virtual bool Initialize(int, int, HWND) = 0;

  virtual void Shutdown() = 0;

  virtual void Frame(float deltatime) = 0;

  virtual void Render() = 0;
};
