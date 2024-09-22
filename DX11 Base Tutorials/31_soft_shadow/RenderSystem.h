#pragma once

#include <memory>

#include "RenderPipeline.h"

class RenderSystem {
public:
  bool Initialize(HWND hwnd);

  void Shutdown();

  void Render(const Scene &scene);

private:
  std::unique_ptr<RenderPipeline> pipeline_;
};
