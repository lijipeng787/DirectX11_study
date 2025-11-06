# ARCHITECTURE_NEXT_STEPS

日期: 2025-11-05

> 目标：在 Shader 参数管理模块冻结后，以最小风险推进核心架构演进，逐步拆除“上帝对象”与顺序式 RenderGraph，建立数据驱动、可验证、可扩展的渲染管线。

---
## 1. RenderGraph DAG 化 ✅(计划)
**动机**：当前顺序列表无法表达资源依赖、阻止循环、推导生命周期。
**交付项**：
- Pass 描述结构: `{ name, reads:[res], writes:[res], shader, tags, params }`
- 构建阶段：资源节点与 Pass 节点生成，拓扑排序 + 循环检测。
- 验证：缺失写入、重复写入同一纹理的告警。
**风险**：与现有执行循环耦合；需保持 API 兼容（旧顺序如无依赖冲突则排序结果一致）。
**估算**：1.5 人日。

## 2. FrameContext 引入
**动机**：减少显式向参数容器塞入时间/摄像机矩阵；统一剔除结果与统计。
**设计**：`struct FrameContext { float deltaTime; XMMATRIX view; XMMATRIX proj; XMMATRIX baseView; XMFLOAT3 cameraPos; }`
在 `RenderGraph::Execute` 生成并传入 Pass 回调，`ChainMerge` 可选择性注入别名。
**估算**：0.5 人日。

## 3. 扩展反射：Slot / Binding Class
**动机**：为自动绑定与冲突检测提供底层信息（避免同 slot 重复绑定）。
**设计**：
`ReflectedParameter` 增加：`uint32_t slot; enum class BindingClass { ConstantBuffer, Texture, Sampler, UAV, StructuredBuffer }`。
**估算**：0.75 人日。

## 4. 覆盖日志分级
**动机**：减少 Release 噪声，保留关键诊断事件。
**方案**：`enum class ParamLogVerbosity { None, ErrorsOnly, ErrorsWarnings, Full }` 全局设置；当前默认 Full，仅 Debug 生效。
**估算**：0.5 人日。

## 5. 自动绑定解析器 PoC
**动机**：减少手工 `.SetParameter()` / `.ReadAsParameter()` 显式声明；利用反射与资源名规则匹配。
**接口**：`AutoBinder::Resolve(pass_decl, reflection_cache, param_container)`：
- 为缺失且 required 的 Texture / ConstantBuffer 尝试匹配资源名或别名。
- 生成报告列出无法自动解决的缺失条目。
**估算**：1 人日。

## 6. ResourceManager 去单例
**动机**：支持多场景 / 离屏渲染 / 测试隔离。
**方案**：构造注入 `ResourceContext`，内部维护缓存；Graphics 不再直接访问单例。
**估算**：2 人日。

## 7. Graphics 拆分
**动机**：降低“上帝对象”复杂度，提高测试性与开放封闭原则遵守。
**分层**：
- `RenderPipeline` (RenderGraph + Pass 执行)
- `SceneSystem` (加载 / 动画 / 标签)
- `ViewSystem` (摄像机 / 剔除 / FrameContext 构建)
**估算**：1.5 人日（第一阶段仅剥离 RenderPipeline）。

## 8. 后续 (排队)
| 任务 | 触发条件 | 价值 |
|------|----------|------|
| UAV / StructuredBuffer 反射 | 着色器添加相关资源 | 自动绑定覆盖面扩大 |
| 结构体数组成员展开 | 场景出现高级材质表 | 精细化验证 |
| 参数差异快照与可视化 | 调试多 Pass 覆盖冲突 | 加速集成诊断 |

---
## 依赖与顺序建议
1. RenderGraph DAG 化 → 为自动绑定与生命周期奠基。
2. FrameContext → 简化后续 Graphics 拆分。
3. Slot / BindingClass 反射 → 支撑 AutoBinder。
4. AutoBinder PoC → 验证反射与命名规则融合价值。
5. 覆盖日志分级 → 控制噪声与性能。
6. ResourceManager 去单例 → 解耦共享状态。
7. Graphics 拆分 → 清理架构边界。

---
## 风险概览与缓解
| 风险 | 缓解 |
|------|------|
| DAG 引入排序差异导致可见性回归 | 保留旧顺序回退路径 + 单元测试覆盖关键 Pass 顺序 |
| Slot 扩展破坏现有 Validator 输出格式 | 新字段默认不参与验证，待稳定后启用 |
| AutoBinder 误绑定错误资源 | Dry-run 模式：仅输出建议，不实际写入参数容器 |
| 去单例导致生命周期泄漏 | 引入所有权单元测试，跟踪引用计数变化 |
| 拆分 Graphics 迁移不完全 | 分阶段迁移 + Facade 保持旧接口可用 |

---
## 成功判定 (近期三项)
- RenderGraph Compile 输出包含排序 + 循环检测报告，无假阴性。
- FrameContext 注入后参数显式传递减少 ≥3 项（viewMatrix/baseViewMatrix/cameraPosition）。
- Slot 反射在 3+ shaders 中正确记录并无冲突日志。

---
## 快速指标采集 (准备)
- 覆盖日志事件计数 (启动后 300 帧取样)。
- Merge 冲突异常频度 (期望=0)。
- Validator 警告种类分布 (Missing vs Unknown vs TypeMismatch)。

---
后续所有阶段引入前建议创建 git tag：`pre-dag-freeze-2025-11-05` 以支持紧急回退。

(文档自动生成于阶段收尾，后续更新追加增量差异段落即可)
