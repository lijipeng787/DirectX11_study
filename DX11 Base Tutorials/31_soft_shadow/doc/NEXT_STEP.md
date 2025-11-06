# 📌 NEXT_STEP

> 最近迭代（P0 阶段）实施细节：任务拆分 / 代码触点 / 验收标准 / 风险。聚焦“多光源接入 + 参数注册去重 + 基础数据驱动准备”。
> 更新时间：2025-11-06

---

## 1. 总体目标（两周窗口）
1. 支持基础多光源（方向光 + 点光）渲染，后续可扩展聚光。
2. 消除重复 Shader 参数注册路径（手动与自动冲突）。
3. 统一 Shader 别名/可选参数声明位置（ShaderBase 扩展）。
4. 为后续 `render_graph.json` 奠定稳态环境（接口收敛）。

---

## 2. 任务一览

| ID | 任务 | 类型 | 预估 | 依赖 | 验收 |
|----|------|------|------|------|------|
| T1 | LightManager 结构与接口 | 功能 | 0.5d | 无 | 方向光+点光可创建 |
| T2 | lights.json 解析 | 功能 | 0.5d | T1 | 解析至少 3 灯光 |
| T3 | Graphics 接入 LightManager | 集成 | 0.5d | T1,T2 | Frame 中遍历更新/写入参数 |
| T4 | Shader 参数注册去重（Compile 前过滤） | 重构 | 0.25d | 反射现有 | 编译日志无重复注册行 |
| T5 | ShaderBase 增加可选/忽略/别名虚函数 | 重构 | 0.5d | T4 | Graphics 中移除手动 optional 列表 |
| T6 | 更新 Validator 使用路径（只保留自动注册） | 清理 | 0.25d | T4,T5 | 所有 Shader 通过自动注册 + 可选/别名生效 |
| T7 | RenderGraph 编译输出新增“跳过已注册”日志 | 可观测 | 0.25d | T4 | 跳过信息出现且不重复 |
| T8 | 添加严格模式开关与文档 | 文档/改造 | 0.25d | T6 | STRICT 下缺失即编译失败 |
| T9 | 为 Light 参数加入来源标记（Global vs Per-Light） | 可观测 | 0.25d | T3 | 覆盖日志区分来源 |
| T10 | 基准场景：3 灯光 + 阴影 + 点光无阴影 | 场景 | 0.5d | T1-T3 | 运行稳定 FPS 无显著降级 |

> 单位：d = 人日。若并行可交叉推进。

---

## 3. 详细设计片段

### 3.1 LightManager 拟稿
```cpp
// LightTypes.h
enum class LightType { Directional, Point, Spot };

struct LightDesc {
    std::string name;
    LightType type;
    DirectX::XMFLOAT3 position {0,0,0};
    DirectX::XMFLOAT3 direction {0,-1,0};
    DirectX::XMFLOAT4 ambient {0,0,0,1};
    DirectX::XMFLOAT4 diffuse {1,1,1,1};
    bool castShadow = false;
    int shadowMapSize = 1024;
    // Point/Spot:
    float range = 10.0f;
    DirectX::XMFLOAT3 attenuation {1.0f, 0.09f, 0.032f};
    // Spot:
    float innerCone = 15.0f;
    float outerCone = 30.0f;
};

class ILightRuntime {
public:
    virtual ~ILightRuntime() = default;
    virtual void Update(float dt) {};
    virtual void FillParameters(ShaderParameterContainer& params, size_t index) const = 0; // index 用于数组式传输（后续）
};

class DirectionalLightRuntime : public ILightRuntime { /* ... */ };
class PointLightRuntime : public ILightRuntime { /* ... */ };

class LightManager {
public:
    bool LoadFromJson(const std::string& file);
    void Update(float dt);
    const std::vector<std::shared_ptr<ILightRuntime>>& GetLights() const { return lights_; }
    void InjectGlobalParameters(ShaderParameterContainer& globals) const; // 聚合或前 N 光源
private:
    std::vector<std::shared_ptr<ILightRuntime>> lights_;
};
```

