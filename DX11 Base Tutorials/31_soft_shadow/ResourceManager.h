#pragma once

#include <d3d11.h>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

// Forward declarations
class Model;
class PBRModel;
class DDSTexture;
class TGATexture;
class RenderTexture;
class OrthoWindow;
class IShader;
class DepthShader;
class ShadowShader;
class SoftShadowShader;
class FontShader;
class TextureShader;
class HorizontalBlurShader;
class VerticalBlurShader;
class PbrShader;
class SceneLightShader;
class RefractionShader;
class SimpleLightShader;

class ResourceManager {
public:
  // Singleton access
  static ResourceManager &GetInstance();

  // Initialization
  bool Initialize(ID3D11Device *device, ID3D11DeviceContext *context,
                  HWND hwnd);

  void Shutdown();

  // Model management
  std::shared_ptr<Model> GetModel(const std::string &name,
                                  const std::string &modelPath,
                                  const std::wstring &texturePath);

  std::shared_ptr<PBRModel> GetPBRModel(const std::string &name,
                                        const std::string &modelPath,
                                        const std::string &albedoPath,
                                        const std::string &normalPath,
                                        const std::string &rmPath);

  // Shader management - template specializations
  template <typename T> std::shared_ptr<T> GetShader(const std::string &name);

  // Texture management
  std::shared_ptr<DDSTexture> GetTexture(const std::wstring &path);

  std::shared_ptr<TGATexture> GetTGATexture(const std::string &path);

  // RenderTexture management (creates new each time)
  std::shared_ptr<RenderTexture> CreateRenderTexture(const std::string &name,
                                                     int width, int height,
                                                     float depth,
                                                     float nearPlane);

  // Get cached RenderTexture by name
  std::shared_ptr<RenderTexture>
  GetRenderTexture(const std::string &name) const;

  // OrthoWindow management
  std::shared_ptr<OrthoWindow> GetOrthoWindow(const std::string &name,
                                              int width, int height);

  // Resource statistics
  void LogResourceStats() const;

  // Clear specific cache
  void ClearModelCache();
  void ClearShaderCache();
  void ClearTextureCache();
  void ClearAllCaches();

  // Check if resource exists
  bool HasModel(const std::string &name) const;
  bool HasShader(const std::string &name) const;

  // Error information system
  const std::string &GetLastError() const { return last_error_; }
  void ClearError() { last_error_.clear(); }

  // Reference counting and memory management
  int GetModelRefCount(const std::string &name) const;
  int GetShaderRefCount(const std::string &name) const;
  int GetTextureRefCount(const std::wstring &path) const;
  int GetRenderTextureRefCount(const std::string &name) const;

  // List unused resources (ref count == 1, only held by ResourceManager)
  std::vector<std::string> GetUnusedModels() const;
  std::vector<std::string> GetUnusedShaders() const;
  std::vector<std::string> GetUnusedRenderTextures() const;

  // Prune unused resources to free memory
  size_t PruneUnusedModels();
  size_t PruneUnusedShaders();
  size_t PruneUnusedRenderTextures();
  size_t PruneAllUnusedResources();

  // Get total resource counts
  size_t GetTotalCachedResources() const;

  ID3D11Device *GetDevice() const { return device_; }
  ID3D11DeviceContext *GetContext() const { return context_; }
  HWND GetHWND() const { return hwnd_; }

private:
  ResourceManager() = default;
  ~ResourceManager() = default;

  // Prevent copying
  ResourceManager(const ResourceManager &) = delete;
  ResourceManager &operator=(const ResourceManager &) = delete;

  // Generic cache retrieval helper
  template <typename T>
  std::shared_ptr<T>
  GetCachedResource(const std::string &key,
                    std::unordered_map<std::string, std::shared_ptr<T>> &cache,
                    std::function<std::shared_ptr<T>()> loader);

  // Create shader instances
  std::shared_ptr<IShader> CreateShader(const std::string &shaderType);

  // Error handling helper
  void SetError(const std::string &error);

  // Device and context
  ID3D11Device *device_ = nullptr;
  ID3D11DeviceContext *context_ = nullptr;
  HWND hwnd_ = nullptr;

