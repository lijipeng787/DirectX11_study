#pragma once

#include "../CommonFramework/GraphicsBase.h"

class DirectX11Device;
class Camera;
class ModelClass;
class TranslateShaderClass;

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

  virtual void Render() override;

private:
  bool RenderToTexture();

  bool RenderScene();

private:
  Camera *camera_ = nullptr;

  ModelClass *model_ = nullptr;

  TranslateShaderClass *translation_shader_ = nullptr;
};
