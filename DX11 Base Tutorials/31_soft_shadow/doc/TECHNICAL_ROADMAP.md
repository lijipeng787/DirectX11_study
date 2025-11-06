# 🚀 TECHNICAL_ROADMAP

> 汇总未来演进路线：阶段目标 / 任务分组 / 依赖关系 / 衡量指标 / 风险缓解。
> 更新时间：2025-11-06

---

## 目录
1. 路线概览（阶段映射）
2. 阶段目标与核心交付物
3. 详细任务拆分（Backlog）
4. 依赖与实施顺序
5. 度量指标（Observability）
6. 风险与缓解策略
7. 演进策略与分支管理建议

---

## 1. 路线概览

| 阶段 | 主题 | 驱动原因 | 预期完成度 |
|------|------|----------|------------|
| P0 | 清理 & 基础稳定 | 减少重复 / 启用多光源测试 | 2 周 |
| P1 | 数据驱动雏形 | 通过 JSON 自由组合管线 / 复用材质 | 3 周 |
| P2 | 架构分层 & 性能可视化 | 剔除优化 / 运行上下文统一 / 可测性提升 | 3-4 周 |
| P3 | DAG & 生命周期 & 诊断 | 复杂管线扩展与调试效率 | 4 周 |
| P4 | 高级资源与工具 | 热重载 / 材质编辑 / 参数可视化 | 持续迭代 |

---

## 2. 阶段目标与交付物

### P0：基础稳定与多光源支持
交付物：
- `LightManager` + `lights.json`（方向光 / 点光 / 聚光）
- 去除重复 Shader 参数注册（只保留 1 条自动注册路径）
- 别名规范统一（统一在 ShaderBase 层声明）
- 验证模式开关：`Strict` / `Warning` / `Disabled` 文档化使用场景

成功判定：
- 支持 ≥3 个光源渲染（方向光 + 点光组合）
- RenderGraph 编译日志不再出现重复自动注册同一 Shader
- SoftShadowShader 别名不被覆盖

### P1：数据驱动雏形
交付物：
- `render_graph.json`（描述 Pass: name / shader / reads[] / writes[] / params{} / tags[]）
- Minimal JSON 解析 → 构建 RenderGraphPass
- `Material` PoC：JSON 定义 + 绑定到对象（替换回调部分参数）
- 纹理/Shader 在 JSON 中引用 `ResourceRegistry` 名称

成功判定：
- 切换 JSON 配置后无需改 C++ 即可 A/B 不同 Pass 顺序
- 至少 1 个对象使用材质 JSON（含 diffuse/normal/rm 三纹理 + 基础参数）
- 手工回调代码减少 ≥30%

### P2：架构分层与可见性优化
交付物：
- `FrameContext`（deltaTime, view, proj, cameraPos, lightMatrices, stats）
- Graphics 拆分第一步：`RenderPipeline`（封装 RenderGraph 执行）
- 脏标记：对象变换更新时标记；剔除阶段只测试脏对象重建体积
- 可见集缓存（上一帧结果 + 统计）

成功判定：
- Graphics 至少减少 25% 行数 / 3+ 函数迁移到 RenderPipeline
- 剔除平均耗时下降（基准对比）
- FrameContext 取代 ≥3 个之前的 Global 参数显式注入（viewMatrix 等）

### P3：DAG 化与生命周期诊断
交付物：
- Pass 声明扩展：`reads` / `writes` / `deps`（隐式由资源推导 + 显式补充）
- 拓扑排序 + 环检测（编译失败给出循环链路）
- 资源使用图（首次写入/最后一次读取）→ 可视化日志
- 编译报告：未使用资源 / 未消费输出 / 重复写 / 潜在覆盖风险

成功判定：
- 编译阶段输出拓扑排序数组 + 无 false positive 循环报告
- 能从报告中识别并移除至少 1 个未使用 Pass 输出
- 资源生命周期日志可读（开始/结束帧内范围）

### P4：高级资源与工具化（持续）
交付物（滚动迭代）：
- 热重载：监控 JSON / Shader 文件变更 → 增量重编 RenderGraph / Shader
- 材质编辑接口（文本或 UI）→ 修改后实时生效
- 参数覆盖可视化（每 Pass 合并前后 diff）
- Sampler / UAV / StructuredBuffer 扩展 variant + Validator

成功判定（滚动）：
- 修改材质 JSON → 运行中实时反映
- 修改 render_graph.json → 自动触发重新编译输出新排序
- Sampler 类型进入 Container 并可在材质中配置

---

## 3. Backlog（按主题分组）

### 多光源与光照扩展
- Light 抽象接口：Directional / Point / Spot
- Shadow 策略：是否投影 / 阴影贴图尺寸 / 投影矩阵生成策略
- LightManager：更新 / JSON 解析 / 查询 / Shader 参数填充
- SoftShadowShader 扩展多光源采样（后续）

### RenderGraph 数据驱动
- JSON 解析器：Schema 校验（缺字段报错）
- Pass 构建器：动态选择 Shader / 参数映射 / Tag 注入
- 别名与参数候选融合（自动匹配失败 → 列出 JSON 中声明的 overrides）

