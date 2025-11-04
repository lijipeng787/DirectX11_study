#pragma once

#include "ShaderParameterContainer.h" // Use unified type definitions

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct ID3D10Blob;
struct ID3D11Device;

struct ReflectedParameter {
  std::string name;
  ShaderParameterType type = ShaderParameterType::Unknown;
  bool required = true;
};

std::vector<ReflectedParameter>
ReflectShader(ID3D11Device *device, ID3D10Blob *vs_blob,
              ID3D10Blob *ps_blob);

enum class ValidationMode {
  Strict,  // Strict mode: all required parameters must exist
  Warning, // Warning mode: report missing parameters but don't block execution
  Disabled // Disable validation
};

class ShaderParameterValidator {
public:
  // Register shader required parameters
  void RegisterShader(const std::string &shader_name,
                      const std::vector<ShaderParameterInfo> &parameters);
  //void RegisterShader(const std::string &shader_name,
  //                    const std::vector<ReflectedParameter> &parameters);

  // Register global parameters (provided by Render() at runtime, not needed in
  // Pass)
  void RegisterGlobalParameter(const std::string &param_name);

  // Check if parameter is a global parameter
  bool IsGlobalParameter(const std::string &param_name) const;

  // Validate single parameter
  bool ValidateParameter(const std::string &parameter_name,
                         ShaderParameterType expected_type) const;

  // Validate all Pass parameters
  // provided_parameters: parameters set at Pass level (excluding global
  // parameters) Note: global parameters are provided at runtime and won't be
  // reported as missing
  bool ValidatePassParameters(
      const std::string &pass_name, const std::string &shader_name,
      const std::unordered_set<std::string> &provided_parameters,
      ValidationMode mode = ValidationMode::Warning) const;

  bool ValidatePassParameters(
    const std::string &pass_name, const std::string &shader_name,
    const std::unordered_map<std::string, ShaderParameterType>
      &provided_parameters,
    ValidationMode mode = ValidationMode::Warning) const;

  // Get missing parameters list (excluding global parameters)
  std::vector<std::string> GetMissingParameters(
      const std::string &shader_name,
      const std::unordered_set<std::string> &provided_parameters) const;

  // Get invalid parameters list (type mismatch)
  std::vector<std::string> GetInvalidParameters(
      const std::string &shader_name,
      const std::unordered_set<std::string> &provided_parameters) const;

  // Check if shader is registered
  bool IsShaderRegistered(const std::string &shader_name) const;

  // Get all parameters required by shader
  std::vector<ShaderParameterInfo>
  GetShaderParameters(const std::string &shader_name) const;

  bool ValidateParameterType(const ShaderParameterContainer &container,
                             const std::string &parameter_name,
                             ShaderParameterType expected_type) const;

  // Clear all registration information
  void Clear();

  // Set validation mode
  void SetValidationMode(ValidationMode mode) { validation_mode_ = mode; }
  ValidationMode GetValidationMode() const { return validation_mode_; }

private:
  // Shader name -> parameter info list
  std::unordered_map<std::string, std::vector<ShaderParameterInfo>>
      shader_parameters_;

  // Global parameter set (provided by Render() at runtime, not needed in Pass)
  std::unordered_set<std::string> global_parameters_;

  ValidationMode validation_mode_ = ValidationMode::Warning;

  // Helper methods for enhanced reporting
  std::string GetTypeName(ShaderParameterType type) const;
  std::string
  FindSimilarParameter(const std::string &param_name,
                       const std::vector<ShaderParameterInfo> &params) const;
  int CalculateLevenshteinDistance(const std::string &source,
                                   const std::string &target) const;

  bool ValidatePassParametersInternal(
    const std::string &pass_name, const std::string &shader_name,
    const std::unordered_map<std::string, ShaderParameterType>
      &provided_parameters,
    ValidationMode mode) const;
};

// Helper functions for resource name to parameter name conversion
namespace RenderGraphNaming {
// Convert resource name to parameter name (default rule: lowercase first
// letter)
std::string ResourceNameToParameterName(const std::string &resource_name);

// Convert resource name to texture parameter name (add Texture suffix)
std::string
ResourceNameToTextureParameterName(const std::string &resource_name);

// Validate parameter name follows naming convention
bool IsValidParameterName(const std::string &name);

// Validate resource name follows naming convention
bool IsValidResourceName(const std::string &name);
} // namespace RenderGraphNaming
