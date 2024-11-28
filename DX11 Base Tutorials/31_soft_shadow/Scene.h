#pragma once

#include <memory>
#include <vector>

#include "../CommonFramework2/Camera.h"
#include "IRenderGroup.h"
#include "light.h"

class Scene {
public:
  void AddRenderGroup(std::shared_ptr<IRenderGroup> renderGroup);

  void SetCamera(std::shared_ptr<Camera> camera);

  void AddLight(std::shared_ptr<Light> light);

  const std::vector<std::shared_ptr<IRenderGroup>> &GetRenderGroups() const {
    return renderGroups_;
  }

  std::shared_ptr<Camera> GetCamera() const { return camera_; }

  const std::vector<std::shared_ptr<Light>> &GetLights() const {
    return lights_;
  }

private:
  std::vector<std::shared_ptr<IRenderGroup>> renderGroups_;
  std::shared_ptr<Camera> camera_;
  std::vector<std::shared_ptr<Light>> lights_;
};
