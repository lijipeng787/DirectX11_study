#pragma once

#include "../CommonFramework2/Camera.h"
#include "../CommonFramework2/GraphicsBase.h"
#include "light.h"
#include "RenderPipeline.h"

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

  std::shared_ptr<Model> m_CubeModel, m_GroundModel, m_SphereModel;

  std::shared_ptr<PBRModel> spherePBRModel;

  std::unique_ptr<Light> light_;

  std::shared_ptr<RenderTexture> render_texture_,
      m_BlackWhiteRenderTexture, m_DownSampleTexure;

  std::shared_ptr<RenderTexture> m_HorizontalBlurTexture,
      m_VerticalBlurTexture, m_UpSampleTexure;

  std::shared_ptr<DepthShader> depth_shader_;

  std::shared_ptr<ShadowShader> m_ShadowShader;

  std::shared_ptr<OrthoWindow> m_SmallWindow, m_FullScreenWindow;

  std::shared_ptr<TextureShader> texture_shader_;

  std::shared_ptr<HorizontalBlurShader> m_HorizontalBlurShader;

  std::shared_ptr<VerticalBlurShader> m_VerticalBlurShader;

  std::shared_ptr<SoftShadowShader> m_SoftShadowShader;

  std::shared_ptr<PbrShader> PBRShader_;

  std::shared_ptr<StandardRenderGroup> cube_group_;

  RenderPipeline render_pipeline_;
};