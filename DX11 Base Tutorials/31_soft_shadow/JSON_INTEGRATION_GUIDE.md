# JSON 配置系统集成指南

## 需要引入的第三方库

为了实现场景配置系统，需要引入 **nlohmann/json** 单头文件库。

### 步骤 1: 下载 nlohmann/json

**方式一**: 直接下载（最简单）
1. 访问 https://github.com/nlohmann/json/releases
2. 下载最新版本的 `json.hpp` 单头文件
3. 或者直接从这里下载: https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp
4. 将文件放在项目的 `include/nlohmann/` 目录中

**方式二**: 使用包管理器（如果支持）
```bash
# vcpkg
vcpkg install nlohmann-json

# conan
conan install nlohmann_json/3.11.2@
```

### 步骤 2: 集成到项目

#### 2.1 创建 include 目录

在项目根目录创建 `include` 目录：
```
DX11 Base Tutorials/31_soft_shadow/
  ├── include/
  │   └── nlohmann/
  │       └── json.hpp          # 从这里下载
  ├── data/
  │   └── scene_config.json     # 已创建
  └── ...
```

#### 2.2 修改项目设置

在 `31_soft_shadow.vcxproj` 中添加包含路径：

```xml
<AdditionalIncludeDirectories>
  $(ProjectDir)include;
  ..\..\DirectXTK\Inc;
  %(AdditionalIncludeDirectories)
</AdditionalIncludeDirectories>
```

#### 2.3 在代码中使用

```cpp
#include <nlohmann/json.hpp>
using json = nlohmann::json;

// 解析 JSON 文件
std::ifstream file("./data/scene_config.json");
json config;
file >> config;

// 访问配置
std::string model_path = config["models"]["cube"]["model_path"];
int shadow_width = config["render_targets"]["shadow_depth"]["width"];
```

### 步骤 3: 验证安装

创建一个简单的测试文件验证库是否正确集成：

```cpp
// test_json.cpp
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

int main() {
    json j;
    j["test"] = "Hello JSON!";
    std::cout << j.dump() << std::endl;
    return 0;
}
```

---

## 配置系统设计

### JSON 配置结构

已创建的 `scene_config.json` 包含以下结构：

```json
{
  "models": { ... },           // 模型资源配置
  "render_targets": { ... },   // 渲染目标配置
  "ortho_windows": { ... },    // 正交窗口配置
  "constants": { ... }         // 常量配置
}
```

### 使用场景配置

配置系统将支持：
- ✅ 替换硬编码资源路径
- ✅ 运行时切换场景
- ✅ 热重载配置
- ✅ 编辑器工具支持

---

## 下一步

一旦您下载并集成了 `json.hpp`，我将实现：
1. `SceneConfigLoader` 类
2. JSON 解析逻辑
3. 配置应用到 Graphics 系统
4. 测试和验证

**请按照步骤 1 和 2 完成 JSON 库的集成，然后告诉我，我将继续实现配置系统。**

---

## 参考资料

- **nlohmann/json GitHub**: https://github.com/nlohmann/json
- **文档**: https://json.nlohmann.me/
- **单头文件下载**: https://github.com/nlohmann/json/releases (选择 `json.hpp` 文件)
