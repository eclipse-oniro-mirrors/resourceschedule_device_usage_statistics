# 业务上下文

本文件阐述本部件的业务背景、系统能力定位、外部系统依赖与交互上下文。权限校验逻辑详见 [编码规范 §5](coding-standards.md#5-权限校验)，本文仅从交互视角描述"谁能调什么"。

## 1. 业务背景

OpenHarmony 设备在使用过程中持续产生应用前台/后台切换、屏幕亮灭、锁屏解锁、休眠唤醒、通知、卡片点击等事件。这些数据是系统进行**资源调度与省电策略**的基础输入：系统需要知道哪些应用是"活跃"的（应保活、给予资源）、哪些是"罕见使用"的（可限制后台、推迟同步）、哪些应用在哪些时段被高频使用（用于智能提醒与冻结策略）。

本部件作为 `resourceschedule` 子系统下的**数据采集与查询中枢**，承担三类职责：

1. **采集**：订阅系统能力（应用管理、窗口管理、电源管理、后台任务管理、公共事件、时间服务）产生的状态变更，统一转化为内部事件流。
2. **统计与分组**：将事件按日/周/月/年聚合为使用时长、启动次数等统计指标；基于活跃度将应用划分为六级优先级分组，支撑资源调度决策。
3. **查询**：向系统应用、其它系统能力（SA）、CLI 工具提供使用统计、分组、高频时段等查询接口。

本部件**不做调度决策本身**，只提供数据与分组判定；具体调度策略由 `resource_schedule_service` 进程内的其它模块或外部消费方依据本部件输出的分组与统计结果制定。

---

## 2. 系统能力定位

- **子系统**：`resourceschedule`（资源调度）
- **部件名**：`device_usage_statistics`
- **SysCap**：
  - `SystemCapability.ResourceSchedule.UsageStatistics.App` — 应用使用统计
  - `SystemCapability.ResourceSchedule.UsageStatistics.AppGroup` — 应用分组
- **SA ID**：1907，宿主进程 `resource_schedule_service`，`run-on-create = true`
- **数据目录**：`/data/service/el1/public/bundle_usage/`（安全等级 S1）

---

## 3. 外部系统依赖

本部件在运行时与以下系统能力交互。`BundleActiveService::InitNecessaryState` 会以 2 秒间隔轮询，直到以下 SA 全部就绪后才完成初始化并 Publish：

| 外部能力 | 用途 | 交互方式 |
|---|---|---|
| AppMgrService（`ability_runtime`） | 监听 Ability/App 前后台切换与进程死亡 | 注册 `ApplicationStateObserver` |
| BundleMgrService（`appexecfwk_standard`） | 解析 uid↔bundleName、查询应用信息（图标/标签/ability 名）、识别桌面应用、应用分身 appIndex | 通过 `BundleActiveBundleMgrHelper` 单例代理 IPC |
| PowerMgrService（`power_manager`） | 监听休眠/唤醒/关机 | 注册 `PowerStateCallback` 与 `AsyncShutdownCallback`；条件编译守卫（见 [编码规范 §9](coding-standards.md#9-条件编译)） |
| BackgroundTaskMgr（`background_task_mgr`） | 监听长时任务起止 | 注册 `ContinuousTaskSubscriber`；条件编译 `BGTASKMGR_ENABLE` |
| CommonEventService | 订阅亮灭屏、用户增删切换、包安装卸载、锁屏解锁公共事件 | 注册 `CommonEventSubscriber` |
| TimeService | 获取启动基时间、墙钟时间 | 通过 `TimeServiceClient` 单例 |
| WindowManager | 监听窗口可见性变化（用于精确判定应用前台） | 注册 `IVisibilityChangedListener`；由配置 `window_visibility_enable` 开关 |
| OsAccountManager（`os_account`） | 获取活跃用户、uid→userId 映射、用户增删回调 | 条件编译 `OS_ACCOUNT_PART_ENABLED` |
| relational_store（NativeRdb） | SQLite 持久化 | 直接链接 `native_rdb` |
| samgr / safwk | SA 注册与生命周期托管 | 框架内置 |
| AccessToken（`access_token`） | 权限校验（系统应用判定、Token 类型判定、权限验证） | `AccessTokenKit` |
| init | 启动阶段拉起 SA | `post-fs-data` job |
| hilog / hisysevent / hitrace | 日志、埋点、链路追踪 | 宏与工具调用 |
| hicollie | 进程看门狗 | 链接 `libhicollie` |

---

## 4. 交互上下文

### 4.1 事件上报方（生产者）

仅以下三个进程的 Native Token 允许通过 IPC 调用 `ReportEvent` 上报事件：

| 进程 | 上报内容 |
|---|---|
| `foundation` | 应用生命周期、通知查看、锁屏解锁等 |
| `subfoundation` | 通知事件（subfoundation 进程上报） |
| `resource_schedule_service` | 本进程内的事件源（应用状态观察者、公共事件订阅者、电源/关机回调、后台任务观察者、窗口可见性监听）经 `BundleActiveReportController` 投递 |

> 任何其它进程调用 `ReportEvent` 均被拒绝。

### 4.2 查询方（消费者）

| 消费方 | 访问方式 | 权限要求 |
|---|---|---|
| 系统应用（HAP） | NAPI `@ohos.bundleState` / `@ohos.resourceschedule.usageStatistics` / Taihe FFI | 系统应用 + `ohos.permission.BUNDLE_ACTIVE_INFO` |
| 其它系统能力（SA） | innerkits `BundleActiveClient`（IPC Proxy） | Native/Shell Token + `ohos.permission.BUNDLE_ACTIVE_INFO` |
| CLI 工具 `ohos-usageStatsQuery` | 链接 innerkits 直接调用 | `ohos.permission.cli.BUNDLE_ACTIVE_INFO`；`check-bundle-period` 子命令仅 Native Token |
| Dump（调试） | `hidumper` / shell | `const.debuggable = 1` + `ohos.permission.DUMP` |

应用查询自身数据（如 `queryCurrentBundleEvents`、查询自身分组）要求调用方为系统应用（非系统应用返回 `ERR_NOT_SYSTEM_APP`）。

### 4.3 回调消费方

注册 `IAppGroupCallback` 的应用（系统应用）在应用分组变化时收到 `OnAppGroupChanged` 回调，载荷为 `AppGroupCallbackInfo`（含 oldGroup、newGroup、userId、changeReason、bundleName）。回调经 IPC oneway 投递，客户端通过 JS 线程事件桥接到 NAPI/Taihe 回调。**每进程仅允许注册一个回调观察者**，重复注册返回 `ERR_REPEAT_REGISTER_APP_GROUP_OBSERVER`。

### 4.4 典型端到端流程

**应用使用统计查询（系统应用）：**
```
应用 JS/ArkTS
  → NAPI/Taihe 框架层（参数校验 + 错误码映射）
  → BundleActiveClient（innerkits，samgr 获取 SA 1907 Proxy）
  → IPC Binder
  → BundleActiveService（权限校验）
  → BundleActiveCore（时间漂移检查、用户服务定位）
  → BundleActiveUserService（合并 DB 历史数据 + 内存当前周期数据）
  → 返回结果
```

**事件上报与落盘（系统内部）：**
```
事件源（AppMgr 观察者 / 公共事件 / 电源回调 / 外部 SA ReportEvent）
  → BundleActiveReportController → BundleActiveReportHandler（FFRT 队列串行化）
  → BundleActiveCore::ReportEvent（时间换算、用户服务创建）
  → BundleActiveUserService（更新内存统计）+ BundleActiveGroupController（更新分组态）
  → 触发延迟落盘任务（定时落盘时机详见 [领域术语 - 落盘/刷新](glossary.md#落盘--刷新flush--refresh-to-disk)）
  → RestoreToDatabase → BundleActiveUsageDatabase（写 SQLite）
```

---

## 5. 配置上下文

运行时可配置项通过 `config_policy` 读取 JSON 文件 `etc/device_usage_statistics/device_usage_statistics_config.json`，可由 system / vendor / sys_prod 多级覆盖。可配置项：

| 配置项 | 默认值 | 含义 |
|---|---|---|
| `application_use_periodically.MinUseTimes` | 1 | 使用活跃期判定的每日最小启动次数下限 |
| `application_use_periodically.MaxUseTimes` | 10 | 使用活跃期判定的每日最大启动次数上限 |
| `application_use_periodically.MinUseDays` | 3 | 使用活跃期判定的最少达标天数 |
| `application_use_high_frequency_judge_threshold.MinTotalUseDays` | 4 | 高频时段判定的近一周最少使用天数 |
| `application_use_high_frequency_judge_threshold.MinTopUseHoursLimit` | 6 | 高频时段候选小时数下限 |
| `application_use_high_frequency_judge_threshold.MinHourUseDays` | 4 | 高频时段单小时的最少使用天数 |
| `application_use_high_frequency_judge_threshold.MaxHighFrequencyHourNum` | 3 | 高频时段返回的最大小时数 |
| `MaxDataSize` | 5（MB） | 数据目录大小上限，超限触发事件行清理 |
| `window_visibility_enable` | false | 是否启用窗口可见性精确前台判定 |

> 系统参数 `persist.sys.device_usage_debug_on`（配合 `const.debuggable = 1`）控制 Debug 模式，缩短所有时间阈值。

---

## 相关文档

- [编码规范 §5](coding-standards.md#5-权限校验) — 权限三模式校验逻辑
- [编码规范 §13](coding-standards.md#13-配置读取) — 配置读取实现细节
- [使用事件](entities/usage-event.md) — 事件类型与上报方
- [README_zh.md](../../README_zh.md) / [README.md](../../README.md) — 对外接口清单与使用说明
