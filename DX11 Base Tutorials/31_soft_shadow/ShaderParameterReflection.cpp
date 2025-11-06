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

ShaderStageMask StageToMask(ShaderStage stage) {
  return static_cast<ShaderStageMask>(stage);
}

void MergeStageMask(ReflectedParameter &parameter, ShaderStage stage) {
  parameter.stage_mask =
      static_cast<ShaderStageMask>(parameter.stage_mask | StageToMask(stage));
}

void AddOrUpdateParameter(ReflectionCache &cache, const std::string &name,
                          ShaderParameterType type, ShaderStage stage,
                          const char *stage_label, bool required = true) {
  if (name.empty()) {
    return;
  }

  const auto stage_mask = StageToMask(stage);

  auto existing_iterator = cache.find(name);
  if (existing_iterator == cache.end()) {
    cache.emplace(name, ReflectedParameter{name, type, required, stage_mask});
    return;
  }

  auto &existing = existing_iterator->second;
  MergeStageMask(existing, stage);
  if (required && !existing.required) {
    existing.required = true;
  }

  if (existing.type == type || type == ShaderParameterType::Unknown) {
    return;
  }

  if (existing.type == ShaderParameterType::Unknown) {
    existing.type = type;
    return;
  }

  Logger::SetModule("ShaderParameterReflection");
  Logger::LogWarning("Parameter type mismatch detected during reflection for "
                     "parameter '" +
                     name + " in stage " + stage_label + ": existing=" +
                     ShaderParameterTypeToString(existing.type) +
                     ", incoming=" + ShaderParameterTypeToString(type));
}

void ReflectTypeRecursive(ID3D11ShaderReflectionType *type,
                          const std::string &qualified_name, ShaderStage stage,
                          const char *stage_label, ReflectionCache &cache) {
  if (type == nullptr) {
    return;
  }

  D3D11_SHADER_TYPE_DESC type_desc = {};
  if (FAILED(type->GetDesc(&type_desc))) {
    return;
  }

  if (type_desc.Elements > 0) {
    const ShaderParameterType parameter_type = MapShaderType(type_desc);
    AddOrUpdateParameter(cache, qualified_name, parameter_type, stage,
                         stage_label);
    return;
  }

  if (type_desc.Class == D3D_SVC_STRUCT) {
    for (UINT member_index = 0; member_index < type_desc.Members;
         ++member_index) {
      ID3D11ShaderReflectionType *member_type =
          type->GetMemberTypeByIndex(member_index);
      const char *member_name = type->GetMemberTypeName(member_index);
      if (member_type == nullptr || member_name == nullptr) {
        continue;
      }

      std::string member_qualified_name = qualified_name;
      if (!member_qualified_name.empty()) {
        member_qualified_name.append(".");
      }
      member_qualified_name.append(member_name);

      ReflectTypeRecursive(member_type, member_qualified_name, stage,
                           stage_label, cache);
    }
    return;
  }

  const ShaderParameterType parameter_type = MapShaderType(type_desc);
  AddOrUpdateParameter(cache, qualified_name, parameter_type, stage,
                       stage_label);
}

void ReflectConstantBuffers(ID3D11ShaderReflection *reflection,
                            ShaderStage stage, const char *stage_label,
                            ReflectionCache &cache) {
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

      ReflectTypeRecursive(type, variable_desc.Name, stage, stage_label, cache);
    }
  }
}

void ReflectResourceBindings(ID3D11ShaderReflection *reflection,
                             ShaderStage stage, const char *stage_label,
                             ReflectionCache &cache) {
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

    switch (bind_desc.Type) {
    case D3D_SIT_TEXTURE:
      AddOrUpdateParameter(cache, bind_desc.Name, ShaderParameterType::Texture,
                           stage, stage_label);
      break;
    case D3D_SIT_SAMPLER:
      AddOrUpdateParameter(cache, bind_desc.Name, ShaderParameterType::Sampler,
                           stage, stage_label, false);
      break;
    default:
      break;
    }
  }
}

void CollectStageParameters(ID3D10Blob *shader_blob, ShaderStage stage,
                            const char *stage_label, ReflectionCache &cache) {
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

  ReflectConstantBuffers(reflection, stage, stage_label, cache);
  ReflectResourceBindings(reflection, stage, stage_label, cache);

  if (reflection != nullptr) {
    reflection->Release();
  }
}

} // namespace

std::vector<ReflectedParameter>
ReflectShader(ID3D11Device *device, ID3D10Blob *vs_blob, ID3D10Blob *ps_blob) {
  (void)device; // Currently unused but retained for future expansion.

  ReflectionCache cache;
  CollectStageParameters(vs_blob, ShaderStage::Vertex, "VS", cache);
  CollectStageParameters(ps_blob, ShaderStage::Pixel, "PS", cache);

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
