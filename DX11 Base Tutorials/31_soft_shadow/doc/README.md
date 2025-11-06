# 📚 Documentation Index

> 文档导航与说明  
> 更新时间：2025-11-06

---

## 🎯 核心文档

项目采用精简的三文档体系，涵盖架构、规划与执行：

| 文档 | 用途 | 更新频率 |
|------|------|----------|
| **[ARCHITECTURE_OVERVIEW.md](ARCHITECTURE_OVERVIEW.md)** | 架构现状、评分、技术细节、风险分析 | 每次结构性变更 |
| **[TECHNICAL_ROADMAP.md](TECHNICAL_ROADMAP.md)** | 演进路线、阶段目标、任务分组、度量指标 | 阶段开始/结束时 |
| **[NEXT_STEP.md](NEXT_STEP.md)** | 当前迭代任务清单、实施细节、代码触点 | 每个迭代周期 |

**推荐阅读顺序**：
1. 先读 `ARCHITECTURE_OVERVIEW.md` 了解现状与技术架构
2. 再读 `TECHNICAL_ROADMAP.md` 了解演进方向
3. 最后读 `NEXT_STEP.md` 开始具体实施

---

## � 文档内容速查

### ARCHITECTURE_OVERVIEW.md 包含

**现状评估**
- 执行摘要（成熟度 45%）
- 分层目标与实现对照
- 子系统评分矩阵（⭐ 评级）
- 核心能力与优势

**问题与风险**
- 技术债务清单（🔴高/🟡中/🟢低优先级）
- 风险清单与影响分析
- 缓解建议

**技术细节**
- Shader 参数系统架构（3 层）
- 命名转换机制（候选生成算法）
- 参数匹配优先级
- Shader Reflection 流程
- 运行路径示意（数据流/参数流/执行流）

### TECHNICAL_ROADMAP.md 包含

**路线规划**
- 阶段映射（P0-P4）
- 阶段目标与交付物
- 成功判定标准

**任务管理**
- Backlog（按主题分组）
- 依赖与实施顺序
- 工作量预估

**质量保障**
- 度量指标（Observability）
- 风险与缓解策略
- 演进策略与分支管理

### NEXT_STEP.md 包含

**P0 阶段实施**
- 两周目标
- 任务拆分表（T1-T10）
- 详细设计片段（代码示例）
- 验收标准清单
- 代码触点与修改范围
- 风险与回退策略

---

## � 按主题查找

### 架构与设计
- 当前架构评估 → `ARCHITECTURE_OVERVIEW.md` 第 1-3 节
- 分层设计目标 → `ARCHITECTURE_OVERVIEW.md` 第 2 节
- 技术债务详情 → `ARCHITECTURE_OVERVIEW.md` 第 5 节

### 参数系统
- 系统架构 → `ARCHITECTURE_OVERVIEW.md` 第 8.1 节
- 命名转换规则 → `ARCHITECTURE_OVERVIEW.md` 附录-术语规范
- 参数流转机制 → `ARCHITECTURE_OVERVIEW.md` 第 8.3 节
- 匹配优先级 → `ARCHITECTURE_OVERVIEW.md` 附录-术语规范

### 路线规划
- 阶段目标 → `TECHNICAL_ROADMAP.md` 第 2 节
- 任务依赖 → `TECHNICAL_ROADMAP.md` 第 4 节
- 当前任务 → `NEXT_STEP.md` 第 2 节
- 实施细节 → `NEXT_STEP.md` 第 3 节

### 质量与度量
- 度量指标 → `TECHNICAL_ROADMAP.md` 第 5 节
- 风险分析 → `ARCHITECTURE_OVERVIEW.md` 第 6 节 + `TECHNICAL_ROADMAP.md` 第 6 节
- 验收标准 → `NEXT_STEP.md` 第 4 节

---

## ✏️ 文档维护指南

### 更新规则
1. **结构性变更**（架构调整/子系统评分变化）→ 更新 `ARCHITECTURE_OVERVIEW.md`
2. **路线调整**（新阶段/任务重排）→ 更新 `TECHNICAL_ROADMAP.md`
3. **迭代推进**（完成任务/新任务）→ 更新 `NEXT_STEP.md`

### 版本控制
- 每次重大架构变更前打 git tag：`pre-<feature>-YYYY-MM-DD`
- 文档顶部更新"更新时间"字段
- 避免全量重写，采用增量修改

### 新增内容原则
- **技术细节**：追加到 `ARCHITECTURE_OVERVIEW.md` 第 8 节或附录
- **新任务**：添加到 `TECHNICAL_ROADMAP.md` Backlog 或 `NEXT_STEP.md` 任务表
- **新阶段**：扩展 `TECHNICAL_ROADMAP.md` 第 2 节
- **避免**：创建新的综合性文档（防止碎片化）

### 报告问题
发现错误或冲突时：
1. 在相关文件顶部添加 `> ⚠️ 待修正：<问题描述>`
2. 或直接修复后提交并说明修改原因

---

## 📊 文档演进历史

**2025-11-06**：文档重构
- 合并 7 个旧文档（已删除）为 3 个新文档
- 提取所有关键内容到新体系
- 建立统一导航与维护规则

**旧文档内容去向**：
- 架构批评 → `ARCHITECTURE_OVERVIEW.md` 第 5-6 节
- 下一步计划 → `TECHNICAL_ROADMAP.md` + `NEXT_STEP.md`
- 命名系统 → `ARCHITECTURE_OVERVIEW.md` 附录
- 参数系统分析 → `ARCHITECTURE_OVERVIEW.md` 第 8.1 节
- 自动化差距 → `ARCHITECTURE_OVERVIEW.md` + `TECHNICAL_ROADMAP.md`
- 实施方案 → `NEXT_STEP.md`
- 系统问答 → `ARCHITECTURE_OVERVIEW.md` 技术细节章节

---

> 此 README 为文档体系唯一导航入口。文档结构调整时需同步更新本文件。

