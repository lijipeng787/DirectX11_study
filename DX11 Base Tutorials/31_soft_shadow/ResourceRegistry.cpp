#include "ResourceRegistry.h"

#include "Logger.h"
#include "Model.h"
#include "OrthoWindow.h"
#include "RenderTexture.h"

#include <iostream>

ResourceRegistry &ResourceRegistry::GetInstance() {
  static ResourceRegistry instance;
  return instance;
}

bool ResourceRegistry::Initialize(ID3D11Device *device,
                                 ID3D11DeviceContext *context, HWND hwnd) {
  std::lock_guard<std::mutex> lock(mutex_);

  if (initialized_) {
    return true;
  }

  if (!device || !context) {
    Logger::SetModule("ResourceRegistry");
    Logger::LogError("Invalid device or context");
    return false;
  }

  device_ = device;
  context_ = context;
  hwnd_ = hwnd;
  initialized_ = true;

  std::cout << "[ResourceRegistry] Initialized successfully" << std::endl;
  return true;
}

void ResourceRegistry::Shutdown() {
  std::lock_guard<std::mutex> lock(mutex_);

  resources_.clear();
  device_ = nullptr;
  context_ = nullptr;
  hwnd_ = nullptr;
  initialized_ = false;

  std::cout << "[ResourceRegistry] Shutdown complete" << std::endl;
}

size_t ResourceRegistry::GetTotalResourceCount() const {
  std::lock_guard<std::mutex> lock(mutex_);

  size_t total = 0;
  for (const auto &[type, cache] : resources_) {
    total += cache.size();
  }
  return total;
}

void ResourceRegistry::ClearAll() {
  std::lock_guard<std::mutex> lock(mutex_);
  resources_.clear();
  std::cout << "[ResourceRegistry] All resources cleared" << std::endl;
}

void ResourceRegistry::LogStats() const {
  std::lock_guard<std::mutex> lock(mutex_);

  std::cout << "\n=== ResourceRegistry Statistics ===" << std::endl;
  std::cout << "Total resource types: " << resources_.size() << std::endl;
  
  for (const auto &[type, cache] : resources_) {
    std::cout << "  Type " << type.name() << ": " << cache.size()
              << " resources" << std::endl;
  }
  
  std::cout << "Total resources: " << GetTotalResourceCount() << std::endl;
  std::cout << "====================================\n" << std::endl;
}
