#pragma once

#include <unordered_set>

#include <DirectXMath.h>

#include "ShaderParameterCallback.h"

class IShader;

class IRenderable {
public:
  IRenderable() = default;

  virtual ~IRenderable() = default;

public:
  virtual void
  Render(const IShader &shader,
         const ShaderParameterContainer &parameterContainer) const = 0;

  virtual DirectX::XMMATRIX GetWorldMatrix() const = 0;

  virtual void SetParameterCallback(ShaderParameterCallback callback) = 0;

  virtual ShaderParameterCallback GetParameterCallback() const = 0;

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