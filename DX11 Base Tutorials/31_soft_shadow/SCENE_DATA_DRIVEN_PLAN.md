# 数据驱动场景管理重构规划 (2025-11-05)

目标: 将现有硬编码/半数据驱动的 `Scene` + 资源缓存模式升级为"声明式 + 组件化 + 增量更新"体系, 支撑更高场景复杂度 (对象数量、层次、动画类型、可见性策略) 并为后续管线优化提供稳定数据层。

---
## 现状痛点速览
- 硬编码与 JSON 混用: `BuildSceneObjects` 中仍含大量固定对象逻辑, JSON 只覆盖部分模型与渲染目标配置。
- 资源缓存重复: `SceneResourceRefs` + `Scene` 内部 model/shader/window/renderTexture 多层缓存。
- 动画与状态耦合: 旋转动画配置/状态分散在 `Scene` 多个 `unordered_map`, 无统一系统、无脏标记。
- Tag 系统扁平: `unordered_set<string>` 缺乏命名空间/层级, 与 RenderPass 匹配方式脆弱。
- 缺少对象层次: 不支持父子变换, 拓展高级场景结构困难。
- 更新链路不清晰: 无 FrameContext, 视锥剔除逻辑不独立, 难以插入其它可见性策略 (区域划分 / LOD)。
- 难以热重载: JSON 加载是一次性, 修改后需重建整个 Scene。

---
## 设计原则
1. 明确数据 → 系统 → 渲染三层: JSON/DTO (纯数据), Runtime Store (组件容器), Systems (Animation/Visibility), RenderGraph 消费者。
2. 声明式对象与组件: 每个对象由唯一 id + 组件列表构成, 组件最小交叉。
3. 可增量更新: 支持 diff: 添加/删除/修改对象与组件; 尽量避免全局重建。
4. 类型安全与验证前置: 加载阶段完成引用解析与结构验证 (父子/资源存在/循环依赖)。
5. 命名空间化 Tag: 采用 `domain.category.value` 形式 (如 `render.depth.write`, `pass.shadow.input`, `layer.ui`). 指定匹配策略 (前缀/完整匹配)。
6. 低耦合资源访问: 通过 `ResourceContext` 接口解析资源 ID, Scene 不再维护重复缓存。
7. 系统边界清晰: AnimationSystem 仅处理时间驱动的参数更新; VisibilitySystem 仅负责可见集合生成。

---
## 目标架构概览
```
+---------------------------+
| JSON Scene File          |
+-------------+-------------+
              v
       Parse & Validate (Schema + Rules)
              v
        Intermediate DTO (SceneDTO)
              v
     Build Runtime Stores (Component Stores)
              v
+---------------------------+    +-------------------+
| SceneObjectStore          |    | ResourceContext   |
| (id -> component indices) |    | (id -> shared_ptr)|
+------------+--------------+    +---------+---------+
             |                             |
             v                             v
      AnimationSystem                VisibilitySystem
             |                             |
             +-------------+---------------+
                           v
                     FrameContext
                           v
                      RenderGraph
```

---
## JSON Schema 草案
顶层字段: `objects[]`, `resources`(引用外部已加载资源 id), `settings`(场景级常量), 可选 `meta`。

### 对象结构
```
{
  "id": "sphere_01",
  "name": "BlueSphere",
  "components": {
    "Transform": {
      "position": [0, 2, 0],
      "rotationEuler": [0, 0, 0],
      "scale": [1, 1, 1],
      "parent": "root"  // 可选
    },
    "ModelRenderer": {
      "model": "sphere",            // 资源 id
      "shader": "soft_shadow",       // 着色器 id
      "reflection": true
    },
    "Animation": {
      "rotate": { "axis": "y", "speed": 45.0, "initial": 0 }
    },
    "Tags": ["render.depth.write", "render.shadow.write", "layer.main"]
  }
}
```

### PostProcess 示例
```
{
  "id": "blur_pass_horizontal",
  "components": {
    "PostProcess": {
      "window": "small_window",
      "shader": "horizontal_blur",
      "inputs": { "texture": "downsampled_shadow" }
    },
    "Tags": ["pass.blur.horizontal", "layer.postprocess", "skip.culling"]
  }
}
```

