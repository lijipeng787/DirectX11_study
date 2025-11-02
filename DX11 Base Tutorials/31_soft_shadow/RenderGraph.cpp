#include "RenderGraph.h"

#include "../CommonFramework2/DirectX11Device.h"
#include "Interfaces.h"
#include "RenderTexture.h"
#include "ResourceManager.h"
#include "ShaderParameterContainer.h"
#include "ShaderParameterValidator.h"

// do not remove these includes
#include "depthshader.h"
#include "fontshader.h"
#include "horizontalblurshader.h"
#include "pbrshader.h"
#include "shadowshader.h"
#include "softshadowshader.h"
#include "textureshader.h"
#include "verticalblurshader.h"
#include "scenelightshader.h"
#include "refractionshader.h"
#include "watershader.h"
#include "simplelightshader.h"
// !do not remove these includes

#include <algorithm>
#include <cctype>
#include <iostream>
#include <typeinfo>

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
RenderGraphPassBuilder &
RenderGraphPassBuilder::ReadAsParameter(const std::string &resource_name,
                                        const std::string &param_name) {
  // Add resource dependency
  pass_->input_resources_.push_back(resource_name);

  // Determine parameter name
  std::string final_param_name;
  if (!param_name.empty()) {
    // Use explicitly provided parameter name
    final_param_name = param_name;
  } else {
    // Use default conversion: ResourceName -> resourceNameTexture
    // Convert PascalCase resource name to camelCase parameter name with Texture
    // suffix
    final_param_name = resource_name;
    if (!final_param_name.empty()) {
      // Convert first letter to lowercase
      final_param_name[0] =
          static_cast<char>(std::tolower(final_param_name[0]));
      // Add Texture suffix if not already present
      if (final_param_name.size() < 7 ||
          final_param_name.substr(final_param_name.size() - 7) != "Texture") {
        final_param_name += "Texture";
      }
    }
  }

  // Store mapping for automatic binding during Compile()
  pass_->resource_to_param_mapping_[resource_name] = final_param_name;

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

ShaderParameterContainer RenderGraphPass::MergeParameters(
    const ShaderParameterContainer &global_params) const {
  // Merge parameters in priority order (later merges override earlier ones):
  // 1. Start with pass-specific parameters (lowest priority)
  //    - Includes manually set parameters via SetParameter/SetTexture
  //    - Includes auto-bound parameters from ReadAsParameter()
  // 2. Merge global parameters (override pass parameters)
  // 3. Note: input_textures_ is now only used for resource dependency tracking
  //    Actual parameter binding happens via pass_parameters_ (set during
  //    Compile)
  ShaderParameterContainer merged = *pass_parameters_;
  merged.Merge(global_params);
  // Note: input_textures_ binding is now handled via pass_parameters_ during
  // Compile() so we don't need to bind them here again
  return merged;
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

  // Merge pass, global, and input texture parameters
  ShaderParameterContainer merged = MergeParameters(global_params);

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

      // Build object-specific parameters:
      // 1. Copy merged pass/global parameters
      // 2. Add world matrix (per-object)
      // 3. Apply object callback (highest priority, can override anything)
      ShaderParameterContainer objParams = merged;
      objParams.Set("worldMatrix", r->GetWorldMatrix());
      if (auto cb = r->GetParameterCallback()) {
        cb(objParams);
      }
      r->Render(*shader_, objParams, device_context);
    }
  }

  if (disable_z_buffer_)
    DirectX11Device::GetD3d11DeviceInstance()->TurnZBufferOn();

  // Always restore back buffer render target after rendering to texture
  // This ensures consistent state for subsequent passes or text rendering
  if (output_texture_) {
    auto dx = DirectX11Device::GetD3d11DeviceInstance();
    dx->SetBackBufferRenderTarget();
    dx->ResetViewport();
    // Ensure Z buffer is on after restoring back buffer (for 3D rendering)
    dx->TurnZBufferOn();
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
  // Resolve resources and automatically bind to parameters if mapped.
  for (auto &pass : sorted_passes_) {
    for (auto &in : pass->input_resources_) {
      auto it = resources_.find(in);
      if (it == resources_.end() || !it->second.texture) {
        std::cerr << "RenderGraph Compile Error: missing input resource " << in
                  << " for pass " << pass->GetName() << std::endl;
        return false;
      }
      pass->input_textures_[in] = it->second.texture;

      // Automatically bind resource to parameter if mapping exists
      auto mapping_it = pass->resource_to_param_mapping_.find(in);
      if (mapping_it != pass->resource_to_param_mapping_.end()) {
        const std::string &param_name = mapping_it->second;
        pass->pass_parameters_->SetTexture(
            param_name, it->second.texture->GetShaderResourceView());
      }
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

  // Validate parameters if enabled
  if (enable_parameter_validation_ && parameter_validator_) {
    for (auto &pass : sorted_passes_) {
      if (!ValidatePassParameters(pass)) {
        std::cerr << "RenderGraph Compile Error: parameter validation failed "
                     "for pass "
                  << pass->GetName() << std::endl;
        return false;
      }
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
  ExecutePasses(renderables, global_params, backDepthCleared);
  DirectX11Device::GetD3d11DeviceInstance()->EndScene();
}

void RenderGraph::ExecutePasses(
    std::vector<std::shared_ptr<IRenderable>> &renderables,
    const ShaderParameterContainer &global_params,
    bool &back_buffer_depth_cleared) {
  if (!compiled_) {
    std::cerr << "RenderGraph ExecutePasses Error: not compiled" << std::endl;
    return;
  }
  for (auto &p : sorted_passes_)
    p->Execute(renderables, global_params, context_, back_buffer_depth_cleared);
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
  std::cout << "\n=== RenderGraph Debug ===" << std::endl;
  // Resources summary.
  std::cout << "Resources:" << std::endl;
  std::vector<std::string> unusedResources;
  for (auto &kv : resources_) {
    const std::string &rname = kv.first;
    const auto &res = kv.second;
    const char *producer = "(imported)";
    for (auto &p : passes_) {
      if (p->output_resource_ == rname) {
        producer = p->GetName().c_str();
        break;
      }
    }
    std::vector<std::string> consumers;
    consumers.reserve(passes_.size());
    for (auto &p : passes_) {
      for (auto &in : p->input_resources_)
        if (in == rname) {
          consumers.push_back(p->GetName());
          break;
        }
    }
    if (producer == "(imported)" && consumers.empty())
      unusedResources.push_back(rname);
    std::cout << "  - " << rname << (res.texture ? " [OK]" : " [MISSING]")
              << (res.is_external ? " (external)" : "")
              << " | producer: " << producer << " | consumers: ";
    if (consumers.empty())
      std::cout << "none";
    else {
      for (size_t i = 0; i < consumers.size(); ++i) {
        if (i)
          std::cout << ",";
        std::cout << consumers[i];
      }
    }
    std::cout << std::endl;
  }
  if (!unusedResources.empty()) {
    std::cout << "Unused resources:" << std::endl;
    for (auto &r : unusedResources)
      std::cout << "  * " << r << std::endl;
  }
  // Passes detailed.
  std::cout << "\nPasses (execution order):" << std::endl;
  std::vector<std::string> isolatedPasses;
  std::vector<std::string> unusedOutputs;
  // Precompute future input usage.
  std::unordered_map<std::string, int> firstConsumerIndex;
  for (size_t i = 0; i < sorted_passes_.size(); ++i) {
    for (auto &in : sorted_passes_[i]->input_resources_) {
      if (!firstConsumerIndex.count(in))
        firstConsumerIndex[in] = (int)i;
    }
  }
  for (size_t i = 0; i < sorted_passes_.size(); ++i) {
    auto &p = sorted_passes_[i];
    bool hasResolvedInput = false;
    for (auto &in : p->input_resources_)
      if (p->input_textures_.count(in)) {
        hasResolvedInput = true;
        break;
      }
    bool hasOutput = !p->output_resource_.empty();
    bool outputUsedLater = false;
    if (hasOutput) {
      for (size_t j = i + 1; j < sorted_passes_.size(); ++j) {
        for (auto &inLater : sorted_passes_[j]->input_resources_) {
          if (inLater == p->output_resource_) {
            outputUsedLater = true;
            break;
          }
        }
        if (outputUsedLater)
          break;
      }
      if (!outputUsedLater)
        unusedOutputs.push_back(p->GetName() + " -> " + p->output_resource_);
    }
    if (!hasResolvedInput && !hasOutput)
      isolatedPasses.push_back(p->GetName());
    std::cout << "  [" << i << "] " << p->GetName();
    if (!p->input_resources_.empty()) {
      std::cout << " inputs={";
      for (size_t j = 0; j < p->input_resources_.size(); ++j) {
        if (j)
          std::cout << ",";
        auto &in = p->input_resources_[j];
        bool resolved = p->input_textures_.count(in) > 0;
        std::cout << in << (resolved ? "" : "(UNRESOLVED)");
      }
      std::cout << "}";
    }
    if (!p->output_resource_.empty()) {
      std::cout << " output=" << p->output_resource_
                << (p->output_texture_ ? "" : "(UNRESOLVED)");
      if (!outputUsedLater)
        std::cout << " (UNUSED)";
    }
    if (!p->render_tags_.empty()) {
      std::cout << " tags={";
      for (size_t j = 0; j < p->render_tags_.size(); ++j) {
        if (j)
          std::cout << ",";
        std::cout << p->render_tags_[j];
      }
      std::cout << "}";
    }
    if (p->disable_z_buffer_)
      std::cout << " ZDisabled";
    if (!hasResolvedInput && hasOutput)
      std::cout << " [WARNING: writes without inputs]";
    std::cout << std::endl;
  }
  if (!isolatedPasses.empty()) {
    std::cout << "Isolated passes (no inputs & no outputs):" << std::endl;
    for (auto &n : isolatedPasses)
      std::cout << "  * " << n << std::endl;
  }
  if (!unusedOutputs.empty()) {
    std::cout << "Unused pass outputs (never read by later passes):"
              << std::endl;
    for (auto &u : unusedOutputs)
      std::cout << "  * " << u << std::endl;
  }
  std::cout << "==========================\n" << std::endl;
}

bool RenderGraph::ValidatePassParameters(
    std::shared_ptr<RenderGraphPass> &pass) const {
  if (!parameter_validator_ || !pass->shader_) {
    return true; // Skip validation if no validator or no shader
  }

  // Collect all parameter names from pass
  // Note: Only parameters explicitly set via SetParameter/SetTexture are
  // validated Input textures (from Read()) are resources, not parameters by
  // themselves
  std::unordered_set<std::string> provided_params;

  // Add pass parameters (set via SetParameter/SetTexture in builder)
  auto pass_param_names = pass->pass_parameters_->GetAllParameterNames();
  for (const auto &name : pass_param_names) {
    provided_params.insert(name);
  }

  // Note: input_textures_ contains resource names as keys, not parameter names
  // They are only used for binding textures in MergeParameters()
  // Actual parameter names come from pass_parameters_ (set via SetTexture)

  // Get shader name from type
  // Extract clean class name from typeid (handles MSVC/GCC name mangling)
  std::string shader_name = typeid(*pass->shader_).name();

  // MSVC: Remove "class " prefix if present
  if (shader_name.find("class ") == 0) {
    shader_name = shader_name.substr(6);
  }
  // GCC/Clang: Handle name mangling (starts with length prefix)
  else if (shader_name.length() > 0 && std::isdigit(shader_name[0])) {
    // Skip length prefix in mangled name (e.g., "13DepthShader" ->
    // "DepthShader")
    size_t i = 0;
    while (i < shader_name.length() && std::isdigit(shader_name[i])) {
      i++;
    }
    shader_name = shader_name.substr(i);
  }

  // Remove any remaining prefixes/suffixes that might exist
  size_t pos = shader_name.find_last_of(" ");
  if (pos != std::string::npos) {
    shader_name = shader_name.substr(pos + 1);
  }

  // Validate parameters
  return parameter_validator_->ValidatePassParameters(
      pass->GetName(), shader_name, provided_params,
      parameter_validator_->GetValidationMode());
}

void RenderGraph::AllocateResources() { /* simplified - allocation handled
                                           externally via ImportTexture */
}
