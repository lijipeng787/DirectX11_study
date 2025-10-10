#pragma once

#include "../CommonFramework/GraphicsBase.h"
#include <memory>

class DirectX11Device;
class Camera;
class ModelClass;
class TextureShader;
class SimpleMoveableSurface;

class GraphicsClass : public GraphicsBase {
public:
  GraphicsClass();

  GraphicsClass(const GraphicsClass &rhs) = delete;

  GraphicsClass &operator=(const GraphicsClass &rhs) = delete;

  virtual ~GraphicsClass() noexcept override;

public:
  bool Initialize(int screenWidth, int screenHeight,
                          HWND hwnd) override;

  void Shutdown() override;

  void Frame(float deltaTime) override;

  void Render() override;

private:
  std::unique_ptr<Camera> camera_;

  ModelClass *model_ = nullptr;

  TextureShader *texture_shader_;

  SimpleMoveableSurface *bitmap_;
};
