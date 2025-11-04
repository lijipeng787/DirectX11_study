#include "ShaderParameterValidator.h"

std::vector<ReflectedParameter>
ReflectShader(ID3D11Device *device, ID3D10Blob *vertex_shader_blob,
              ID3D10Blob *pixel_shader_blob) {
  (void)device;
  (void)vertex_shader_blob;
  (void)pixel_shader_blob;
  // TODO: Implement D3D11 shader reflection to populate parameter metadata.
  return {};
}

void ShaderParameterValidator::RegisterShader(
    const std::string &shader_name,
    const std::vector<ReflectedParameter> &parameters) {
  std::vector<ShaderParameterInfo> converted;
  converted.reserve(parameters.size());
  for (const auto &parameter : parameters) {
    converted.emplace_back(parameter.name, parameter.type,
                           parameter.required);
  }
  RegisterShader(shader_name, converted);
}
