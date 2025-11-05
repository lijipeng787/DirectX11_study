# ResourceRegistry 重构总结

## 已完成

1. ✅ 创建 `ResourceRegistry.h` 和 `ResourceRegistry.cpp`
   - 统一的类型安全资源注册系统
   - 使用 `std::type_index` + `std::any` 实现类型擦除
   - 线程安全（mutex保护）

2. ✅ 修改 `Scene.h`
   - 删除 `SceneResourceRefs` 巨型结构体
   - 删除 `Initialize(resources, ...)` 中的 resources 参数
   - 删除内部 5 个资源缓存 map (model_cache_, shader_cache_, etc.)
   - 删除 `BuildResourceCaches()` 方法
   - 修改所有 `GetXxxByName()` 方法签名，移除 resources 参数

3. ✅ 修改 `Scene.cpp` 部分
   - 添加 `#include "ResourceRegistry.h"`
   - 重写所有 `GetXxxByName()` 方法，直接调用 `ResourceRegistry::GetInstance().Get<T>(id)`
   - 删除 `BuildResourceCaches()` 实现（117行代码）

## 待完成

### 1. 修改 Scene.cpp 中的 `BuildSceneObjects()`

当前签名：
```cpp
void Scene::BuildSceneObjects(const SceneResourceRefs &resources,
                              const SceneConstants &constants,
                              StandardRenderGroup *cube_group,
                              StandardRenderGroup *pbr_group)
```

需要改为：
```cpp
void Scene::BuildSceneObjects(const SceneConstants &constants,
                              StandardRenderGroup *cube_group,
                              StandardRenderGroup *pbr_group)
```

**需要替换的地方**（约150行代码）：
- `resources.scene_assets.cube` → `ResourceRegistry::GetInstance().Get<Model>("cube")`
- `resources.shader_assets.soft_shadow` → `ResourceRegistry::GetInstance().Get<IShader>("soft_shadow")`
- 等等...

### 2. 修改 Scene.cpp 中的 `BuildSceneObjectsFromJson()`

当前签名：
```cpp
bool Scene::BuildSceneObjectsFromJson(const nlohmann::json &j,
                                      const SceneResourceRefs &resources,
                                      const SceneConstants &constants,
                                      StandardRenderGroup *cube_group,
                                      StandardRenderGroup *pbr_group)
```

需要改为：
```cpp
bool Scene::BuildSceneObjectsFromJson(const nlohmann::json &j,
                                      const SceneConstants &constants,
                                      StandardRenderGroup *cube_group,
                                      StandardRenderGroup *pbr_group)
```

**需要修改的调用**（约 10 处）：
```cpp
// 修改前
model = GetModelByName(model_name, resources);

// 修改后
model = GetModelByName(model_name);
```

### 3. 修改 Graphics.cpp

这是**最大的改动**，需要：

1. **删除 SceneResourceRefs 的构建代码**（约60行硬编码）：
```cpp
// 删除这些代码
SceneResourceRefs scene_resources;
scene_resources.scene_assets.cube = scene_assets_.cube;
scene_resources.scene_assets.sphere = scene_assets_.sphere;
// ... 还有50多行
```

2. **在资源加载时注册到 ResourceRegistry**：
```cpp
// 在 Initialize() 或 LoadResources() 中添加
auto& registry = ResourceRegistry::GetInstance();

// 注册模型
registry.Register("cube", scene_assets_.cube);
registry.Register("sphere", scene_assets_.sphere);
registry.Register("ground", scene_assets_.ground);
registry.Register("pbr_sphere", scene_assets_.pbr_sphere);

// 注册shader
registry.Register("depth", shader_assets_.depth);
registry.Register("shadow", shader_assets_.shadow);
registry.Register("soft_shadow", shader_assets_.soft_shadow);
// ...

// 注册render texture
registry.Register("shadow_map", render_targets_.shadow_map);
registry.Register("downsampled_shadow", render_targets_.downsampled_shadow);
// ...

// 注册ortho window
registry.Register("small_window", ortho_windows_.small_window);
registry.Register("fullscreen_window", ortho_windows_.fullscreen_window);
```

3. **修改 Scene 初始化调用**：
```cpp
// 修改前
if (!scene_.Initialize(scene_resources, scene_constants, "./data/scene.json",
                       cube_group_.get(), pbr_group_.get())) {
  // ...
}

// 修改后
if (!scene_.Initialize(scene_constants, "./data/scene.json",
                       cube_group_.get(), pbr_group_.get())) {
  // ...
}
```

## 优势

1. **消除硬编码**：不再需要手动维护 SceneResourceRefs 的映射
2. **统一接口**：所有资源通过 `registry.Get<T>(id)` 访问
3. **类型安全**：编译期类型检查
4. **可扩展**：添加新资源类型无需修改 Scene 代码
5. **支持热重载**：ResourceRegistry 提供 Register/Unregister 接口

## 代码量变化

- 删除：约 300 行（SceneResourceRefs 定义 + 缓存代码 + 映射代码）
- 添加：约 150 行（ResourceRegistry 实现）
- 净减少：约 150 行

## 下一步行动

1. 修改 Scene.cpp 的 BuildSceneObjects（150行）
2. 修改 Scene.cpp 的 BuildSceneObjectsFromJson（10处调用）
3. 修改 Graphics.cpp 的资源注册逻辑（60行硬编码 → 30行注册）
4. 编译测试
5. 运行时验证

## 风险

- **需要全面测试**：资源ID字符串必须匹配（如 "cube" vs "Cube"）
- **错误处理**：ResourceRegistry::Get() 返回 nullptr 时需要检查
- **编译依赖**：需要在 vcxproj 中添加 ResourceRegistry.cpp
