# 系统事件统计

本文件是系统事件统计实体的说明。事件类型定义见 [使用事件](usage-event.md)，区间聚合规则见 [应用使用统计](package-usage-stats.md)。

## 概念

系统事件统计（Device Event Stats）是按时间区间聚合的系统级事件与通知次数的汇总，描述休眠、唤醒、锁屏、解锁、通知查看等事件在区间内的发生次数与总时长。用于分析设备使用模式与系统状态分布。

## 关键指标

| 指标 | 含义 |
|---|---|
| name | 事件名称（系统事件为 `openharmony`；通知统计为应用 bundleName） |
| eventId | 事件类型 ID |
| count | 区间内发生次数 |
| beginTimeStamp | 统计区间起始 |
| endTimeStamp | 统计区间结束 |
| lastEventTime | 最近一次发生时间 |
| totalTime | 区间内总时长（毫秒，仅对有时长的事件） |

## 事件来源

系统事件统计来源于使用事件中的系统级事件（bundleName 为 `openharmony`）：

| eventId | 事件 |
|---|---|
| 13 | 屏幕亮（SCREEN_INTERACTIVE） |
| 14 | 屏幕灭（SCREEN_NON_INTERACTIVE） |
| 17 | 锁屏（KEYGUARD_SHOWN） |
| 18 | 解锁（KEYGUARD_HIDDEN） |
| 22 | 系统休眠（SYSTEM_SLEEP） |
| 23 | 系统唤醒（SYSTEM_WAKEUP） |

通知次数统计来源于 `NOTIFICATION_SEEN(19)` 事件，按应用 bundleName 聚合。

## 使用方式

### 查询接口

- `queryDeviceEventStats(begin, end)`：查询系统事件（休眠/唤醒/锁屏/解锁）统计。
- `queryNotificationEventStats(begin, end)`：查询应用通知次数统计。

两个接口均按区间聚合，返回 `Array<DeviceEventStats>`。

### 权限

系统应用或 Native Token + `BUNDLE_ACTIVE_INFO`。

## 处理机制

系统事件在 `BundleActivePeriodStats` 中由四个 `BundleActiveEventTracker` 追踪：
- 屏幕交互（SCREEN_INTERACTIVE）
- 屏幕非交互（SCREEN_NON_INTERACTIVE）
- 锁屏显示（KEYGUARD_SHOWN）
- 锁屏隐藏（KEYGUARD_HIDDEN）

每个 tracker 记录当前区间内的开始时间、上次事件时间、累计时长、发生次数。查询时合并 DB 历史与内存当前周期数据，按 eventId 聚合。

## 持久化

系统事件统计随使用统计一同持久化，事件原始记录存于 `event.db` 的 `DeviceEventLog<ts>` 表，聚合后的统计指标存于对应区间的 `PackageLog<ts>` 表（bundleName 为 `openharmony`）。保留策略详见 [架构 §4.3](../architecture.md#43-数据库分表与版本演进)。

---

## 相关文档

- [使用事件](usage-event.md) — 系统事件 eventId 定义（13~23）
- [应用使用统计](package-usage-stats.md) — 区间类型与聚合规则
