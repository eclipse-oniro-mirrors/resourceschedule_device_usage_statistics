# AGENTS.md — 设备使用信息统计（device_usage_statistics）

本文件是本仓库的项目宪法，面向在仓库内进行开发、维护、修复缺陷的 AI 编码代理与人类开发者。它定义项目定位、必须遵守的约束、构建验证方式，以及知识库路由。所有约定均来源于代码实际实现，未经验证的内容不会出现在此处。

本文件聚焦**行为约束与路由**；详细表格、阈值、命令清单由 `docs/knowledge/` 下各主题文档作为单一源（SSOT）维护。约束后标注的路由链接指向详情。

---

## 1. 项目定位

### 1.1 做什么

本部件为 OpenHarmony 标准系统提供**设备使用信息统计**能力，归属于 `resourceschedule` 子系统。核心职责：

- **应用使用统计（App Usage）**：采集并查询应用前台使用时长、启动次数、连续运行后台任务时长等，支持按日/周/月/年区间聚合。
- **应用分组（App Group）**：基于使用活跃度将应用划分为 ALIVE / DAILY / FIXED / RARE / LIMIT / NEVER 六级优先级分组，并支持外部强制设置、分组变化回调。
- **系统事件与通知统计**：统计休眠、唤醒、锁屏、解锁等系统事件以及应用通知次数。
- **模块与卡片使用记录**：记录 FA/模块启动次数、卡片点击/移除记录。
- **高频使用时段查询**：基于最近一周事件推断应用的高频使用小时集合。
- **事件上报服务**：作为 SA（System Ability）接收来自 `foundation` / `subfoundation` / `resource_schedule_service` 进程的使用事件上报，缓存后定时落盘。

部件对外暴露两套 SysCap：
- `SystemCapability.ResourceSchedule.UsageStatistics.App`
- `SystemCapability.ResourceSchedule.UsageStatistics.AppGroup`

### 1.2 技术栈

| 维度 | 选型 |
|---|---|
| 实现语言 | C++（C++17 及以上） |
| 系统能力框架 | OpenHarmony SystemAbility（samgr 注册、safwk 托管）、IDL 生成 Proxy/Stub |
| 持久化 | relational_store（NativeRdb，底层 SQLite），按时间分表的多库文件 |
| 并发运行时 | FFRT（函数流式运行时），`ffrt::queue` + `ffrt::mutex` |
| 应用侧接口 | NAPI（`@ohos.bundleState`、`@ohos.resourceschedule.usageStatistics`）+ Taihe FFI（ArkTS Native，含 boot ABC） |
| 跨进程通信 | IPC/Binder，IDL 文件驱动序列化 |
| 日志与埋点 | hilog（domain `0xD001710`、tag `BUNDLE_ACTIVE`）、hisysevent、hitrace |
| 构建系统 | GN + bundle.json 部件化描述 |
| 目标系统形态 | standard（标准系统） |

### 1.3 运行身份

- **SA ID**：`1907`（`DEVICE_USAGE_STATISTICS_SYS_ABILITY_ID`）
- **宿主进程**：`resource_schedule_service`（由 `sa_profile/1907.json` 声明）
- **服务库**：`libusagestatservice.z.so`，`run-on-create = true`
- **启动时机**：init `post-fs-data` 阶段（见 `init/device_usage_statistics_service.cfg`）
- **数据目录**：`/data/service/el1/public/bundle_usage/`（安全等级 S1）

---

## 2. 项目宪法

### 2.1 必须遵守的约束

