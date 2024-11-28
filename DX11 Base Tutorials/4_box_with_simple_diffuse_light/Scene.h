#pragma once

#include <memory>
#include <vector>

#include "../CommonFramework/Camera.h"
#include "IRenderGroup.h"
#include "lightclass.h"

class Scene {
public:
  void AddRenderGroup(std::shared_ptr<IRenderGroup> renderGroup);

  void SetCamera(std::shared_ptr<Camera> camera);

  void AddLight(std::shared_ptr<LightClass> light);

  const std::vector<std::shared_ptr<IRenderGroup>> &GetRenderGroups() const {
    return renderGroups_;
  }

  std::shared_ptr<Camera> GetCamera() const { return camera_; }

  const std::vector<std::shared_ptr<LightClass>> &GetLights() const {
    return lights_;
  }

private:
  std::vector<std::shared_ptr<IRenderGroup>> renderGroups_;

  std::shared_ptr<Camera> camera_;

  std::vector<std::shared_ptr<LightClass>> lights_;
};