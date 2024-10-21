#pragma once

#include "IRenderable.h"

#include <memory>

class ModelClass;
class PBRModelClass;
class IShader;
class OrthoWindowClass;

class RenderableObject : public IRenderable {
public:
  RenderableObject(std::shared_ptr<ModelClass> model,
                   std::shared_ptr<IShader> shader);

  RenderableObject(std::shared_ptr<PBRModelClass> model,
                   std::shared_ptr<IShader> shader);

  RenderableObject(std::shared_ptr<OrthoWindowClass> window_model,
                   std::shared_ptr<IShader> shader);

public:
  void Render(const IShader &shader,
              const ShaderParameterContainer &parameters) const override;

  DirectX::XMMATRIX GetWorldMatrix() const override;

  void SetParameterCallback(ShaderParameterCallback callback) override;

  ShaderParameterCallback GetParameterCallback() const override;

public:
  void SetObjectParameters(const ShaderParameterContainer &params);

  void SetWorldMatrix(const DirectX::XMMATRIX &worldMatrix);

private:
  std::shared_ptr<ModelClass> model_;
  std::shared_ptr<PBRModelClass> pbr_model_;

  std::shared_ptr<OrthoWindowClass> window_model_;

  bool is_window_model_ = false;
  bool is_pbr_model_ = false;

  std::shared_ptr<IShader> shader_;

  ShaderParameterContainer object_parameters_;

  DirectX::XMMATRIX world_matrix_ = DirectX::XMMatrixIdentity();

  ShaderParameterCallback parameter_callback_;
};