# 领域术语

本文件收录本项目特有、或 AI 容易理解偏差的业务术语。业界通用的工程概念（如 IPC、SQLite、GTest 等）不再赘述。

---

## Bundle

OpenHarmony 中"应用包"的概念。在本部件语境下，"bundle" 与"应用"等价。一个 bundle 可对应多个 uid（见"应用分身"）。系统级事件（休眠/唤醒/锁屏/解锁）统一以虚拟 bundle 名 `openharmony` 标记（常量 `DEVICE_EVENT_PACKAGE_NAME`）。

## 应用优先级分组（App Group）

依据使用活跃度将应用分为六个层级。完整分级表、判定机制与超时阈值详见 [应用优先级分组](entities/app-group.md)。

> **易误解点**：分组数值与"活跃度"**反向相关**——数值越小越活跃。判定"空闲态"时，分组值 > RARE(40) 即视为空闲。

## 空闲态（Idle State）

判定某应用是否处于"使用不活跃"状态。当应用当前分组 > `ACTIVE_GROUP_RARE`（40）时即为空闲。注意"空闲"由分组派生，并非独立计算的布尔标志。

## 使用活跃期（Use Period）

`IsBundleUsePeriod` 接口的判定概念：基于最近一周的应用每日启动次数，统计落在配置区间 `[minUseTimes, maxUseTimes]` 内的天数是否达到 `minUseDays`。该接口仅对 Native Token 开放，用于系统内部资源调度决策，不暴露给应用。

## 区间类型（Interval Type）

应用使用统计按时间区间聚合的粒度（BY_OPTIMIZED / BY_DAILY / BY_WEEKLY / BY_MONTHLY / BY_ANNUALLY）。完整枚举表详见 [应用使用统计](entities/package-usage-stats.md)。

> **易误解点**：JS 传 `BY_OPTIMIZED(0)` 与 `BY_ANNUALLY(4)` 到 Service 后会被映射为内部 `PERIOD_BEST(4)` 与 `PERIOD_YEARLY(3)`，二者并非简单一一对应。内部常量 `PERIOD_BEST = 4` 不等于 JS 的 `BY_ANNUALLY = 4`。详见 [应用使用统计 - 区间类型](entities/package-usage-stats.md#区间类型)。

## 使用事件（Usage Event）

描述一次应用状态变更或系统事件的记录。事件由内部整数 `eventId` 标识（2~25），覆盖应用前后台、长时任务起止、屏幕亮灭、锁屏解锁、休眠唤醒、通知查看、卡片点击/移除、系统交互等。详见 [使用事件实体](entities/usage-event.md)。

> **易误解点**：同一逻辑行为可能对应多个事件 ID。如"应用进入前台"在 Ability 级别为 `ABILITY_FOREGROUND(2)`，在 App 级别为 `APP_FOREGROUND(24)`；"进入后台"对应 `ABILITY_BACKGROUND(3)` 与 `APP_BACKGROUND(25)`。二者由不同观察者上报，服务侧统一处理。

## 连续任务（Continuous Task / Long Time Task）

应用在后台持续运行的长时任务（如音乐播放、导航）。由 `BackgroundTaskMgr` 通知起止，对应事件 `LONG_TIME_TASK_STARTTED(5)` / `LONG_TIME_TASK_ENDED(6)`。会触发应用提升至 ALIVE 组。

## 启动基时间（Boot-based Time）

以设备本次启动为基准的计时（`TimeServiceClient::GetBootTimeMs()`），单调递增、不受系统时钟调整影响。本部件用"累计启动时长"度量应用距上次使用的间隔，可跨重启累加（持久化到 `DurationLog`）。

> **易误解点**：分组判活的"距上次使用时长"并非墙钟时间差，而是累计启动时长差——设备休眠期间该时长不增长。同理"累计亮屏时长"仅在屏幕点亮时增长。具体阈值详见 [应用优先级分组 - 分组判定机制](entities/app-group.md#分组判定机制)。

## 累计亮屏时长（Screen-on Duration）

设备在亮屏状态下累计运行的时长（基于启动基时间），持久化于 `DurationLog`。与"累计启动时长"共同构成分组判活的**双维度**：一个 bundle 必须在两个维度上同时超过阈值才会被降组。阈值详见 [应用优先级分组](entities/app-group.md#分组判定机制)。

## 墙钟时间（Wall Time）

系统日历时间（毫秒），用于事件入库与查询时间窗口。由 `BundleActiveCore` 将启动基时间转换为墙钟时间后写入数据库。当墙钟时间漂移超过 10 分钟（`TIME_CHANGE_THRESHOLD_MILLIS`）时触发全量刷新与时表重命名。

## 分组原因码（Reason in Group）

持久化于 `BundleHistoryLog` 的复合整数，编码分组变更的来源（高字节控制来源 + 低字节触发事件）。完整清单详见 [应用优先级分组 - 分组原因码](entities/app-group.md#分组原因码)。

## 落盘 / 刷新（Flush / Refresh to Disk）

将内存中的统计数据写入数据库的过程。触发时机：每 30 分钟定时（DEBUG 模式 2 分钟）、跨日、系统时间变更、关机、用户切换。**非实时**：事件先进入内存统计，再批量落盘。

## 应用分身（App Twin）

同一 bundle 对应多个 uid 的情形（应用分身/双开）。数据模型与持久化均以 `bundleName + uid` 组合键区分。卸载时若 `appIndex != 0` 仅删除对应 uid 的数据。

## FA / 模块（Module）

应用的 HAP 模块（历史称 Feature Ability）。模块使用记录统计每个模块的启动次数与最近使用时间，按启动次数排序返回。详见 [模块使用记录](entities/module-usage-record.md)。

## 卡片（Form）

应用的桌面卡片/Widget。卡片使用记录统计点击次数与移除事件，关联到所属模块记录下。详见 [卡片使用记录](entities/form-record.md)。

## 高频使用时段（High Frequency Period）

基于最近一周事件推断的、某应用使用频次最高的小时集合（0~23）。判定算法详见 [高频使用时段](entities/high-frequency-period.md)。

## SA（System Ability）

OpenHarmony 系统能力，由 samgr 注册、safwk 托管。本部件 SA ID 为 1907，宿主进程 `resource_schedule_service`。

## Debug 模式

需同时满足 `const.debuggable = 1`（ENG 构建）且 `persist.sys.device_usage_debug_on` 为真。开启后将所有时间阈值大幅缩短（如落盘间隔 30min→2min、分组判活 12h→2min、日表周期 24h→10min），用于测试快速验证。

## 相关文档

- [架构原则](architecture.md) — 分层、事件管线、分组状态机
- [编码规范](coding-standards.md) — 时间基准、并发模型、权限校验
