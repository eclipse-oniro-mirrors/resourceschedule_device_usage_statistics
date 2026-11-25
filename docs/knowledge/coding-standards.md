# 编码规范

本文件仅收录本项目**特有**的编码规则与约定。业界通用的 C++ 编码规范、命名风格、安全实践不再赘述。

---

## 1. 命名空间与命名

- 所有代码归属命名空间 `OHOS::DeviceUsageStats`；分组相关常量归子命名空间 `DeviceUsageStatsGroupConst`。
- 类名统一以 `BundleActive` 前缀（如 `BundleActiveCore`、`BundleActiveUserService`）；NAPI 框架层例外（`BundleState*`、`UsageStatistics*`）。
- 头文件与源文件命名：`bundle_active_*.h` / `bundle_active_*.cpp`，全小写下划线。
- 常量命名：全局常量全大写下划线（`THIRTY_MINUTE`）；`inline const std::string` 常量亦同。
- 成员变量后缀下划线（`bundleName_`、`userStatServices_`）。

---

## 2. 日志

- **必须**使用项目日志宏：`BUNDLE_ACTIVE_LOGD / LOGI / LOGW / LOGE / LOGF`，禁止直接调用 `HILOG_*`。
- 日志 domain 固定 `0xD001710`，tag 固定 `BUNDLE_ACTIVE`（由 `bundle_active_log.h` 定义，不可修改）。
- 日志格式串必须使用 hilog 隐私占位符：`%{public}s` 标注可公开字段，`%{private}s` 标注敏感字段（如完整 bundle 列表）。
- 日志宏自动附加 `[function:line]` 与文件名，调用时无需手写位置信息。
- 日志级别可通过 `BundleActiveLog::SetLogLevel` 运行时调整，默认 `DEBUG`。

---

## 3. 时间基准

本部件存在三套时间度量，**严禁混用**：

| 时间 | 来源 | 用途 |
|---|---|---|
| 启动基时间（boot-based） | `TimeServiceClient::GetBootTimeMs()` | 事件上报时打点；累计启动时长用于分组判活 |
| 墙钟时间（wall time） | `TimeServiceClient` 或 `BundleActiveUtil::GetSystemTimeMs()` | 事件入库、查询时间窗口、统计周期起止 |
| 累计亮屏时长 | `BundleActiveUserHistory` 维护 | 分组判活第二维度 |

约定：
- 事件进入 `ReportEvent` 时**立即**取启动基时间打点，由 `BundleActiveCore::ConvertToSystemTimeLocked` 转墙钟时间后入库。
- 分组判活（`GetLevelIndex`）使用的"距上次使用时长"是**累计启动时长差**与**累计亮屏时长差**，非墙钟时间差。
- `GetFFRTDelayTime(ms)` 将毫秒转为微秒供 FFRT delay API 使用。

---

## 4. 并发与异步

- **必须**使用 FFRT，禁止 `std::mutex` / `std::thread` / `std::async`。
- 共享状态互斥使用 `ffrt::mutex`（普通）或 `ffrt::recursive_mutex`（需重入的场景）。
- 两个专用 FFRT 队列：
  - `DeviceUsageReportHandleQueue`（`BundleActiveReportHandler`）：事件上报、落盘、用户切换。
  - `DeviceUsageGroupHandleQueue`（`BundleActiveGroupHandler`）：分组校验、空闲检查。
- 一次性延迟任务用 `ffrt::submit(func, ffrt::task_attr().delay(us))`。
- `BundleActiveReportHandler` / `BundleActiveGroupHandler` 采用"消息 ID + 延迟"模式，相同消息 ID 重复投递会追加到队列；分组检查消息按 `eventId_userId_uid_bundleName_delayTime` 键去重（后投递替换前者）。

### 锁使用约定

核心互斥量及其保护范围：

