#pragma once

#include <memory>

#include <Windows.h>

#include "IShader.h"
#include "Scene.h"

class RenderPipeline {
public:
  RenderPipeline() = default;

  RenderPipeline(const RenderPipeline &) = delete;

  RenderPipeline &operator=(const RenderPipeline &) = delete;

  ~RenderPipeline() = default;

public:
  bool Initialize(HWND hwnd);

  void Shutdown();

  bool Render(const Scene &scene) const;

  void SetShader(std::unique_ptr<IShader> shader);

private:
  std::unique_ptr<IShader> shader_;
};