### 3.2 lights.json 示例
```json
{
  "lights": [
    { "name": "sun", "type": "directional", "direction": [0.3, -0.7, 0.5], "ambient": [0.15,0.15,0.15,1], "diffuse": [1,1,1,1], "castShadow": true, "shadowMapSize": 2048 },
    { "name": "fill", "type": "directional", "direction": [-0.5,-0.3,-0.2], "diffuse": [0.3,0.35,0.4,1], "castShadow": false },
    { "name": "point1", "type": "point", "position": [5,3,0], "diffuse": [1,0.6,0.3,1], "range": 15, "attenuation": [1,0.07,0.017] }
  ]
}
```

### 3.3 ShaderBase 扩展
```cpp
class ShaderBase : public IShader {
public:
    virtual const std::vector<const char*>& GetOptionalParameters() const { static std::vector<const char*> v; return v; }
    virtual const std::vector<const char*>& GetIgnoredParameters() const { static std::vector<const char*> v; return v; }
    virtual const std::vector<std::pair<const char*, const char*>>& GetAliasParameters() const { static std::vector<std::pair<const char*, const char*>> v; return v; }
};
// SoftShadowShader 例：重写别名与可选参数
```

### 3.4 注册去重伪代码
```cpp
// RenderGraph::Compile()
if (parameter_validator_) {
  for (auto& pass : sorted_passes_) {
    auto* base = dynamic_cast<ShaderBase*>(pass->shader_.get());
    if (!base) continue;
    auto name = base->GetShaderName();
    if (parameter_validator_->IsShaderRegistered(name)) {
        Logger::LogInfo("Skip auto-register (already registered): " + name);
        continue;
    }
    auto reflected = base->GetReflectedParameters();
    // 应用别名/忽略/可选
    auto adjusted = ApplyShaderMeta(reflected, base);
    parameter_validator_->RegisterShader(name, adjusted);
  }
}
```

### 3.5 全局参数注入更新
将原单光源相关：`lightPosition`, `lightDirection`, `ambientColor`, `diffuseColor` 替换为：
```
light0Direction, light0Diffuse, light0Ambient
light1Direction, light1Diffuse ...
point0Position, point0Diffuse, point0Range
```
（初期限制前 4 光源；后续数组式常量缓冲优化）。

---

## 4. 验收标准（Check List）
| 类别 | 验收点 |
|------|--------|
| 多光源渲染 | 三个光源分别影响场景（观察光照差异与方向阴影） |
| 性能 | 与单光源相比帧率下降可控（≤20%）在测试场景中 |
| 参数注册 | 编译日志出现 “Skip auto-register” 且无重复警告 |
| 别名生效 | SoftShadowShader 中使用 `texture` 仍正确绑定反射出的 `shaderTexture` |
| Validator 严格模式 | 缺失必需参数时编译失败并输出缺失列表 |
| 文档 | ROADMAP 标记 P0 进行中；OVERVIEW 中高优先级更新为“进行中” |

---

## 5. 代码触点与修改范围
| 文件 | 修改类型 |
|------|----------|
| `Graphics.cpp` | 移除单光源逻辑 / 接入 LightManager 更新注入 |
| `Light.cpp` / 新增 `LightManager.*` | 新增管理与多类型 LightDesc 转换 |
| `RenderGraph.cpp` | 添加注册去重逻辑 + 日志输出 |
| `ShaderBase.h/.cpp` | 扩展元数据虚方法接口 |
| `ShaderParameterValidator.cpp` | 保持接口；仅使用自动注册路径 |
| `ARCHITECTURE_OVERVIEW.md` | 标记多光源工作“进行中” |
| `TECHNICAL_ROADMAP.md` | P0 进度更新段落（后续） |

---

## 6. 风险与回退
| 风险 | 回退策略 |
|------|----------|
| 多光源影响性能剧烈 | 限制最大光源数（宏或配置）恢复单光源路径 |
| 别名失效导致参数验证失败 | 临时恢复 Graphics 手动注册路径（保留函数） |
| 自动注册跳过误判 | 添加调试标志强制重新注册输出对比 |
| 光源参数过多污染 Global 容器 | 引入 FrameContext 过渡（记录光源数组） |

---

## 7. 后续准备（P1 预热）
- 收集当前所有 Pass 资源读写关系，生成临时 JSON skeleton。
- 统计常用材质参数集合（PBR / Diffuse / Shadow）作为材质 JSON 原型。
- 调研常量缓冲布局（多光源批量传输）结构设计。

---

> 本文件是“近期迭代执行板”。完成后条目移动到 ROADMAP（历史完成列表），新一轮迭代重置表格。保持轻量，不做冗长回顾。
