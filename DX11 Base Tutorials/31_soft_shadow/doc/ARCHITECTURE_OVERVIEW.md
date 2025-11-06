# 🏗️ ARCHITECTURE_OVERVIEW

> 统一的架构现状 / Review / 不足与风险清单（取代分散的多篇评审类文档）。
> 更新时间：2025-11-06

---

## 目录
1. 执行摘要
2. 分层目标与当前结构对照
3. 关键子系统现状评估（评分矩阵）
4. 核心能力与优势
5. 架构不足与技术债务（分级）
6. 风险清单与影响分析
7. 优先级行动建议（概要版）
8. 附：主要运行路径（数据/参数/渲染）

---

## 1. 执行摘要
当前代码已经实现：
- 基础渲染管线（多 Pass 顺序执行 + 软阴影模糊链）。
- Shader 反射与自动参数候选匹配（含模糊匹配与单纹理回退）。
- 类型安全参数容器（`std::variant`）与参数合并优先级链（Global < Pass < Object < Callback）。
- 双资源层：`ResourceManager`（创建/缓存） + `ResourceRegistry`（统一注册/泛型访问）。

仍存在的主要问题：
- 单光源硬编码，无法多光源性能/质量测试。
- RenderGraph 仍是顺序列表，无 DAG / 生命周期 / 依赖明确化。
- Graphics “上帝对象”职责过宽（设备/场景/剔除/动画/管线）。
- Shader 参数注册存在重复路径（手动 + 编译期自动覆盖）。
- 缺少材质 / Prefab / RenderGraph JSON 化等数据驱动能力。
- 参数系统对 Sampler / UAV / StructuredBuffer 支持缺失。

总体成熟度（主观）：45%（基础能力 → 结构化工程化过渡阶段）。

---

## 2. 分层目标与当前结构对照

| 规划层 | 目标职责 | 现状实现 | 差距 |
|--------|----------|----------|------|
| Platform | DX11 设备 / 窗口 / 输入薄封装 | `DirectX11Device` | 基本满足 |
| Resource Layer | 统一资源创建/缓存/生命周期 | `ResourceManager` + `ResourceRegistry` 双层 | 所有权边界不清晰；无热重载 |
| Scene Layer | 场景对象 / 动画 / 变换 | `Scene` + JSON 加载 + 动画配置 | 动画与加载耦合；无多场景管理 |
| Visibility Layer | 剔除 / 脏标记 / 可见集 | 简易 `FrustumClass` + 每帧线性遍历 | 无脏标记；无可见性缓存 |
| Pipeline Declaration | 声明式 RenderGraph (DAG) | 顺序容器 + 手工 AddPass | 无依赖表达 / 环检测 |
| Pipeline Execution | Pass 编译/排序/调试 | 顺序执行 + 简单打印 | 无生命周期推导 / 失败降级 |
| Presentation/UI | 字体 / 文本 / 合成 | `Font`, `Text`, OrthoWindow | 满足演示需求 |

---

## 3. 子系统现状评估

| 子系统 | 评分 | 优势 | 主要不足 |
|--------|------|------|----------|
| RenderGraph | ⭐⭐☆☆☆ | 可读性好（链式 AddPass） | 无 DAG / 无资源生命周期 / 编译期仅顺序复制 |
| Shader 参数系统 | ⭐⭐⭐⭐☆ | 强类型 / 反射 / 自动候选匹配 | 重复注册、Sampler 类型未进入容器 |
| 资源管理 | ⭐⭐⭐☆☆ | 缓存完善 / 统计与引用计数 | 双层重复 / 无热重载 / 无统一生命周期清理策略 |
| 场景系统 | ⭐⭐☆☆☆ | JSON 配置基础完备（模型/动画） | 无灯光/材质数据驱动 / 动画与加载耦合 |
| 光照系统 | ⭐☆☆☆☆ | 可用于单光源阴影演示 | 单光源硬编码 / 无配置 / 无多类型光源 |
| 后处理链 | ⭐⭐☆☆☆ | Pass 可插入扩展 | 配置硬编码 / 参数需手写 / 无动态组合 |
| 可见性与剔除 | ⭐⭐☆☆☆ | 基础视锥剔除可用 | 无体积层次结构 / 无脏标记增量更新 |
| 文档体系 | ⭐⭐⭐☆☆ | 内容充分、问题识别全面 | 冗余/重复/状态漂移，缺统一权威入口 |

评分说明：⭐=20%（演示级），⭐⭐=40%（基础），⭐⭐⭐=60%（可用），⭐⭐⭐⭐=80%（工程化），⭐⭐⭐⭐⭐=100%（成熟）。

---

