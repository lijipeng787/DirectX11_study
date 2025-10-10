#pragma once

#include "../CommonFramework/GraphicsBase.h"

class DirectX11Device;
class Camera;
class ModelClass;
class AlphaMapShaderClass;

class GraphicsClass : public GraphicsBase {
public:
  GraphicsClass() {}

  GraphicsClass(const GraphicsClass &rhs) = delete;

  GraphicsClass &operator=(const GraphicsClass &rhs) = delete;

  virtual ~GraphicsClass() {}

public:
  virtual bool Initialize(int, int, HWND) override;

  virtual void Shutdown() override;

  virtual void Frame(float) override;

  virtual void Render() override;

private:
  Camera *camera_ = nullptr;

  ModelClass *model_ = nullptr;

  AlphaMapShaderClass *alphamap_shader_ = nullptr;
};
