# 高频使用时段

本文件是高频使用时段实体的说明（判定算法、配置项、查询接口）。配置项默认值见 [业务上下文 §5](../business-context.md#5-配置上下文)。

## 概念

高频使用时段（High Frequency Period）是基于最近一周事件推断的、某应用使用频次最高的小时集合（0~23）。用于智能提醒时机选择、应用预加载、省电策略等场景。

## 关键属性

| 属性 | 类型 | 含义 |
|---|---|---|
| bundleName | string | 应用包名 |
| highFreqHours | array&lt;int32&gt; | 高频使用小时集合（0~23） |

## 判定算法

基于最近一周（7 天）的日粒度使用事件，按以下步骤推断每个应用的高频时段：

1. **总使用天数过滤**：统计该应用近一周有使用的不同天数，须 ≥ `minTotalUseDays`（默认 4，配置项 `MinTotalUseDays`），否则跳过。
2. **小时使用统计**：对每个小时（0~23），统计近一周在该小时内有使用的不同天数。
3. **候选小时筛选**：按使用天数降序排列小时，取前 `minTopUseHoursLimit`（默认 6，配置项 `MinTopUseHoursLimit`）个作为候选。
4. **单小时达标过滤**：候选小时的使用天数须 ≥ `minHourUseDays`（默认 4，配置项 `MinHourUseDays`）。
5. **结果截断**：达标小时按使用天数降序取前 `maxHighFreqHourNum`（默认 3，配置项 `MaxHighFrequencyHourNum`）个作为最终结果。

> 配置项均可通过 `etc/device_usage_statistics/device_usage_statistics_config.json` 覆盖（见 [业务上下文 - 配置上下文](../business-context.md#5-配置上下文)）。

## 使用方式

### 查询接口

- `queryHighFrequencyPeriodBundle()`：查询所有应用的高频使用时段，返回 `Array<HighFrequencyPeriod>`。
- `queryHighFrequencyUsageBundleInfos(userId, maxNum, queryDayRange)`：查询 Top-N 高频使用应用（按启动次数排序），`maxNum` 默认 20，`queryDayRange` 默认 7、上限 7。

### 权限

系统应用或 Native Token + `BUNDLE_ACTIVE_INFO`。

### Dump

`hidumper` 子命令 `HighFreqHourUsage <userId>`（需 ENG 构建 + DUMP 权限）。

## 数据来源

高频时段推断完全基于已落盘的日粒度使用事件（`event.db` 的 `DeviceEventLog<ts>` 表），查询时拉取近一周事件后在内存中聚合计算，不额外持久化推断结果。保留策略见 [架构 §4.3](../architecture.md#43-数据库分表与版本演进)。

---

## 相关文档

- [使用事件](usage-event.md) — 日粒度事件来源（`DeviceEventLog` 表）
