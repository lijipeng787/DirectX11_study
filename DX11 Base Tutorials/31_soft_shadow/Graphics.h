#pragma once

#include "../CommonFramework2/Camera.h"
#include "../CommonFramework2/GraphicsBase.h"
#include "RenderPipeline.h"
#include "light.h"

class Camera;
class DirectX11Device;
class Model;
class DepthShader;
class ShadowShader;
class RenderTexture;
class OrthoWindow;
class VerticalBlurShader;
class HorizontalBlurShader;
class SoftShadowShader;
class TextureShader;
class PBRModel;
class PbrShader;
class StandardRenderGroup;

class GraphicsClass : public GraphicsBase {
public:
  GraphicsClass() = default;

  GraphicsClass(const GraphicsClass &rhs) = delete;

  GraphicsClass &operator=(const GraphicsClass &rhs) = delete;

  virtual ~GraphicsClass() = default;

public:
  virtual bool Initialize(int, int, HWND) override;

  virtual void Shutdown() override;

  virtual void Frame(float deltatime) override;

  virtual void Render() override;

public:
  inline void SetPosition(float x, float y, float z) {
    pos_x_ = x;
    pos_y_ = y;
    pos_z_ = z;
  }

  inline void SetRotation(float x, float y, float z) {
    rot_x_ = x;
    rot_y_ = y;
    rot_z_ = z;
  }

private:
  void SetupRenderPipeline();

private:
  float pos_x_ = 0.0f, pos_y_ = 0.0f, pos_z_ = 0.0f;

  float rot_x_ = 0.0f, rot_y_ = 0.0f, rot_z_ = 0.0f;

  std::unique_ptr<Camera> camera_;

  std::shared_ptr<Model> cube_model_, ground_model_, sphere_model_;

  std::shared_ptr<PBRModel> sphere_pbr_model_;

  std::unique_ptr<Light> light_;

  std::shared_ptr<RenderTexture> render_texture_, blackwhiter_render_texture_,
      downsample_texure_;

  std::shared_ptr<RenderTexture> horizontal_blur_texture_,
      vertical_blur_texture_, upsample_texure_;

  std::shared_ptr<DepthShader> depth_shader_;

  std::shared_ptr<ShadowShader> shadow_shader_;

  std::shared_ptr<OrthoWindow> small_window_, fullscreen_window_;

  std::shared_ptr<TextureShader> texture_shader_;

  std::shared_ptr<HorizontalBlurShader> horizontal_blur_shader_;

  std::shared_ptr<VerticalBlurShader> vertical_blur_shader_;

  std::shared_ptr<SoftShadowShader> soft_shadow_shader_;

  std::shared_ptr<PbrShader> pbr_shader_;

  std::shared_ptr<StandardRenderGroup> cube_group_;

  RenderPipeline render_pipeline_;
};