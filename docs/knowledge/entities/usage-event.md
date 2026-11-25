# 使用事件

本文件是使用事件实体的说明（事件类型表、关键属性、处理流程、持久化）。术语速查见 [领域术语](../glossary.md)，架构级事件管线见 [架构 §4.1](../architecture.md#41-事件处理管线)。

## 概念

使用事件（Usage Event）是本部件的最小数据单元，描述一次应用状态变更或系统事件的记录。所有外部状态变更（应用前后台、屏幕亮灭、锁屏解锁、休眠唤醒、通知、卡片、长时任务等）在进入本部件时统一转化为使用事件，经串行化队列处理后写入统计态与分组态。

## 事件类型

事件由整数 `eventId` 标识，取值 2~25：

| eventId | 常量 | 含义 | 上报方 |
|---|---|---|---|
| 2 | ABILITY_FOREGROUND | Ability 进入前台 | AppStateObserver / 窗口可见性 |
| 3 | ABILITY_BACKGROUND | Ability 进入后台 | AppStateObserver / 窗口可见性 |
| 4 | ABILITY_STOP | Ability 终止 | AppStateObserver |
| 5 | LONG_TIME_TASK_STARTTED | 长时任务启动 | ContinuousTaskObserver |
| 6 | LONG_TIME_TASK_ENDED | 长时任务结束 | ContinuousTaskObserver |
| 7 | SYSTEM_INTERACTIVE | 系统交互 | 外部 SA |
| 8 | USER_INTERACTIVE | 用户交互 | 外部 SA |
| 9 | END_OF_THE_DAY | 日结束（内部） | UserService 跨日 |
| 10 | SHUTDOWN | 关机 | ShutdownCallback |
| 11 | STARTUP | 启动 | UserService 初始化 |
| 12 | FLUSH | 刷新（内部） | UserService |
| 13 | SCREEN_INTERACTIVE | 屏幕亮 | CommonEventSubscriber |
| 14 | SCREEN_NON_INTERACTIVE | 屏幕灭 | CommonEventSubscriber |
| 15 | FORM_IS_CLICKED | 卡片点击 | 外部 SA |
| 16 | FORM_IS_REMOVED | 卡片移除 | 外部 SA |
| 17 | KEYGUARD_SHOWN | 锁屏 | CommonEventSubscriber |
| 18 | KEYGUARD_HIDDEN | 解锁 | CommonEventSubscriber |
| 19 | NOTIFICATION_SEEN | 通知查看 | 外部 SA |
| 20 | SYSTEM_LOCK | 系统锁屏 | CommonEventSubscriber |
| 21 | SYSTEM_UNLOCK | 系统解锁 | CommonEventSubscriber |
| 22 | SYSTEM_SLEEP | 系统休眠 | PowerStateCallback |
| 23 | SYSTEM_WAKEUP | 系统唤醒 | PowerStateCallback |
| 24 | APP_FOREGROUND | App 进入前台 | AppStateObserver |
| 25 | APP_BACKGROUND | App 进入后台 | AppStateObserver |

> **注意**：Ability 级前台（2）与 App 级前台（24）是不同观察者上报的，二者均会被处理。当启用窗口可见性（`window_visibility_enable`）时，Ability 前台事件会延迟到窗口实际可见后才上报，避免后台预加载被误计为前台使用。

系统级事件（13~23、9~12）的 bundleName 统一标记为虚拟名 `openharmony`（`DEVICE_EVENT_PACKAGE_NAME`）。

## 关键属性

| 属性 | 类型 | 含义 |
|---|---|---|
| bundleName | string | 应用包名（系统事件为 `openharmony`） |
| eventId | int32 | 事件类型（见上表） |
| timeStamp | int64 | 事件发生时间（启动基时间打点，入库前转墙钟时间） |
| uid | int32 | 应用 uid（区分应用分身） |
| abilityId | int64 | Ability 标识 |
| abilityName | string | Ability 名称 |
| moduleName | string | 模块名称（模块/卡片事件） |
| continuousTaskAbilityName | string | 长时任务 Ability 名（长时任务事件） |
| formName | string | 卡片名称（卡片事件） |
| formDimension | int32 | 卡片尺寸规格（卡片事件） |
| formId | int64 | 卡片实例 ID（卡片事件） |

## 使用方式

### 上报

仅 `foundation` / `subfoundation` / `resource_schedule_service` 进程的 Native Token 可经 `ReportEvent`（oneway IPC）上报。进程内事件源经 `BundleActiveReportController` 投递到同一队列。

### 查询

- `queryBundleEvents(begin, end, maxNum?)`：查询所有应用事件。
- `queryCurrentBundleEvents(begin, end, maxNum?)`：查询当前应用事件（须系统应用）。
- `maxNum` 上限 1000，默认 1000，结果按时间倒序截断。

### Dump

`hidumper` 子命令 `Events <beginTime> <endTime> <userId>`（需 ENG 构建 + DUMP 权限）。

## 处理机制

1. 上报时以 `GetBootTimeMs()` 打点。
2. 经 `BundleActiveReportHandler` 串行化（FFRT 队列）。
3. `BundleActiveCore` 检查时间漂移、转墙钟时间、定位用户服务。
4. 卡片事件 → `ReportFormEvent`；模块前台事件 → `ReportModuleEvent`；其余 → `ReportEvent`（更新四区间统计）。
5. 同时转发到 `BundleActiveGroupController::ReportEvent` 更新分组态。
6. 触发延迟落盘任务（定时落盘时机详见 [领域术语 - 落盘/刷新](../glossary.md#落盘--刷新flush--refresh-to-disk)），事件写入 `event.db` 的 `DeviceEventLog<ts>` 表。

## 持久化

原始事件持久化于 `event.db` 的 `DeviceEventLog<timestamp>` 表（按周期分表），列：`userId, bundleName, eventId, timeStamp, abilityId, uid`。保留策略详见 [架构 §4.3](../architecture.md#43-数据库分表与版本演进)。

---

## 相关文档

- [应用优先级分组](app-group.md) — 事件驱动分组提升的 eventId 映射
- [应用使用统计](package-usage-stats.md) — 事件更新四区间统计态
- [架构 §4.1](../architecture.md#41-事件处理管线) — 事件处理管线与消息码
- [编码规范 §7](../coding-standards.md#7-数据模型序列化) — BundleActiveEvent Parcel 序列化
