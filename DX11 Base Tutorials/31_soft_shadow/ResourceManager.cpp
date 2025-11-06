#include "ResourceManager.h"

#include "DepthShader.h"
#include "FontShader.h"
#include "HorizontalBlurShader.h"
#include "Logger.h"
#include "Model.h"
#include "OrthoWindow.h"
#include "PbrShader.h"
#include "RefractionShader.h"
#include "RenderTexture.h"
#include "SceneLightShader.h"
#include "ShadowShader.h"
#include "SimpleLightShader.h"
#include "SoftShadowShader.h"
#include "Texture.h"
#include "TextureShader.h"
#include "VerticalBlurShader.h"

#include <iostream>
#include <sstream>

using namespace std;

ResourceManager &ResourceManager::GetInstance() {
  static ResourceManager instance;
  return instance;
}

bool ResourceManager::Initialize(ID3D11Device *device,
                                 ID3D11DeviceContext *context, HWND hwnd) {
  if (initialized_) {
    return true; // Already initialized
  }

  if (!device || !context) {
    SetError("Invalid device or context");
    return false;
  }

  device_ = device;
  context_ = context;
  hwnd_ = hwnd;
  initialized_ = true;

  cout << "ResourceManager initialized successfully" << endl;
  return true;
}

void ResourceManager::Shutdown() {
  lock_guard<mutex> lock(cache_mutex_);

  // Clear all caches
  model_cache_.clear();
  pbr_model_cache_.clear();
  texture_cache_.clear();
  tga_texture_cache_.clear();
  shader_cache_.clear();
  render_texture_cache_.clear();
  ortho_window_cache_.clear();

  device_ = nullptr;
  context_ = nullptr;
  hwnd_ = nullptr;
  initialized_ = false;

  cout << "ResourceManager shutdown complete" << endl;
}

template <typename T>
std::shared_ptr<T> ResourceManager::GetCachedResource(
    const std::string &key,
    std::unordered_map<std::string, std::shared_ptr<T>> &cache,
    std::function<std::shared_ptr<T>()> loader) {

  lock_guard<mutex> lock(cache_mutex_);

  // Check if already cached
  auto it = cache.find(key);
  if (it != cache.end()) {
    return it->second;
  }

  // Load new resource
  auto resource = loader();
  if (resource) {
    cache[key] = resource;
  }

  return resource;
}

std::shared_ptr<Model>
ResourceManager::GetModel(const std::string &name, const std::string &modelPath,
                          const std::wstring &texturePath) {
  if (!initialized_) {
    SetError("ResourceManager not initialized");
    return nullptr;
  }

  return GetCachedResource<Model>(name, model_cache_, [&]() {
    auto model = make_shared<Model>();
    if (!model->Initialize(modelPath, texturePath, device_)) {
      SetError("Failed to initialize model '" + name + "' from " + modelPath);
      return shared_ptr<Model>();
    }
    cout << "Loaded model: " << name << endl;
    return model;
  });
}

std::shared_ptr<PBRModel> ResourceManager::GetPBRModel(
    const std::string &name, const std::string &modelPath,
    const std::string &albedoPath, const std::string &normalPath,
    const std::string &rmPath) {
  if (!initialized_) {
    SetError("ResourceManager not initialized");
    return nullptr;
  }

  return GetCachedResource<PBRModel>(name, pbr_model_cache_, [&]() {
    auto model = make_shared<PBRModel>();
    if (!model->Initialize(modelPath.c_str(), albedoPath, normalPath, rmPath,
                           device_)) {
      SetError("Failed to load PBR model '" + name + "' from " + modelPath);
      return shared_ptr<PBRModel>();
    }
    cout << "Loaded PBR model: " << name << endl;
    return model;
  });
}

std::shared_ptr<DDSTexture>
ResourceManager::GetTexture(const std::wstring &path) {
  Logger::SetModule("ResourceManager");
  if (!initialized_) {
    Logger::LogError("GetTexture - Not initialized");
    return nullptr;
  }

  lock_guard<mutex> lock(cache_mutex_);

  auto it = texture_cache_.find(path);
  if (it != texture_cache_.end()) {
    return it->second;
  }

  auto texture = make_shared<DDSTexture>();
  if (!texture->Initialize(path.c_str(), device_)) {
    Logger::LogError(L"Failed to load texture: " + path);
    return nullptr;
  }

  texture_cache_[path] = texture;
  wcout << L"Loaded texture: " << path << endl;
  return texture;
}