| 互斥量 | 类型 | 所属类 | 保护范围 |
|---|---|---|---|
| `mutex_` | `ffrt::mutex` | `BundleActiveCore` | 核心编排：时间换算、用户服务定位、ReportEvent 链路 |
| `callbackMutex_` | `ffrt::recursive_mutex` | `BundleActiveCore` | 分组回调观察者列表（需重入） |
| `statsMutex_` | `ffrt::recursive_mutex` | `BundleActiveUserService` | 单用户统计态内存（事件处理与查询时持有，需重入） |
| `mutex_` / `initMutex_` | `ffrt::mutex` | `BundleActiveGroupController` | 分组态初始化与状态更新 |
| `userHistoryMutex_` / `setGroupMutex_` | `ffrt::mutex` | `BundleActiveUserHistory` | 分组历史（DurationLog）与强制设置 |
| `databaseMutex_` | `ffrt::mutex` | `BundleActiveUsageDatabase` | DB 内部自锁，上层无需额外加锁 |

锁层级（不得反向嵌套）：`Core > UserService / GroupController > Database`。查询路径可与上报路径并发，各自持有 `statsMutex_` 时不阻塞对方。

---

## 5. 权限校验

每个对外 IPC 方法**必须在进入业务逻辑前**完成权限校验，三选一：

| 模式 | 适用方法 | 校验逻辑 |
|---|---|---|
| Native + 进程名 | `ReportEvent` | `CheckNativePermission`（TOKEN_NATIVE / TOKEN_SHELL + BUNDLE_ACTIVE_INFO）且进程名为 foundation / subfoundation / resource_schedule_service |
| 仅 Native | `IsBundleUsePeriod` | `CheckNativePermission` |
| 系统应用或 Native | 其余查询/设置 | HAP → `CheckBundleIsSystemAppAndHasPermission`（系统应用 + BUNDLE_ACTIVE_INFO）；Native/Shell → `CheckNativePermission` |

附加规则：
- `SetAppGroup` 禁止应用设置自身分组（`localBundleName == bundleName` 时返回 `ERR_PERMISSION_DENIED`）。
- 查询自身数据（bundleName 为空或等于调用方）时调用方须为系统应用。
- Dump 需 `const.debuggable = 1` 且 `ohos.permission.DUMP`。
- 框架层（NAPI/Taihe）**禁止**做权限判断，只做参数校验与错误码映射。

---

## 6. 错误码

错误码布局：`Reserved(12bit) | Subsystem(8bit) | Module(8bit) | Code(4bit)`，模块常量 `DEVICE_USAGE_STATS_MODULE_COMMON = 0x01`，子系统偏移基址 `ErrCodeOffset(SUBSYS_IAWARE, 0x01)`。

JS 侧错误码（定义于 `bundle_state_inner_errors.h`）：

| 错误码 | 含义 |
|---|---|
| 201 | 权限拒绝（`ERR_PERMISSION_DENIED`） |
| 202 | 非系统应用（`ERR_NOT_SYSTEM_APP`） |
| 401 | 参数错误（`ERR_PARAM_ERROR`） |
| 401001 | 参数数量错误（`ERR_PARAMETERS_NUMBER`） |
| 401002 ~ 401004 | 回调类型 / 参数空 / bundleName 类型校验 |
| 401005 ~ 401008 | 时间参数校验（beginTime / endTime 类型与范围） |
| 401009 ~ 401010 | 区间参数校验（byInterval 类型与范围） |
| 401011 ~ 401012 | 分组参数校验（newGroup 类型与范围） |
| 401013 ~ 401014 | 数量参数校验（maxNum 类型与范围） |
| 401015 ~ 401019 | 回调观察者校验（回调类型 / 重复注册 / 观察者空 / 异步回调） |
| 10000001 ~ 10000008 | 内存/系统服务异常 |
| 1000000301 ~ 1000000605 | SA 获取/查询失败等 |
| 10100001 | 分组操作重复（`ERR_APPLICATION_GROUP_OPERATION_REPEATED`） |
| 10100002 | 数据库无分组信息（`ERR_NO_APP_GROUP_INFO_IN_DATABASE`） |
| 1010000101 | 回调重复注册/注销 |