### PBR 示例
```
{
  "id": "pbr_sphere_main",
  "components": {
    "Transform": { "position": [0,2,-2] },
    "PBRModelRenderer": {
      "model": "sphere_pbr",
      "shader": "pbr"
    },
    "Tags": ["render.depth.write", "render.shadow.write", "render.pbr"]
  }
}
```

### Schema 验证要点
- 唯一 id 与合法字符 ( `[a-zA-Z0-9_.-]+` ).
- 引用资源/着色器必须存在于 ResourceContext 注册表。
- 父对象存在且无循环 (DFS 检查)。
- 组件互斥/组合规则 (如 `ModelRenderer` 与 `PBRModelRenderer` 不能并存)。
- Tag 命名空间限定前缀集: `render.`, `pass.`, `layer.`, `system.`, `skip.`。

---
## 组件与存储设计
| 组件 | 数据 | 运行期结构 | 说明 |
|------|------|------------|------|
| Transform | position, rotationEuler, scale, parent | SoA: vectors + parent index | 支持层次与脏标记 (local/world 分离) |
| ModelRenderer | model_id, shader_id, reflection(bool) | Struct array | 回调生成参数 (texture) |
| PBRModelRenderer | model_id, shader_id | Struct array | 单独类型避免可选字段过多 |
| PostProcess | window_id, shader_id, inputs(map) | Struct array + small map | inputs 用于自动绑定纹理参数 |
| Animation | rotate(axis,speed,initial) | Struct array | 扩展支持多动画 clip 时再升级 |
| Tags | vector<string> | Flattened tag registry + per object tag indices | 用于快速匹配与过滤 |

使用 ComponentStore 管理: 每种组件一个 `std::vector<ComponentData>` 与 `std::unordered_map<object_id, index>`。SceneObjectStore 保存: `object_id -> { component bitmask / indices }`。

---
## 更新/执行流程
1. Loader 构建所有组件数据与对象映射。
2. 每帧: 
   - AnimationSystem: 根据 `deltaTime` 修改 Transform 的 rotation (写入 local, 标记脏)。
   - TransformSystem: 处理脏链 (parent → 子) 更新 worldMatrix 缓存。
   - VisibilitySystem: 使用摄像机+world bounds 计算可见集合 (输出 object_id 列表)。
   - FrameContext: 聚合时间、摄像机、可见集合、可能的环境参数。
   - RenderGraph 构建参数容器时只遍历可见对象集。

---
## Diff / 热重载策略
1. 监控 scene 文件时间戳变化。
2. 重新解析新 JSON 得到新 DTO。
3. 计算 diff:
   - Added ids → 新建组件实例。
   - Removed ids → 从每个组件 store 删除, 标记延迟释放 (若渲染中)。
   - Modified → 比较组件字段, 只更新有变化的部分 (Transform 改变触发脏标记)。
4. 不重建未变化对象 worldMatrix。

---
## Tag 系统命名空间规范
- 结构: `segment.segment[.segment]*` 最少两段。
- 保留前缀:
  - `render.`: 与渲染阶段相关 (如 `render.depth.write`).
  - `pass.`: 具体 RenderGraph pass 标识 (`pass.blur.horizontal`).
  - `layer.`: UI/世界/后处理层级 (`layer.main`, `layer.postprocess`).
  - `system.`: 系统内部标记 (`system.debug`).
  - `skip.`: 行为控制 (`skip.culling`).
- 匹配规则: RenderPass 可声明 `required_tags` (全包含) 与 `any_of_tags` (至少一个), 外加 `excluded_tags`。
- 验证: Loader 确保 tag 使用受限前缀; 报告未使用的标签 (潜在拼写错误)。