1. **安全门禁前置**：每一个对外 IPC 方法（`BundleActiveService` 的所有接口）必须在进入业务逻辑前完成权限校验。三模式校验（ReportEvent 进程名限定 / IsBundleUsePeriod 仅 Native / 其余系统应用或 Native）详见 [编码规范 §5](docs/knowledge/coding-standards.md#5-权限校验)。`SetAppGroup` 禁止应用设置自身分组；Dump 需 `const.debuggable = 1` 且 `ohos.permission.DUMP`。**禁止**新增任何绕过校验的快捷路径。

2. **事件上报入口唯一**：所有外部事件必须经由 `BundleActiveService::ReportEvent`（oneway IPC）进入，由 `BundleActiveReportHandler` 串行化处理后再分发到 `BundleActiveCore` 与 `BundleActiveGroupController`。进程内事件源同样经 `BundleActiveReportController` 投递到同一个处理队列。**禁止**直接调用 `BundleActiveCore::ReportEvent` 或 `BundleActiveUserService` 绕过队列。

3. **持久化层收口**：所有数据库读写必须经过 `BundleActiveUsageDatabase` facade。**禁止**在其它层级直接操作 RDB。详细规则与 schema 变更流程见 [编码规范 §8](docs/knowledge/coding-standards.md#8-持久化)。

4. **时间基准不混用**：三套时间度量（启动基时间 / 墙钟时间 / 累计亮屏时长）各有用途，**严禁混用**。详见 [编码规范 §3](docs/knowledge/coding-standards.md#3-时间基准) 与 [领域术语](docs/knowledge/glossary.md)。

5. **并发模型用 FFRT**：异步任务一律使用 FFRT，两个核心队列（`DeviceUsageReportHandleQueue` / `DeviceUsageGroupHandleQueue`）。共享状态用 `ffrt::mutex` / `ffrt::recursive_mutex`。**禁止**使用 `std::mutex` 或裸线程。锁层级与线程安全详见 [编码规范 §4](docs/knowledge/coding-standards.md#4-并发与异步)。

6. **构建加固必选**：所有 shared/static library 目标必须开启 PAC 返回保护 + CFI（含跨 DSO）+ 栈保护；NAPI/Service 库还需符号裁剪。具体 flag 清单见 [编码规范 §10](docs/knowledge/coding-standards.md#10-构建加固)。

7. **条件编译守卫**：电源/后台任务/多账号相关代码必须包裹在对应宏中，**守卫拼写不可"修正"**（如 `POWERMANGER` 是历史拼写）。完整守卫表见 [编码规范 §9](docs/knowledge/coding-standards.md#9-条件编译)。

8. **数据滚动清理**：事件表按 7 天滚动清理，日/周/月/年统计表按上限分表保留。超 `MaxDataSize` 时按天递增清理。保留策略详见 [架构 §4.3](docs/knowledge/architecture.md#43-数据库分表与版本演进)。

### 2.2 禁止做的事

1. **禁止**在 `services/` 之外编写业务逻辑；`frameworks/`、`interfaces/` 只负责接口适配与数据模型编排。
2. **禁止**向 `interfaces/innerkits/include` 之外的模块暴露内部头文件；innerapi 版本符号由 versionscript 收口。
3. **禁止**在 NAPI/Taihe 层做权限判断或时间换算；这些职责在 Service 层完成，框架层仅做参数校验与结果映射。
4. **禁止**让分组态（`BundleActivePackageHistory`）与统计态（`BundleActivePeriodStats`）的内存模型互相直接写入；二者仅通过事件流与 `BundleActiveCore` 协调。
5. **禁止**为绕过测试而暴露内部接口；测试通过 `usagestatservice_static` + `WEAK_FUNC` mock 与 `-Dprivate=public` 访问私有成员，不得在产品头文件中开放。
6. **禁止**修改 `ibundle_active_service_ipc_interface_code.h` 与 `iapp_group_callback_ipc_interface_code.h` 中已有枚举值的编号（向后兼容）；新增 IPC code 必须追加。这两个文件修改需 CODEOWNERS 评审。
7. **禁止**在 `ReportEvent` 链路中执行同步阻塞的数据库查询；所有重操作必须经由 FFRT 队列异步化。

> 架构级不变式（"什么不能变"的系统保证）见 [架构 §6](docs/knowledge/architecture.md#6-架构不变式)，与上述行为禁止（"Agent 不得做"）视角不同、互不替代。

### 2.3 常见陷阱（Agent 高频出错模式）

| 陷阱 | 后果 | 正确做法 |
|---|---|---|
| "修正" POWERMANGER 拼写 | 构建断裂 | 保持历史拼写，见 [编码规范 §9](docs/knowledge/coding-standards.md#9-条件编译) |
| 在 ReportEvent 链路同步查 DB | 阻塞 oneway IPC 调用方 | 重操作入 FFRT 队列异步化 |
| 混用墙钟时间与启动时间 | 分组判活错误 | 打点用 `GetBootTimeMs`，入库转墙钟，判活用累计启动时长差 |
| 新增导出符号未加 versionscript | 链接时被裁剪，运行时符号丢失 | 同步更新对应 `.versionscript` 的 global 段 |
| 改 IPC 枚举编号 | 向后不兼容，线上设备升级异常 | 仅追加新编号，不改已有值 |
| 在 NAPI/Taihe 层做权限判断 | 绕过 Service 门禁 | 框架层只做参数校验，权限在 Service 层 |
| 调整 Parcel 字段顺序 | 旧客户端反序列化错乱 | 新增字段末尾追加 |
| 直接操作 RdbStore 绕过 Database facade | 持久化逻辑分散 | 一律经 `BundleActiveUsageDatabase` |
| 新增事件未登记 `eventIdMatchReason_` | 分组不响应新事件 | 同步更新 `BundleActiveGroupController` 映射表 |
| 用 `std::mutex` 替代 `ffrt::mutex` | 破坏并发模型一致性 | 一律用 FFRT 同步原语 |

---

## 3. 构建验证

### 3.1 编译

本部件使用 GN 构建，部件元数据在 `bundle.json`，构建脚本入口为根目录 `BUILD.gn`，可配置项在 `device_usage_statistics.gni`。

主要构建目标：

| 目标 | 产物 | 说明 |
|---|---|---|
| `usagestatservice` | `libusagestatservice.z.so` | SA 1907 服务实现（在 `resource_schedule_service` 进程内加载） |
| `usagestatsinner` | `libusagestatsinner.z.so` | innerkits 客户端 + IDL 生成 Proxy/Stub（platformsdk innerapi） |
| `usagestatsutils` | `libusagestatsutils.z.so` | 日志与时间/字符串工具（platformsdk innerapi） |
| `bundlestate` | `module/bundlestate.z.so` | 旧版 NAPI 模块 `@ohos.bundleState` |
| `usagestatistics` | `module/resourceschedule/usagestatistics.z.so` | 新版 NAPI 模块 `@ohos.resourceschedule.usageStatistics` |
| `usageStatistics_taihe_native` + `usage_statistics_abc` | native lib + boot ABC | Taihe FFI 模块（ArkTS Native） |
| `ohos-usageStatsQuery` | `/system/bin/cli_tool/executable/ohos-usageStatsQuery` | CLI 查询工具 |
| `device_usage_statistics_sa_profile` | `1907.json` | samgr SA profile |
| `device_usage_statistics_service_init` | `init/device_usage_statistics_service.cfg` | init 配置 |

构建分组：
- `bfwk_group_all`：框架与 innerkits（`bundlestate` + `usagestatsinner` + `usagestatsutils`）+ Taihe kit
- `service_group_all`：服务侧（SA profile + init cfg + `usagestatistics` + `usagestatservice`）+ CLI 工具

特性开关（`device_usage_statistics.gni`）：
- `device_usage_statistics_device_enable`（默认 true）：总开关，关闭后不产出任何目标。
- `os_account_part_enabled`：随 `account_os_account` 部件存在性自动判定。
- `device_usage_statistics_with_powermgr_power_manager_enable`：随 `powermgr_power_manager` 部件存在性自动判定。
- `bgtaskmgr_enable`：随 `resourceschedule_background_task_mgr` 部件存在性自动判定。

#### 独立验证命令

本部件在全量构建中由 OpenHarmony 构建系统集成编译。如需独立增量验证：

```bash
# 增量构建单个目标（在 OpenHarmony 源码根目录）
./build.sh --part-name device_usage_statistics --build-target usagestatservice
./build.sh --part-name device_usage_statistics --build-target usagestatsinner

# 构建并运行单元测试
./build.sh --part-name device_usage_statistics --build-target test_all --gn-args is_debug=true

# CFI/栈保护在编译期自动启用，构建失败即说明加固配置被破坏
```

### 3.2 测试

测试目标统一聚合在 `BUILD.gn` 的 `group("test_all")`（`testonly = true`），包含三类：

| 类型 | 路径 | 说明 |
|---|---|---|
| C++ 单元测试 | `test/unittest/` | 7 个 GTest 可执行目标，依赖 `usagestatservice_static`（带 `-DDEVICE_USAGE_UNIT_TEST`），使用 `-Dprivate=public -Dprotected=public` 暴露私有成员，配合 `mock/` 下 `WEAK_FUNC` 替身 |
| Fuzz 测试 | `test/fuzztest/` | 9 个 LibFuzzer 目标，依赖 `usagestatservice_static`（`use_libfuzzer` 时追加 `-DDEVICE_USAGES_STATISTICS_TEST_ENABLE`，启用 WAL 日志） |
| JS 单元测试 | `interfaces/test/unittest/device_usage_statistics_jsunittest/` | 同时验证 `@ohos.bundleState` 与 `@ohos.resourceschedule.usageStatistics`，HAP 配置声明 `ohos.permission.BUNDLE_ACTIVE_INFO` |

测试资源：
- `test/resource/deviceUsage/parseconfig/*.json`：配置文件样例（含异常用例），由 `ohos_test.xml` / `deviceUsageConfig.xml` 推送到 `/sys_prod/etc/device_usage_statistics/` 或 `/system/variant/phone/base/etc/device_usage_statistics/`。

### 3.3 验证清单

修改代码后，提交前请逐项确认：

- [ ] 若改动了 IPC 接口（IDL / interface code 枚举），确认 Proxy/Stub 由 IDL 工具重新生成，且未改变已有枚举编号。
- [ ] 若改动了对外 API（NAPI / Taihe / innerkits），确认 `versionscript` 已覆盖新增导出符号，且未删除既有导出。
- [ ] 若改动了数据模型（`BundleActiveEvent` 等 sequenceable），确认 Parcel 读写字段顺序一致，并同步更新 Taihe IDL 结构体。
- [ ] 若新增了事件类型，确认 `BundleActiveGroupController::eventIdMatchReason_` 与分组链路同步登记。
- [ ] 若新增了对外接口，确认已在 `BundleActiveService` 完成权限校验，并在 NAPI/Taihe 层补齐参数校验与错误码（端到端步骤见 [编码规范 §14](docs/knowledge/coding-standards.md#14-新增对外接口端到端步骤)）。
- [ ] 若改动了数据库 schema，确认 `UpgradeDatabase` 升级路径覆盖旧版本，并更新 `BUNDLE_ACTIVE_CURRENT_VERSION`。
- [ ] 运行 `test/unittest` 与 `test/fuzztest` 相关目标，确保无回归。

### 3.4 回滚与容错

- **构建失败**：优先检查条件编译守卫拼写、versionscript 覆盖、加固 flag 配置；不要通过删除加固配置来"修复"构建。
- **测试回归**：对照 §2.3 陷阱表逐项排查；分组相关回归检查双维度阈值与 `eventIdMatchReason_`。
- **无法运行验证**：在 PR 描述中注明未验证项与原因，由 CODEOWNERS 在 CI 补验；不得标注"已验证"。
- **schema 变更回退**：数据库升级不可逆（`UpgradeDatabase` 已执行则旧库已迁移），回滚需提供降级脚本或清除数据目录 `/data/service/el1/public/bundle_usage/`。

---

## 4. 知识路由

本仓库的知识库文档位于 `docs/knowledge/`，各文档末尾的"相关文档"链接块提供交叉导航。进行任务前，按 §4.1 入口表选择文档；进入子文档后沿"相关文档"自行导航。SSOT 清单见 §4.3。

### 4.1 入口文档

| 任务场景 | 入口文档 |
|---|---|
| 理解业务术语（分组、事件、区间类型、空闲、活跃等） | [`docs/knowledge/glossary.md`](docs/knowledge/glossary.md) |
| 理解部件定位、外部系统依赖、交互上下文 | [`docs/knowledge/business-context.md`](docs/knowledge/business-context.md) |
| 理解分层、设计原则、技术选型、逻辑架构、非功能设计 | [`docs/knowledge/architecture.md`](docs/knowledge/architecture.md) |
| 编码约定（日志宏、时间基准、FFRT、权限、错误码、命名等） | [`docs/knowledge/coding-standards.md`](docs/knowledge/coding-standards.md) |
| 理解核心业务对象（事件、分组、使用统计、模块记录、卡片记录、高频时段） | [`docs/knowledge/entities/`](docs/knowledge/entities/)（7 篇实体文档） |
| 查询对外接口语义与用法（方法签名、参数、返回值） | [`README_zh.md`](README_zh.md) / [`README.md`](README.md) |

### 4.2 编辑前自检（强制）

开始任何代码修改前，必须在回复中声明：

1. **任务类别**：属于[新增接口 / 修复缺陷 / 重构 / 新增事件 / schema 变更 / 其它]中的哪一类。
2. **已读文档**：根据 §4.1 入口表已阅读的文档列表。
3. **识别约束**：本次修改触发的约束清单（从 §2.1/§2.2 + 已读文档中提取）。
4. **验证计划**：将执行的验证项（对应 §3.3 清单）。

未声明上述四项前不得修改代码。

### 4.3 单一源（SSOT）清单

各知识点的**唯一定义位置**。其它文档引用时须链接到此，不得复制完整内容：

| 知识点 | 源 |
|---|---|
| 六级分组表（ALIVE/DAILY/.../NEVER） | entities/app-group.md |
| 分组提升超时表（前台→ALIVE 1h 等） | entities/app-group.md |
| 分组原因码（高/低字节清单） | entities/app-group.md |
| 双维度阈值（启动/亮屏） | entities/app-group.md |
| 区间类型枚举表 | entities/package-usage-stats.md |
| JS↔内部枚举映射说明 | entities/package-usage-stats.md |
| 权限三模式校验表 | coding-standards.md §5 |
| 条件编译守卫表 | coding-standards.md §9 |
| 构建加固配置 | coding-standards.md §10 |
| 错误码表 | coding-standards.md §6 |
| 数据保留策略（30/30/12/10 + 7天） | architecture.md §4.3 |
| 落盘时机（30min/DEBUG 2min） | glossary.md "落盘/刷新"条目 |
| FFRT 队列与并发模型 | coding-standards.md §4 |
| IPC 接口码规则 | coding-standards.md §11 |
| 实体文档结构模板 | coding-standards.md §15 |
