# 🔍 参数绑定优先级系统深度批评

> 针对 ShaderParameterContainer 优先级链的架构审查与问题诊断  
> 分析时间：2025-11-06  
> 审查范围：优先级链设计 / 实现一致性 / 边界情况 / 性能开销

---

## ✅ 后续修复状态（2025-11-06 后续迭代补充）
本节评估代码在最近一次修复后的真实状态，指出仍残留的问题，区分“已解决”与“未完全解决”。

### 已修复的 P0 级问题
| 问题 | 原描述 | 当前状态 | 备注 |
|------|--------|----------|------|
| Callback 覆盖被 Object 反转 | RenderableObject::Render 最终再次合并 Object 覆盖回调 | 已修复 | 去除二次合并，回调结果保持最高优先级 |
| worldMatrix 无来源标记 | 使用 SetMatrix 默认 Manual | 已修复 | 在 Pass 中以 `ParameterOrigin::Object` 设置；回调不再覆盖它 |
| Auto-register 覆盖手工别名 | RenderGraph::Compile 重新注册 Shader 参数 | 部分修复 | 增加“已注册则跳过”，仍建议统一注册入口 |

### 仍存在或新出现的问题
| 类别 | 描述 | 影响 | 优先级 |
|------|------|------|--------|
| 合并路径重复 | RenderGraphPass 与 RenderPass 都各自实现 Global→Pass→Object→Callback 合并逻辑 | 维护成本升高，易产生行为偏差 | P1 |
| Merge API 不统一 | ChainMerge 与手动多次 MergeWithPriority 混用 | 源码阅读成本高；难以建立单点测试 | P1 |
| Callback Origin 标记粒度不足 | ScopedOriginOverride 只能全局覆盖 default_origin，不区分批量/单个参数 | 精细调试困难 | P2 |
| 缺失参数访问审计 | 无方法列出最终某参数被覆盖多少次及其来源链 | 难以定位复杂覆盖行为 | P2 |
| 缺集中“最终合并点” | RenderGraph / RenderPass / 自定义 Execute lambda 三处可能构造最终容器 | 一致性风险 | P1 |
| 未落实增量/脏标记 | 每对象仍执行全量构造与复制 | 未来大场景扩展受限 | P2 |
| 无严格模式开关 | 模糊匹配 & fallback 始终启用 | 难以在调试阶段收紧规则 | P2 |
| 类型系统仍不完整 | Sampler / UAV / StructuredBuffer 未进入统一 variant | 阻碍高级效果实验 | P1 |

### 当前优先级链的执行路径（修复后）
```
Global + Pass → (单次 Merge) → 加入 Object 参数 → worldMatrix(Object) → Callback(最高) → RenderableObject::Render(不再二次合并)
```
说明：`RenderableObject::Render` 现在只转发已合并容器，不做额外覆盖；修复了原始的回调优先级反转问题。

### 风险残留与改进建议（更新）
1. 建议收敛为单一函数：`BuildFinalParameters(global, pass, object, callback)`，集中处理顺序与来源标记，便于单测。
2. 抽象 `ParameterAuditTrail`：记录添加时间戳 / 来源枚举 / 覆盖次数，实现 `DumpHistory("shadowStrength")`。
3. 提供严格模式：禁用 fallback（`shaderTexture`/`texture`）与模糊匹配，暴露命名不足。
4. 为性能预留接口：分离静态参数（光照、投影矩阵）与动态参数（worldMatrix、动画），仅对动态层做每帧拷贝。
5. ScopedOriginOverride 建议新增单参数精准 API：`SetWithOrigin(name, value, origin)`（避免全局 default_origin 影响其他参数写入）。
6. 在文档中补充：当出现多层 Pass 嵌套或自定义 Execute 时，需显式遵守“Object 总是在 Callback 之前合并”。

