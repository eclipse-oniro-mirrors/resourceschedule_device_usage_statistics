# 应用使用统计

本文件是应用使用统计实体的说明（SSOT），包含区间类型枚举表、JS↔内部枚举映射、聚合规则。术语速查见 [领域术语](../glossary.md)。

## 概念

应用使用统计（Package Usage Stats）是按时间区间聚合的单应用使用指标集合，描述某应用在指定区间内的前台使用时长、启动次数、连续任务时长等。支持按日/周/月/年/自动四种区间粒度查询。

## 关键指标

| 指标 | 含义 |
|---|---|
| bundleName | 应用包名 |
| lastTimeUsed | 最近一次前台使用时间（墙钟） |
| totalInFrontTime | 区间内前台总时长（毫秒） |
| bundleStartedCount | 区间内应用启动次数（前台切换次数） |
| startCount | 区间内 Ability 启动次数 |
| lastTimeContinuousTask | 最近一次连续任务时间 |
| totalContinuousTaskUsedTime | 区间内连续任务总时长 |
| lastEvent | 最近事件 ID |
| uid | 应用 uid（区分分身） |
| appIndex | 应用分身索引 |

## 区间类型

| JS 枚举 | 值 | 含义 |
|---|---|---|
| BY_OPTIMIZED | 0 | 自动选择最优粒度（按跨度判定：日/周/月/年） |
| BY_DAILY | 1 | 按日 |
| BY_WEEKLY | 2 | 按周 |
| BY_MONTHLY | 3 | 按月 |
| BY_ANNUALLY | 4 | 按年 |

> JS 的 `BY_OPTIMIZED(0)` 与 `BY_ANNUALLY(4)` 到 Service 后映射为内部 `PERIOD_BEST(4)` 与 `PERIOD_YEARLY(3)`，非简单一一对应。

## 使用方式

### 查询接口

- `queryBundleStatsInfos(begin, end)`：查询当前应用（调用方）的统计，按最优区间。仅返回调用方自身数据。
- `queryBundleStatsInfoByInterval(byInterval, begin, end)`：按指定区间查询所有应用统计。
- `queryAppStatsInfos(begin, end)`：按最优区间查询所有应用统计，返回 `Map<bundleName, Array<stats>>`，含 `appIndex`。
- `queryLastUseTime(appInfos)`：查询指定应用集合的最近使用时间，返回 `Map<bundleName, Array<stats>>`。

### 聚合规则

- 查询时合并 DB 历史分表数据与内存当前周期数据。
- 同一 `bundleName + uid` 跨多区间的结果合并：时长/次数累加，最近时间取最大。
- `BY_OPTIMIZED` 由 DB 依据 `endTime - beginTime` 跨度自动选择日/周/月/年。
- 应用分身按 uid 区分，查询结果含 `appIndex`。

### 权限

- 查询自身数据（`queryBundleStatsInfos`）：系统应用 + `BUNDLE_ACTIVE_INFO`。
- 查询所有应用数据：系统应用或 Native Token + `BUNDLE_ACTIVE_INFO`。

## 内存模型与持久化

每个用户维护四个区间的内存统计（`BundleActivePeriodStats`），事件到来时同步更新四个区间。定时落盘（时机详见 [领域术语 - 落盘/刷新](../glossary.md#落盘--刷新flush--refresh-to-disk)）/跨日/关机/时间变更时落盘到对应库文件：

| 区间 | 库文件 | 表 |
|---|---|---|
| 日 | daily.db | `PackageLog<ts>` |
| 周 | weekly.db | `PackageLog<ts>` |
| 月 | monthly.db | `PackageLog<ts>` |
| 年 | yearly.db | `PackageLog<ts>` |

各区间表的保留张数与事件表滚动清理策略详见 [架构 §4.3](../architecture.md#43-数据库分表与版本演进)。

表列：`userId, bundleName, bundleStartedCount, lastTime, lastTimeContinuousTask, totalTime, totalTimeContinuousTask, uid`。

## 跨日与时间漂移

事件时间超过当前日周期截止时触发 `RenewStatsInMemory`：为仍在运行的应用注入 `END_OF_THE_DAY` 事件以提交时长 → 落盘当前统计 → 清理过期表 → 重新加载新周期统计。墙钟漂移超 10 分钟时全量落盘 + 按时间差重命名所有时表 + 重载。保留策略详见 [架构 §4.3](../architecture.md#43-数据库分表与版本演进)。

---

## 相关文档

- [使用事件](usage-event.md) — 事件到来时更新四区间统计
- [系统事件统计](device-event-stats.md) — 共用 `PackageLog` 表（bundleName 为 `openharmony`）
