# 应用优先级分组

本文件是应用优先级分组实体的说明（SSOT），包含六级分组表、分组判定机制（提升超时表、双维度阈值）、分组原因码、回调机制。术语速查见 [领域术语](../glossary.md)。

## 概念

应用优先级分组（App Group）是本部件依据应用使用活跃度对每个应用维护的优先级标签，供资源调度系统决定保活、限频、冻结等策略。分组是**数值越小越活跃**的六级体系（外加内部态）：

| 组别 | 编码 | 含义 | 自动检查间隔 |
|---|---|---|---|
| ALIVE | 10 | 最近活跃使用（前台/交互/长时任务） | 2h |
| DAILY | 20 | 每日级使用（如通知触发） | 4h |
| FIXED | 30 | 固定期使用 | 24h |
| RARE | 40 | 罕见使用 | 48h |
| LIMIT | 50 | 受限（系统判定低活跃） | — |
| NEVER | 60 | 从未使用（初始默认） | — |

内部态（不直接暴露给应用）：
- `FORCED_SET = 5`：外部强制设置，冻结自动调整直至再次被设置。
- `UNKNOWN = -1`：未定义。

## 分组判定机制

### 事件驱动提升

当匹配事件到来时立即提升分组并设置超时时间戳：

| 触发事件 | 提升至 | 超时（生产） |
|---|---|---|
| 前台 / 用户交互 / 长时任务启动 | ALIVE | 1h |
| 通知查看（NOTIFICATION_SEEN） | DAILY | 12h |
| 系统交互（SYSTEM_INTERACTIVE） | ALIVE | 10min |

提升后调度一条延迟分组检查消息，到期重新评估。

### 超时驱动降级

延迟消息到期或周期检查时，基于**双维度**查表得出新分组：
- 维度一：累计启动时长距上次使用差，阈值 `{0, 12h, 24h, 48h}`。
- 维度二：累计亮屏时长距上次使用差，阈值 `{0, 0, 1h, 2h}`。

取两个维度同时满足的最大索引（0~3），映射到 `{ALIVE, DAILY, FIXED, RARE}`。若仍在超时窗口内则钳制回 ALIVE / DAILY。

> DEBUG 模式下双维度阈值大幅缩短（启动 `{0, 2min, 4min, 16min}`、亮屏 `{0, 0, 2min, 4min}`），用于测试快速验证降级。

### 强制设置

`SetAppGroup` 接口以 `FORCED` 原因设置分组后，该 bundle 冻结自动提升/降级，直至下次被显式设置。禁止应用设置自身分组。

## 分组原因码

每次分组变更记录复合原因码（持久化于 `BundleHistoryLog.reasonInGroup`）：
- 高字节（控制来源）：DEFAULT / TIMEOUT / USAGE / FORCED / CALCULATED。
- 低字节（触发事件）：SYSTEM / NOTIFY_SEEN / USER_INTERACTION / FOREGROUND / BACKGROUND / ALIVE_TIMEOUT / LONG_TIME_TASK_STARTTED / CALCULATED_RESTORED / ALIVE_NOT_TIMEOUT。

## 使用方式

### 查询分组

- `queryAppGroup(bundleName?)` / `queryAppGroupSync(bundleName?)`：查询指定或当前应用分组。查询多 uid 的 bundle 时返回**最小**分组值（最活跃）。
- 查询自身（bundleName 空）须为系统应用；查询指定 bundle 须系统应用或 Native Token + `BUNDLE_ACTIVE_INFO`。

### 设置分组

- `setAppGroup(bundleName, newGroup)`：强制设置分组。`newGroup` 取 `GroupType` 枚举（10/20/30/40/50/60）。
- 禁止设置自身分组；重复设置相同分组与原因返回 `ERR_APPLICATION_GROUP_OPERATION_REPEATED`。

### 空闲判定

- `isIdleState(bundleName)` / `isIdleStateSync(bundleName)`：分组 > RARE(40) 即为空闲。

### 分组变化回调

分组实际变化时，服务侧向已注册的观察者投递回调，载荷包含：`oldGroup`（变更前分组）、`newGroup`（变更后分组）、`userId`、`changeReason`（变更原因码，见上文）、`bundleName`。

**注册与注销：**
- `registerAppGroupCallBack(callback)`：注册分组变化回调。**每进程仅允许注册一个观察者**，重复注册返回 `ERR_REPEAT_REGISTER_APP_GROUP_OBSERVER`。
- `unregisterAppGroupCallBack()`：注销回调。未注册时注销返回 `ERR_APP_GROUP_OBSERVER_IS_NULLPTR`。

**投递流程：**
1. 分组状态机检测到分组实际变化。
2. 经 FFRT 队列投递到核心编排器，先写 hisysevent（`UPDATE_GROUP_SCENE`），再 fan-out 到所有注册观察者。
3. HAP 观察者按 userId 过滤（仅通知同用户的应用）；Native 观察者始终通知。
4. 客户端经 oneway IPC 接收回调，NAPI/Taihe 层通过 JS 线程事件（高优先级）桥接到应用回调函数。

**可靠性：**
- 服务侧为每个观察者附加死亡Recipient，观察者进程死亡时自动移除。
- 客户端持有服务死亡Recipient，服务（SA 1907）死亡后 5 秒重试获取 Proxy 并自动重注册此前注册的回调，保证链路在服务重启后恢复。

## 持久化

分组态持久化于 `usageGroup.db` 的 `BundleHistoryLog` 表，每行记录一个 `bundleName + uid` 的分组状态：当前分组、原因码、上次使用启动时间、上次使用亮屏时间、活跃超时时间戳、每日超时时间戳、首次使用时间。设备启动时长与亮屏时长总量持久化于同库 `DurationLog`（单行），用于跨重启续算。

## 多用户与分身

每个 OS 账号维护独立的分组态。应用分身（同 bundle 多 uid）按 `bundleName + uid` 分别维护分组，查询时取最小值；卸载分身仅清理对应 uid。

---

## 相关文档

- [使用事件](usage-event.md) — 事件驱动分组提升的 eventId（2~25）
- [架构 §4.2](../architecture.md#42-分组状态机) — 分组状态机架构视角
- [编码规范 §5](../coding-standards.md#5-权限校验) — SetAppGroup 权限校验
