#include "RenderEntity.h"
#include "Interfaces.h"
#include "Model.h"
#include "orthowindow.h"

using namespace DirectX;

RenderEntity::RenderEntity(std::shared_ptr<Model> model,
                           std::shared_ptr<IShader> shader)
    : model_(model), shader_(shader), world_matrix_(XMMatrixIdentity()) {}

RenderEntity::RenderEntity(std::shared_ptr<PBRModel> model,
                           std::shared_ptr<IShader> shader)
    : pbr_model_(model), shader_(shader), world_matrix_(XMMatrixIdentity()),
      is_pbr_model_(true) {}

RenderEntity::RenderEntity(std::shared_ptr<OrthoWindow> windowModel,
                           std::shared_ptr<IShader> shader)
    : window_model_(windowModel), shader_(shader), is_window_model_(true) {}

void RenderEntity::Render(const IShader &shader,
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

void RenderEntity::SetParameterCallback(ShaderParameterCallback callback) {
  parameter_callback_ = callback;
}

ShaderParameterCallback RenderEntity::GetParameterCallback() const {
  return parameter_callback_;
}

XMMATRIX RenderEntity::GetWorldMatrix() const noexcept { return world_matrix_; }

void RenderEntity::SetObjectParameters(const ShaderParameterContainer &params) {
  object_parameters_ = params;
}

void RenderEntity::SetWorldMatrix(const XMMATRIX &worldMatrix) {
  world_matrix_ = worldMatrix;
}