### 验证与测试缺口（最新）
缺失的测试仍未补齐：
- 回调修改后对象参数不再覆盖（已手工验证，缺自动测试）。
- RenderGraph 与 RenderPass 两条路径结果是否一致。
- Auto-register 跳过逻辑是否保持别名有效（`projectionMatrix` → `orthoMatrix`）。
- 多纹理 Shader 的匹配优先级回退是否在严格模式下能够关闭（功能缺失）。

### 结论（阶段性）
最关键的“回调优先级”缺陷已修复，避免了最具破坏性的覆盖反转；但系统仍停留在“可工作”而非“可进化”状态：合并逻辑分散，缺乏审计与性能策略，也未建立单点测试。继续投入的边际收益高——一次性清理将减少未来实验时的心智负担。

---

---

## 📋 目录
1. [系统概览与声明优先级](#1-系统概览与声明优先级)
2. [核心问题清单](#2-核心问题清单)
3. [架构不一致性](#3-架构不一致性)
4. [语义混乱与误导](#4-语义混乱与误导)
5. [性能与效率问题](#5-性能与效率问题)
6. [类型安全漏洞](#6-类型安全漏洞)
7. [可测试性与调试性](#7-可测试性与调试性)
8. [边界情况与未定义行为](#8-边界情况与未定义行为)
9. [正面评价（难得的优点）](#9-正面评价难得的优点)
10. [修复优先级建议](#10-修复优先级建议)

---

## 1. 系统概览与声明优先级

### 1.1 声称的优先级链

文档宣称：**Global < Pass < Object < Callback**

```
Global Parameters (最低优先级)
    ↓
Pass Parameters
    ↓
Object Parameters
    ↓
Callback Parameters (最高优先级)
```

### 1.2 实际代码路径

**RenderGraph::Execute** (Per-Pass):
```cpp
ShaderParameterContainer merged = MergeWithPriority(
    global_params,      // Lower priority
    *pass_parameters_,  // Higher priority
    ParameterOrigin::Global,
    ParameterOrigin::Pass
);
```

**RenderGraphPass::Execute** (Per-Object):
```cpp
ShaderParameterContainer objParams = merged;  // Copy pass+global
objParams.SetMatrix("worldMatrix", r->GetWorldMatrix());  // Direct write
if (auto cb = r->GetParameterCallback()) {
    cb(objParams);  // Callback directly modifies container
}
r->Render(*shader_, objParams, device_context);
```

**RenderableObject::Render**:
```cpp
ShaderParameterContainer combinedParams = parameters;  // Copy incoming
combinedParams = MergeWithPriority(
    combinedParams,       // Lower (contains pass+global+callback)
    object_parameters_,   // Higher (object-stored parameters)
    ParameterOrigin::Pass,     // ❌ 错误！incoming 不只是 Pass
    ParameterOrigin::Object
);
```

---

## 2. 核心问题清单

### 🔴 P0 严重问题（破坏优先级语义）

1. **优先级顺序反转**：`RenderableObject::Render` 的 merge 顺序违背声明
2. **Origin 标签错误**：`RenderableObject` 将 incoming 参数错误标记为 `Pass`
3. **Callback 优先级丢失**：Callback 直接修改容器，无 Origin 追踪
4. **worldMatrix 无 Origin**：直接 `SetMatrix` 默认为 `Manual`，丢失语义

### 🟡 P1 中等问题（设计缺陷）

5. **多次拷贝开销**：每次合并都全量拷贝 `std::unordered_map`
6. **Origin 不一致传播**：ChainMerge 与分阶段合并的 Origin 可能不匹配
7. **冗余 MergeWithPriority**：代码中同时使用 `MergeWithPriority` 和 `ChainMerge`
8. **缺少优先级验证**：没有运行时检查是否违背优先级链

### 🟢 P2 改进空间（工程质量）

9. **日志开关全局化**：静态开关影响所有实例，无法区分 Pass/Object
10. **异常处理粗暴**：类型冲突直接 `throw`，无降级/跳过机制
11. **缺少合并追踪**：无法查询某参数的最终来源链
12. **测试覆盖不足**：缺少跨 Pass+Object+Callback 的集成测试

---

## 3. 架构不一致性

### 3.1 问题：优先级顺序反转

**期望**：`Object < Callback`（Callback 应覆盖 Object）

**实际**：`RenderableObject::Render` 执行顺序：
```cpp
// Step 1: Pass 调用前已执行 Callback
if (auto cb = r->GetParameterCallback()) {
    cb(objParams);  // Callback 修改
}

// Step 2: RenderableObject::Render 再合并 Object
combinedParams = MergeWithPriority(
    combinedParams,       // 包含 Callback 修改
    object_parameters_,   // Object 参数
    ParameterOrigin::Pass,
    ParameterOrigin::Object
);
```

**结果**：`object_parameters_` 覆盖了 Callback 的修改！**完全反转**。

#### 具体场景

```cpp
// 场景：动态调整阴影强度
pass->SetParameterCallback([](auto& params) {
    params.SetFloat("shadowStrength", 0.9f);  // Callback 希望覆盖
});

renderable->SetObjectParameters(container_with_shadowStrength_0.3f);  // Object 预设

// 实际结果：0.3f（Object 赢了）
// 期望结果：0.9f（Callback 应该赢）
```

### 3.2 问题：Origin 标签错误

`RenderableObject::Render` 中的合并：
```cpp
combinedParams = MergeWithPriority(
    combinedParams,       // 实际包含 Global+Pass+Callback
    object_parameters_,
    ParameterOrigin::Pass,     // ❌ 误导性标签
    ParameterOrigin::Object
);
```

**问题**：
- `combinedParams` 在此时已经包含 `Global + Pass + Callback`（甚至可能已有 worldMatrix）
- 标记为 `Pass` 会让日志显示错误的覆盖来源
- 破坏了 Origin 追踪的语义完整性

### 3.3 问题：worldMatrix 默认为 Manual

```cpp
objParams.SetMatrix("worldMatrix", r->GetWorldMatrix());
```

调用 `SetMatrix` → `AssignValue(..., ParameterOrigin::Manual)`

**问题**：
- `worldMatrix` 是 Per-Object 的核心参数，应该标记为 `Object` Origin
- 如果 Callback 想覆盖 worldMatrix（例如动画位移），会被误判为 `Manual → Callback` 而非 `Object → Callback`

---

## 4. 语义混乱与误导

### 4.1 ChainMerge vs 分阶段 MergeWithPriority

代码中同时存在两种合并模式：

**模式 A**：ChainMerge（一次性合并 4 层）
```cpp
auto result = ShaderParameterContainer::ChainMerge(
    global, pass, &object, &callback
);
```

**模式 B**：分阶段合并（实际使用）
```cpp
// Stage 1: Global + Pass
auto merged = MergeWithPriority(global, pass);

// Stage 2: 在 Execute 中逐 Object 处理
ShaderParameterContainer objParams = merged;
objParams.SetMatrix("worldMatrix", ...);
if (callback) callback(objParams);

// Stage 3: Object 合并（在 RenderableObject 中）
combinedParams = MergeWithPriority(combinedParams, object_parameters_);
```

**问题**：
1. **两种路径不等价**：ChainMerge 的 Origin 传播逻辑与分阶段不同
2. **`ChainMerge` 未被使用**：在 RenderGraph 主路径中完全被绕过（仅在 RenderPass.cpp 和 Graphics.cpp 中有 2 处调用，但并非主要渲染路径）
3. **测试覆盖偏差**：`TestChainMergePriorityOrder` 测试的是未使用的代码路径

### 4.2 ParameterOrigin 的语义模糊

当前 Origin 枚举：
```cpp
enum class ParameterOrigin {
    Unknown,
    Manual,   // 手动 Set？还是默认值？
    Global,   // 全局参数
    Pass,     // Pass 参数
    Object,   // Object 参数
    Callback  // Callback 动态设置
};
```

**问题**：
- **Manual 语义不清**：是开发者手动调用 `Set*` 时的默认值，还是指"未通过优先级链"？
- **Unknown 何时出现**：只在初始化时？还是合并时丢失 Origin？
- **无法区分子来源**：Pass 内部可能包含多个子来源（编译期注册、手动设置、输入纹理注入）

### 4.3 回退语义的不透明性

在 `AssignValue` 中：
```cpp
ParameterOrigin resolved_origin = origin;
if (resolved_origin == ParameterOrigin::Unknown) {
    resolved_origin = previous_origin != ParameterOrigin::Unknown
                          ? previous_origin
                          : ParameterOrigin::Manual;
}
```

**问题**：
- 如果传入 `Unknown`，会保留旧的 Origin → 这是 **Merge 语义** 还是 **赋值语义**？
- 调用者很难预测最终 Origin 是什么
- 日志中的 Origin 可能与实际调用点不匹配

---

## 5. 性能与效率问题

### 5.1 每次合并的全量拷贝

```cpp
ShaderParameterContainer MergeWithPriority(
    const ShaderParameterContainer &lower,
    const ShaderParameterContainer &higher
) {
    ShaderParameterContainer result;  // ❌ 空容器
    result.ApplyOverrides(lower, ...);   // ❌ 拷贝所有 lower 的参数
    result.ApplyOverrides(higher, ...);  // ❌ 再遍历所有 higher 的参数
    return result;  // ❌ RVO 但仍然是值返回
}
```

**开销**：
- **3 次哈希表操作**：lower 遍历 + higher 遍历 + result 插入
- **无法复用**：每次合并都创建新容器
- **Per-Object 放大**：场景中 100 个物体 = 100 次全量拷贝

**对比工业实现**：
- Unreal Engine 的 `FMaterialParameterInfo` 使用**共享指针 + COW（Copy-On-Write）**
- Unity 的 `MaterialPropertyBlock` 使用 **Sparse Set** + 差异追踪

### 5.2 无增量更新机制

```cpp
// 每帧都要执行的代码
for (auto &r : renderables) {
    ShaderParameterContainer objParams = merged;  // ❌ 全量拷贝
    objParams.SetMatrix("worldMatrix", ...);      // ✅ 唯一变化
    if (cb) cb(objParams);                         // ❌ 可能覆盖大量参数
    r->Render(*shader_, objParams, ...);
}
```

**问题**：
- 大部分参数（光照、阴影配置、纹理）每帧不变
- 没有脏标记（Dirty Flag）跳过重复合并
- 无法利用前一帧的合并结果

### 5.3 Origin 追踪的内存开销

```cpp
std::unordered_map<std::string, ParamValue> typed_parameters_;
std::unordered_map<std::string, ParameterOrigin> parameter_origins_;  // ❌ 第二个哈希表
```

**问题**：
- **双倍查找**：每次访问参数需要查 2 个哈希表
- **内存翻倍**：每个参数名存储 2 次（在两个 map 的 key 中）
- **缓存不友好**：两个独立哈希表破坏空间局部性

**改进方案**：
```cpp
struct ParameterEntry {
    ParamValue value;
    ParameterOrigin origin;
};
std::unordered_map<std::string, ParameterEntry> parameters_;  // 单哈希表
```

---

## 6. 类型安全漏洞

### 6.1 运行时类型检查的脆弱性

```cpp
if (existing_type != incoming_type) {
    // 直接抛异常，无法恢复
    throw std::runtime_error(...);
}
```

**问题场景**：
1. **Pass 设置了 float 参数 "shadowStrength"**
2. **Object 错误地设置了 Vector4 的 "shadowStrength"**
3. **渲染循环直接崩溃**（无降级、无跳过、无日志后继续）

**工业实践**：
- **类型转换**：Float → Vector4 自动扩展为 (v, 0, 0, 0)
- **警告跳过**：记录错误但保留旧值，继续渲染
- **编译期检查**：模板参数编码类型（如 `ParameterHandle<float>`）

### 6.2 std::variant 的 std::get_if 忽略错误

```cpp
template <typename T>
bool TryGet(const std::string &name, T &out) const {
    auto it = typed_parameters_.find(name);
    if (it == typed_parameters_.end()) {
        return false;  // ✅ OK
    }
    if (const auto *value = std::get_if<T>(&it->second)) {
        out = *value;
        return true;  // ✅ OK
    }
    return false;  // ❌ 类型不匹配静默失败
}
```

**问题**：
- 调用者无法区分"参数不存在"和"类型不匹配"
- 日志中不会有任何提示（除非开启 type_mismatch_logging）
- 调试时难以追踪参数绑定失败的原因

### 6.3 ID3D11ShaderResourceView* 的生命周期隐患

```cpp
void SetTexture(const std::string &name, ID3D11ShaderResourceView *texture) {
    auto param_value = decltype(typed_parameters_)::mapped_type(texture);
    AssignValue(name, param_value);  // ❌ 裸指针存储，无引用计数
}
```

**问题**：
1. **无所有权语义**：不知道是 weak reference 还是 strong reference
2. **悬空指针风险**：如果纹理在 RenderTexture 中被销毁，指针失效
3. **无法追踪引用**：ResourceManager 不知道参数容器持有纹理

**修复方案**：
- 使用 `Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>` 自动管理引用计数
- 或者存储资源名称（string），延迟到绑定时查询 ResourceRegistry

---

## 7. 可测试性与调试性

### 7.1 无法查询参数的完整来源链

**当前能力**：
```cpp
auto origin = container.GetOrigin("shadowStrength");  // ❌ 此 API 不存在
// 只能在合并时通过日志看到覆盖
```

**缺失功能**：
- **查询某参数的覆盖历史**（Global:0.2 → Pass:0.5 → Callback:0.9）
- **导出当前参数快照**（用于回放/调试）
- **diff 两个容器**（对比 Pass A 和 Pass B 的参数差异）

### 7.2 日志开关的全局性破坏隔离

```cpp
static void SetTypeMismatchLoggingEnabled(bool enabled);
static void SetOverrideLoggingEnabled(bool enabled);
```

**问题**：
- 开启日志后 **所有** Pass/Object 的参数操作都会打印
- 无法只调试某个 Pass 的参数绑定
- 多线程环境下无法区分不同 RenderGraph 实例的日志

**改进**：
```cpp
struct LoggingConfig {
    bool log_type_mismatches = true;
    bool log_overrides = false;
    std::function<bool(const std::string&)> should_log_parameter;
};
ShaderParameterContainer container(LoggingConfig config);
```

### 7.3 测试覆盖的盲区

**已有测试**：
- ✅ `TestMergeWithPriorityOverrides`（2 层合并）
- ✅ `TestChainMergePriorityOrder`（4 层合并，但未实际使用）
- ✅ `TestTypeConflictThrows`（类型冲突）

**缺失测试**：
- ❌ **RenderGraph 完整路径测试**（Global → Pass → Object → Callback 的真实执行顺序）
- ❌ **Origin 传播正确性测试**（检查每个阶段的 Origin 是否正确）
- ❌ **worldMatrix 覆盖测试**（Callback 能否覆盖 worldMatrix）
- ❌ **多 Pass 共享 Global 参数测试**（检查隔离性）
- ❌ **性能基准测试**（1000 个 Object 的合并开销）

---

## 8. 边界情况与未定义行为

### 8.1 Callback 直接修改导致的竞态

**代码路径**：
```cpp
// RenderGraphPass::Execute
ShaderParameterContainer objParams = merged;  // Copy
if (auto cb = r->GetParameterCallback()) {
    cb(objParams);  // ❌ Callback 直接修改容器
}
```

**问题场景**：
```cpp
// 场景 1：Callback 清空容器
pass->SetParameterCallback([](auto& params) {
    params = ShaderParameterContainer();  // ❌ 清空所有参数！
});

// 场景 2：Callback 删除关键参数（假设有 Remove API）
pass->SetParameterCallback([](auto& params) {
    params.Remove("worldMatrix");  // ❌ 删除必需参数
});
```

**缺失保护**：
- 没有"只读视图"（const 容器 + 追加写入 API）
- 没有参数锁定机制（禁止 Callback 修改某些关键参数）

### 8.2 循环覆盖导致的 Origin 混乱

```cpp
ShaderParameterContainer a, b;
a.SetFloat("value", 1.0f);  // Origin: Manual

b = ShaderParameterContainer::MergeWithPriority(a, b, 
    ParameterOrigin::Global, ParameterOrigin::Pass);  // value 标记为 Global

a = ShaderParameterContainer::MergeWithPriority(b, a,
    ParameterOrigin::Pass, ParameterOrigin::Object);  // value 标记为 Pass（覆盖了 Global）
```

**问题**：
- Origin 的"最后写入"语义可能与"优先级链"语义冲突
- 无法追踪参数的"首次来源"vs"当前来源"

### 8.3 空容器合并的语义歧义

```cpp
ShaderParameterContainer empty;
auto result = MergeWithPriority(empty, pass_params);
// result 的 Origin 是什么？Pass？还是 Unknown（因为 lower 是空的）？
```

**当前实现**：
- `ApplyOverrides` 会遍历空 map（无操作）
- `pass_params` 的 Origin 保留为 Pass ✅
- **但**：如果反过来 `MergeWithPriority(pass_params, empty)`，会不会覆盖？

---

## 9. 正面评价（难得的优点）

### ✅ 9.1 类型安全的 Variant 设计

使用 `std::variant<XMMATRIX, XMFLOAT3, ...>` 而非 `void*`：
- **编译期类型检查**：`std::get_if<T>` 会检查类型匹配
- **避免内存布局错误**：不会出现 float[4] 被当作 Matrix 读取的 bug
- **异常安全**：`std::variant` 保证析构函数调用

### ✅ 9.2 统一的参数接口

所有 Shader 参数通过同一个容器传递，避免：
- **函数签名爆炸**：不需要 `SetMatrix(...)`, `SetVector3(...)`, ... 的重载地狱
- **扩展性好**：添加新类型（如 `StructuredBuffer`）只需修改 variant 定义
- **便于序列化**：容器可以导出为 JSON（虽然当前未实现）

### ✅ 9.3 Origin 追踪的尝试

虽然实现有缺陷，但**追踪参数来源的想法是正确的**：
- 便于调试参数覆盖问题
- 为未来的参数验证/审计打下基础
- 体现了对"数据流追溯"的意识

### ✅ 9.4 避免了字符串枚举的陷阱

使用 `enum class ShaderParameterType` 和 `enum class ParameterOrigin` 而非字符串：
- **类型安全**：编译期检查，避免拼写错误
- **高效比较**：整数比较而非字符串比较
- **便于重构**：重命名时有编译器支持

---

## 10. 修复优先级建议

### 🔴 P0：立即修复（破坏核心语义）

#### P0-1：修复 RenderableObject 的优先级顺序
```cpp
// 当前错误实现
void RenderableObject::Render(...) {
    combinedParams = MergeWithPriority(
        combinedParams,       // 已包含 Callback
        object_parameters_,   // Object 覆盖了 Callback ❌
        ParameterOrigin::Pass,
        ParameterOrigin::Object
    );
}

// 修复方案 1：在 Pass 阶段正确合并
void RenderGraphPass::Execute(...) {
    ShaderParameterContainer merged = MergeParameters(global_params);
    
    for (auto &r : renderables) {
        // 正确的优先级顺序
        auto objParams = ShaderParameterContainer::ChainMerge(
            merged,                         // Global + Pass
            r->GetObjectParameters(),       // Object
            nullptr,                        // 预留 Material
            r->GetParameterCallback()       // Callback（最高）
        );
        objParams.SetMatrix("worldMatrix", r->GetWorldMatrix());
        r->Render(*shader_, objParams, device_context);
    }
}

// 修复方案 2：RenderableObject 不再自行合并
void RenderableObject::Render(...) {
    // 直接使用传入的参数（已经按优先级合并完成）
    if (is_window_model_)
        window_model_->Render(shader, parameters, deviceContext);
    else
        model_->Render(shader, parameters, deviceContext);
}
```

#### P0-2：统一使用 ChainMerge 或废弃它
```cpp
// 选择 A：只保留 ChainMerge，删除分阶段合并
ShaderParameterContainer RenderGraphPass::MergeParameters(...) {
    return ShaderParameterContainer::ChainMerge(
        global_params, *pass_parameters_, nullptr, nullptr
    );
}

// 选择 B：废弃 ChainMerge，明确每个合并点的 Origin
// （保持现状，但需要文档化每个合并点）
```

#### P0-3：正确标记 worldMatrix 的 Origin
```cpp
void RenderGraphPass::Execute(...) {
    ShaderParameterContainer objParams = merged;
    // 使用 AssignValue 而非 SetMatrix，明确 Origin
    objParams.AssignValue("worldMatrix", 
                         r->GetWorldMatrix(), 
                         ShaderParameterContainer::ParameterOrigin::Object);
    // 或者扩展 SetMatrix 接口：
    // objParams.SetMatrix("worldMatrix", r->GetWorldMatrix(), ParameterOrigin::Object);
}
```

### 🟡 P1：近期优化（设计缺陷）

#### P1-1：引入 COW（Copy-On-Write）减少拷贝
```cpp
class ShaderParameterContainer {
private:
    struct SharedData {
        std::unordered_map<std::string, ParamValue> parameters;
        std::unordered_map<std::string, ParameterOrigin> origins;
        std::atomic<int> ref_count{1};
    };
    std::shared_ptr<SharedData> data_;
    
    void EnsureUnique() {
        if (data_.use_count() > 1) {
            data_ = std::make_shared<SharedData>(*data_);  // COW
        }
    }
    
public:
    void SetMatrix(const std::string &name, const XMMATRIX &value) {
        EnsureUnique();  // 只在写入时拷贝
        // ... 原逻辑
    }
};
```

#### P1-2：添加参数来源查询 API
```cpp
struct ParameterSourceInfo {
    ParameterOrigin origin;
    ShaderParameterType type;
    size_t override_count;  // 被覆盖次数
};

class ShaderParameterContainer {
public:
    std::optional<ParameterSourceInfo> GetParameterInfo(const std::string &name) const;
    void DumpParameters(std::ostream &out) const;  // 导出所有参数 + Origin
};
```

#### P1-3：改进类型冲突处理
```cpp
enum class TypeConflictPolicy {
    Throw,          // 当前行为：抛异常
    KeepOld,        // 保留旧值，记录警告
    Coerce,         // 尝试类型转换
    UseDefault      // 使用默认值
};

class ShaderParameterContainer {
    TypeConflictPolicy type_conflict_policy_ = TypeConflictPolicy::Throw;
public:
    void SetTypeConflictPolicy(TypeConflictPolicy policy);
};
```

### 🟢 P2：长期改进（工程质量）

#### P2-1：参数锁定机制
```cpp
class ShaderParameterContainer {
    std::unordered_set<std::string> locked_parameters_;
public:
    void LockParameter(const std::string &name);  // 禁止后续修改
    void UnlockParameter(const std::string &name);
    bool IsParameterLocked(const std::string &name) const;
};
```

#### P2-2：只读视图给 Callback
```cpp
class ReadOnlyParameterView {
    const ShaderParameterContainer &container_;
public:
    // 只提供 Get* 方法，无 Set* 方法
    float GetFloat(const std::string &name) const;
    // ...
};

using SafeShaderParameterCallback = std::function<
    void(ReadOnlyParameterView view, ShaderParameterContainer &writable)
>;
```

#### P2-3：性能监控埋点
```cpp
struct ParameterContainerStats {
    size_t merge_count = 0;
    size_t copy_count = 0;
    size_t parameter_count = 0;
    std::chrono::microseconds total_merge_time{0};
};

class ShaderParameterContainer {
    static ParameterContainerStats global_stats_;
public:
    static ParameterContainerStats GetStats();
    static void ResetStats();
};
```

---

## 总结：优先级系统的根本问题

### 核心矛盾

**设计意图**：建立清晰的 4 层优先级链（Global → Pass → Object → Callback）

**实现现实**：
1. 优先级顺序在 `RenderableObject` 中**反转**
2. Origin 标签与实际来源**不匹配**
3. Callback 的优先级通过"直接修改"实现，**绕过了追踪系统**
4. 性能开销（每 Object 全量拷贝）与试验性目标**不匹配**

### 根源分析

这是一个**渐进式演化导致的架构债务**：
1. 最初只有 Global + Pass 两层（简单 MergeWithPriority）
2. 添加 Object 参数时，在 `RenderableObject::Render` 中加入了合并（错误时机）
3. 添加 Callback 时，用"直接修改容器"绕过了复杂度（破坏追踪）
4. 添加 Origin 追踪时，未重构已有的合并路径（导致标签错误）

### 适合试验性目标的折中方案

考虑到你的目标是"方便试验各种图形技术"，而非工业级引擎，**不建议完全重构**，但需要：

#### 最小必要修复
1. **修复 RenderableObject 优先级反转**（P0-1）
2. **删除未使用的 ChainMerge 或统一路径**（P0-2）
3. **添加参数来源查询 API**（P1-2），方便调试

#### 可接受的技术债务
以下问题在"试验性项目"中**可以容忍**：
- ✅ **性能开销**（每帧拷贝参数容器）：如果场景 <100 物体，影响 <1ms
- ✅ **Origin 追踪不完美**：只要不影响渲染结果，略有偏差可接受
- ✅ **类型冲突直接抛异常**：试验阶段可以快速暴露问题

#### 不可接受的缺陷
以下问题会**阻碍技术试验**，必须修复：
- ❌ **优先级反转**：会导致参数覆盖行为不可预测
- ❌ **缺少参数查询**：调试参数绑定问题会浪费大量时间
- ❌ **Origin 标签错误**：日志会误导问题诊断

---

## 最终建议

### 如果你有 2 小时
- 修复 P0-1（优先级顺序）
- 添加参数导出函数（`DumpParameters`）

### 如果你有 1 天
- 完成所有 P0 修复
- 添加 P1-2（参数查询 API）
- 写一个集成测试验证完整优先级链

### 如果你想彻底重构
考虑参考 Unity 的 `MaterialPropertyBlock` 模式：
- **分离声明与存储**：Material 持有默认参数，PropertyBlock 只存储覆盖
- **延迟合并**：在 SetPass 时才真正应用参数到 GPU
- **Sparse 存储**：只存储与默认值不同的参数

但对于试验性项目，**完全重构可能过度设计**。

---

> **批评总结**：这是一个有良好初衷、但执行时逐步偏离设计目标的系统。核心问题是**实现路径与声明语义不一致**，而非设计本身错误。针对试验性目标，建议**轻量级修复核心 bug**，而非全面重构。
