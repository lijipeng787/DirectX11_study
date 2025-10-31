// 示例：如何在Graphics.cpp中使用参数验证系统

// 在GraphicsClass::SetupRenderGraph()中，初始化验证器并注册着色器参数

/*
void GraphicsClass::SetupRenderGraph() {
  // ... existing code ...

  // 1. 创建参数验证器
  static ShaderParameterValidator validator;
  validator.SetValidationMode(ValidationMode::Warning); // 或 Strict

  // 2. 注册各个着色器需要的参数
  validator.RegisterShader("ShadowShader", {
      {"worldMatrix", ShaderParameterType::Matrix, true},
      {"viewMatrix", ShaderParameterType::Matrix, true},
      {"projectionMatrix", ShaderParameterType::Matrix, true},
      {"lightViewMatrix", ShaderParameterType::Matrix, true},
      {"lightProjectionMatrix", ShaderParameterType::Matrix, true},
      {"lightPosition", ShaderParameterType::Vector3, true},
      {"depthMapTexture", ShaderParameterType::Texture, true}
  });

  validator.RegisterShader("SoftShadowShader", {
      {"worldMatrix", ShaderParameterType::Matrix, true},
      {"viewMatrix", ShaderParameterType::Matrix, true},
      {"projectionMatrix", ShaderParameterType::Matrix, true},
      {"texture", ShaderParameterType::Texture, true},
      {"shadowTexture", ShaderParameterType::Texture, true},
      {"ambientColor", ShaderParameterType::Vector4, true},
      {"diffuseColor", ShaderParameterType::Vector4, true},
      {"lightPosition", ShaderParameterType::Vector3, true}
  });

  validator.RegisterShader("PbrShader", {
      {"worldMatrix", ShaderParameterType::Matrix, true},
      {"viewMatrix", ShaderParameterType::Matrix, true},
      {"projectionMatrix", ShaderParameterType::Matrix, true},
      {"diffuseTexture", ShaderParameterType::Texture, true},
      {"normalMap", ShaderParameterType::Texture, true},
      {"rmTexture", ShaderParameterType::Texture, true},
      {"lightDirection", ShaderParameterType::Vector3, true},
      {"cameraPosition", ShaderParameterType::Vector3, true}
  });

  validator.RegisterShader("TextureShader", {
      {"worldMatrix", ShaderParameterType::Matrix, true},
      {"viewMatrix", ShaderParameterType::Matrix, true},
      {"projectionMatrix", ShaderParameterType::Matrix, true},
      {"orthoMatrix", ShaderParameterType::Matrix, true},
      {"texture", ShaderParameterType::Texture, true}
  });

  validator.RegisterShader("HorizontalBlurShader", {
      {"orthoMatrix", ShaderParameterType::Matrix, true},
      {"screenWidth", ShaderParameterType::Float, true},
      {"texture", ShaderParameterType::Texture, true}
  });

  validator.RegisterShader("VerticalBlurShader", {
      {"orthoMatrix", ShaderParameterType::Matrix, true},
      {"screenHeight", ShaderParameterType::Float, true},
      {"texture", ShaderParameterType::Texture, true}
  });

  validator.RegisterShader("DepthShader", {
      {"worldMatrix", ShaderParameterType::Matrix, true},
      {"lightViewMatrix", ShaderParameterType::Matrix, true},
      {"lightProjectionMatrix", ShaderParameterType::Matrix, true}
  });

  // 3. 将验证器附加到RenderGraph
  render_graph_.SetParameterValidator(&validator);
  render_graph_.EnableParameterValidation(true);

  // 4. 继续设置Passes（现有代码）
  SetupRenderPasses();

  // 5. 编译时会自动验证参数
  if (!render_graph_.Compile()) {
    // 验证失败会在这里报告
    return false;
  }

  // ... rest of setup ...
}
*/

// 使用资源名到参数名的转换辅助函数
/*
// 在SetupRenderPasses()中使用命名转换
void GraphicsClass::SetupRenderPasses() {
  // ... existing code ...

  // 旧方式：
  render_graph_.AddPass("ShadowPass")
      .Read("DepthMap")
      .SetTexture("depthMapTexture", depth_tex->GetShaderResourceView());

  // 新方式（使用命名转换）：
  using namespace RenderGraphNaming;
  std::string param_name = ResourceNameToTextureParameterName("DepthMap");
  // param_name = "depthMapTexture"
  render_graph_.AddPass("ShadowPass")
      .Read("DepthMap")
      .SetTexture(param_name, depth_tex->GetShaderResourceView());
}
*/

