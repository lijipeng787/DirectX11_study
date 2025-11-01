#pragma once

#include <memory>

#include "../CommonFramework2/Camera.h"
#include "../CommonFramework2/GraphicsBase.h"
#include "Frustum.h"
#include "RenderGraph.h"
#include "RenderPipeline.h"
#include "ShaderParameterValidator.h"
#include "light.h"
#include "text.h"

class Camera;
class DirectX11Device;
class StandardRenderGroup;
class Model;
class PBRModel;
class RenderTexture;
class OrthoWindow;
class DepthShader;
class ShadowShader;
class SoftShadowShader;
class TextureShader;
class HorizontalBlurShader;
class VerticalBlurShader;
class PbrShader;
class FontShader;
class Font;
class Text;

class Graphics : public GraphicsBase {
public:
  Graphics() = default;

  Graphics(const Graphics &rhs) = delete;

  Graphics &operator=(const Graphics &rhs) = delete;

  virtual ~Graphics() = default;

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
  bool InitializeDevice(int screenWidth, int screenHeight, HWND hwnd);

  bool InitializeCamera();

  bool InitializeLight();

  bool InitializeResources(HWND hwnd);

  bool InitializeRenderingPipeline();

private:
  void SetupRenderPipeline();

  bool SetupRenderGraph();

  void SetupRenderPasses();

  void SetupRenderableObjects();

  void RegisterShaderParameters();

  // Frustum culling helper
  bool IsObjectVisible(std::shared_ptr<IRenderable> renderable,
                       const FrustumClass &frustum) const;

private:
  struct SceneAssets {
    std::shared_ptr<Model> cube;
    std::shared_ptr<Model> sphere;
    std::shared_ptr<Model> ground;
    std::shared_ptr<PBRModel> pbr_sphere;
  };

  struct RenderTargetAssets {
    std::shared_ptr<RenderTexture> shadow_depth;
    std::shared_ptr<RenderTexture> shadow_map;
    std::shared_ptr<RenderTexture> downsampled_shadow;
    std::shared_ptr<RenderTexture> horizontal_blur;
    std::shared_ptr<RenderTexture> vertical_blur;
    std::shared_ptr<RenderTexture> upsampled_shadow;
    std::shared_ptr<RenderTexture> reflection_map;
  };

  struct ShaderAssets {
    std::shared_ptr<DepthShader> depth;
    std::shared_ptr<ShadowShader> shadow;
    std::shared_ptr<TextureShader> texture;
    std::shared_ptr<HorizontalBlurShader> horizontal_blur;
    std::shared_ptr<VerticalBlurShader> vertical_blur;
    std::shared_ptr<SoftShadowShader> soft_shadow;
    std::shared_ptr<PbrShader> pbr;
  };

  struct OrthoWindowAssets {
    std::shared_ptr<OrthoWindow> small_window;
    std::shared_ptr<OrthoWindow> fullscreen_window;
  };

  float pos_x_ = 0.0f, pos_y_ = 0.0f, pos_z_ = 0.0f;

  float rot_x_ = 0.0f, rot_y_ = 0.0f, rot_z_ = 0.0f;

  unsigned int screenWidth = 0, screenHeight = 0;

  std::unique_ptr<Camera> camera_;

  std::unique_ptr<Light> light_;

  std::unique_ptr<FrustumClass> frustum_;

  std::unique_ptr<Text> text_;
  std::shared_ptr<Font> font_;
  std::shared_ptr<FontShader> font_shader_;

  std::shared_ptr<StandardRenderGroup> cube_group_;

  static constexpr bool use_render_graph_ = true;

  RenderPipeline render_pipeline_;
  RenderGraph render_graph_;

  // Parameter validation system
  ShaderParameterValidator parameter_validator_;

  // Renderable objects storage (shared between pipeline and graph)
  std::vector<std::shared_ptr<IRenderable>> renderable_objects_;

  SceneAssets scene_assets_;
  RenderTargetAssets render_targets_;
  ShaderAssets shader_assets_;
  OrthoWindowAssets ortho_windows_;
};