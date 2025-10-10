#pragma once

#include "../CommonFramework/GraphicsBase.h"

class DirectX11Device;
class Camera;
class ModelClass;
class LightClass;
class LightShaderClass;

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

  void SetFameTime(float frame_time);

private:
  bool RenderRefractionToTexture();

  bool RenderReflectionToTexture();

  bool RenderScene();

private:
  float frame_time_ = 0.0f;

  Camera *camera_ = nullptr;

  ModelClass *model_ = nullptr;

  LightShaderClass *light_shader_ = nullptr;

  LightClass *m_Light1 = nullptr, *m_Light2 = nullptr, *m_Light3 = nullptr,
             *m_Light4 = nullptr;
};
