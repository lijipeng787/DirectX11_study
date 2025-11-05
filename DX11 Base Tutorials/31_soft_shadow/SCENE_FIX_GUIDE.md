# Scene.cpp 快速修复指南

## 需要替换的模式

### 1. 函数签名修改

**文件位置：** Scene.cpp 第 215 行和第 493 行

#### BuildSceneObjects
```cpp
// 查找这行
void Scene::BuildSceneObjects(const SceneResourceRefs &resources,

// 替换为
void Scene::BuildSceneObjects(
```
（注意：保留后面的参数）

#### BuildSceneObjectsFromJson  
```cpp
// 查找这行
bool Scene::BuildSceneObjectsFromJson(const nlohmann::json &j,
                                      const SceneResourceRefs &resources,

// 替换为
bool Scene::BuildSceneObjectsFromJson(const nlohmann::json &j,
```
（注意：删除 resources 参数行）

### 2. 资源访问替换（约150处）

使用 VS Code/Visual Studio 的"查找替换"功能（Ctrl+H）：

#### Model 资源
```
查找：resources\.scene_assets\.cube
替换：ResourceRegistry::GetInstance().Get<Model>("cube")

查找：resources\.scene_assets\.sphere
替换：ResourceRegistry::GetInstance().Get<Model>("sphere")

查找：resources\.scene_assets\.ground
替换：ResourceRegistry::GetInstance().Get<Model>("ground")

查找：resources\.scene_assets\.refraction\.ground
替换：ResourceRegistry::GetInstance().Get<Model>("refraction_ground")

查找：resources\.scene_assets\.refraction\.wall
替换：ResourceRegistry::GetInstance().Get<Model>("refraction_wall")

查找：resources\.scene_assets\.refraction\.water
替换：ResourceRegistry::GetInstance().Get<Model>("refraction_water")
```

#### PBR Model 资源
```
查找：resources\.scene_assets\.pbr_sphere
替换：ResourceRegistry::GetInstance().Get<PBRModel>("pbr_sphere")
```

#### Shader 资源
```
查找：resources\.shader_assets\.depth
替换：ResourceRegistry::GetInstance().Get<IShader>("depth")

查找：resources\.shader_assets\.shadow
替换：ResourceRegistry::GetInstance().Get<IShader>("shadow")

查找：resources\.shader_assets\.texture
替换：ResourceRegistry::GetInstance().Get<IShader>("texture")

查找：resources\.shader_assets\.horizontal_blur
替换：ResourceRegistry::GetInstance().Get<IShader>("horizontal_blur")

查找：resources\.shader_assets\.vertical_blur
替换：ResourceRegistry::GetInstance().Get<IShader>("vertical_blur")

查找：resources\.shader_assets\.soft_shadow
替换：ResourceRegistry::GetInstance().Get<IShader>("soft_shadow")

查找：resources\.shader_assets\.pbr
替换：ResourceRegistry::GetInstance().Get<IShader>("pbr")

查找：resources\.shader_assets\.diffuse_lighting
替换：ResourceRegistry::GetInstance().Get<IShader>("diffuse_lighting")

查找：resources\.shader_assets\.refraction\.scene_light
替换：ResourceRegistry::GetInstance().Get<IShader>("scene_light")

查找：resources\.shader_assets\.refraction\.refraction
替换：ResourceRegistry::GetInstance().Get<IShader>("refraction")
```

#### RenderTexture 资源
```
查找：resources\.render_targets\.shadow_map
替换：ResourceRegistry::GetInstance().Get<RenderTexture>("shadow_map")

查找：resources\.render_targets\.downsampled_shadow
替换：ResourceRegistry::GetInstance().Get<RenderTexture>("downsampled_shadow")

查找：resources\.render_targets\.horizontal_blur
替换：ResourceRegistry::GetInstance().Get<RenderTexture>("horizontal_blur")

查找：resources\.render_targets\.vertical_blur
替换：ResourceRegistry::GetInstance().Get<RenderTexture>("vertical_blur")
```

#### OrthoWindow 资源
```
查找：resources\.ortho_windows\.small_window
替换：ResourceRegistry::GetInstance().Get<OrthoWindow>("small_window")

查找：resources\.ortho_windows\.fullscreen_window
替换：ResourceRegistry::GetInstance().Get<OrthoWindow>("fullscreen_window")
```

### 3. GetXxxByName() 调用修改

#### GetModelByName
```
查找：GetModelByName\(([^,]+), resources\)
替换：GetModelByName($1)
```

#### GetPBRModelByName
```
查找：GetPBRModelByName\(([^,]+), resources\)
替换：GetPBRModelByName($1)
```

#### GetShaderByName
```
查找：GetShaderByName\(([^,]+), resources\)
替换：GetShaderByName($1)
```

#### GetRenderTextureByName
```
查找：GetRenderTextureByName\(([^,]+), resources\)
替换：GetRenderTextureByName($1)
```

#### GetOrthoWindowByName
```
查找：GetOrthoWindowByName\(([^,]+), resources\)
替换：GetOrthoWindowByName($1)
```

## 验证检查清单

完成替换后，执行以下检查：

- [ ] Scene.cpp 中没有 `SceneResourceRefs` 的引用
- [ ] Scene.cpp 中没有 `resources.` 的访问
- [ ] 所有 `GetXxxByName()` 调用只有一个参数
- [ ] `BuildSceneObjects` 没有 resources 参数
- [ ] `BuildSceneObjectsFromJson` 没有 resources 参数
- [ ] 编译无错误
- [ ] 运行时资源加载成功

## 批量替换技巧

### Visual Studio
1. 按 `Ctrl+H` 打开查找替换
2. 勾选"使用正则表达式"
3. 逐个执行上面的替换规则

### VS Code
1. 按 `Ctrl+H` 打开查找替换
2. 点击 `.*` 图标启用正则表达式
3. 执行替换

## 注意事项

1. **资源ID必须与Graphics.cpp注册的一致**
   - Graphics注册 `"cube"` → Scene访问 `"cube"` ✅
   - Graphics注册 `"cube"` → Scene访问 `"Cube"` ❌

2. **检查返回的指针**
   ```cpp
   auto model = ResourceRegistry::GetInstance().Get<Model>("cube");
   if (!model) {
       Logger::LogError("Model 'cube' not found in registry!");
       return;
   }
   ```

3. **嵌套资源访问**
   ```cpp
   // 如果代码中有这种嵌套调用
   refraction_assets.ground
   
   // 记得它对应Graphics注册的 "refraction_ground"
   ResourceRegistry::GetInstance().Get<Model>("refraction_ground")
   ```

## 完成后的最终测试

```bash
# 1. 编译项目
# 确保没有编译错误

# 2. 运行程序
# 检查控制台输出

# 3. 查看日志
# 如果有 "Resource 'xxx' not found" 错误，检查：
#    - Graphics.cpp 中是否注册了该资源
#    - 资源ID拼写是否一致
#    - 类型是否正确（Model vs PBRModel）
```

## 预期结果

重构完成后，Scene.cpp应该：
- ❌ 不再有任何 `SceneResourceRefs` 引用
- ❌ 不再有 `resources.scene_assets.xxx` 访问
- ✅ 所有资源通过 `ResourceRegistry::GetInstance().Get<T>(id)` 获取
- ✅ 代码更简洁，职责更清晰

祝重构顺利！ 🚀