std::shared_ptr<TGATexture>
ResourceManager::GetTGATexture(const std::string &path) {
  Logger::SetModule("ResourceManager");
  if (!initialized_) {
    Logger::LogError("GetTGATexture - Not initialized");
    return nullptr;
  }

  lock_guard<mutex> lock(cache_mutex_);

  auto it = tga_texture_cache_.find(path);
  if (it != tga_texture_cache_.end()) {
    return it->second;
  }

  auto texture = make_shared<TGATexture>();
  if (!texture->Initialize(path.c_str(), device_)) {
    Logger::LogError("Failed to load TGA texture: " + path);
    return nullptr;
  }

  tga_texture_cache_[path] = texture;
  cout << "Loaded TGA texture: " << path << endl;
  return texture;
}

std::shared_ptr<IShader>
ResourceManager::CreateShader(const std::string &shaderType) {
  Logger::SetModule("ResourceManager");
  if (!initialized_) {
    Logger::LogError("CreateShader - Not initialized");
    return nullptr;
  }

  shared_ptr<IShader> shader;

  if (shaderType == "DepthShader") {
    shader = make_shared<DepthShader>();
  } else if (shaderType == "ShadowShader") {
    shader = make_shared<ShadowShader>();
  } else if (shaderType == "SoftShadowShader") {
    shader = make_shared<SoftShadowShader>();
  } else if (shaderType == "TextureShader") {
    shader = make_shared<TextureShader>();
  } else if (shaderType == "HorizontalBlurShader") {
    shader = make_shared<HorizontalBlurShader>();
  } else if (shaderType == "VerticalBlurShader") {
    shader = make_shared<VerticalBlurShader>();
  } else if (shaderType == "PbrShader") {
    shader = make_shared<PbrShader>();
  } else if (shaderType == "FontShader") {
    shader = make_shared<FontShader>();
  } else if (shaderType == "SceneLightShader") {
    shader = make_shared<SceneLightShader>();
  } else if (shaderType == "RefractionShader") {
    shader = make_shared<RefractionShader>();
  } else if (shaderType == "SimpleLightShader") {
    shader = make_shared<SimpleLightShader>();
  } else {
    Logger::LogError("Unknown shader type: " + shaderType);
    return nullptr;
  }

  if (!shader->Initialize(hwnd_, device_)) {
    Logger::LogError("Failed to initialize shader: " + shaderType);
    return nullptr;
  }

  cout << "Created shader: " << shaderType << endl;
  return shader;
}

std::shared_ptr<RenderTexture>
ResourceManager::CreateRenderTexture(const std::string &name, int width,
                                     int height, float depth, float nearPlane) {
  Logger::SetModule("ResourceManager");
  if (!initialized_) {
    Logger::LogError("CreateRenderTexture - Not initialized");
    return nullptr;
  }

  lock_guard<mutex> lock(cache_mutex_);

  auto renderTexture = make_shared<RenderTexture>();
  if (!renderTexture->Initialize(width, height, depth, nearPlane)) {
    Logger::LogError("Failed to create RenderTexture: " + name);
    return nullptr;
  }

  // Cache it for later retrieval
  render_texture_cache_[name] = renderTexture;

  cout << "Created RenderTexture: " << name << " (" << width << "x" << height
       << ")" << endl;
  return renderTexture;
}

std::shared_ptr<RenderTexture>
ResourceManager::GetRenderTexture(const std::string &name) const {
  lock_guard<mutex> lock(cache_mutex_);

  auto it = render_texture_cache_.find(name);
  if (it != render_texture_cache_.end()) {
    return it->second;
  }

  return nullptr;
}