约定：
- 参数校验错误统一走 `BundleStateCommon::HandleParamErr` / `HandleNewParamErr`，抛出 401 或 10000008（`ERR_NEW_PARAM_ERROR`）。
- SA 侧错误经 `GetReflectErrCode`（`errCode / 100`）映射到 JS 错误码。
- 新增接口必须在 `bundle_state_inner_errors.h` 与 Taihe 的 `bundle_state_idl_err_code.h` 同步登记错误码与文案。

---

## 7. 数据模型序列化

- 所有跨 IPC 传递的数据模型实现 `Parcelable`（`Marshalling` / `Unmarshalling`），并登记为 IDL `sequenceable`。
- `Marshalling` / `Unmarshalling` 字段**顺序必须完全一致**；新增字段必须追加在末尾，禁止调整已有字段顺序（向后兼容）。
- 同步更新 NAPI 结果映射（`BundleStateCommon::Get*ForResult`）与 Taihe IDL 结构体（`*.taihe`）。

---

## 8. 持久化

- 所有数据库操作必须经 `BundleActiveUsageDatabase`，禁止其它层级直接持有 `NativeRdb::RdbStore`。
- 数据库 schema 变更必须：
  1. 在 `bundle_active_constant.h` 增补列名/列索引常量。
  2. 在 `BundleActiveUsageDatabase` 的建表语句中追加列。
  3. 在 `bundle_active_usage_database_upgrade.cpp` 的 `UpgradeDatabase` 中编写旧版本升级路径。
  4. 递增 `BUNDLE_ACTIVE_CURRENT_VERSION`。
- 日志模式：产品环境 `MODE_OFF`，测试（`DEVICE_USAGES_STATISTICS_TEST_ENABLE`）`MODE_WAL`。

---

## 9. 条件编译

必须严格包裹以下守卫，且拼写与历史保持一致：

| 宏 | 守护范围 | 由何启用 |
|---|---|---|
| `DEVICE_USAGES_STATISTICS_POWERMANGER_ENABLE` | 电源管理相关（PowerStateCallback、ShutdownCallback） | `powermgr_power_manager` 部件存在 |
| `BGTASKMGR_ENABLE` | 长时任务观察者 | `resourceschedule_background_task_mgr` 部件存在 |
| `OS_ACCOUNT_PART_ENABLED` | 多账号相关 | `account_os_account` 部件存在 |
| `DEVICE_USAGES_STATISTICS_TEST_ENABLE` | fuzzer WAL 日志模式 | `use_libfuzzer`（仅静态库测试目标） |
| `DEVICE_USAGE_UNIT_TEST` | 测试静态库（暴露私有成员） | `usagestatservice_static` / `usagestatsinner_static` |

> `POWERMANGER` 是历史拼写（应为 `POWERMANAGER`），**禁止"修正"**，否则破坏构建。

---

## 10. 构建加固

所有 shared / static library 目标必须配置：
- `branch_protector_ret = "pac_ret"`
- `sanitize { cfi = true; cfi_cross_dso = true; debug = false }`
- `cflags_cc` 含 `-fstack-protector-strong`
- NAPI / Service 库额外含 `-fdata-sections -ffunction-sections -fvisibility=hidden -Os`

innerapi 库必须配置 `version_script` 收口导出符号：
- `usagestatsinner` → `libusagestatsinner.versionscript`
- `usagestatsutils` → `libusagestatsutils.versionscript`
- Taihe native → `usageStatistics_taihe_native.versionscript`

新增对外符号必须同步加入对应 `versionscript` 的 global 段，否则链接时被裁剪。

---

## 11. IPC 接口码

