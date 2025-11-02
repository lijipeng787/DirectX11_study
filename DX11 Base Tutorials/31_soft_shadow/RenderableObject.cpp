#include "RenderableObject.h"

#include "BoundingVolume.h"
#include "Interfaces.h"
#include "Model.h"
#include "orthowindow.h"

using namespace DirectX;

RenderableObject::RenderableObject(std::shared_ptr<Model> model,
                                   std::shared_ptr<IShader> shader)
    : model_(model), shader_(shader), world_matrix_(XMMatrixIdentity()) {}

RenderableObject::RenderableObject(std::shared_ptr<PBRModel> model,
                                   std::shared_ptr<IShader> shader)
    : pbr_model_(model), shader_(shader), world_matrix_(XMMatrixIdentity()),
      is_pbr_model_(true) {}

RenderableObject::RenderableObject(std::shared_ptr<OrthoWindow> windowModel,
                                   std::shared_ptr<IShader> shader)
    : window_model_(windowModel), shader_(shader), is_window_model_(true) {}

void RenderableObject::Render(const IShader &shader,
                              const ShaderParameterContainer &parameters,
                              ID3D11DeviceContext *deviceContext) const {

  ShaderParameterContainer combinedParams = parameters;
  combinedParams.Merge(object_parameters_);

  if (is_window_model_)
    window_model_->Render(shader, combinedParams, deviceContext);
  else {
    if (is_pbr_model_)
      pbr_model_->Render(shader, combinedParams, deviceContext);
    else
      model_->Render(shader, combinedParams, deviceContext);
  }
}

void RenderableObject::SetParameterCallback(ShaderParameterCallback callback) {
  parameter_callback_ = callback;
}

ShaderParameterCallback RenderableObject::GetParameterCallback() const {
  return parameter_callback_;
}

XMMATRIX RenderableObject::GetWorldMatrix() const noexcept {
  return world_matrix_;
}

void RenderableObject::SetObjectParameters(
    const ShaderParameterContainer &params) {
  object_parameters_ = params;
}

void RenderableObject::SetWorldMatrix(const XMMATRIX &worldMatrix) {
  world_matrix_ = worldMatrix;
}

BoundingVolume RenderableObject::GetWorldBoundingVolume() const {
  if (model_) {
    BoundingVolume localBounds = model_->GetLocalBoundingVolume();
    return localBounds.Transform(world_matrix_);
  }

  BoundingVolume emptyBounds;
  return emptyBounds;
}