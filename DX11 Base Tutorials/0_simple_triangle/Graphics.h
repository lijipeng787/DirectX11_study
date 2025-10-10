#pragma once

#include "../CommonFramework/GraphicsBase.h"
#include <memory>

class DirectX11Device;
class Camera;
class ModelClass;
class ColorShaderClass;

class GraphicsClass : public GraphicsBase {
public:
  GraphicsClass();

  GraphicsClass(const GraphicsClass &) = delete;

  GraphicsClass &operator=(const GraphicsClass &) = delete;

  virtual ~GraphicsClass() noexcept override = default;

public:
  bool Initialize(int screenWidth, int screenHeight, HWND hwnd) override;

  void Shutdown() override;

  void Frame(float deltaTime) override;

  void Render() override;

private:
  DirectX11Device *directx_device_ = nullptr; // non-owning singleton

  std::unique_ptr<Camera> camera_;

  std::unique_ptr<ModelClass> model_;

  std::unique_ptr<ColorShaderClass> color_shader_;
};
