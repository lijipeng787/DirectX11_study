#pragma once

#include "../CommonFramework/GraphicsBase.h"

class DirectX11Device;
class Camera;
class ModelClass;
class TextureShaderClass;
class TransparentShaderClass;

class GraphicsClass : public GraphicsBase {
public:
  GraphicsClass();

  GraphicsClass(const GraphicsClass &rhs) = delete;

  GraphicsClass &operator=(const GraphicsClass &rhs) = delete;

  virtual ~GraphicsClass();

public:
  virtual bool Initialize(int, int, HWND) override;

  virtual void Shutdown() override;

  virtual void Frame(float) override;

  virtual bool Render() override;

private:
  bool RenderToTexture();

  bool RenderScene();

private:
  Camera *camera_ = nullptr;

  ModelClass *model_1_ = nullptr;

  ModelClass *model_2_ = nullptr;

  TextureShaderClass *texture_shader_ = nullptr;

  TransparentShaderClass *transparent_shader_ = nullptr;
};