## 4. 核心能力与优势
- Shader 反射已贯穿：减少硬编码参数名；未来可扩展自动绑定（AutoBinder）。
- 参数容器设计合理：支持覆盖溯源/合并策略，为调试与诊断打基础。
- 资源注册泛型化（`ResourceRegistry`）：为后续材质、光源、Prefab 统一接入打好入口。
- RenderGraph 打印提供初步可视化（资源生产者/消费者/未使用输出），利于演化成编译诊断报告。
- 动画配置已 JSON 化（旋转轴/初始角度/速度），具备向更丰富行为系统扩展的范式。

---

## 5. 架构不足与技术债务

### 🔴 高优先级
1. 单光源硬编码（`Graphics::InitializeLight` / `Frame` 内移动逻辑）阻塞多光源测试与阴影性能评估。
2. RenderGraph 缺少真实编译阶段：无依赖排序/环检测/资源生命周期分析，无法支撑复杂管线扩展。
3. Graphics 职责过载：创建、场景、动画、剔除、执行与调试杂糅，难以单元测试或独立扩展。
4. Shader 参数注册重复路径：`Graphics::RegisterShaderParameters` 与 `RenderGraph::Compile` 自动注册可能产生覆盖/别名失效。

### 🟡 中优先级
5. 资源层双角色：`ResourceManager`（创建+缓存）与 `ResourceRegistry`（注册+访问），生命周期与所有权职责边界模糊。
6. 无材质系统：对象回调直接塞参数，无法复用/序列化/热更，PBR 参数散落。
7. 参数系统缺类型扩展：Sampler / UAV / StructuredBuffer 反射与存储不对齐。
8. 可见性策略单一：每帧全量视锥测试，无脏标记/层次加速结构。

### 🟢 低优先级
9. Tag 系统扁平字符串集，缺命名空间/层级/类别（如 render.layer / material.tag）。
10. 初始化模式多处“默认构造 + Initialize()”二阶段，削弱异常安全与编译期保障。
11. 缺统一 FrameContext（时间、摄像机、光照、统计）传递，当前通过 Global 参数容器隐式堆积。
12. 文档多版本交叉导致状态漂移与认知成本增高。

---

## 6. 风险清单与影响分析

| 风险 | 描述 | 影响 | 当前缓解 | 建议 |
|------|------|------|----------|------|
| 重复参数注册 | 手动 + 自动两套逻辑可能冲突别名/optional | 模糊警告/验证误报 | 无 | 编译前 `IsShaderRegistered` 过滤 + 别名归一 |
| 光照系统单点 | 仅一个 `Light` 对象 | 无法扩大测试场景 | 暂不涉及性能测试 | 引入 `LightManager` + JSON |
| RenderGraph 不可扩展 | 无拓扑/生命周期 | 新增复杂 Pass 成本高 | 简单打印辅助 | 引入 DAG & Resource lifetime map |
| 缺材质抽象 | 回调散乱 | 参数复用低 / 难以序列化 | variant 可容纳参数 | `Material` 聚合 Shader + 参数快照 |
| 缺脏标记 | 变换更新全量剔除 | 可扩展性与性能隐患 | frustum 基础存在 | AABB 更新标记 + 可见集缓存 |
| 单例滥用 | ResourceManager 全局 | 测试隔离/多实例渲染受限 | 函数式封装尚可 | 构造注入 + 上下文对象 |
| Fallback 模糊匹配 | 可能掩盖命名错误 | 调试时间延长 | 有日志警告 | 模式开关 + 严格模式统计 |
| 无热重载 | 改配置需重编 | 调试缓慢 | 无 | 文件监控 + 增量重编 |

---

## 7. 优先级行动建议（概要）

| 阶段 | 目标 | 关键产出 | 成功指标 |
|------|------|----------|----------|
| P0 | 稳定基础、去重复、引入光源数据 | `LightManager` / 注册去重 / 别名统一 | 多光源渲染成功；自动注册无重复日志 |
| P1 | 数据驱动雏形 | `render_graph.json` / `Material` PoC | 通过 JSON 重排管线；材质可复用 |
| P2 | 架构分层与可见性优化 | `FrameContext` / Graphics 拆分第一步 / 脏标记 | CPU 剔除耗时下降；模块单测可写 |
| P3 | 高级扩展与诊断 | DAG / 资源生命周期 / 参数可视化 | 编译报告含排序/环检测/未使用资源 |

详细路线与任务拆分见 `TECHNICAL_ROADMAP.md` 与近期执行详单 `NEXT_STEP.md`。

---

## 8. 运行路径示意

