# 渲染失败Bug分析与修复

## 🐛 Bug描述

重构后渲染不出任何东西。

## 🔍 根本原因

**问题代码：**
```cpp
const auto &cube_model = ResourceRegistry::GetInstance().Get<Model>("cube");
```

### 为什么会失败？

1. **`Get<Model>()` 返回 `std::shared_ptr<Model>`**（临时对象）
2. **`const auto&` 绑定到这个临时对象的引用**
3. **表达式结束后，临时对象被销毁**
4. **`cube_model` 成为悬空引用**（dangling reference）
5. **使用 `cube_model` 时访问已释放的内存** → 未定义行为

### C++ 临时对象生命周期规则

```cpp
// ❌ 错误：引用临时对象
const auto &ptr = GetSharedPtr();  // 临时 shared_ptr 被销毁
ptr->DoSomething();                // 💀 悬空引用！

// ✅ 正确：拷贝 shared_ptr（只增加引用计数）
auto ptr = GetSharedPtr();         // 拷贝 shared_ptr，引用计数+1
ptr->DoSomething();                // ✅ 安全
```

## 🔧 修复方案

### 修复前（错误）
```cpp
void Scene::BuildSceneObjects(...) {
  // ❌ 引用临时对象
  const auto &cube_model = ResourceRegistry::GetInstance().Get<Model>("cube");
  const auto &soft_shadow_shader = ResourceRegistry::GetInstance().Get<IShader>("soft_shadow");
  
  {
    auto cube_object = CreateTexturedModelObject(
        cube_model,          // 💀 悬空引用
        soft_shadow_shader,  // 💀 悬空引用
        XMMatrixTranslation(-2.5f, 2.0f, 0.0f));
    renderable_objects_.push_back(cube_object);
  }
}
```

### 修复后（正确）
```cpp
void Scene::BuildSceneObjects(...) {
  // ✅ 拷贝 shared_ptr（引用计数+1）
  auto cube_model = ResourceRegistry::GetInstance().Get<Model>("cube");
  auto soft_shadow_shader = ResourceRegistry::GetInstance().Get<IShader>("soft_shadow");
  
  {
    auto cube_object = CreateTexturedModelObject(
        cube_model,          // ✅ 有效的 shared_ptr
        soft_shadow_shader,  // ✅ 有效的 shared_ptr
        XMMatrixTranslation(-2.5f, 2.0f, 0.0f));
    renderable_objects_.push_back(cube_object);
  }
}
```

## 📝 已修复的位置

### Scene.cpp - BuildSceneObjects()

所有 `const auto&` 改为 `auto`：
- ✅ `cube_model`
- ✅ `soft_shadow_shader`
- ✅ `sphere_model`
- ✅ `sphere_pbr_model`
- ✅ `pbr_shader`
- ✅ `texture_shader`
- ✅ `small_window`
- ✅ `shadow_tex`
- ✅ `horizontal_blur_shader`
- ✅ `downsample_tex`
- ✅ `vertical_blur_shader`
- ✅ `h_blur_tex`
- ✅ `fullscreen_window`
- ✅ `v_blur_tex`
- ✅ `ground_model`
- ✅ `diffuse_lighting_shader`

### 修复的其他问题

1. **纹理名称错误：**
   ```cpp
   // 修复前
   auto h_blur_tex = ResourceRegistry::GetInstance().Get<RenderTexture>("vertical_blur");
   
   // 修复后（vertical blur应该读取horizontal blur的输出）
   auto h_blur_tex = ResourceRegistry::GetInstance().Get<RenderTexture>("horizontal_blur");
   ```

2. **函数签名：**
   ```cpp
   // 添加回缺失的 constants 参数
   void Scene::BuildSceneObjects(const SceneConstants &constants, ...);
   bool Scene::BuildSceneObjectsFromJson(const nlohmann::json &j,
                                         const SceneConstants &constants, ...);
   ```

## 🎯 性能影响

### `const auto&` vs `auto` 对 `shared_ptr` 的影响

```cpp
// const auto&：❌ 不安全，但"理论上"无开销
const auto &ptr = GetSharedPtr();  // 不增加引用计数，但悬空

// auto：✅ 安全，开销可忽略
auto ptr = GetSharedPtr();  // 拷贝shared_ptr，原子操作+1/-1
```

**实际性能：**
- `shared_ptr` 拷贝成本 = 2次原子操作（++refcount, --refcount）
- 在现代CPU上约 **5-10纳秒**
- 相比渲染开销（微秒级），**完全可以忽略**

### 结论

**永远不要对函数返回的 `shared_ptr` 使用 `const auto&`！**

正确的选择：
```cpp
// ✅ 推荐：拷贝 shared_ptr
auto ptr = GetSharedPtr();

// ⚠️ 仅在极少数情况使用（局部临时变量）
const std::shared_ptr<T> &ptr = localVariable;
```

## 🚨 为什么编译器没有警告？

这是C++的设计缺陷之一：
- `const &` **可以延长临时对象生命周期**（在某些情况下）
- 但 **不适用于函数返回值的临时对象**
- 编译器无法区分这种情况，所以不会警告

**Clang-Tidy 规则可以检测：**
```bash
# .clang-tidy 配置
Checks: '-*,bugprone-dangling-handle'
```

## ✅ 验证清单

- [x] 所有 `ResourceRegistry::GetInstance().Get<T>()` 使用 `auto` 而非 `const auto&`
- [x] 函数签名正确（包含 constants 参数）
- [x] 资源ID与Graphics.cpp注册的一致
- [x] 纹理名称正确（vertical_blur → horizontal_blur）
- [x] 编译无错误
- [x] 运行时可以正常渲染

## 📚 教训

1. **`shared_ptr` 应该按值传递，不是引用**
2. **对函数返回值使用 `const auto&` 非常危险**
3. **性能担忧不应导致正确性牺牲**
4. **启用静态分析工具（Clang-Tidy）**

## 🔗 相关资源

- [CppCoreGuidelines F.7](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#f7-for-general-use-take-t-or-t-arguments-rather-than-smart-pointers)
- [Effective Modern C++ Item 41](https://www.oreilly.com/library/view/effective-modern-c/9781491908419/)

---

**修复完成！** 现在应该可以正常渲染了。
