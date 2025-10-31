#pragma once

#include "ShaderParameterLayout.h" // Use unified type definitions

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// ShaderParameterType and ShaderParameterInfo are now defined in
// ShaderParameterLayout.h This file uses those unified definitions

enum class ValidationMode {
  Strict,  // 严格模式：所有必需参数必须存在
  Warning, // 警告模式：报告缺失参数但不阻止执行
  Disabled // 禁用验证
};

class ShaderParameterValidator {
public:
  // 注册着色器需要的参数
  void RegisterShader(const std::string &shader_name,
                      const std::vector<ShaderParameterInfo> &parameters);

  // 注册全局参数（这些参数在运行时由Render()提供，不需要在Pass中设置）
  void RegisterGlobalParameter(const std::string &param_name);

  // 检查参数是否为全局参数
  bool IsGlobalParameter(const std::string &param_name) const;

  // 验证单个参数
  bool ValidateParameter(const std::string &parameter_name,
                         ShaderParameterType expected_type) const;

  // 验证Pass的所有参数
  // provided_parameters: Pass级别设置的参数（不包括全局参数）
  // 注意：全局参数会在运行时提供，不会被报告为缺失
  bool ValidatePassParameters(
      const std::string &pass_name, const std::string &shader_name,
      const std::unordered_set<std::string> &provided_parameters,
      ValidationMode mode = ValidationMode::Warning) const;

  // 获取缺失的参数列表（排除全局参数）
  std::vector<std::string> GetMissingParameters(
      const std::string &shader_name,
      const std::unordered_set<std::string> &provided_parameters) const;

  // 获取无效的参数列表（类型不匹配）
  std::vector<std::string> GetInvalidParameters(
      const std::string &shader_name,
      const std::unordered_set<std::string> &provided_parameters) const;

  // 检查着色器是否已注册
  bool IsShaderRegistered(const std::string &shader_name) const;

  // 获取着色器需要的所有参数
  std::vector<ShaderParameterInfo>
  GetShaderParameters(const std::string &shader_name) const;

  // 清除所有注册信息
  void Clear();

  // 设置验证模式
  void SetValidationMode(ValidationMode mode) { validation_mode_ = mode; }
  ValidationMode GetValidationMode() const { return validation_mode_; }

private:
  // 着色器名称 -> 参数信息列表
  std::unordered_map<std::string, std::vector<ShaderParameterInfo>>
      shader_parameters_;

  // 全局参数集合（运行时由Render()提供，不需要在Pass中设置）
  std::unordered_set<std::string> global_parameters_;

  ValidationMode validation_mode_ = ValidationMode::Strict;
};

// 资源名到参数名的转换辅助函数
namespace RenderGraphNaming {
// 将资源名转换为参数名（默认规则：首字母小写）
std::string ResourceNameToParameterName(const std::string &resource_name);

// 将资源名转换为纹理参数名（添加Texture后缀）
std::string
ResourceNameToTextureParameterName(const std::string &resource_name);

// 验证参数名是否符合命名约定
bool IsValidParameterName(const std::string &name);

// 验证资源名是否符合命名约定
bool IsValidResourceName(const std::string &name);
} // namespace RenderGraphNaming
