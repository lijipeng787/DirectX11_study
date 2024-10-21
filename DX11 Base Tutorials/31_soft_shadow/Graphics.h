#pragma once

#include "../CommonFramework/Camera.h"
#include "../CommonFramework/GraphicsBase.h"
#include "RenderPipeline.h"
#include "lightclass.h"

constexpr int SHADOWMAP_WIDTH = 1024;
constexpr int SHADOWMAP_HEIGHT = 1024;

class DirectX11Device;
class ModelClass;
class DepthShaderClass;
class ShadowShaderClass;
class RenderTextureClass;
class OrthoWindowClass;
class HorizontalBlurShaderClass;
class VerticalBlurShaderClass;
class SoftShadowShaderClass;
class TextureShaderClass;
class PBRModelClass;
class PbrShaderClass;

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

  std::shared_ptr<ModelClass> m_CubeModel, m_GroundModel, m_SphereModel;

  std::shared_ptr<PBRModelClass> spherePBRModel;

  std::unique_ptr<LightClass> light_;

  std::shared_ptr<RenderTextureClass> render_texture_,
      m_BlackWhiteRenderTexture, m_DownSampleTexure;

  std::shared_ptr<RenderTextureClass> m_HorizontalBlurTexture,
      m_VerticalBlurTexture, m_UpSampleTexure;

  std::shared_ptr<DepthShaderClass> depth_shader_;

  std::shared_ptr<ShadowShaderClass> m_ShadowShader;

  std::shared_ptr<OrthoWindowClass> m_SmallWindow, m_FullScreenWindow;

  std::shared_ptr<TextureShaderClass> texture_shader_;

  std::shared_ptr<HorizontalBlurShaderClass> m_HorizontalBlurShader;

  std::shared_ptr<VerticalBlurShaderClass> m_VerticalBlurShader;

  std::shared_ptr<SoftShadowShaderClass> m_SoftShadowShader;

  std::shared_ptr<PbrShaderClass> PBRShader_;

  RenderPipeline render_pipeline_;
};