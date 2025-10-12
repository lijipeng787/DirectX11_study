#pragma once

#include "../CommonFramework2/Camera.h"
#include "../CommonFramework2/GraphicsBase.h"
#include "RenderGraph.h"
#include "RenderPipeline.h"
#include "light.h"

class Camera;
class DirectX11Device;
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
  void SetupRenderGraph();

private:
  float pos_x_ = 0.0f, pos_y_ = 0.0f, pos_z_ = 0.0f;

  float rot_x_ = 0.0f, rot_y_ = 0.0f, rot_z_ = 0.0f;

  std::unique_ptr<Camera> camera_;

  std::unique_ptr<Light> light_;

  std::shared_ptr<StandardRenderGroup> cube_group_;

  // Keep both for comparison/compatibility
  RenderPipeline render_pipeline_;
  RenderGraph render_graph_;

  // Flag to choose which rendering path to use
  bool use_render_graph_ = true; // Set to true to use RenderGraph

  // Renderable objects storage (shared between pipeline and graph)
  std::vector<std::shared_ptr<IRenderable>> renderable_objects_;
};