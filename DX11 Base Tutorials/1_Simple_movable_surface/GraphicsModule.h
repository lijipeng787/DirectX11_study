#pragma once

#include "../CommonFramework/GraphicsBase.h"

class DirectX11Device;
class Camera;
class ModelClass;
class TextureShader;
class SimpleMoveableSurface;

class GraphicsModule : public GraphicsBase {
public:
  GraphicsModule();

  GraphicsModule(const GraphicsModule &rhs) = delete;

  GraphicsModule &operator=(const GraphicsModule &rhs) = delete;

  virtual ~GraphicsModule();

public:
  virtual bool Initialize(int screenWidth, int screenHeight,
                          HWND hwnd) override;

  virtual void Shutdown() override;

  virtual void Frame(float) override;

  virtual bool Render() override;

private:
  Camera *camera_ = nullptr;

  ModelClass *model_ = nullptr;

  TextureShader *texture_shader_;

  SimpleMoveableSurface *bitmap_;
};
