#pragma once

#include "GameObject.h"
#include "IRenderGroup.h"


#include <memory>
#include <vector>


class StandardRenderGroup : public IRenderGroup {
public:
  void PreRender() override {}

  void PostRender() override {}

  const std::vector<std::shared_ptr<IRenderable>> &
  GetRenderables() const override {
    return renderables_;
  }

  void AddRenderable(std::shared_ptr<IRenderable> renderable) {
    renderables_.push_back(std::move(renderable));
  }

  void AddGameObject(std::shared_ptr<GameObject> gameObject) {
    gameObjects_.push_back(std::move(gameObject));
  }

  const std::vector<std::shared_ptr<GameObject>> &GetGameObjects() const {
    return gameObjects_;
  }

private:
  std::vector<std::shared_ptr<IRenderable>> renderables_;
  std::vector<std::shared_ptr<GameObject>> gameObjects_;
};