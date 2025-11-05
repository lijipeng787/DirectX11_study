# 类型系统Bug修复：type_index不匹配

## 🐛 Bug描述

所有shader查找失败，Scene加载0个对象：
```
[Scene] [ERROR] Scene object missing shader: cube
[Scene] [ERROR] Scene object missing shader: sphere
...
[Scene] [INFO] Loaded 0 objects from JSON
```

## 🔍 根本原因

**类型不匹配导致ResourceRegistry查找失败！**

### 问题分析

1. **注册时的类型：**
   ```cpp
   // shader_assets_.soft_shadow 的类型是 std::shared_ptr<SoftShadowShader>
   registry.Register("soft_shadow", shader_assets_.soft_shadow);
   
   // 实际调用的是：
   // template<typename T>
   // void Register(const std::string& id, std::shared_ptr<T> resource)
   // 其中 T = SoftShadowShader
   ```

2. **查找时的类型：**
   ```cpp
   // GetShaderByName 查找 std::shared_ptr<IShader>
   return ResourceRegistry::GetInstance().Get<IShader>(name);
   ```

3. **为什么失败？**
   ```cpp
   // ResourceRegistry使用 std::type_index 作为key
   resources_[std::type_index(typeid(T))]
   
   // 注册时：typeid(SoftShadowShader) → type_index_A
   // 查找时：typeid(IShader)           → type_index_B
   // type_index_A ≠ type_index_B  ❌ 找不到！
   ```

### C++类型系统规则

```cpp
std::shared_ptr<Derived> ≠ std::shared_ptr<Base>  // 在 typeid 中是不同类型！

// 即使 Derived 继承自 Base：
class SoftShadowShader : public IShader { ... };

// 这两个类型在 std::type_index 中完全不同：
typeid(std::shared_ptr<SoftShadowShader>) ≠ typeid(std::shared_ptr<IShader>)
```

## 🔧 修复方案

### 方案1：显式指定注册类型（✅ 推荐）

```cpp
// 修复前
registry.Register("soft_shadow", shader_assets_.soft_shadow);

// 修复后：显式指定为IShader类型
registry.Register<IShader>("soft_shadow", shader_assets_.soft_shadow);
```

这样会调用：
```cpp
template<typename T>
void Register(const std::string& id, std::shared_ptr<T> resource) {
    // T = IShader (显式指定)
    // resource 是 shared_ptr<SoftShadowShader>，可以隐式转换为 shared_ptr<IShader>
    resources_[std::type_index(typeid(IShader))][id] = resource;
}
```

### 方案2：重载Register接受多态（复杂）

```cpp
// 需要特化处理基类指针
template<typename Base, typename Derived>
void RegisterAs(const std::string& id, std::shared_ptr<Derived> resource) {
    static_assert(std::is_base_of_v<Base, Derived>);
    std::shared_ptr<Base> base_ptr = resource;
    Register(id, base_ptr);
}
```

## ✅ 已修复的注册

### Graphics.cpp

```cpp
// 修复前（类型不匹配）
registry.Register("depth", shader_assets_.depth);                     // → SoftShadowShader
registry.Register("shadow", shader_assets_.shadow);                   // → ShadowShader
registry.Register("texture", shader_assets_.texture);                 // → TextureShader
registry.Register("horizontal_blur", shader_assets_.horizontal_blur); // → HorizontalBlurShader
registry.Register("vertical_blur", shader_assets_.vertical_blur);     // → VerticalBlurShader
registry.Register("soft_shadow", shader_assets_.soft_shadow);         // → SoftShadowShader
registry.Register("pbr", shader_assets_.pbr);                         // → PbrShader
registry.Register("diffuse_lighting", shader_assets_.diffuse_lighting); // → SimpleLightShader

// 修复后（显式转换为IShader）
registry.Register<IShader>("depth", shader_assets_.depth);
registry.Register<IShader>("shadow", shader_assets_.shadow);
registry.Register<IShader>("texture", shader_assets_.texture);
registry.Register<IShader>("horizontal_blur", shader_assets_.horizontal_blur);
registry.Register<IShader>("vertical_blur", shader_assets_.vertical_blur);
registry.Register<IShader>("soft_shadow", shader_assets_.soft_shadow);
registry.Register<IShader>("pbr", shader_assets_.pbr);
registry.Register<IShader>("diffuse_lighting", shader_assets_.diffuse_lighting);
```

