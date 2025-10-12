#include "RenderGraph.h"

#include "../CommonFramework2/DirectX11Device.h"
#include "IRenderable.h"
#include "IShader.h"
#include "RenderTexture.h"
#include "ResourceManager.h"
#include "ShaderParameterContainer.h"

// do not remove these includes
#include "depthshader.h"
#include "horizontalblurshader.h"
#include "pbrshader.h"
#include "shadowshader.h"
#include "softshadowshader.h"
#include "textureshader.h"
#include "verticalblurshader.h"
// !do not remove these includes

#include <algorithm>
#include <iostream>

using namespace std;

// RenderPassContext helpers
std::shared_ptr<RenderTexture>
RenderPassContext::GetInput(const std::string &name) {
  if (!inputs_)
    return nullptr;
  auto it = inputs_->find(name);
  return it != inputs_->end() ? it->second : nullptr;
}
std::shared_ptr<RenderTexture> RenderPassContext::GetOutput() {
  return output_;
}

// Builder implementation
RenderGraphPassBuilder &
RenderGraphPassBuilder::SetShader(std::shared_ptr<IShader> shader) {
  pass_->shader_ = shader;
  return *this;
}
RenderGraphPassBuilder &
RenderGraphPassBuilder::Read(const std::string &resource_name) {
  pass_->input_resources_.push_back(resource_name);
  return *this;
}
RenderGraphPassBuilder &
RenderGraphPassBuilder::Write(const std::string &resource_name) {
  pass_->output_resource_ = resource_name;
  return *this;
}
RenderGraphPassBuilder &
RenderGraphPassBuilder::AddRenderTag(const std::string &tag) {
  pass_->render_tags_.push_back(tag);
  return *this;
}
RenderGraphPassBuilder &RenderGraphPassBuilder::DisableZBuffer(bool disable) {
  pass_->disable_z_buffer_ = disable;
  return *this;
}
RenderGraphPassBuilder &
RenderGraphPassBuilder::SetParameter(const std::string &name,
                                     const DirectX::XMMATRIX &value) {
  pass_->pass_parameters_->SetMatrix(name, value);
  return *this;
}
RenderGraphPassBuilder &
RenderGraphPassBuilder::SetParameter(const std::string &name, float value) {
  pass_->pass_parameters_->SetFloat(name, value);
  return *this;
}
RenderGraphPassBuilder &
RenderGraphPassBuilder::SetParameter(const std::string &name,
                                     const DirectX::XMFLOAT3 &value) {
  pass_->pass_parameters_->SetVector3(name, value);
  return *this;
}
RenderGraphPassBuilder &
RenderGraphPassBuilder::SetParameter(const std::string &name,
                                     const DirectX::XMFLOAT4 &value) {
  pass_->pass_parameters_->SetVector4(name, value);
  return *this;
}
RenderGraphPassBuilder &
RenderGraphPassBuilder::SetTexture(const std::string &name,
                                   ID3D11ShaderResourceView *srv) {
  pass_->pass_parameters_->SetTexture(name, srv);
  return *this;
}
RenderGraphPassBuilder &RenderGraphPassBuilder::Execute(ExecuteFunc func) {
  pass_->custom_execute_ = func;
  return *this;
}

// Pass implementation
RenderGraphPass::RenderGraphPass(const std::string &name) : name_(name) {
  pass_parameters_ = std::make_shared<ShaderParameterContainer>();
}
RenderGraphPassBuilder RenderGraphPass::GetBuilder() {
  return RenderGraphPassBuilder(this);
}