  // Resource caches
  std::unordered_map<std::string, std::shared_ptr<Model>> model_cache_;
  std::unordered_map<std::string, std::shared_ptr<PBRModel>> pbr_model_cache_;
  std::unordered_map<std::wstring, std::shared_ptr<DDSTexture>> texture_cache_;
  std::unordered_map<std::string, std::shared_ptr<TGATexture>>
      tga_texture_cache_;
  std::unordered_map<std::string, std::shared_ptr<IShader>> shader_cache_;
  std::unordered_map<std::string, std::shared_ptr<RenderTexture>>
      render_texture_cache_;
  std::unordered_map<std::string, std::shared_ptr<OrthoWindow>>
      ortho_window_cache_;

  // Error information
  std::string last_error_;

  // Thread safety (optional, for future use)
  mutable std::mutex cache_mutex_;

  // Initialization flag
  bool initialized_ = false;
};

// Template specializations for GetShader
template <>
inline std::shared_ptr<DepthShader>
ResourceManager::GetShader<DepthShader>(const std::string &name) {
  return std::dynamic_pointer_cast<DepthShader>(GetCachedResource<IShader>(
      name, shader_cache_, [this]() { return CreateShader("DepthShader"); }));
}

template <>
inline std::shared_ptr<ShadowShader>
ResourceManager::GetShader<ShadowShader>(const std::string &name) {
  return std::dynamic_pointer_cast<ShadowShader>(GetCachedResource<IShader>(
      name, shader_cache_, [this]() { return CreateShader("ShadowShader"); }));
}

template <>
inline std::shared_ptr<SoftShadowShader>
ResourceManager::GetShader<SoftShadowShader>(const std::string &name) {
  return std::dynamic_pointer_cast<SoftShadowShader>(
      GetCachedResource<IShader>(name, shader_cache_, [this]() {
        return CreateShader("SoftShadowShader");
      }));
}

template <>
inline std::shared_ptr<TextureShader>
ResourceManager::GetShader<TextureShader>(const std::string &name) {
  return std::dynamic_pointer_cast<TextureShader>(GetCachedResource<IShader>(
      name, shader_cache_, [this]() { return CreateShader("TextureShader"); }));
}

template <>
inline std::shared_ptr<HorizontalBlurShader>
ResourceManager::GetShader<HorizontalBlurShader>(const std::string &name) {
  return std::dynamic_pointer_cast<HorizontalBlurShader>(
      GetCachedResource<IShader>(name, shader_cache_, [this]() {
        return CreateShader("HorizontalBlurShader");
      }));
}

template <>
inline std::shared_ptr<VerticalBlurShader>
ResourceManager::GetShader<VerticalBlurShader>(const std::string &name) {
  return std::dynamic_pointer_cast<VerticalBlurShader>(
      GetCachedResource<IShader>(name, shader_cache_, [this]() {
        return CreateShader("VerticalBlurShader");
      }));
}

template <>
inline std::shared_ptr<PbrShader>
ResourceManager::GetShader<PbrShader>(const std::string &name) {
  return std::dynamic_pointer_cast<PbrShader>(GetCachedResource<IShader>(
      name, shader_cache_, [this]() { return CreateShader("PbrShader"); }));
}

template <>
inline std::shared_ptr<FontShader>
ResourceManager::GetShader<FontShader>(const std::string &name) {
  return std::dynamic_pointer_cast<FontShader>(GetCachedResource<IShader>(
      name, shader_cache_, [this]() { return CreateShader("FontShader"); }));
}

template <>
inline std::shared_ptr<SceneLightShader>
ResourceManager::GetShader<SceneLightShader>(const std::string &name) {
  return std::dynamic_pointer_cast<SceneLightShader>(
      GetCachedResource<IShader>(name, shader_cache_, [this]() {
        return CreateShader("SceneLightShader");
      }));
}

template <>
inline std::shared_ptr<RefractionShader>
ResourceManager::GetShader<RefractionShader>(const std::string &name) {
  return std::dynamic_pointer_cast<RefractionShader>(
      GetCachedResource<IShader>(name, shader_cache_, [this]() {
        return CreateShader("RefractionShader");
      }));
}

template <>
inline std::shared_ptr<SimpleLightShader>
ResourceManager::GetShader<SimpleLightShader>(const std::string &name) {
  return std::dynamic_pointer_cast<SimpleLightShader>(
      GetCachedResource<IShader>(name, shader_cache_, [this]() {
        return CreateShader("SimpleLightShader");
      }));
}