std::shared_ptr<OrthoWindow>
ResourceManager::GetOrthoWindow(const std::string &name, int width,
                                int height) {
  Logger::SetModule("ResourceManager");
  if (!initialized_) {
    Logger::LogError("GetOrthoWindow - Not initialized");
    return nullptr;
  }

  return GetCachedResource<OrthoWindow>(name, ortho_window_cache_, [&]() {
    auto window = make_shared<OrthoWindow>();
    if (!window->Initialize(width, height)) {
      Logger::LogError("Failed to create OrthoWindow: " + name);
      return shared_ptr<OrthoWindow>();
    }
    cout << "Created OrthoWindow: " << name << " (" << width << "x" << height
         << ")" << endl;
    return window;
  });
}

void ResourceManager::LogResourceStats() const {
  lock_guard<mutex> lock(cache_mutex_);

  cout << "\n=== ResourceManager Statistics ===" << endl;
  cout << "Models cached: " << model_cache_.size() << endl;
  cout << "PBR Models cached: " << pbr_model_cache_.size() << endl;
  cout << "Textures cached: " << texture_cache_.size() << endl;
  cout << "TGA Textures cached: " << tga_texture_cache_.size() << endl;
  cout << "Shaders cached: " << shader_cache_.size() << endl;
  cout << "RenderTextures cached: " << render_texture_cache_.size() << endl;
  cout << "OrthoWindows cached: " << ortho_window_cache_.size() << endl;
  cout << "==================================\n" << endl;
}

void ResourceManager::ClearModelCache() {
  lock_guard<mutex> lock(cache_mutex_);
  model_cache_.clear();
  pbr_model_cache_.clear();
  cout << "Model cache cleared" << endl;
}

void ResourceManager::ClearShaderCache() {
  lock_guard<mutex> lock(cache_mutex_);
  shader_cache_.clear();
  cout << "Shader cache cleared" << endl;
}

void ResourceManager::ClearTextureCache() {
  lock_guard<mutex> lock(cache_mutex_);
  texture_cache_.clear();
  tga_texture_cache_.clear();
  cout << "Texture cache cleared" << endl;
}

void ResourceManager::ClearAllCaches() {
  lock_guard<mutex> lock(cache_mutex_);
  model_cache_.clear();
  pbr_model_cache_.clear();
  texture_cache_.clear();
  tga_texture_cache_.clear();
  shader_cache_.clear();
  render_texture_cache_.clear();
  ortho_window_cache_.clear();
  cout << "All caches cleared" << endl;
}

bool ResourceManager::HasModel(const std::string &name) const {
  lock_guard<mutex> lock(cache_mutex_);
  return model_cache_.find(name) != model_cache_.end();
}

bool ResourceManager::HasShader(const std::string &name) const {
  lock_guard<mutex> lock(cache_mutex_);
  return shader_cache_.find(name) != shader_cache_.end();
}

// Error handling helper
void ResourceManager::SetError(const std::string &error) {
  last_error_ = error;
  Logger::SetModule("ResourceManager");
  Logger::LogError(error);
}

// Reference counting methods
int ResourceManager::GetModelRefCount(const std::string &name) const {
  lock_guard<mutex> lock(cache_mutex_);
  auto it = model_cache_.find(name);
  if (it != model_cache_.end()) {
    return static_cast<int>(it->second.use_count());
  }
  return 0;
}

int ResourceManager::GetShaderRefCount(const std::string &name) const {
  lock_guard<mutex> lock(cache_mutex_);
  auto it = shader_cache_.find(name);
  if (it != shader_cache_.end()) {
    return static_cast<int>(it->second.use_count());
  }
  return 0;
}

int ResourceManager::GetTextureRefCount(const std::wstring &path) const {
  lock_guard<mutex> lock(cache_mutex_);
  auto it = texture_cache_.find(path);
  if (it != texture_cache_.end()) {
    return static_cast<int>(it->second.use_count());
  }
  return 0;
}

int ResourceManager::GetRenderTextureRefCount(const std::string &name) const {
  lock_guard<mutex> lock(cache_mutex_);
  auto it = render_texture_cache_.find(name);
  if (it != render_texture_cache_.end()) {
    return static_cast<int>(it->second.use_count());
  }
  return 0;
}

// Get unused resources
std::vector<std::string> ResourceManager::GetUnusedModels() const {
  lock_guard<mutex> lock(cache_mutex_);
  std::vector<std::string> unused;
  for (const auto &[name, model] : model_cache_) {
    if (model.use_count() == 1) { // Only held by ResourceManager
      unused.push_back(name);
    }
  }
  return unused;
}

