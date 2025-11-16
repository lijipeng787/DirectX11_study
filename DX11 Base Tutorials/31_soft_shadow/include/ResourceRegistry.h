#pragma once

#include <any>
#include <d3d11.h>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <typeindex>
#include <unordered_map>

// Forward declarations
class Model;
class PBRModel;
class DDSTexture;
class TGATexture;
class RenderTexture;
class OrthoWindow;
class IShader;

// Unified resource registry with type-safe access
class ResourceRegistry {
public:
  // Singleton access
  static ResourceRegistry &GetInstance();

  // Initialization
  bool Initialize(ID3D11Device *device, ID3D11DeviceContext *context,
                  HWND hwnd);
  void Shutdown();

  // Generic registration (called during loading phase)
  template <typename T>
  void Register(const std::string &id, std::shared_ptr<T> resource);

  // Generic retrieval with type safety
  template <typename T> std::shared_ptr<T> Get(const std::string &id) const;

  // Check existence
  template <typename T> bool Has(const std::string &id) const;

  // Unregister (for hot-reload)
  template <typename T> bool Unregister(const std::string &id);

  // Get all IDs of a specific type
  template <typename T> std::vector<std::string> GetAllIds() const;

  // Statistics
  size_t GetTotalResourceCount() const;
  template <typename T> size_t GetResourceCount() const;

  // Reference counting
  template <typename T> int GetRefCount(const std::string &id) const;

  // Clear specific type
  template <typename T> void ClearType();

  void ClearAll();

  // Logging
  void LogStats() const;

  // Device access (for resource loading)
  ID3D11Device *GetDevice() const { return device_; }
  ID3D11DeviceContext *GetContext() const { return context_; }
  HWND GetHWND() const { return hwnd_; }

private:
  ResourceRegistry() = default;
  ~ResourceRegistry() = default;
  ResourceRegistry(const ResourceRegistry &) = delete;
  ResourceRegistry &operator=(const ResourceRegistry &) = delete;

  // Type-erased storage: TypeIndex -> (ID -> any)
  std::unordered_map<std::type_index, std::unordered_map<std::string, std::any>>
      resources_;

  // Device context
  ID3D11Device *device_ = nullptr;
  ID3D11DeviceContext *context_ = nullptr;
  HWND hwnd_ = nullptr;

  // Thread safety
  mutable std::mutex mutex_;
  bool initialized_ = false;

  // Helper: get type-specific cache
  template <typename T>
  std::unordered_map<std::string, std::any> &GetCacheForType();

  template <typename T>
  const std::unordered_map<std::string, std::any> &GetCacheForType() const;
};

// Template implementations

template <typename T>
void ResourceRegistry::Register(const std::string &id,
                                std::shared_ptr<T> resource) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto &cache = resources_[std::type_index(typeid(T))];
  cache[id] = resource;
}

template <typename T>
std::shared_ptr<T> ResourceRegistry::Get(const std::string &id) const {
  std::lock_guard<std::mutex> lock(mutex_);

  auto type_it = resources_.find(std::type_index(typeid(T)));
  if (type_it == resources_.end()) {
    // Type not registered at all
    std::cerr << "[ResourceRegistry] Type not found: " << typeid(T).name()
              << std::endl;
    return nullptr;
  }

  auto id_it = type_it->second.find(id);
  if (id_it == type_it->second.end()) {
    // ID not found for this type
    std::cerr << "[ResourceRegistry] ID '" << id << "' not found for type "
              << typeid(T).name() << std::endl;
    std::cerr << "  Available IDs for this type: ";
    for (const auto &[available_id, _] : type_it->second) {
      std::cerr << "'" << available_id << "' ";
    }
    std::cerr << std::endl;
    return nullptr;
  }

  try {
    return std::any_cast<std::shared_ptr<T>>(id_it->second);
  } catch (const std::bad_any_cast &) {
    std::cerr << "[ResourceRegistry] Type mismatch for ID '" << id << "'"
              << std::endl;
    return nullptr;
  }
}

template <typename T> bool ResourceRegistry::Has(const std::string &id) const {
  std::lock_guard<std::mutex> lock(mutex_);

  auto type_it = resources_.find(std::type_index(typeid(T)));
  if (type_it == resources_.end()) {
    return false;
  }

  return type_it->second.find(id) != type_it->second.end();
}

template <typename T> bool ResourceRegistry::Unregister(const std::string &id) {
  std::lock_guard<std::mutex> lock(mutex_);

  auto type_it = resources_.find(std::type_index(typeid(T)));
  if (type_it == resources_.end()) {
    return false;
  }

  return type_it->second.erase(id) > 0;
}

template <typename T>
std::vector<std::string> ResourceRegistry::GetAllIds() const {
  std::lock_guard<std::mutex> lock(mutex_);

  std::vector<std::string> ids;
  auto type_it = resources_.find(std::type_index(typeid(T)));
  if (type_it == resources_.end()) {
    return ids;
  }

  ids.reserve(type_it->second.size());
  for (const auto &[id, _] : type_it->second) {
    ids.push_back(id);
  }
  return ids;
}

template <typename T> size_t ResourceRegistry::GetResourceCount() const {
  std::lock_guard<std::mutex> lock(mutex_);

  auto type_it = resources_.find(std::type_index(typeid(T)));
  if (type_it == resources_.end()) {
    return 0;
  }
  return type_it->second.size();
}

template <typename T>
int ResourceRegistry::GetRefCount(const std::string &id) const {
  auto resource = Get<T>(id);
  return resource ? resource.use_count() : 0;
}

template <typename T> void ResourceRegistry::ClearType() {
  std::lock_guard<std::mutex> lock(mutex_);
  resources_.erase(std::type_index(typeid(T)));
}