void RenderGraphPass::Execute(
    std::vector<std::shared_ptr<IRenderable>> &renderables,
    const ShaderParameterContainer &global_params,
    ID3D11DeviceContext *device_context, bool &back_buffer_depth_cleared) {
  // Set target or back buffer.
  if (output_texture_) {
    output_texture_->SetRenderTarget();
    output_texture_->ClearRenderTarget(0.0f, 0.0f, 0.0f, 1.0f);
  } else {
    auto dx = DirectX11Device::GetD3d11DeviceInstance();
    dx->SetBackBufferRenderTarget();
    if (!back_buffer_depth_cleared)
      back_buffer_depth_cleared = true; // depth cleared by BeginScene.
  }

  ShaderParameterContainer merged =
      *pass_parameters_; // pass first, then global override.
  merged.Merge(global_params);
  for (auto &kv : input_textures_)
    merged.SetTexture(kv.first, kv.second->GetShaderResourceView());

  if (disable_z_buffer_)
    DirectX11Device::GetD3d11DeviceInstance()->TurnZBufferOff();

  if (custom_execute_) {
    RenderPassContext ctx;
    ctx.shader = shader_;
    ctx.renderables = &renderables;
    ctx.global_params = const_cast<ShaderParameterContainer *>(&global_params);
    ctx.pass_params = pass_parameters_.get();
    ctx.device_context = device_context;
    ctx.inputs_ = &input_textures_;
    ctx.output_ = output_texture_;
    custom_execute_(ctx);
  } else {
    for (auto &r : renderables) {
      bool draw = render_tags_.empty();
      if (!draw) {
        for (auto &t : r->GetTags()) {
          if (std::find(render_tags_.begin(), render_tags_.end(), t) !=
              render_tags_.end()) {
            draw = true;
            break;
          }
        }
      }
      if (!draw)
        continue;
      ShaderParameterContainer objParams = merged;
      objParams.Set("worldMatrix", r->GetWorldMatrix());
      if (auto cb = r->GetParameterCallback())
        cb(objParams);
      r->Render(*shader_, objParams, device_context);
    }
  }

  if (disable_z_buffer_)
    DirectX11Device::GetD3d11DeviceInstance()->TurnZBufferOn();
  if (output_texture_) {
    auto dx = DirectX11Device::GetD3d11DeviceInstance();
    dx->SetBackBufferRenderTarget();
    dx->ResetViewport();
  }
}

// Graph implementation
void RenderGraph::Initialize(ID3D11Device *device,
                             ID3D11DeviceContext *context) {
  device_ = device;
  context_ = context;
}

void RenderGraph::DeclareTexture(const std::string &name, int width, int height,
                                 float depth, float nearPlane) {
  // Currently unused simplified path; keep placeholder for future.
  (void)depth;
  (void)nearPlane; // silence unused warnings.
  GraphResource res;
  res.name = name;
  res.is_external = false;
  resources_[name] = res;
}

void RenderGraph::ImportTexture(const std::string &name,
                                std::shared_ptr<RenderTexture> texture) {
  GraphResource res;
  res.name = name;
  res.texture = texture;
  res.is_external = true;
  resources_[name] = res;
}

RenderGraphPassBuilder RenderGraph::AddPass(const std::string &name) {
  auto pass = std::make_shared<RenderGraphPass>(name);
  passes_.push_back(pass);
  return pass->GetBuilder();
}

bool RenderGraph::Compile() {
  // Simple: execution order == declaration order.
  sorted_passes_ = passes_;
  // Resolve resources.
  for (auto &pass : sorted_passes_) {
    for (auto &in : pass->input_resources_) {
      auto it = resources_.find(in);
      if (it == resources_.end() || !it->second.texture) {
        std::cerr << "RenderGraph Compile Error: missing input resource " << in
                  << " for pass " << pass->GetName() << std::endl;
        return false;
      }
      pass->input_textures_[in] = it->second.texture;
    }
    if (!pass->output_resource_.empty()) {
      auto it = resources_.find(pass->output_resource_);
      if (it == resources_.end() || !it->second.texture) {
        std::cerr << "RenderGraph Compile Error: missing output resource "
                  << pass->output_resource_ << " for pass " << pass->GetName()
                  << std::endl;
        return false;
      }
      pass->output_texture_ = it->second.texture;
    }
  }
  compiled_ = true;
  return true;
}

void RenderGraph::Execute(
    std::vector<std::shared_ptr<IRenderable>> &renderables,
    const ShaderParameterContainer &global_params) {
  if (!compiled_) {
    std::cerr << "RenderGraph Execute Error: not compiled" << std::endl;
    return;
  }
  DirectX11Device::GetD3d11DeviceInstance()->BeginScene(0, 0, 0, 1);
  bool backDepthCleared = false;
  for (auto &p : sorted_passes_)
    p->Execute(renderables, global_params, context_, backDepthCleared);
  DirectX11Device::GetD3d11DeviceInstance()->EndScene();
}

std::shared_ptr<RenderTexture>
RenderGraph::GetTexture(const std::string &name) const {
  auto it = resources_.find(name);
  return it != resources_.end() ? it->second.texture : nullptr;
}

void RenderGraph::Clear() {
  passes_.clear();
  sorted_passes_.clear();
  resources_.clear();
  compiled_ = false;
}

void RenderGraph::PrintGraph() const {
  std::cout << "RenderGraph passes: " << sorted_passes_.size() << std::endl;
}

void RenderGraph::AllocateResources() { /* simplified - allocation handled
                                           externally via ImportTexture */
}