---
## Dirty 标记 & 性能
- Transform: `dirty_local` (修改局部) / `dirty_world` (需重新合成)。修改自身或 parent 世界更新触发 world 脏链遍历 (迭代或 DAG 层级列表)。
- AnimationSystem: 只写局部旋转, 触发 `dirty_local`。
- Visibility: 缓存上一帧可见集合, 摄像机或对象 AABB 变化才重新计算。
- 后续优化: 引入分区结构 (例如 QuadTree) 时 VisibilitySystem 内部替换实现, 外部不变。

---
## 验证测试计划
| 用例 | 期望 |
|------|------|
| 缺失模型引用 | 加载失败 + 错误列表包含缺失资源名 |
| 父子循环 | 加载失败 + 错误列出循环链 |
| 重复对象 id | 加载失败 + 指明重复 id |
| Tag 非法前缀 | 加载失败 + 指明具体 tag |
| 热重载修改单对象 position | 仅该对象 worldMatrix 变化; 其它保持 |
| 移除对象后热重载 | 对象从所有组件 store 消失; 可见集合不含其 id |

---
## 增量实施路线 (与 TODO 对应)
1) JSON Schema & DTO 定义 (加最小单元测试)。
2) ComponentStore + ObjectStore 框架落地 (空数据结构)。
3) Transform + AnimationSystem 基础实现 (单旋转用例)。
4) Tag 命名空间重写 + 验证器。
5) ResourceContext 接入, 移除 Scene 内部缓存。
6) Loader 管线 (parse→validate→build) 替换旧 `BuildSceneObjectsFromJson`。
7) VisibilitySystem 抽离 (复用现有视锥剔除逻辑)。
8) FrameContext 集成 RenderGraph 入口。
9) 热重载框架 + 简单 diff (添加/修改/删除)。
10) 扩展 PostProcess / PBR / 回调参数绑定适配新组件。

---
## 风险与缓解
| 风险 | 描述 | 缓解 |
|------|------|------|
| 初期双体系共存 | 旧 Scene 与新组件并行导致混乱 | 使用宏/配置切换, 保持单入口 |
| 资源引用时序 | 资源未加载先解析场景 | 引入阶段化: 先资源加载→再场景 |
| 性能回退 | 组件映射增加间接层 | 基线测试 + SoA 布局 + 避免频繁查找 |
| 热重载一致性 | 删除对象仍被 RenderGraph 引用 | 标记“移除待清理”并在下一帧过滤 |

---
## 后续扩展预留点
- 多相机: FrameContext 支持 camera array & per-layer visibility。
- LOD 支持: VisibilitySystem 增加距离与屏幕像素估算。
- 组件订阅事件: 变换/资源更新事件广播 (后期再加)。
- 序列化回写: 将运行期新增对象回写 JSON (编辑器模式)。

---
## 成功判定 (第一阶段完成标准)
- 新 Loader 能完整替代旧 JSON 路径, 硬编码路径可删除或隔离。
- Scene 不再维护 model/shader/renderTexture/orthoWindow 缓存 map。
- AnimationSystem 与 TransformSystem 输出一致世界矩阵 (与旧实现功能等价)。
- Tag 重构后 RenderPass 过滤逻辑使用命名空间前缀匹配, 不再依赖模糊字符串。
- 单元测试覆盖: 引用缺失/循环/重复 id/非法 tag/热重载修改。

---
## 初始工作量估算 (粗略)
| 步骤 | 估算 (人日) |
|------|-------------|
| Schema & DTO + 测试 | 0.75 |
| Component/ObjectStore 框架 | 0.5 |
| Transform + AnimationSystem | 0.75 |
| Tag 系统重构 | 0.5 |
| ResourceContext 接入 | 0.5 |
| Loader 管线 | 1.0 |
| VisibilitySystem 抽离 | 0.75 |
| FrameContext 集成 | 0.5 |
| 热重载 diff | 1.0 |
| 兼容/回归测试与清理 | 0.75 |
| 合计 | ~7.0 |

---
## 下一具体行动建议
立即执行: 步骤 1 + 2 (定义 Schema / DTO 与空 ComponentStore 框架), 以最小代码面启动, 并加入单元测试基线。

(文件添加日期: 2025-11-05)
