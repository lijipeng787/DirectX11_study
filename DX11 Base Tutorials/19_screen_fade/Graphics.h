#pragma once

#include "../CommonFramework/GraphicsBase.h"

class DirectX11Device;
class Camera;
class ModelClass;
class TextureShaderClass;
class RenderTextureClass;
class SimpleMoveableSurface;
class FadeShaderClass;

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
  bool RenderToTexture(float rotation_);

  bool RenderScene();

  bool RenderNormalScene(float rotation_);

  bool RenderFadingScene();

private:
  float frame_time_ = 0.0f;

  float fadein_time_ = 0.0f, accumulated_time_ = 0.0f, fade_percentage_ = 0.0f;

  bool is_fade_done_ = false;

  Camera *camera_ = nullptr;

  ModelClass *model_ = nullptr;

  TextureShaderClass *texture_shader_ = nullptr;

  RenderTextureClass *render_texture_ = nullptr;

  SimpleMoveableSurface *bitmap_ = nullptr;

  FadeShaderClass *fade_shader_ = nullptr;
};