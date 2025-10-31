# RenderGraph 命名约定和参数验证系统

## 命名约定规范

### 资源名（Resource Names）
- **格式**: PascalCase（首字母大写的驼峰命名）
- **用途**: RenderGraph中的资源标识符（`Read()`, `Write()`, `ImportTexture()`）
- **示例**:
  - `DepthMap`
  - `ShadowMap`
  - `DownsampledShadow`
  - `HorizontalBlur`
  - `VerticalBlur`

### 参数名（Parameter Names）
- **格式**: camelCase（首字母小写的驼峰命名）
- **用途**: 着色器参数标识符（`SetParameter()`, `SetTexture()`）
- **命名规则**:
  - 纹理参数：通常以 `Texture` 后缀结尾（如 `depthMapTexture`, `shadowTexture`）
  - 通用纹理：可以使用简短的 `texture` 名称
  - 矩阵参数：以 `Matrix` 结尾（如 `worldMatrix`, `orthoMatrix`）
  - 向量参数：以 `Color`, `Position`, `Direction` 等描述性后缀结尾
  - 标量参数：使用描述性名称（如 `screenWidth`, `screenHeight`）

- **示例**:
  - `depthMapTexture` (对应资源 `DepthMap`)
  - `shadowTexture` (对应资源 `ShadowMap`)
  - `texture` (通用纹理参数)
  - `orthoMatrix`
  - `diffuseColor`, `ambientColor`
  - `screenWidth`, `screenHeight`

### 通用参数（Global Parameters）
这些参数由渲染系统自动提供，名称固定：
- `worldMatrix` - 世界矩阵
- `viewMatrix` - 视图矩阵
- `projectionMatrix` - 投影矩阵
- `lightViewMatrix` - 光照视图矩阵
- `lightProjectionMatrix` - 光照投影矩阵
- `lightPosition` - 光源位置
- `lightDirection` - 光源方向
- `cameraPosition` - 相机位置

## 资源名到参数名的转换

### 自动转换规则
1. **默认规则**: 资源名转换为参数名时，首字母小写，其余保持不变
   - `DepthMap` → `depthMap` (需要手动添加 `Texture` 后缀)

2. **推荐做法**: 使用显式的资源名到参数名映射
   - `DepthMap` → `depthMapTexture`
   - `ShadowMap` → `shadowTexture`

## 参数验证系统的必要性

### 问题
1. **运行时错误**: 参数名拼写错误只在运行时发现，难以调试
2. **缺少参数**: 着色器需要的参数未设置，导致渲染错误
3. **类型不匹配**: 参数类型错误（如矩阵参数传入了纹理）
4. **维护困难**: 不清楚着色器需要哪些参数

### 解决方案
参数验证系统提供：
1. **编译时验证**: 在RenderGraph编译时检查参数名
2. **早期发现**: 在设置阶段就发现错误，而不是运行时
3. **清晰错误**: 提供明确的错误信息，指出缺失或错误的参数
4. **文档化**: 自动生成着色器参数需求文档

## 参数验证系统工作原理

### 架构设计

```
ShaderParameterValidator
├── RegisterShaderParameters()  // 注册着色器需要的参数
├── ValidateParameter()         // 验证单个参数
├── ValidateAllParameters()     // 验证所有参数
└── GetMissingParameters()      // 获取缺失的参数列表
```

### 工作流程

1. **注册阶段**: 着色器注册它们需要的参数
   ```cpp
   validator.RegisterShaderParameters("ShadowShader", {
       "worldMatrix", "viewMatrix", "projectionMatrix",
       "lightViewMatrix", "lightProjectionMatrix",
       "depthMapTexture", "lightPosition"
   });
   ```

2. **验证阶段**: 在RenderGraph编译时验证
   ```cpp
   // 在 Compile() 阶段
   if (!validator.ValidateAllParameters(pass_name, shader_name, parameters)) {
       // 报告错误
   }
   ```

3. **错误报告**: 提供详细的错误信息
   ```
   Error: Pass "ShadowPass" uses shader "ShadowShader"
   Missing parameters: depthMapTexture
   Invalid parameters: depthMap (should be depthMapTexture)
   ```

### 验证级别

1. **严格模式（Strict）**: 所有参数必须存在且类型正确
2. **警告模式（Warning）**: 报告缺失参数但不阻止执行
3. **禁用模式（Disabled）**: 不进行验证（向后兼容）

