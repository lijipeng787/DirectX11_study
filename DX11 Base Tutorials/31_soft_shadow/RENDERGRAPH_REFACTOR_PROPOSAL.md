# RenderGraph 参数系统重构建议

## 发现的问题

### 1. 参数合并逻辑不够清晰

**问题位置**: `RenderGraphPass::Execute()` 第119-123行

**问题描述**:
- 注释与实际行为不一致：注释说"pass first, then global override"，但实际是global覆盖pass
- 参数合并顺序不够直观，难以理解优先级

**当前代码**:
```cpp
ShaderParameterContainer merged = *pass_parameters_; // pass first, then global override.
merged.Merge(global_params);
for (auto &kv : input_textures_)
    merged.SetTexture(kv.first, kv.second->GetShaderResourceView());
```

**问题分析**:
- `Merge()` 方法会让后合并的参数覆盖先前的参数，所以实际优先级是：global > pass
- 输入纹理的绑定方式不够清晰

### 2. 输入资源与参数绑定分离

**问题位置**: `Graphics.cpp::SetupRenderPasses()`

**问题描述**:
- `Read()` 声明了资源依赖，但仍需要手动 `SetTexture()` 绑定
- 资源名称和参数名称不一致（如 "DepthMap" vs "depthMapTexture"）
- 容易出错：忘记绑定或绑定错误

**示例**:
```cpp
render_graph_.AddPass("ShadowPass")
    .Read("DepthMap")  // 声明依赖
    .SetTexture("depthMapTexture", depth_tex->GetShaderResourceView()); // 手动绑定
```

### 3. 参数设置模式重复

**问题位置**: `Graphics.cpp::SetupRenderPasses()`

**问题描述**:
- 每个Pass的参数设置模式高度重复
- 缺少统一的抽象和辅助函数

### 4. 参数命名约定不一致

**问题描述**:
- 资源名："DepthMap"、"ShadowMap"
- 参数名："depthMapTexture"、"shadowTexture"、"texture"
- 没有统一的命名规范

## 重构方案

### 方案1: 改进参数合并逻辑的清晰度

**目标**: 让参数合并顺序和优先级更清晰

**改进点**:
1. 修正注释，反映实际行为
2. 提取参数合并为独立方法
3. 添加明确的注释说明优先级

### 方案2: 自动绑定输入资源

**目标**: 让 `Read()` 自动绑定资源到参数

**改进点**:
1. 添加 `ReadAsParameter()` 方法，自动绑定资源到参数
2. 支持资源名到参数名的映射配置
3. 默认使用资源名作为参数名（可配置）

### 方案3: 提取参数合并逻辑

**目标**: 将参数合并逻辑抽象为独立方法

**改进点**:
1. 创建 `MergePassParameters()` 方法
2. 统一参数合并的入口
3. 便于测试和维护

### 方案4: 统一命名约定

**目标**: 建立统一的资源名和参数名规范

**改进点**:
1. 文档化命名约定
2. 提供命名转换辅助函数
3. 添加参数名验证

## 已实施的改进

### ✅ 改进1: 提取参数合并逻辑

**实施内容**:
- 在 `RenderGraphPass` 中添加了 `MergeParameters()` 私有方法
- 将参数合并逻辑从 `Execute()` 中提取出来
- 添加了清晰的注释说明参数优先级

**改进后的代码**:
```cpp
// RenderGraph.h
private:
  // Merge parameters in priority order: pass -> global -> input textures
  // Returns merged parameter container ready for object-level customization
  ShaderParameterContainer MergeParameters(
      const ShaderParameterContainer &global_params) const;
```

```cpp
// RenderGraph.cpp
ShaderParameterContainer RenderGraphPass::MergeParameters(
    const ShaderParameterContainer &global_params) const {
  // Merge parameters in priority order (later merges override earlier ones):
  // 1. Start with pass-specific parameters (lowest priority)
  // 2. Merge global parameters (override pass parameters)
  // 3. Bind input textures (override any conflicting parameters)
  ShaderParameterContainer merged = *pass_parameters_;
  merged.Merge(global_params);
  for (const auto &[param_name, texture] : input_textures_) {
    merged.SetTexture(param_name, texture->GetShaderResourceView());
  }
  return merged;
}
```

**好处**:
- 参数合并逻辑集中在一处，易于理解和维护
- 可以单独测试参数合并逻辑
- `Execute()` 方法更简洁

### ✅ 改进2: 改进注释和代码可读性

**实施内容**:
- 修正了参数合并的注释，准确反映实际行为
- 添加了对象级参数构建的注释
- 使用结构化绑定 `const auto &[param_name, texture]` 提高可读性

**改进后的代码**:
```cpp
// Merge pass, global, and input texture parameters
ShaderParameterContainer merged = MergeParameters(global_params);

// ... later in object rendering loop ...

// Build object-specific parameters:
// 1. Copy merged pass/global parameters
// 2. Add world matrix (per-object)
// 3. Apply object callback (highest priority, can override anything)
ShaderParameterContainer objParams = merged;
objParams.Set("worldMatrix", r->GetWorldMatrix());
if (auto cb = r->GetParameterCallback()) {
  cb(objParams);
}
```

**好处**:
- 参数优先级关系更清晰
- 代码意图更明确
- 便于新开发者理解

## 推荐的改进优先级

1. **已完成**: ✅ 方案1 - 改进参数合并逻辑的清晰度
2. **已完成**: ✅ 方案3 - 提取参数合并逻辑
3. **待实施**: 方案2 - 自动绑定输入资源（影响易用性）
4. **待实施**: 方案4 - 统一命名约定（影响长期维护）

## 未来改进建议

### 方案2: 自动绑定输入资源（待实施）

**建议实现**:
```cpp
// RenderGraphPassBuilder 添加新方法
RenderGraphPassBuilder &ReadAsParameter(
    const std::string &resource_name, 
    const std::string &param_name = "") {
  pass_->input_resources_.push_back(resource_name);
  if (!param_name.empty()) {
    pass_->input_param_mapping_[resource_name] = param_name;
  }
  return *this;
}
```

**使用示例**:
```cpp
// 旧方式（容易出错）
render_graph_.AddPass("ShadowPass")
    .Read("DepthMap")
    .SetTexture("depthMapTexture", depth_tex->GetShaderResourceView());

// 新方式（自动绑定）
render_graph_.AddPass("ShadowPass")
    .ReadAsParameter("DepthMap", "depthMapTexture"); // 自动绑定
```

### 方案4: 统一命名约定（待实施）

**建议规范**:
- 资源名：使用 PascalCase，如 `DepthMap`, `ShadowMap`
- 参数名：使用 camelCase，如 `depthMapTexture`, `shadowTexture`
- 提供辅助函数进行转换：
  ```cpp
  std::string ResourceNameToParamName(const std::string &resource_name);
  ```