### 材质系统
- Material 数据结构：shaderName + textures[] + scalars/vectors + flags
- 绑定策略：对象持有材质引用；回调仅做动态参数（worldMatrix 等）
- 预设加载：materials.json + name 索引
- 可见性：材质 Tag 支持（如 `opaque`, `alpha`）

### 可见性与性能
- BoundingVolume 脏标记（变换更新时重算）
- 剔除统计：测试对象数 / 可见数 / 剔除耗时 / 命中率
- 可见集缓存：未脏对象跳过重测（后续空间结构）

### DAG / 生命周期
- 构建资源使用图：写入边 / 读取边
- 计算最后使用点 → 提前释放（未来：RT 池化）
- 环检测：DFS 或 Kahn 算法检测依赖闭环
- 报告格式：JSON 或结构化文本供后续可视化

### 诊断与工具
- 参数覆盖快照：`before_merge` / `after_object_callback`
- 覆盖来源计数统计（Global/Pass/Object/Callback）
- 验证模式统计：每编译出现的 Warning 种类与次数
- 性能采样：每 Pass CPU 时间（可选）

### 资源与热重载
- 文件监控：简单轮询时间戳或平台事件
- 增量重编：只重建受影响 Shader / RenderGraph Pass
- 安全回退：异常时使用最近一次成功快照

---

## 4. 依赖与实施顺序

| 任务 | 依赖前置 | 推荐顺序原因 |
|------|----------|--------------|
| LightManager | 无 | 解锁多光源场景与后续性能测试 |
| 去重复注册 | 反射已存在 | 降低后续调试噪音 |
| render_graph.json | 注册路径稳定 | 避免格式频繁调整造成混乱 |
| Material PoC | RenderGraph JSON 基础 | 使 Pass 参数简化更具收益 |
| FrameContext | 光照/摄像机参数稳定 | 减少 Global 参数散乱 |
| Graphics 拆分 | FrameContext | 可集中挪走渲染执行逻辑 |
| DAG 编译 | JSON 管线稳定 | 避免图结构频繁变动影响实现复杂度 |
| 生命周期分析 | DAG 输出可用 | 基于排序与依赖图生成边界 |
| 热重载 | JSON + DAG 稳定 | 否则易触发大量非确定性编译问题 |

---

## 5. 度量指标（Observability）

| 指标 | 说明 | 收集方式 | 目标 |
|------|------|----------|------|
| Pass 编译耗时 | RenderGraph::Compile 时间 | 计时器包裹 | < 10ms（常规场景） |
| 自动绑定成功率 | 无需手动指定的资源绑定百分比 | 统计匹配路径 | ≥ 95% |
| 参数警告频度 | 每次编译 Warning 条目 | Validator 汇总 | 持续下降，P0 后 ≤ 5 |
| 光源数量上限 | 场景中成功渲染的光源数 | 场景测试 | 初版 ≥ 8（无性能崩溃） |
| 剔除命中率 | 可见 / 总测试对象 | 统计模块 | ≥ 60%（典型场景） |
| 未使用资源数 | 编译报告列出未消费输出 | 对比变更趋势 | 逐步归零 |
| 生命周期跨度 | 资源存活 Pass 范围平均长度 | 分析图计算 | 随 DAG 优化缩短 |

---

## 6. 风险与缓解策略

| 风险 | 描述 | 缓解策略 |
|------|------|-----------|
| JSON 复杂化早期过度设计 | 数据驱动早期膨胀 | 迭代 Schema：先最小字段，后扩展 |
| DAG 引入排序差异 | 与原顺序渲染结果不一致 | 保留“顺序退回模式”开关 + 单测关键顺序 |
| 多光源性能退化 | N 光源阴影开销爆炸 | 初期限制投影光源数量 + 分级质量设置 |
| 材质系统侵入性高 | 影响现有回调参数逻辑 | PoC 只覆盖 1-2 对象 + 双路径并行 |
| 生命周期提前释放错误 | 资源仍被后续 Pass 使用 | 在执行前验证引用计数/使用标记 |
| 热重载不稳定 | 部分状态不一致导致崩溃 | 分阶段：先日志提示 → 后增量应用 |
| 可见性缓存失效 | 脏标记错误导致对象消失 | 调试模式：强制全量与缓存结果对比 |
| Sampler/UAV 扩展破坏现有接口 | variant 改动大 | 使用 `std::monostate` 过渡 + 编译期静态断言 |

---

## 7. 演进策略与分支管理建议

| 策略 | 说明 |
|------|------|
| feature 分支粒度 | 每个中型任务（LightManager / render_graph.json / Material）单独分支，避免巨型合并冲突 |
| 里程碑 Tag | 在关键阶段前打 Tag：`pre-light-manager`, `pre-dag`, `pre-material-poc` |
| 变更文档制度 | 每次合并更新本文件“更新时间” + 修改章节只加“增量说明” |
| 单测覆盖策略 | 先针对核心纯逻辑模块（名称生成 / DAG 排序 / 生命周期推导）编写测试，图形调用延后 |
| 回退策略 | 新管线采用“并行模式”，运行时可选 legacy 顺序执行（环境变量或配置开关） |

---

> 后续新增项：每次评审只追加“增量变更”段落，不重复全量复制；保证 Roadmap 成为单一真源。详细近期执行拆分见 `NEXT_STEP.md`。
