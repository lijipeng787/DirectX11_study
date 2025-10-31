#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

enum class ShaderParameterType {
  Matrix,
  Vector3,
  Vector4,
  Texture,
  Float,
  Unknown
};

struct ShaderParameterInfo {
  std::string name;
  ShaderParameterType type;
  bool required; // 是否必需

  ShaderParameterInfo(const std::string &n, ShaderParameterType t,
                      bool req = true)
      : name(n), type(t), required(req) {}
};

enum class ValidationMode {
  Strict,   // 严格模式：所有必需参数必须存在
  Warning,  // 警告模式：报告缺失参数但不阻止执行
  Disabled  // 禁用验证
};

class ShaderParameterValidator {
public:
  // 注册着色器需要的参数
  void RegisterShader(const std::string &shader_name,
                     const std::vector<ShaderParameterInfo> &parameters);

  // 验证单个参数
  bool ValidateParameter(const std::string &parameter_name,
                         ShaderParameterType expected_type) const;

  // 验证Pass的所有参数
  bool ValidatePassParameters(const std::string &pass_name,
                               const std::string &shader_name,
                               const std::unordered_set<std::string>
                                   &provided_parameters,
                               ValidationMode mode = ValidationMode::Warning) const;

  // 获取缺失的参数列表
  std::vector<std::string>
  GetMissingParameters(const std::string &shader_name,
                       const std::unordered_set<std::string>
                           &provided_parameters) const;

  // 获取无效的参数列表（类型不匹配）
  std::vector<std::string>
  GetInvalidParameters(const std::string &shader_name,
                       const std::unordered_set<std::string>
                           &provided_parameters) const;

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

  ValidationMode validation_mode_ = ValidationMode::Warning;
};

// 资源名到参数名的转换辅助函数
namespace RenderGraphNaming {
// 将资源名转换为参数名（默认规则：首字母小写）
std::string ResourceNameToParameterName(const std::string &resource_name);

// 将资源名转换为纹理参数名（添加Texture后缀）
std::string ResourceNameToTextureParameterName(
    const std::string &resource_name);

// 验证参数名是否符合命名约定
bool IsValidParameterName(const std::string &name);

// 验证资源名是否符合命名约定
bool IsValidResourceName(const std::string &name);
}