### 8.1 Shader 参数系统架构
```
┌────────────────────────────────────────────────┐
│     Shader参数管理系统 (3层架构)                │
├────────────────────────────────────────────────┤
│ [1] ShaderParameterContainer (存储层)          │
│     - 类型安全 variant (Matrix/Vector/Float/Texture) │
│     - 参数来源跟踪 (Global/Pass/Object/Callback) │
│     - 优先级合并机制 (ChainMerge)              │
│                                                │
│ [2] ShaderParameterValidator (验证层)          │
│     - 类型验证 / 缺失检测 / Fuzzy matching     │
│     - 全局参数识别                             │
│                                                │
│ [3] RenderGraph Auto-binding (自动绑定层)      │
│     - 资源名→参数名转换                        │
│     - Candidate 生成与匹配                     │
│     - Shader Reflection 集成                   │
└────────────────────────────────────────────────┘
```

**类型系统**：
```cpp
std::variant<XMMATRIX, XMFLOAT3, XMFLOAT4, float, ID3D11ShaderResourceView*>
// 限制：Sampler/UAV/StructuredBuffer 待扩展
```

**参数来源优先级**：
```
Global (最低) < Pass < Object < Callback (最高)
```

### 8.2 数据 & 资源流
```
Scene JSON ──▶ ResourceManager (Create) ──▶ ResourceRegistry (Register) ──▶ RenderGraph (ImportTexture / AddPass)
```

### 8.3 参数流
```
Global(每帧构建) + Pass(构建期注入) + Object(渲染前世界矩阵) + Callback(最终覆盖)
          │
          ▼
ShaderParameterContainer::ChainMerge() → 最终 Shader 调用集
```

### 8.4 渲染执行流
```
Graphics::Render()
  ├─ 更新摄像机 & 光照矩阵
  ├─ 构建 Global 参数容器
  ├─ 视锥剔除（线性遍历）
  └─ RenderGraph::Execute(sorted_passes_ 顺序遍历)
        ├─ 每 Pass 设置 RenderTarget / Merge 参数
        ├─ 遍历可见对象 + worldMatrix + 回调
        └─ 自定义 Execute 覆盖（反射/最终/文本）
```

### 8.5 Shader Reflection 流程
```
ShaderBase::Initialize() 
  → ReflectShader(vs_blob, ps_blob)
    ├─ 遍历 ConstantBuffer 变量 (Matrix/Vector/Float)
    ├─ 遍历 ResourceBinding (Texture/Sampler)
    ├─ 类型映射 (D3D11 → ShaderParameterType)
    └─ 合并 VS + PS 参数 (stage_mask)
  → 存储到 reflected_parameters_
  
RenderGraph::Compile()
  → 读取 reflected_parameters_
  → 自动注册到 Validator (跳过已注册)
  → 应用别名/可选/忽略元数据
```

---

## 附：术语规范
- "自动绑定" = 利用反射 + 候选名列表 + 模糊策略匹配纹理参数。
- "DAG 化" = 引入依赖关系与拓扑排序，阻止循环，推导资源使用生命周期（最后读后释放）。
- "材质" = Shader + 参数快照 + （未来）引用共享纹理集合。
- "Prefab" = 物体实例构建模板（模型引用 + 材质 + 初始变换 + 标签集合）。

### 命名约定规范
**资源名（Resource Names）**: `PascalCase`（首字母大写），用于 RenderGraph 资源标识。
- 语义化、业务相关：`ShadowMap`, `ReflectionBuffer`, `BloomResult`

**参数名（Parameter Names）**: `camelCase`（首字母小写），用于 Shader 参数。
- 技术性、通用：`shaderTexture`, `depthMapTexture`, `worldMatrix`
- 纹理以 `Texture` 结尾，矩阵以 `Matrix` 结尾

**命名转换机制**（`GenerateParameterCandidates`）:
将资源名转换为参数候选列表，支持自动绑定：
```
"DepthMap" → [
    "depthMapTexture",   // Phase 1: 保留后缀
    "depthMap",
    "depthTexture",      // Phase 2: 去除 Map/Texture/RT 后缀
    "depth",
    "shaderTexture",     // Phase 3: 通用 fallback
    "texture"
]
```

**匹配优先级**（`RenderGraph::Compile`）:
1. 显式指定参数名（`.ReadAsParameter("Res", "param")`）
2. 候选列表精确匹配
3. 模糊匹配（keyword 子串）
4. 单纹理参数回退（shader 只有 1 个 Texture）
5. 通用 fallback（`shaderTexture`, `texture`）

此机制解决多 Pass 使用同一 Shader 时的参数绑定问题，避免重复手工指定。

---

> 本文档为当前唯一权威架构状态入口；其它历史评审类文档后续将归档或删除。更新策略：增量修改；每次结构性迁移需刷新“执行摘要 + 评分矩阵 + 行动建议”。