std::vector<std::string> ResourceManager::GetUnusedShaders() const {
  lock_guard<mutex> lock(cache_mutex_);
  std::vector<std::string> unused;
  for (const auto &[name, shader] : shader_cache_) {
    if (shader.use_count() == 1) {
      unused.push_back(name);
    }
  }
  return unused;
}

std::vector<std::string> ResourceManager::GetUnusedRenderTextures() const {
  lock_guard<mutex> lock(cache_mutex_);
  std::vector<std::string> unused;
  for (const auto &[name, rtt] : render_texture_cache_) {
    if (rtt.use_count() == 1) {
      unused.push_back(name);
    }
  }
  return unused;
}

// Prune unused resources
size_t ResourceManager::PruneUnusedModels() {
  lock_guard<mutex> lock(cache_mutex_);
  size_t count = 0;
  for (auto it = model_cache_.begin(); it != model_cache_.end();) {
    if (it->second.use_count() == 1) {
      cout << "Pruning unused model: " << it->first << endl;
      it = model_cache_.erase(it);
      count++;
    } else {
      ++it;
    }
  }

  for (auto it = pbr_model_cache_.begin(); it != pbr_model_cache_.end();) {
    if (it->second.use_count() == 1) {
      cout << "Pruning unused PBR model: " << it->first << endl;
      it = pbr_model_cache_.erase(it);
      count++;
    } else {
      ++it;
    }
  }
  return count;
}

size_t ResourceManager::PruneUnusedShaders() {
  lock_guard<mutex> lock(cache_mutex_);
  size_t count = 0;
  for (auto it = shader_cache_.begin(); it != shader_cache_.end();) {
    if (it->second.use_count() == 1) {
      cout << "Pruning unused shader: " << it->first << endl;
      it = shader_cache_.erase(it);
      count++;
    } else {
      ++it;
    }
  }
  return count;
}

size_t ResourceManager::PruneUnusedRenderTextures() {
  lock_guard<mutex> lock(cache_mutex_);
  size_t count = 0;
  for (auto it = render_texture_cache_.begin();
       it != render_texture_cache_.end();) {
    if (it->second.use_count() == 1) {
      cout << "Pruning unused RenderTexture: " << it->first << endl;
      it = render_texture_cache_.erase(it);
      count++;
    } else {
      ++it;
    }
  }
  return count;
}

size_t ResourceManager::PruneAllUnusedResources() {
  size_t total = 0;
  cout << "\n=== Pruning Unused Resources ===" << endl;
  total += PruneUnusedModels();
  total += PruneUnusedShaders();
  total += PruneUnusedRenderTextures();
  cout << "Total resources pruned: " << total << endl;
  cout << "=================================\n" << endl;
  return total;
}

size_t ResourceManager::GetTotalCachedResources() const {
  lock_guard<mutex> lock(cache_mutex_);
  return model_cache_.size() + pbr_model_cache_.size() + texture_cache_.size() +
         tga_texture_cache_.size() + shader_cache_.size() +
         render_texture_cache_.size() + ortho_window_cache_.size();
}

// Explicit template instantiations
template std::shared_ptr<Model> ResourceManager::GetCachedResource<Model>(
    const std::string &,
    std::unordered_map<std::string, std::shared_ptr<Model>> &,
    std::function<std::shared_ptr<Model>()>);

template std::shared_ptr<PBRModel> ResourceManager::GetCachedResource<PBRModel>(
    const std::string &,
    std::unordered_map<std::string, std::shared_ptr<PBRModel>> &,
    std::function<std::shared_ptr<PBRModel>()>);

template std::shared_ptr<IShader> ResourceManager::GetCachedResource<IShader>(
    const std::string &,
    std::unordered_map<std::string, std::shared_ptr<IShader>> &,
    std::function<std::shared_ptr<IShader>()>);

template std::shared_ptr<OrthoWindow>
ResourceManager::GetCachedResource<OrthoWindow>(
    const std::string &,
    std::unordered_map<std::string, std::shared_ptr<OrthoWindow>> &,
    std::function<std::shared_ptr<OrthoWindow>()>);