## 📊 为什么Model不需要修复？

```cpp
// Models 已经声明为 Model 类型
struct SceneAssets {
    std::shared_ptr<Model> cube;    // ✅ 直接是 Model
    std::shared_ptr<Model> sphere;  // ✅ 直接是 Model
    std::shared_ptr<Model> ground;  // ✅ 直接是 Model
};

// 注册时类型就是 Model
registry.Register("cube", scene_assets_.cube);  // shared_ptr<Model>

// 查找时也是 Model
GetModelByName() → Get<Model>(name);  // shared_ptr<Model>

// ✅ 类型匹配！
```

但Shader不同：
```cpp
// Shaders 声明为具体类型
struct ShaderAssets {
    std::shared_ptr<SoftShadowShader> soft_shadow;  // ❌ 具体类型
    std::shared_ptr<PbrShader> pbr;                 // ❌ 具体类型
};

// 查找时使用基类
GetShaderByName() → Get<IShader>(name);  // ❌ 类型不匹配！
```

## 🎯 教训

### 1. std::type_index不考虑继承关系

```cpp
class Base {};
class Derived : public Base {};

// 这些是不同的类型：
typeid(Base) ≠ typeid(Derived)
typeid(std::shared_ptr<Base>) ≠ typeid(std::shared_ptr<Derived>)
```

### 2. 使用基类查找时，必须以基类注册

```cpp
// ❌ 错误：注册为Derived，查找Base
registry.Register("id", std::make_shared<Derived>());
auto ptr = registry.Get<Base>("id");  // 失败！

// ✅ 正确：显式转换为Base
registry.Register<Base>("id", std::make_shared<Derived>());
auto ptr = registry.Get<Base>("id");  // 成功！
```

### 3. 统一查找接口的类型

如果所有查找都通过`Get<IShader>()`，那么所有注册也必须用`Register<IShader>()`。

## 🔍 调试技巧

添加到ResourceRegistry::Get的调试输出：
```cpp
template <typename T>
std::shared_ptr<T> Get(const std::string &id) const {
    auto type_it = resources_.find(std::type_index(typeid(T)));
    if (type_it == resources_.end()) {
        std::cerr << "[ResourceRegistry] Type not found: " 
                  << typeid(T).name() << std::endl;
        return nullptr;
    }
    
    auto id_it = type_it->second.find(id);
    if (id_it == type_it->second.end()) {
        std::cerr << "[ResourceRegistry] ID '" << id 
                  << "' not found for type " << typeid(T).name() << std::endl;
        std::cerr << "  Available IDs: ";
        for (const auto &[available_id, _] : type_it->second) {
            std::cerr << "'" << available_id << "' ";
        }
        std::cerr << std::endl;
        return nullptr;
    }
    
    return std::any_cast<std::shared_ptr<T>>(id_it->second);
}
```

## ✅ 验证清单

- [x] 所有shader注册使用 `Register<IShader>`
- [x] 所有shader查找使用 `Get<IShader>`
- [x] Model注册/查找类型一致（都是Model）
- [x] PBRModel注册/查找类型一致（都是PBRModel）
- [x] RenderTexture注册/查找类型一致
- [x] OrthoWindow注册/查找类型一致

## 🎉 预期结果

修复后应该看到：
```
=== Setting up RenderGraph ===
[Graphics] Registered shader parameters for validation
[Scene] [INFO] Loaded 14 objects from JSON  ✅
=== RenderGraph Setup Complete ===
```

---

**Bug已修复！现在类型系统正确匹配了。** ✨
