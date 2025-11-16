#pragma once

#include <DirectXMath.h>
#include <Windows.h>
#include <d3d11.h>
#include <memory>
#include <unordered_set>
#include <vector>

#include "ShaderParameter.h"

// ============================================================================
// Render Interfaces
// ============================================================================

// Forward declarations
struct ID3D11DeviceContext;

// ============================================================================
// IShader - Shader Interface
// ============================================================================

class IShader {
public:
  IShader() = default;

  virtual ~IShader() = default;

  virtual bool Initialize(HWND hwnd, ID3D11Device *device) = 0;

  virtual void Shutdown() = 0;

  virtual bool Render(int indexCount,
                      const ShaderParameterContainer &parameters,
                      ID3D11DeviceContext *deviceContext) const = 0;
};

// ============================================================================
// IRenderable - Renderable Object Interface
// ============================================================================

class IRenderable {
public:
  IRenderable() = default;

  virtual ~IRenderable() = default;

public:
  virtual void Render(const IShader &shader,
                      const ShaderParameterContainer &parameterContainer,
                      ID3D11DeviceContext *deviceContext) const = 0;

  virtual DirectX::XMMATRIX GetWorldMatrix() const noexcept = 0;

  virtual void SetWorldMatrix(const DirectX::XMMATRIX &worldMatrix) = 0;

  virtual void SetParameterCallback(ShaderParameterCallback callback) = 0;

  virtual ShaderParameterCallback GetParameterCallback() const = 0;

  virtual const ShaderParameterContainer &GetObjectParameters() const {
    static ShaderParameterContainer empty_parameters;
    return empty_parameters;
  }

public:
  void AddTag(const std::string &tag) { tags_.insert(tag); }

  void RemoveTag(const std::string &tag) { tags_.erase(tag); }

  bool HasTag(const std::string &tag) const {
    return tags_.find(tag) != tags_.end();
  }

  const std::unordered_set<std::string> &GetTags() const { return tags_; }

private:
  std::unordered_set<std::string> tags_;
};

// ============================================================================
// IRenderGroup - Render Group Interface
// ============================================================================

class IRenderGroup {
public:
  IRenderGroup() = default;

  virtual ~IRenderGroup() = default;

public:
  virtual void PreRender() = 0;

  virtual void PostRender() = 0;

  virtual const std::vector<std::shared_ptr<IRenderable>> &
  GetRenderables() const = 0;
};

// ============================================================================
// StandardRenderGroup - Default Render Group Implementation
// ============================================================================

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

private:
  std::vector<std::shared_ptr<IRenderable>> renderables_;
};
