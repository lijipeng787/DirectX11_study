#pragma once

#include <DirectXMath.h>
#include <d3d11.h>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class RenderTexture;
class IShader;
class IRenderable;
class ShaderParameterContainer;
class ShaderParameterValidator;

// Minimal resource record used by current implementation.
struct GraphResource {
  std::string name;
  std::shared_ptr<RenderTexture> texture; // Provided or allocated.
  bool is_external = false;               // Imported from ResourceManager.
};

class RenderGraphPass;

struct RenderPassContext {
  std::shared_ptr<IShader> shader;
  std::vector<std::shared_ptr<IRenderable>> *renderables;
  ShaderParameterContainer *global_params;
  ShaderParameterContainer *pass_params;
  ID3D11DeviceContext *device_context;

  std::shared_ptr<RenderTexture> GetInput(const std::string &name);
  std::shared_ptr<RenderTexture> GetOutput();
  const std::unordered_map<std::string, std::shared_ptr<RenderTexture>> &
  GetInputs() const {
    return *inputs_;
  }

private:
  friend class RenderGraphPass;
  std::unordered_map<std::string, std::shared_ptr<RenderTexture>> *inputs_{};
  std::shared_ptr<RenderTexture> output_{};
};

class RenderGraphPassBuilder;

class RenderGraphPass {
public:
  explicit RenderGraphPass(const std::string &name);
  const std::string &GetName() const { return name_; }
  RenderGraphPassBuilder GetBuilder();

  void Execute(std::vector<std::shared_ptr<IRenderable>> &renderables,
               const ShaderParameterContainer &global_params,
               ID3D11DeviceContext *device_context,
               bool &back_buffer_depth_cleared);

private:
  friend class RenderGraphPassBuilder;
  friend class RenderGraph;

  // Merge parameters in priority order: pass -> global -> input textures
  // Returns merged parameter container ready for object-level customization
  ShaderParameterContainer
  MergeParameters(const ShaderParameterContainer &global_params) const;

  std::string name_;
  std::shared_ptr<IShader> shader_;
  std::vector<std::string> input_resources_;
  std::string output_resource_;
  std::vector<std::string> render_tags_;
  std::shared_ptr<ShaderParameterContainer> pass_parameters_;
  bool disable_z_buffer_ = false;

  using ExecuteFunc = std::function<void(RenderPassContext &)>;
  ExecuteFunc custom_execute_;

  std::unordered_map<std::string, std::shared_ptr<RenderTexture>>
      input_textures_;
  std::shared_ptr<RenderTexture> output_texture_;
};

class RenderGraphPassBuilder {
public:
  RenderGraphPassBuilder(RenderGraphPass *pass) : pass_(pass) {}
  RenderGraphPassBuilder &SetShader(std::shared_ptr<IShader> shader);
  RenderGraphPassBuilder &Read(const std::string &resource_name);
  RenderGraphPassBuilder &Write(const std::string &resource_name);
  RenderGraphPassBuilder &AddRenderTag(const std::string &tag);
  RenderGraphPassBuilder &DisableZBuffer(bool disable = true);
  RenderGraphPassBuilder &SetParameter(const std::string &name,
                                       const DirectX::XMMATRIX &value);
  RenderGraphPassBuilder &SetParameter(const std::string &name, float value);
  RenderGraphPassBuilder &SetParameter(const std::string &name,
                                       const DirectX::XMFLOAT3 &value);
  RenderGraphPassBuilder &SetParameter(const std::string &name,
                                       const DirectX::XMFLOAT4 &value);
  RenderGraphPassBuilder &SetTexture(const std::string &name,
                                     ID3D11ShaderResourceView *srv);
  using ExecuteFunc = std::function<void(RenderPassContext &)>;
  RenderGraphPassBuilder &Execute(ExecuteFunc func);

private:
  RenderGraphPass *pass_;
};

class RenderGraph {
public:
  void Initialize(ID3D11Device *device, ID3D11DeviceContext *context);
  void DeclareTexture(
      const std::string &name, int width, int height, float depth = 100.0f,
      float nearPlane = 0.1f); // Currently unused but kept for future.
  void ImportTexture(const std::string &name,
                     std::shared_ptr<RenderTexture> texture);
  RenderGraphPassBuilder AddPass(const std::string &name);
  bool Compile();
  void Execute(std::vector<std::shared_ptr<IRenderable>> &renderables,
               const ShaderParameterContainer &global_params);
  std::shared_ptr<RenderTexture> GetTexture(const std::string &name) const;
  void Clear();
  void PrintGraph() const; // Detailed debug: resources, passes, bindings.

  // Parameter validation
  void SetParameterValidator(ShaderParameterValidator *validator) {
    parameter_validator_ = validator;
  }
  void EnableParameterValidation(bool enable) {
    enable_parameter_validation_ = enable;
  }

private:
  void AllocateResources();
  bool ValidatePassParameters(std::shared_ptr<RenderGraphPass> &pass) const;
  ID3D11Device *device_ = nullptr;
  ID3D11DeviceContext *context_ = nullptr;
  std::unordered_map<std::string, GraphResource> resources_;
  std::vector<std::shared_ptr<RenderGraphPass>> passes_;
  std::vector<std::shared_ptr<RenderGraphPass>> sorted_passes_;
  bool compiled_ = false;

  // Parameter validation
  ShaderParameterValidator *parameter_validator_ = nullptr;
  bool enable_parameter_validation_ = true;
};
