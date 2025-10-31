# RenderGraph 参数验证系统实现总结

## 已完成的工作

### 1. ✅ 统一命名约定规范

**文档**: `RENDERGRAPH_NAMING_CONVENTION.md`

**内容**:
- 资源名命名规范：PascalCase（如 `DepthMap`, `ShadowMap`）
- 参数名命名规范：camelCase（如 `depthMapTexture`, `shadowTexture`）
- 通用参数列表：`worldMatrix`, `viewMatrix` 等
- 资源名到参数名的转换规则

### 2. ✅ 实现参数验证系统

**核心文件**:
- `ShaderParameterValidator.h` - 验证器接口
- `ShaderParameterValidator.cpp` - 验证器实现

**主要功能**:
- 注册着色器参数需求
- 验证Pass参数是否完整
- 报告缺失和无效参数
- 支持三种验证模式：Strict、Warning、Disabled

### 3. ✅ 集成到RenderGraph

**修改的文件**:
- `RenderGraph.h` - 添加验证器支持
- `RenderGraph.cpp` - 实现验证逻辑
- `ShaderParameterContainer.h` - 添加 `GetAllParameterNames()` 方法

**集成点**:
- 在 `Compile()` 阶段自动验证所有Pass的参数
- 支持启用/禁用验证
- 可配置验证模式

### 4. ✅ 命名转换辅助函数

**位置**: `ShaderParameterValidator.cpp` 中的 `RenderGraphNaming` 命名空间

**函数**:
- `ResourceNameToParameterName()` - 资源名转参数名（首字母小写）
- `ResourceNameToTextureParameterName()` - 资源名转纹理参数名（添加Texture后缀）
- `IsValidParameterName()` - 验证参数名格式
- `IsValidResourceName()` - 验证资源名格式

## 参数验证系统工作原理

### 架构流程

```
1. 注册阶段（Setup）
   └─> RegisterShader() 注册着色器需要的参数
   
2. 编译阶段（Compile）
   └─> ValidatePassParameters() 验证每个Pass的参数
       ├─> 收集Pass提供的参数名
       ├─> 从验证器获取着色器需要的参数
       └─> 比较并报告差异
       
3. 验证模式
   ├─> Strict: 严格模式，缺失参数会阻止编译
   ├─> Warning: 警告模式，报告问题但不阻止执行
   └─> Disabled: 禁用验证（向后兼容）
```

### 验证时机

参数验证在 `RenderGraph::Compile()` 阶段进行：

```cpp
bool RenderGraph::Compile() {
  // ... 资源解析 ...
  
  // 验证参数（如果启用）
  if (enable_parameter_validation_ && parameter_validator_) {
    for (auto &pass : sorted_passes_) {
      if (!ValidatePassParameters(pass)) {
        return false; // 验证失败
      }
    }
  }
  
  compiled_ = true;
  return true;
}
```

### 验证内容

1. **必需参数检查**: 检查着色器需要的所有必需参数是否都已提供
2. **未知参数警告**: 检查是否有未注册的参数（可能是拼写错误）
3. **命名格式验证**: 验证参数名是否符合camelCase约定

## 使用示例

### 基本使用

```cpp
// 1. 创建验证器
ShaderParameterValidator validator;
validator.SetValidationMode(ValidationMode::Warning);

// 2. 注册着色器参数
validator.RegisterShader("ShadowShader", {
    {"worldMatrix", ShaderParameterType::Matrix, true},
    {"viewMatrix", ShaderParameterType::Matrix, true},
    {"depthMapTexture", ShaderParameterType::Texture, true}
});

// 3. 附加到RenderGraph
render_graph_.SetParameterValidator(&validator);
render_graph_.EnableParameterValidation(true);

// 4. 编译时会自动验证
render_graph_.Compile(); // 如果有问题会报告
```

### 使用命名转换

```cpp
using namespace RenderGraphNaming;

// 自动转换资源名到参数名
std::string param_name = ResourceNameToTextureParameterName("DepthMap");
// param_name = "depthMapTexture"

render_graph_.AddPass("ShadowPass")
    .Read("DepthMap")
    .SetTexture(param_name, texture);
```

## 验证系统的必要性

### 解决的问题

1. **早期错误发现**: 在编译时而非运行时发现参数错误
2. **清晰的错误信息**: 明确指出缺失或错误的参数
3. **文档化**: 自动记录着色器参数需求
4. **类型安全**: 通过注册参数类型，提供类型检查基础

### 实际效果

**验证前**:
```
运行时错误：Parameter not found: depthMapTexture
（需要调试才能找到问题）
```

**验证后**:
```
编译时警告：
[ShaderParameterValidator] WARNING: Pass "ShadowPass" (Shader: "ShadowShader"):
  Missing required parameters: depthMapTexture
  Unknown parameters: depthMap (可能拼写错误)
```

## 未来改进方向

1. **类型验证**: 实际验证参数类型是否匹配（不仅仅是名称）
2. **自动注册**: 从着色器文件自动提取参数需求
3. **IDE集成**: 提供代码补全和实时验证
4. **参数文档生成**: 自动生成API文档

## 文件清单

- ✅ `ShaderParameterValidator.h` - 验证器头文件
- ✅ `ShaderParameterValidator.cpp` - 验证器实现
- ✅ `RENDERGRAPH_NAMING_CONVENTION.md` - 命名约定文档
- ✅ `PARAMETER_VALIDATION_USAGE_EXAMPLE.cpp` - 使用示例
- ✅ `RenderGraph.h` - 集成验证器接口
- ✅ `RenderGraph.cpp` - 集成验证逻辑
- ✅ `ShaderParameterContainer.h` - 添加参数名提取方法

所有代码已通过编译检查，可以直接使用！

