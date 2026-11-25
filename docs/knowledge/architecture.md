# 架构原则

本文件阐述本部件的分层结构、核心组件协作、架构设施与非功能设计。设计原则（§1，"为什么这样设计"）与架构不变式（§6，"什么不能变"）分属不同视角，二者不互相重复：§1 叙述设计理念，§6 列举不变式清单。编码级约束（权限/并发/条件编译等）详见 [编码规范](coding-standards.md)。

## 1. 设计原则

1. **分层解耦**：接口层（interfaces）、框架层（frameworks）、服务层（services）、工具层（utils）职责单一，依赖单向向下。业务逻辑归属见架构不变式 [§6](#6-架构不变式)，行为禁止见 [AGENTS.md §2.2](../../AGENTS.md#22-禁止做的事)。
2. **事件驱动**：所有外部状态变更统一转化为内部事件流，经串行化队列处理，保证时序与一致性。
3. **统计态与分组态分离**：使用统计（`BundleActivePeriodStats`）与应用分组（`BundleActivePackageHistory`）是两套独立内存模型，仅通过事件流与 `BundleActiveCore` 协调，互不直接写入。
4. **持久化收口**：数据库访问仅经 `BundleActiveUsageDatabase` facade，上层不感知 RDB 细节。
5. **延迟落盘**：事件先入内存统计，按周期/事件触发批量落盘，避免高频 IO。
6. **配置可覆盖**：阈值与开关通过 `config_policy` 多级配置，不在代码中硬编码可调参数。
7. **安全前置**：每个对外接口在 Service 入口完成权限校验后再进入业务逻辑。

---

## 2. 技术选型

| 维度 | 选型 | 选型理由 |
|---|---|---|
| 实现语言 | C++17 | 系统部件性能与类型安全要求 |
| SA 框架 | SystemAbility + safwk + samgr | OpenHarmony 标准 SA 托管与生命周期管理 |
| IPC | IDL 驱动 Proxy/Stub 生成 | 接口契约集中、序列化一致、向后兼容可控 |
| 持久化 | relational_store（NativeRdb / SQLite） | 轻量嵌入式关系库，支持按时间分表的多库文件管理 |
| 并发 | FFRT（`ffrt::queue` + `ffrt::mutex`） | OpenHarmony 推荐的函数流式运行时，支持任务依赖与延迟调度 |
| 应用接口 | NAPI + Taihe FFI（ArkTS Native） | 兼容旧版 JS 模块与新版 ArkTS Native，Taihe 生成 async/promise/sync 重载 |
| 配置 | cJSON + config_policy | 支持多级（system/vendor/sys_prod）配置覆盖 |
| 日志/埋点 | hilog + hisysevent + hitrace | 统一日志域、关键事件埋点、链路追踪 |

---

## 3. 逻辑架构

### 3.1 分层

```
┌─────────────────────────────────────────────────────────┐
│ 应用层（JS/ArkTS）                                        │
│  @ohos.bundleState  /  @ohos.resourceschedule.usageStatistics  │
└───────────────┬─────────────────────────────────────────┘
                │ NAPI / Taihe FFI
┌───────────────▼─────────────────────────────────────────┐
│ 框架层 frameworks/                                       │
│  参数校验 → 错误码映射 → 结果对象编排                     │
└───────────────┬─────────────────────────────────────────┘
                │ BundleActiveClient（innerkits）
┌───────────────▼─────────────────────────────────────────┐
│ 接口层 interfaces/innerkits                              │
│  IPC Proxy（IDL 生成）+ 数据模型 + 客户端单例 + 死亡重连  │
└───────────────┬─────────────────────────────────────────┘
                │ IPC Binder（SA 1907）
┌───────────────▼─────────────────────────────────────────┐
│ 服务层 services/                                         │
│  common/        — SA 入口、核心编排、持久化、事件源       │
│  packageusage/  — 使用统计（每用户服务、事件追踪、周期统计）│
│  packagegroup/  — 应用分组（状态机、用户历史、分组处理器）  │
└───────────────┬─────────────────────────────────────────┘
                │ NativeRdb
┌───────────────▼─────────────────────────────────────────┐
│ 持久化 /data/service/el1/public/bundle_usage/            │
│  6 个 SQLite 库文件，按时间分表                          │
└─────────────────────────────────────────────────────────┘
```

### 3.2 服务层核心组件

```
                    BundleActiveService（SA 入口、权限、Dump）
                              │ 拥有
                    BundleActiveCore（核心编排器）
            ┌─────────────────┼──────────────────────┐
            │                 │                      │
  BundleActiveReportController  BundleActiveUserService   BundleActiveGroupController
  （单例，持有 ReportHandler）   （每用户一份，统计态）     （单例，分组状态机）
            │                 │                      │
  BundleActiveReportHandler     BundleActiveUsageDatabase  BundleActiveGroupHandler
  （FFRT 队列：上报/落盘）      （每实例一份，DB facade）  （FFRT 队列：分组校验）
                                                         │
                                              BundleActiveUserHistory
                                              （分组态内存 + DurationLog）
```

**事件源**（均经 `BundleActiveReportController` 投递到同一队列）：
- `BundleActiveAppStateObserver`（AppMgr 前后台）
- `BundleActiveContinuousTaskObserver`（长时任务）
- `BundleActiveWindowVisibilityManager` + `BundleActiveVisibilityChangedListener`（窗口可见性）
- `BundleActiveCommonEventSubscriber`（亮灭屏/用户/包/锁屏公共事件）
- `BundleActivePowerStateCallbackService`（休眠/唤醒）
- `BundleActiveShutdownCallbackService`（关机）
- 外部 SA 经 IPC `ReportEvent`

### 3.3 接口层与框架层

- **innerkits**（`libusagestatsinner.z.so`）：`BundleActiveClient` 单例封装 SA 1907 的 IPC Proxy，附带死亡重连（服务死后 5 秒重试并自动重注册分组回调）。导出 8 个数据模型头文件。
- **NAPI 旧版**（`@ohos.bundleState`，`bundlestate.z.so`）：6 个查询方法 + `IntervalType` 枚举，结果字段沿用旧命名（`stateType` / `stateOccurredTime`）。
- **NAPI 新版**（`@ohos.resourceschedule.usageStatistics`，`usagestatistics.z.so`）：16 个方法（含 sync 变体）+ `IntervalType` / `GroupType` 枚举，结果字段新命名（`eventId` / `eventOccurredTime`）。
- **Taihe FFI**（`usageStatistics_taihe_native.z.so` + boot ABC）：ArkTS Native 接口，IDL 自动生成 async/promise/sync 重载，结构化类型（`Optional` / `Map` / `Array`），随系统启动加载。
- **CLI 工具**（`ohos-usageStatsQuery`）：链接 innerkits，11 个子命令，输出 JSON。

---

## 4. 架构设施

### 4.1 事件处理管线

事件经唯一入口 `BundleActiveService::ReportEvent`（oneway IPC）进入 → `BundleActiveReportHandler`（FFRT 队列串行化）→ `BundleActiveCore::ReportEvent`（时间换算 + 用户服务定位）→ 分发至：
- `BundleActiveUserService::ReportEvent`（更新四区间内存统计：日/周/月/年）
- `BundleActiveGroupController::ReportEvent`（更新分组态，触发延迟分组校验）

**消息码**（ReportHandler）：`MSG_REPORT_EVENT`、`MSG_FLUSH_TO_DISK`、`MSG_REMOVE_USER`、`MSG_BUNDLE_UNINSTALLED`、`MSG_SWITCH_USER`、`MSG_BUNDLE_INSTALLED`。

### 4.2 分组状态机

分组由**事件驱动提升**与**超时驱动降级**双机制维护：

- **提升**：匹配事件到来时立即提升分组并设置超时时间戳（前台/交互/长时任务→ALIVE、通知查看→DAILY、系统交互→ALIVE）。具体超时值与触发事件映射详见 [应用优先级分组 - 分组判定机制](entities/app-group.md#分组判定机制)。
- **降级**：延迟消息到期或周期检查时，基于双维度（累计启动时长差 + 累计亮屏时长差）查表得出新分组；若仍在超时窗口内则钳制回 ALIVE/DAILY。双维度阈值详见 [应用优先级分组](entities/app-group.md#分组判定机制)。
- **强制设置**：`SetAppGroup` 以 `FORCED` 原因设置后冻结自动调整，直至再次被设置。
- **回调**：分组实际变化时经 `BundleActiveCore::OnAppGroupChanged` 发 hisysevent 并 fan-out 到所有注册观察者。

分组检查消息按 `eventId_userId_uid_bundleName_delayTime` 键去重，保证同一 bundle 的待办检查只保留最新一次。

### 4.3 数据库分表与版本演进

6 个 SQLite 库文件（`daily/weekly/monthly/yearly/event/usageGroup`），统计表按周期时间戳分表（`PackageLog<ts>`、`DeviceEventLog<ts>`、`ModuleRecordLog<ts>`、`FormRecordLog<ts>`），通过二分查找定位目标表。

保留策略：日/周/月/年表分别上限 30/30/12/10 张，事件表 7 天滚动（DEBUG 6×10 分钟）。库版本 `BUNDLE_ACTIVE_CURRENT_VERSION = 3`，升级路径在 `UpgradeDatabase` 实现（V1→V3 增加 `uid` 与 `firstUseTime` 列并回填；V2→V3 增加 `firstUseTime`）。

### 4.4 时间漂移处理

`BundleActiveCore::CheckTimeChangeAndGetWallTime` 比较实际墙钟时间与预期外推时间，漂移超 `TIME_CHANGE_THRESHOLD_MILLIS`（10 分钟）时触发：全量落盘 → `RenewTableTime`（按时间差重命名所有时表）→ 重新加载统计 → 重置时间基准。

### 4.5 用户切换与卸载

- 用户切换：前一用户落盘 + `OnStatsChanged` 调度，新用户延迟触发统计变更。
- 用户删除：`OnUserRemoved` 清理该用户所有数据。
- 包卸载：5 秒延迟后清理卸载 bundle 的所有表行（应用分身仅清理对应 uid）；卸载期间的该 uid 事件被丢弃。

---

## 5. 非功能设计

### 5.1 性能

- 事件处理异步化：`ReportEvent` 为 oneway IPC，重操作经 FFRT 队列异步执行，不阻塞调用方。
- 内存统计优先：查询合并 DB 历史与内存当前周期，减少 DB 命中。
- 字符串驻留：`BundleActivePeriodStats` 内 `packetNamesCache_` 做字符串 intern，降低重复 bundle 名内存占用。
- 数据目录大小监控：`BundleActiveEventReporter` 每 24 小时上报各库文件大小到 hisysevent；超 `MaxDataSize` 时按天递增清理事件行。

### 5.2 可靠性

- SA 死亡重连：`BundleActiveClient` 注册死亡Recipient，服务死后 5 秒重试获取 Proxy 并自动重注册分组回调。
- 系统服务就绪等待：`InitNecessaryState` 以 2 秒间隔轮询 7 个依赖 SA 就绪后才初始化。
- 关机刷新：`ShutdownCallback` 触发全量落盘。
- hicollie 看门狗：链接 `libhicollie` 监控进程健康。

### 5.3 安全

- 权限三模式校验（见编码规范）。
- 构建加固：PAC 返回地址保护 + CFI（含跨 DSO）+ 栈保护 + 符号裁剪。
- 数据目录安全等级 S1。
- Dump 双重门禁（ENG 构建 + DUMP 权限）。

### 5.4 可测试性

- 静态库测试目标 `usagestatservice_static`（`-DDEVICE_USAGE_UNIT_TEST`）+ `-Dprivate=public -Dprotected=public` 暴露私有成员。
- `mock/` 下 `WEAK_FUNC` 替身替换外部依赖（AccountHelper、Client、Core、Database、ServiceRegistry）。
- Debug 模式大幅缩短时间阈值，支持测试快速验证跨日/分组降级场景。
- Fuzz 测试覆盖事件反序列化、客户端 Proxy、服务端 Stub、配置解析、并行化路径。

---

## 6. 架构不变式

以下为本部件的系统级不变式（"什么不能变"）。行为级禁止（"Agent 不得做 X"）见 `AGENTS.md` §2.2，二者视角不同、不互相替代。

1. 业务逻辑仅在 `services/` 内；`frameworks/`、`interfaces/` 不得包含业务判断。
2. 内部头文件不向 `interfaces/innerkits/include` 之外暴露；innerapi 符号由 versionscript 收口。
3. 分组态与统计态内存模型不互相直接写入。
4. 不在 `ReportEvent` 链路执行同步阻塞 DB 查询。
5. IPC 接口码枚举值不可变更，新增必须追加。
6. 数据模型 Parcel 字段顺序不可调整，新增必须末尾追加。

> 条件编译守卫拼写不变式（`POWERMANGER` 历史拼写不可修正）详见 [编码规范 §9](coding-standards.md#9-条件编译)。

---

## 相关文档

- [编码规范](coding-standards.md) — 权限/并发/条件编译/构建加固等编码级约束
- [应用优先级分组](entities/app-group.md) — 分组判定机制与阈值（SSOT）
- [使用事件](entities/usage-event.md) — 事件类型与处理管线
- [应用使用统计](entities/package-usage-stats.md) — 区间类型与聚合规则（SSOT）
