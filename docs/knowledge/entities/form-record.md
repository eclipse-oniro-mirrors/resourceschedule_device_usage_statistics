# 卡片使用记录

本文件是卡片使用记录实体的说明。卡片记录嵌套于模块使用记录中（见 [模块使用记录](module-usage-record.md)），由卡片事件触发更新。

## 概念

卡片使用记录（Form Record）记录应用桌面卡片（Widget）的点击次数与移除事件，关联到所属模块记录下。用于卡片使用热度分析、智能卡片推荐、卡片排序等场景。

## 关键属性

| 属性 | 类型 | 含义 |
|---|---|---|
| bundleName | string | 应用包名 |
| moduleName | string | 所属模块名称 |
| formName | string | 卡片名称 |
| formDimension | int32 | 卡片尺寸规格 |
| formId | int64 | 卡片实例 ID（唯一标识） |
| touchCount | int32 | 点击次数 |
| formLastUsedTime | int64 | 最近使用时间（墙钟） |
| userId | int32 | 用户 ID |
| uid | int32 | 应用 uid |

## 使用方式

### 采集

卡片事件由外部 SA 经 `ReportEvent` 上报：
- `FORM_IS_CLICKED(15)`：卡片被点击。服务侧定位或创建对应 `formId` 的卡片记录，累加 `touchCount`，更新 `formLastUsedTime`。
- `FORM_IS_REMOVED(16)`：卡片被移除。服务侧从内存移除该卡片记录，并从数据库 `FormRecordLog` 表删除对应行。

### 查询

卡片记录不单独提供查询接口，而是作为模块使用记录的嵌套列表返回（`queryModuleUsageRecords` 结果中每个模块的 `formRecords` 字段）。卡片记录按 `touchCount` 降序排列。

### 权限

随模块使用记录查询权限一致：系统应用或 Native Token + `BUNDLE_ACTIVE_INFO`。

## 持久化

卡片记录持久化于 `daily.db` 的 `FormRecordLog<timestamp>` 表（按日分表），列：`userId, bundleName, moduleName, formName, formDimension, formId, touchCount, lastTime, uid`。随模块记录一同落盘与清理（保留策略见 [架构 §4.3](../architecture.md#43-数据库分表与版本演进)）。卡片的唯一性以 `formId` 判定。

---

## 相关文档

- [使用事件](usage-event.md) — `FORM_IS_CLICKED(15)` / `FORM_IS_REMOVED(16)` 事件
- [模块使用记录](module-usage-record.md) — 卡片记录嵌套于模块记录下
