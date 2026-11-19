# 模块使用记录

本文件是模块使用记录实体的说明。模块记录嵌套卡片记录（见 [卡片使用记录](form-record.md)），由 `ABILITY_FOREGROUND` 事件触发更新。

## 概念

模块使用记录（Module Usage Record）记录应用内每个 HAP 模块（历史称 FA / Feature Ability）的启动次数与最近使用时间，并关联该模块下的卡片使用记录。用于"最近使用模块"排序、桌面推荐、智能服务卡片等场景。

## 关键属性

| 属性 | 类型 | 含义 |
|---|---|---|
| bundleName | string | 应用包名 |
| moduleName | string | 模块名称 |
| abilityName | string | 入口 Ability 名称 |
| launchedCount | int32 | 模块启动次数 |
| lastModuleUsedTime | int64 | 最近使用时间（墙钟） |
| deviceId | string | 设备 ID |
| appLabelId | uint32 | 应用标签资源 ID |
| labelId | uint32 | 模块标签资源 ID |
| descriptionId | uint32 | 模块描述资源 ID |
| abilityLableId | uint32 | Ability 标签资源 ID |
| abilityDescriptionId | uint32 | Ability 描述资源 ID |
| abilityIconId | uint32 | Ability 图标资源 ID |
| installFreeSupported | bool | 是否支持免安装 |
| isNewAdded | bool | 是否新增 |
| removed | bool | 是否已移除 |
| userId | int32 | 用户 ID |
| uid | int32 | 应用 uid |
| formRecords | array | 该模块下的卡片记录列表（见 [卡片使用记录](form-record.md)） |

## 使用方式

### 查询接口

- `queryModuleUsageRecords(maxNum?)`：查询模块使用记录，按最近使用时间由近及远排序。
- `maxNum` 上限 1000，默认 1000，超出返回 `ERR_MAX_RECORDS_NUM_BIGER_THEN_ONE_THOUSAND`。

### 结果增强

查询时服务侧通过 `BundleActiveBundleMgrHelper` 向 `BundleMgrService` 查询每个模块的元信息（应用标签、模块标签、Ability 名称、图标、是否免安装等），与使用记录合并后返回。模块已卸载时标记 `removed`。

### 权限

系统应用或 Native Token + `BUNDLE_ACTIVE_INFO`。

## 处理机制

仅 `ABILITY_FOREGROUND(2)` 事件触发模块记录更新：服务侧定位或创建对应 `bundleName + moduleName` 的模块记录，累加 `launchedCount`，更新 `lastModuleUsedTime`。

## 持久化

模块记录持久化于 `daily.db` 的 `ModuleRecordLog<timestamp>` 表（按日分表），列：`userId, bundleName, moduleName, launchedCount, lastTime, uid`。随使用统计一同落盘与清理（保留策略见 [架构 §4.3](../architecture.md#43-数据库分表与版本演进)）。查询时合并 DB 历史与内存当前周期数据。

---

## 相关文档

- [使用事件](usage-event.md) — `ABILITY_FOREGROUND(2)` 触发模块记录更新
- [卡片使用记录](form-record.md) — 嵌套于模块记录下
- [应用使用统计](package-usage-stats.md) — 共用 `daily.db` 与落盘机制
