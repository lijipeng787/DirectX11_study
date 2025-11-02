# 场景配置系统使用说明

## 快速开始

### 修改场景配置

直接编辑 `data/scene_config.json` 文件即可修改场景配置。

### 配置文件位置

```
data/scene_config.json
```

### 基本格式

```json
{
  "models": { ... },           // 模型资源路径
  "render_targets": { ... },   // 渲染目标配置
  "ortho_windows": { ... },    // 正交窗口配置
  "constants": { ... }         // 场景常量
}
```

---

## 常用配置

### 修改模型路径

```json
{
  "models": {
    "cube": {
      "model_path": "./data/cube.txt",
      "texture_path": "./data/wall01.dds"
    }
  }
}
```

### 调整渲染分辨率

```json
{
  "render_targets": {
    "shadow_depth": {
      "width": 2048,        // 从 1024 改为 2048
      "height": 2048,       // 从 1024 改为 2048
      "depth": 1000.0,
      "near": 1.0
    }
  }
}
```

### 屏幕自适应尺寸

使用 `-1` 表示使用屏幕尺寸：

```json
{
  "render_targets": {
    "reflection_map": {
      "width": -1,    // 自动使用屏幕宽度
      "height": -1,   // 自动使用屏幕高度
      "depth": 1000.0,
      "near": 1.0
    }
  }
}
```

### 调整场景参数

```json
{
  "constants": {
    "water_plane_height": 3.0,           // 修改水面高度
    "water_reflect_refract_scale": 0.02  // 修改反射/折射缩放
  }
}
```

---

## 技术细节

### 配置文件加载

配置在 `Graphics::Initialize()` 时自动加载：

```cpp
// 1. 尝试从 JSON 加载
SceneConfig::LoadFromJson(scene_config_, "./data/scene_config.json");

// 2. 如果失败，自动使用默认配置
```

### 默认配置

如果 JSON 文件缺失或格式错误，系统自动使用硬编码的默认配置。

### 配置优先级

1. JSON 配置（如果成功加载）
2. 默认配置（fallback）

---

## 配置项说明

### models

普通模型配置（cube, sphere, ground）。

### refraction

折射场景模型（ground, wall, bath, water）。

### render_targets

渲染目标配置：
- `shadow_depth` - 阴影深度图
- `shadow_map` - 阴影贴图
- `downsampled_shadow` - 降采样阴影
- `horizontal_blur` / `vertical_blur` - 模糊效果
- `upsampled_shadow` - 上采样阴影
- `reflection_map` - 反射图
- `water_refraction` - 水面折射
- `water_reflection` - 水面反射

### ortho_windows

正交窗口配置（用于后处理效果）。

### constants

场景常量参数。

---

## 故障排除

### 配置未生效

1. 检查 JSON 文件格式是否正确
2. 查看日志中的配置加载信息
3. 确认文件路径正确

### JSON 解析错误

系统会自动使用默认配置，并在日志中输出错误信息。

### 性能问题

大幅增加渲染目标尺寸可能影响性能，建议逐步调整。

---

## 相关文档

- `JSON_CONFIG_FINAL_SUMMARY.md` - 完整实施总结
- `JSON_INTEGRATION_GUIDE.md` - JSON 库集成指南
- `CONFIG_SYSTEM_STATUS.md` - 系统状态报告

---

## 示例

完整的配置文件示例请参考 `data/scene_config.json`。

