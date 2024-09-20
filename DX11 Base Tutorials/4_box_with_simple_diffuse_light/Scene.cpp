#include "Scene.h"

void Scene::AddRenderGroup(std::shared_ptr<IRenderGroup> renderGroup) {
  renderGroups_.push_back(std::move(renderGroup));
}

void Scene::SetCamera(std::shared_ptr<Camera> camera) {
  camera_ = std::move(camera);
}

void Scene::AddLight(std::shared_ptr<LightClass> light) {
  lights_.push_back(std::move(light));
}