#pragma once

#include "BoundingVolume.h"
#include "Interfaces.h"

#include <memory>

class Model;
class PBRModel;
class IShader;
class OrthoWindow;

class RenderableObject : public IRenderable {
public:
  explicit RenderableObject(std::shared_ptr<Model> model,
                            std::shared_ptr<IShader> shader);

  explicit RenderableObject(std::shared_ptr<PBRModel> model,
                            std::shared_ptr<IShader> shader);

  explicit RenderableObject(std::shared_ptr<OrthoWindow> window_model,
                            std::shared_ptr<IShader> shader);

  RenderableObject(RenderableObject &) noexcept = default;

  RenderableObject &operator=(RenderableObject &) noexcept = default;

  RenderableObject(RenderableObject &&) noexcept = default;

  RenderableObject &operator=(RenderableObject &&) noexcept = default;

  virtual ~RenderableObject() = default;

public:
  void Render(const IShader &shader, const ShaderParameterContainer &parameters,
              ID3D11DeviceContext *deviceContext) const override;

  DirectX::XMMATRIX GetWorldMatrix() const noexcept override;

  void SetParameterCallback(ShaderParameterCallback callback) override;

  ShaderParameterCallback GetParameterCallback() const override;

public:
  void SetObjectParameters(const ShaderParameterContainer &params);

  void SetWorldMatrix(const DirectX::XMMATRIX &worldMatrix);

  // Get world-space bounding volume (for frustum culling)
  BoundingVolume GetWorldBoundingVolume() const;

private:
  std::shared_ptr<Model> model_;
  std::shared_ptr<PBRModel> pbr_model_;

  std::shared_ptr<OrthoWindow> window_model_;

  bool is_window_model_ = false;
  bool is_pbr_model_ = false;

  std::shared_ptr<IShader> shader_;

  ShaderParameterContainer object_parameters_;

  DirectX::XMMATRIX world_matrix_ = DirectX::XMMatrixIdentity();

  ShaderParameterCallback parameter_callback_;
};