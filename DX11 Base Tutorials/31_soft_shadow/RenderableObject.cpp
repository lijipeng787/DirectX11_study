#include "RenderableObject.h"
#include "IShader.h"
#include "ModelClass.h"
#include "orthowindowclass.h"

using namespace DirectX;

RenderableObject::RenderableObject(std::shared_ptr<ModelClass> model,
                                   std::shared_ptr<IShader> shader)
    : model_(model), shader_(shader), world_matrix_(XMMatrixIdentity()) {}

RenderableObject::RenderableObject(
    std::shared_ptr<OrthoWindowClass> windowModel,
    std::shared_ptr<IShader> shader)
    : window_model_(windowModel), shader_(shader), is_window_model_(true) {}

void RenderableObject::Render(
    const IShader &shader, const ShaderParameterContainer &parameters) const {

  ShaderParameterContainer combinedParams = parameters;
  combinedParams.Merge(object_parameters_);

  if (is_window_model_)
    window_model_->Render(shader, combinedParams);
  else
    model_->Render(shader, combinedParams);
}

void RenderableObject::SetParameterCallback(ShaderParameterCallback callback) {
  parameter_callback_ = callback;
}

ShaderParameterCallback RenderableObject::GetParameterCallback() const {
  return parameter_callback_;
}

XMMATRIX RenderableObject::GetWorldMatrix() const { return world_matrix_; }

void RenderableObject::SetObjectParameters(
    const ShaderParameterContainer &params) {
  object_parameters_ = params;
}

void RenderableObject::SetWorldMatrix(const XMMATRIX &worldMatrix) {
  world_matrix_ = worldMatrix;
}