- `ibundle_active_service_ipc_interface_code.h` 与 `iapp_group_callback_ipc_interface_code.h` 中**已有枚举值编号不可变更**（向后兼容）。
- 新增 IPC code 必须追加到枚举末尾。
- 这两个文件修改需 CODEOWNERS 评审（见仓库 `CODEOWNERS`）。

---

## 12. 调试与 Dump

- Debug 模式由 `bundle_active_debug_mode.cpp` 的 `extern const bool DEBUG_ON` 控制，需同时满足 `const.debuggable = 1` 与 `persist.sys.device_usage_debug_on`。
- Dump 子命令：`Events` / `PackageUsage` / `ModuleUsage` / `HighFreqHourUsage`，参数数量固定校验。
- 测试代码不得引入产品头文件；通过 `usagestatservice_static`（`-DDEVICE_USAGE_UNIT_TEST`）+ `mock/` 下 `WEAK_FUNC` 替身访问私有成员。

---

## 13. 配置读取

- 运行时配置经 `config_policy` 的 `GetCfgFiles` 读取 `etc/device_usage_statistics/device_usage_statistics_config.json`，支持 system / vendor / sys_prod 多级覆盖，逐文件合并（后者覆盖前者）。
- 配置缺失或非法值时使用默认值，禁止因配置异常导致服务崩溃。
- 新增配置项必须同步更新 `BundleActiveConfigReader` 的结构与解析逻辑，并在 `test/resource/deviceUsage/parseconfig/` 增补样例。

---

## 14. 新增对外接口端到端步骤

新增一个对外查询/设置接口时，按以下顺序完成全链路（各步骤的详细约束见对应章节）：

1. **IDL 定义**：在 `IBundleActiveService.idl` 增加方法声明 → 由 IDL 工具重新生成 Proxy/Stub。
2. **IPC 接口码**：`ibundle_active_service_ipc_interface_code.h` 枚举末尾追加新编号（不改已有编号，见 §11）。
3. **Service 实现**：`BundleActiveService` 增加方法，入口完成权限校验（见 §5），业务逻辑在 `services/` 内。
4. **数据模型**：若新增 sequenceable，`Marshalling`/`Unmarshalling` 字段末尾追加（见 §7），同步 Taihe IDL 结构体。
5. **innerkits 客户端**：`BundleActiveClient` 增加对应方法，更新 `libusagestatsinner.versionscript` 的 global 段（见 §10）。
6. **NAPI / Taihe 层**：参数校验 + 错误码映射（**不做权限判断**），更新对应 versionscript。
7. **错误码**：`bundle_state_inner_errors.h` + Taihe 的 `bundle_state_idl_err_code.h` 同步登记错误码与文案（见 §6）。
8. **测试**：`test/unittest` 增补用例；评估 `test/fuzztest` 是否需新增 fuzz 目标。
9. **验证**：对照 `AGENTS.md` §3.3 验证清单逐项确认。

---

## 相关文档

- [架构原则](architecture.md) — 分层结构与架构不变式
- [entities/](entities/) — 数据模型字段定义（序列化与 Parcel 字段）

---

## 15. 实体文档结构模板

所有 `entities/*.md` 遵循以下骨架。无对应内容的节可省略，但**顺序与命名须一致**：

1. `## 概念` — 实体定义与职责
2. `## 关键属性` — 数据结构字段表（记录型实体）
   `## 关键指标` — 聚合指标表（统计型实体：package-usage-stats / device-event-stats）
3. `## [领域专有节]` — 按实体需要命名（如事件类型表、区间类型表、判定算法）
4. `## 使用方式` — 查询/设置/采集接口 + 权限要求
5. `## 处理机制` — 统一命名（记录型实体）；状态机/算法型实体可保留专有名（如"分组判定机制""判定算法"）
6. `## 持久化` — 库文件、表名、列、保留策略；不持久化的实体改为 `## 数据来源`

每篇实体文档末尾附 `## 相关文档` 链接块，指向关联实体与主题文档。
