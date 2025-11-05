#include "ShaderParameterValidator.h"

#include "Logger.h"

#include <algorithm>
#include <d3d11shader.h>
#include <d3dcompiler.h>
#include <sstream>
#include <unordered_map>

namespace {

// Map a reflected shader variable type to our ShaderParameterType enum.
ShaderParameterType MapShaderType(const D3D11_SHADER_TYPE_DESC &type_desc) {
  switch (type_desc.Class) {
  case D3D_SVC_MATRIX_ROWS:
  case D3D_SVC_MATRIX_COLUMNS:
    return ShaderParameterType::Matrix;
  case D3D_SVC_VECTOR:
    if (type_desc.Columns == 3) {
      return ShaderParameterType::Vector3;
    }
    if (type_desc.Columns == 4) {
      return ShaderParameterType::Vector4;
    }
    break;
  case D3D_SVC_SCALAR:
    if (type_desc.Type == D3D_SVT_FLOAT) {
      return ShaderParameterType::Float;
    }
    break;
  default:
    break;
  }
  return ShaderParameterType::Unknown;
}

using ReflectionCache = std::unordered_map<std::string, ReflectedParameter>;

void AddOrUpdateParameter(ReflectionCache &cache, const std::string &name,
                          ShaderParameterType type, const char *stage_label) {
  if (name.empty()) {
    return;
  }

  auto existing_iterator = cache.find(name);
  if (existing_iterator == cache.end()) {
    cache.emplace(name, ReflectedParameter{name, type, true});
    return;
  }

  if (existing_iterator->second.type == type ||
      type == ShaderParameterType::Unknown) {
    return;
  }

  if (existing_iterator->second.type == ShaderParameterType::Unknown) {
    existing_iterator->second.type = type;
    return;
  }

  Logger::SetModule("ShaderParameterReflection");
  Logger::LogWarning(
      "Parameter type mismatch detected during reflection for "
      "parameter '" +
      name + "' in stage " + stage_label + ": existing=" +
      ShaderParameterTypeToString(existing_iterator->second.type) +
      ", incoming=" + ShaderParameterTypeToString(type));
}

void ReflectConstantBuffers(ID3D11ShaderReflection *reflection,
                            const char *stage_label, ReflectionCache &cache) {
  if (reflection == nullptr) {
    return;
  }

  D3D11_SHADER_DESC shader_desc = {};
  if (FAILED(reflection->GetDesc(&shader_desc))) {
    return;
  }

  for (UINT buffer_index = 0; buffer_index < shader_desc.ConstantBuffers;
       ++buffer_index) {
    ID3D11ShaderReflectionConstantBuffer *cbuffer =
        reflection->GetConstantBufferByIndex(buffer_index);
    if (cbuffer == nullptr) {
      continue;
    }

    D3D11_SHADER_BUFFER_DESC buffer_desc = {};
    if (FAILED(cbuffer->GetDesc(&buffer_desc))) {
      continue;
    }

    for (UINT variable_index = 0; variable_index < buffer_desc.Variables;
         ++variable_index) {
      ID3D11ShaderReflectionVariable *variable =
          cbuffer->GetVariableByIndex(variable_index);
      if (variable == nullptr) {
        continue;
      }

      D3D11_SHADER_VARIABLE_DESC variable_desc = {};
      if (FAILED(variable->GetDesc(&variable_desc))) {
        continue;
      }

      if ((variable_desc.uFlags & D3D_SVF_USED) == 0) {
        continue; // Skip unused variables
      }

      ID3D11ShaderReflectionType *type = variable->GetType();
      if (type == nullptr) {
        continue;
      }

      D3D11_SHADER_TYPE_DESC type_desc = {};
      if (FAILED(type->GetDesc(&type_desc))) {
        continue;
      }

      const ShaderParameterType parameter_type = MapShaderType(type_desc);
      AddOrUpdateParameter(cache, variable_desc.Name, parameter_type,
                           stage_label);
    }
  }
}

void ReflectResourceBindings(ID3D11ShaderReflection *reflection,
                             const char *stage_label, ReflectionCache &cache) {
  if (reflection == nullptr) {
    return;
  }

  D3D11_SHADER_DESC shader_desc = {};
  if (FAILED(reflection->GetDesc(&shader_desc))) {
    return;
  }

  for (UINT resource_index = 0; resource_index < shader_desc.BoundResources;
       ++resource_index) {
    D3D11_SHADER_INPUT_BIND_DESC bind_desc = {};
    if (FAILED(
            reflection->GetResourceBindingDesc(resource_index, &bind_desc))) {
      continue;
    }

    if (bind_desc.Type == D3D_SIT_TEXTURE) {
      AddOrUpdateParameter(cache, bind_desc.Name, ShaderParameterType::Texture,
                           stage_label);
    }
  }
}

void CollectStageParameters(ID3D10Blob *shader_blob, const char *stage_label,
                            ReflectionCache &cache) {
  if (shader_blob == nullptr) {
    return;
  }

  ID3D11ShaderReflection *reflection = nullptr;
  HRESULT reflect_result = D3DReflect(
      shader_blob->GetBufferPointer(), shader_blob->GetBufferSize(),
      IID_ID3D11ShaderReflection, reinterpret_cast<void **>(&reflection));
  if (FAILED(reflect_result)) {
    std::ostringstream stream;
    stream << "D3DReflect failed for stage " << stage_label << ": 0x"
           << std::hex << std::uppercase
           << static_cast<uint32_t>(reflect_result);
    Logger::SetModule("ShaderParameterReflection");
    Logger::LogWarning(stream.str());
    return;
  }

  ReflectConstantBuffers(reflection, stage_label, cache);
  ReflectResourceBindings(reflection, stage_label, cache);

  if (reflection != nullptr) {
    reflection->Release();
  }
}

} // namespace

std::vector<ReflectedParameter>
ReflectShader(ID3D11Device *device, ID3D10Blob *vs_blob, ID3D10Blob *ps_blob) {
  (void)device; // Currently unused but retained for future expansion.

  ReflectionCache cache;
  CollectStageParameters(vs_blob, "VS", cache);
  CollectStageParameters(ps_blob, "PS", cache);

  std::vector<ReflectedParameter> parameters;
  parameters.reserve(cache.size());
  for (const auto &entry : cache) {
    parameters.push_back(entry.second);
  }

  std::sort(parameters.begin(), parameters.end(),
            [](const ReflectedParameter &lhs, const ReflectedParameter &rhs) {
              return lhs.name < rhs.name;
            });

  return parameters;
}

void ShaderParameterValidator::RegisterShader(
    const std::string &shader_name,
    const std::vector<ReflectedParameter> &parameters) {
  std::vector<ShaderParameterInfo> converted;
  converted.reserve(parameters.size());
  for (const auto &parameter : parameters) {
    converted.emplace_back(parameter.name, parameter.type, parameter.required);
  }
  RegisterShader(shader_name, converted);
}
