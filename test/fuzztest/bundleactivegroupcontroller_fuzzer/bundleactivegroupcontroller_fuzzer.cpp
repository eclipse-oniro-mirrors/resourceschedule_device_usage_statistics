/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "bundleactivegroupcontroller_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <fuzzer/FuzzedDataProvider.h>

#include "bundle_active_event.h"
#include "bundle_active_group_common.h"
#include "bundle_active_group_controller.h"
#include "bundle_active_group_handler.h"
#include "bundle_active_package_history.h"
#include "bundle_active_user_history.h"

namespace OHOS {
namespace DeviceUsageStats {
namespace {

using namespace DeviceUsageStatsGroupConst;

constexpr size_t MAX_NAME_LEN = 32;
constexpr int64_t FIXED_BOOT_TS = 1000LL;
constexpr int32_t FIXED_USER_ID = 100;
constexpr int32_t FIXED_UID = 20010000;
constexpr int64_t MAX_BOOT_TS = 4102444800000LL;
constexpr int64_t FIRST_CHECK_DELTA = 1000LL;
constexpr int64_t FORCED_CHECK_DELTA = 2000LL;
constexpr int64_t SCREEN_OFF_DELTA = 500LL;
constexpr int64_t SHUTDOWN_DELTA = 600LL;
constexpr int64_t SCREEN_NOOP_DELTA = 100LL;
constexpr int64_t SCREEN_QUERY_DELTA = 200LL;
constexpr int64_t SCREEN_OFF_UPDATE_DELTA = 300LL;
constexpr int64_t SCREEN_OFF_NOOP_DELTA = 400LL;
constexpr int64_t LONG_TIMEOUT_DELTA = 5000LL;
constexpr int64_t BACKWARD_TIME_DELTA = -1LL;
constexpr int32_t MISSING_USER_HISTORY_DELTA = 99999;
constexpr int32_t MISSING_LEVEL_USER_DELTA = 99998;
constexpr int32_t MISSING_WRITE_USER_DELTA = 99997;
constexpr int32_t MISSING_PRINT_USER_DELTA = 99996;
constexpr int32_t MISSING_FIRST_USE_USER_DELTA = 99995;
constexpr int32_t MISSING_SET_GROUP_USER_DELTA = 88888;
constexpr int32_t UNKNOWN_REMOVE_EVENT_ID = 999;
constexpr int32_t UNKNOWN_PROCESS_EVENT_ID = 99;

// 覆盖 ReportEvent 内 eventIdMatchReason_ switch 的六个分支
constexpr int32_t REPORT_EVENTS[] = {
    BundleActiveEvent::ABILITY_FOREGROUND,
    BundleActiveEvent::ABILITY_BACKGROUND,
    BundleActiveEvent::SYSTEM_INTERACTIVE,
    BundleActiveEvent::USER_INTERACTIVE,
    BundleActiveEvent::NOTIFICATION_SEEN,
    BundleActiveEvent::LONG_TIME_TASK_STARTTED,
};
constexpr size_t REPORT_EVENTS_CNT = sizeof(REPORT_EVENTS) / sizeof(REPORT_EVENTS[0]);

// 进程级一次性初始化
static bool g_isInited = false;

static std::string NormalizeBundleName(const std::string& bundleName)
{
    return bundleName.empty() ? "com.ohos.bundleactive.fuzz" : bundleName;
}

static int64_t AddBootTime(int64_t base, int64_t delta)
{
    if (delta > 0 && base > MAX_BOOT_TS - delta) {
        return MAX_BOOT_TS;
    }
    if (delta < 0 && base < FIXED_BOOT_TS - delta) {
        return FIXED_BOOT_TS;
    }
    return base + delta;
}

static int64_t ConsumeBootTime(FuzzedDataProvider& fdp)
{
    return fdp.ConsumeIntegralInRange<int64_t>(FIXED_BOOT_TS, MAX_BOOT_TS);
}

static void GlobalInit()
{
    if (g_isInited) {
        return;
    }
    // 设置 native token，使内部鉴权路径可以通过
    DelayedSingleton<FuzztestHelper>::GetInstance()->NativeTokenGet();
    auto& ctrl = BundleActiveGroupController::GetInstance();
    // debug=true：超时阈值以分钟计，更易覆盖状态机分支
    ctrl.Init(true);
    // 直接创建 BundleActiveUserHistory（BundleActiveCore 可为 nullptr）
    if (ctrl.bundleUserHistory_ == nullptr) {
        ctrl.bundleUserHistory_ =
            std::make_shared<BundleActiveUserHistory>(FIXED_BOOT_TS, nullptr);
    }
    g_isInited = true;
}

struct GroupFuzzInput {
    std::string bundleName;
    int32_t userId;
    int32_t uid;
    int64_t bootTs;
};

static GroupFuzzInput BuildGroupFuzzInput(FuzzedDataProvider& fdp)
{
    GroupFuzzInput input;
    input.bundleName = NormalizeBundleName(fdp.ConsumeRandomLengthString(MAX_NAME_LEN));
    input.userId = fdp.ConsumeBool() ? FIXED_USER_ID : fdp.ConsumeIntegral<int32_t>();
    input.uid = fdp.ConsumeBool() ? FIXED_UID : fdp.ConsumeIntegral<int32_t>();
    input.bootTs = ConsumeBootTime(fdp);
    return input;
}

static void FuzzControllerBasic(BundleActiveGroupController& ctrl, const GroupFuzzInput& input)
{
    ctrl.SetBundleGroupEnable(false);
    (void)ctrl.GetBundleGroupEnable();
    ctrl.SetBundleGroupEnable(true);

    {
        auto handler = ctrl.GetBundleGroupHandler();
        (void)handler;
    }

    (void)ctrl.IsScreenOn();

    ctrl.SetBundleGroupEnable(false);
    {
        BundleActiveEvent ev;
        ev.bundleName_ = input.bundleName;
        ev.uid_        = input.uid;
        ev.timeStamp_  = input.bootTs;
        ev.eventId_    = BundleActiveEvent::ABILITY_FOREGROUND;
        ctrl.ReportEvent(ev, input.bootTs, input.userId);
    }
    ctrl.SetBundleGroupEnable(true);

    for (size_t i = 0; i < REPORT_EVENTS_CNT; i++) {
        BundleActiveEvent ev;
        ev.bundleName_ = input.bundleName;
        ev.uid_        = input.uid;
        ev.timeStamp_  = input.bootTs;
        ev.eventId_    = REPORT_EVENTS[i];
        ctrl.ReportEvent(ev, input.bootTs, input.userId);
    }
}

static void FuzzControllerHistory(BundleActiveGroupController& ctrl, const GroupFuzzInput& input)
{
    ctrl.CheckAndUpdateGroup(input.bundleName, input.userId, input.uid, input.bootTs);

    ctrl.CheckAndUpdateGroup(input.bundleName, input.userId, input.uid,
        AddBootTime(input.bootTs, FIRST_CHECK_DELTA));

    if (ctrl.bundleUserHistory_ != nullptr) {
        auto bundleHist = ctrl.bundleUserHistory_->GetUsageHistoryForBundle(
            input.bundleName, input.userId, input.bootTs, false, input.uid);
        if (bundleHist != nullptr) {
            bundleHist->reasonInGroup_ = GROUP_CONTROL_REASON_FORCED;
            ctrl.CheckAndUpdateGroup(input.bundleName, input.userId, input.uid,
                AddBootTime(input.bootTs, FORCED_CHECK_DELTA));
            bundleHist->reasonInGroup_ = GROUP_CONTROL_REASON_DEFAULT;
        }
    }

    ctrl.OnBundleUninstalled(input.userId, input.bundleName, input.uid, 0);
    ctrl.CheckAndUpdateGroup(input.bundleName, input.userId, input.uid, input.bootTs);
    ctrl.OnBundleUninstalled(input.userId, input.bundleName, input.uid, 1);
    ctrl.OnUserRemoved(input.userId);
    ctrl.OnBundleUninstalled(input.userId, input.bundleName, input.uid, 0);
}

static void FuzzControllerScreen(BundleActiveGroupController& ctrl, const GroupFuzzInput& input)
{
    ctrl.OnScreenChanged(true, input.bootTs);
    ctrl.OnScreenChanged(false, AddBootTime(input.bootTs, SCREEN_OFF_DELTA));
    if (auto handler = ctrl.GetBundleGroupHandler()) {
        handler->PostSyncTask([]() {});
    }

    ctrl.CheckEachBundleState(input.userId);
    ctrl.ShutDown(AddBootTime(input.bootTs, SHUTDOWN_DELTA), input.userId);
    ctrl.OnUserSwitched(input.userId, input.userId);
}

static void FuzzControllerQueryAndPersist(BundleActiveGroupController& ctrl, const GroupFuzzInput& input)
{
    (void)ctrl.IsBundleInstalled(input.bundleName, input.userId);
    (void)ctrl.IsBundleIdle(input.bundleName, input.userId);

    {
        int32_t appGroup = 0;
        ctrl.QueryAppGroup(appGroup, "", input.userId);
    }
    {
        int32_t appGroup = 0;
        ctrl.QueryAppGroup(appGroup, input.bundleName, input.userId);
    }

    ctrl.SetAppGroup(input.bundleName, input.userId, ACTIVE_GROUP_ALIVE,
        GROUP_CONTROL_REASON_USAGE, input.bootTs, false);
    ctrl.SetAppGroup(input.bundleName, input.userId, ACTIVE_GROUP_DAILY,
        GROUP_CONTROL_REASON_TIMEOUT, input.bootTs, true);
    ctrl.RestoreToDatabase(input.userId);
    ctrl.RestoreDurationToDatabase();

    (void)ctrl.IsUsedOverOneWeek(input.bundleName, input.userId);
    {
        auto savedHistory = ctrl.bundleUserHistory_;
        ctrl.bundleUserHistory_ = nullptr;
        (void)ctrl.IsUsedOverOneWeek(input.bundleName, input.userId);
        ctrl.bundleUserHistory_ = savedHistory;
    }

    ctrl.PeriodCheckBundleState(input.userId);
    ctrl.CheckIdleStatsOneTime();
}

static void FuzzControllerPrivateHelpers(BundleActiveGroupController& ctrl, const GroupFuzzInput& input)
{
    {
        BundleActiveEvent ev;
        ev.bundleName_ = input.bundleName;
        ev.uid_ = input.uid;
        ev.timeStamp_ = input.bootTs;
        ctrl.SendCheckBundleMsg(ev, input.userId, FIVE_SECOND,
            BundleActiveGroupHandler::MSG_CHECK_DEFAULT_BUNDLE_STATE);
    }
    // master 已删除旧的超时计算 helper，超时判定并入 CheckAndUpdateGroup：
    // 依据 history 的 bundleAliveTimeoutTimeStamp_ / bundleDailyTimeoutTimeStamp_ 与当前
    // bootBasedTimeStamp 的比较决定保组或降级。这里构造“未超时/已超时”history 后走公开
    // 入口 CheckAndUpdateGroup，覆盖与旧 helper 等价的分支。
    if (ctrl.bundleUserHistory_ != nullptr) {
        int64_t checkTs = AddBootTime(input.bootTs, FIRST_CHECK_DELTA);
        auto hist = ctrl.bundleUserHistory_->GetUsageHistoryForBundle(
            input.bundleName, input.userId, input.bootTs, true, input.uid);
        if (hist != nullptr) {
            // 把上次使用时间拨到足够早，保证 GetNewGroup->GetLevelIndex 能取到最高档
            // （debug 模式阈值最长为 16 分钟）
            hist->lastBootFromUsedTimeStamp_ = -ONE_HOUR;
            hist->lastScreenUsedTimeStamp_ = -ONE_HOUR;

            // alive 超时点在未来：命中 ALIVE 未超时分支
            hist->reasonInGroup_ = GROUP_CONTROL_REASON_DEFAULT;
            hist->currentGroup_ = ACTIVE_GROUP_NEVER;
            hist->bundleAliveTimeoutTimeStamp_ = INT64_MAX;
            hist->bundleDailyTimeoutTimeStamp_ = 0;
            ctrl.CheckAndUpdateGroup(input.bundleName, input.userId, input.uid, checkTs);

            // 仅 daily 超时点在未来：命中 DAILY 未超时分支
            hist->reasonInGroup_ = GROUP_CONTROL_REASON_DEFAULT;
            hist->currentGroup_ = ACTIVE_GROUP_NEVER;
            hist->bundleAliveTimeoutTimeStamp_ = 0;
            hist->bundleDailyTimeoutTimeStamp_ = INT64_MAX;
            ctrl.CheckAndUpdateGroup(input.bundleName, input.userId, input.uid, checkTs);

            // 两个超时点均已过：命中超时降级分支（GetNewGroup 结果直接生效）
            hist->reasonInGroup_ = GROUP_CONTROL_REASON_DEFAULT;
            hist->currentGroup_ = ACTIVE_GROUP_NEVER;
            hist->bundleAliveTimeoutTimeStamp_ = 0;
            hist->bundleDailyTimeoutTimeStamp_ = 0;
            ctrl.CheckAndUpdateGroup(input.bundleName, input.userId, input.uid, checkTs);

            // 直接覆盖 GetNewGroup；不存在的 bundle 取到空 history，等价旧 helper 的空指针防护路径
            (void)ctrl.GetNewGroup(input.bundleName, input.userId, input.bootTs, input.uid);
            (void)ctrl.GetNewGroup(input.bundleName + "_timeout_noexist_key", input.userId,
                input.bootTs, input.uid);
        }
    }
}

static void FuzzGroupController(FuzzedDataProvider& fdp)
{
    auto& ctrl = BundleActiveGroupController::GetInstance();
    GroupFuzzInput input = BuildGroupFuzzInput(fdp);
    FuzzControllerBasic(ctrl, input);
    FuzzControllerHistory(ctrl, input);
    FuzzControllerScreen(ctrl, input);
    FuzzControllerQueryAndPersist(ctrl, input);
    FuzzControllerPrivateHelpers(ctrl, input);
    ctrl.SetBundleGroupEnable(true);
}

static std::shared_ptr<BundleActivePackageHistory> FuzzUserHistoryLookups(
    const std::shared_ptr<BundleActiveUserHistory>& history, const GroupFuzzInput& input)
{
    {
        auto noHistory = history->GetUserHistory(input.userId + MISSING_USER_HISTORY_DELTA, false);
        (void)noHistory;
    }
    {
        auto newHistory = history->GetUserHistory(input.userId, true);
        (void)newHistory;
    }
    {
        auto existing = history->GetUserHistory(input.userId, false);
        (void)existing;
    }

    auto bundleHist = history->GetUsageHistoryForBundle(input.bundleName,
        input.userId, input.bootTs, true, input.uid);
    {
        auto hitHist = history->GetUsageHistoryForBundle(input.bundleName,
            input.userId, input.bootTs, false, input.uid);
        (void)hitHist;
    }
    {
        auto noHist = history->GetUsageHistoryForBundle(
            input.bundleName + "_noexist_key", input.userId, input.bootTs, false, input.uid);
        (void)noHist;
    }
    return bundleHist;
}

static void FuzzUserHistoryTimeAndLevel(const std::shared_ptr<BundleActiveUserHistory>& history,
    const GroupFuzzInput& input, const std::shared_ptr<BundleActivePackageHistory>& bundleHist)
{
    (void)history->GetBootBasedTimeStamp(input.bootTs);
    (void)history->GetScreenOnTimeStamp(input.bootTs);

    history->UpdateBootBasedAndScreenTime(true, input.bootTs);
    history->UpdateBootBasedAndScreenTime(true, AddBootTime(input.bootTs, SCREEN_NOOP_DELTA));
    (void)history->GetScreenOnTimeStamp(AddBootTime(input.bootTs, SCREEN_QUERY_DELTA));
    history->UpdateBootBasedAndScreenTime(false, AddBootTime(input.bootTs, SCREEN_OFF_UPDATE_DELTA));
    history->UpdateBootBasedAndScreenTime(false, AddBootTime(input.bootTs, SCREEN_OFF_NOOP_DELTA));
    history->UpdateBootBasedAndScreenTime(false, AddBootTime(input.bootTs, SCREEN_OFF_DELTA), true);

    if (bundleHist != nullptr) {
        (void)history->GetLevelIndex(input.bundleName, input.userId, input.bootTs,
            {0, 0, 0, 0}, {0, 0, 0, 0}, input.uid);
        (void)history->GetLevelIndex(input.bundleName, input.userId, input.bootTs,
            {0, 0, TWO_MINUTE, FOUR_MINUTE},
            {0, TWO_MINUTE, FOUR_MINUTE, SIXTEEN_MINUTE},
            input.uid);
        (void)history->GetLevelIndex(input.bundleName, input.userId, input.bootTs,
            {0, 0, TWELVE_HOUR, TWENTY_FOUR_HOUR},
            {0, TWENTY_FOUR_HOUR, FOURTY_EIGHT_HOUR, FOURTY_EIGHT_HOUR * 2},
            input.uid);
    }

    (void)history->GetLevelIndex(input.bundleName, input.userId + MISSING_LEVEL_USER_DELTA,
        input.bootTs, {0, 0, 0, 0}, {0, 0, 0, 0}, input.uid);
    (void)history->GetLevelIndex(input.bundleName + "_noexist_key", input.userId,
        input.bootTs, {0, 0, 0, 0}, {0, 0, 0, 0}, input.uid);
}

static void FuzzUserHistoryReportUsage(const std::shared_ptr<BundleActiveUserHistory>& history,
    const GroupFuzzInput& input, const std::shared_ptr<BundleActivePackageHistory>& bundleHist)
{
    if (bundleHist != nullptr) {
        BundleActiveEvent event;
        event.bundleName_ = input.bundleName;
        event.timeStamp_  = input.bootTs;
        event.uid_        = input.uid;

        bundleHist->reasonInGroup_ = GROUP_CONTROL_REASON_FORCED;
        history->ReportUsage(bundleHist, event, ACTIVE_GROUP_ALIVE,
            GROUP_EVENT_REASON_FOREGROUND, input.bootTs, AddBootTime(input.bootTs, FIRST_CHECK_DELTA), input.userId);

        bundleHist->reasonInGroup_ = GROUP_CONTROL_REASON_DEFAULT;
        bundleHist->currentGroup_  = ACTIVE_GROUP_NEVER;

        history->ReportUsage(bundleHist, event, ACTIVE_GROUP_ALIVE,
            GROUP_EVENT_REASON_FOREGROUND, input.bootTs, AddBootTime(input.bootTs, LONG_TIMEOUT_DELTA), input.userId);

        history->ReportUsage(bundleHist, event, ACTIVE_GROUP_DAILY,
            GROUP_EVENT_REASON_FOREGROUND, input.bootTs, AddBootTime(input.bootTs, LONG_TIMEOUT_DELTA), input.userId);

        history->ReportUsage(bundleHist, event, ACTIVE_GROUP_RARE,
            GROUP_EVENT_REASON_FOREGROUND, input.bootTs, AddBootTime(input.bootTs, LONG_TIMEOUT_DELTA), input.userId);

        history->ReportUsage(bundleHist, event, ACTIVE_GROUP_ALIVE,
            GROUP_EVENT_REASON_FOREGROUND, input.bootTs, AddBootTime(input.bootTs, BACKWARD_TIME_DELTA), input.userId);

        history->ReportUsage(bundleHist, event, ACTIVE_GROUP_ALIVE,
            GROUP_EVENT_REASON_FOREGROUND, 0, LONG_TIMEOUT_DELTA, input.userId);

        history->ReportUsage(bundleHist, event, ACTIVE_GROUP_ALIVE,
            GROUP_EVENT_REASON_BACKGROUND, input.bootTs, AddBootTime(input.bootTs, BACKWARD_TIME_DELTA), input.userId);
    }
}

static void FuzzUserHistorySetGroup(const std::shared_ptr<BundleActiveUserHistory>& history,
    const GroupFuzzInput& input, const std::shared_ptr<BundleActivePackageHistory>& bundleHist)
{
    {
        int32_t r = history->SetAppGroup(input.bundleName,
            input.userId + MISSING_SET_GROUP_USER_DELTA, input.uid, input.bootTs,
            ACTIVE_GROUP_DAILY, GROUP_CONTROL_REASON_USAGE, false);
        (void)r;
    }

    {
        int32_t r = history->SetAppGroup(input.bundleName + "_nokey2", input.userId, input.uid, input.bootTs,
            ACTIVE_GROUP_DAILY, GROUP_CONTROL_REASON_USAGE, false);
        (void)r;
    }

    if (bundleHist != nullptr) {
        bundleHist->currentGroup_  = ACTIVE_GROUP_DAILY;
        bundleHist->reasonInGroup_ = GROUP_CONTROL_REASON_USAGE;
        int32_t r = history->SetAppGroup(input.bundleName, input.userId, input.uid, input.bootTs,
            ACTIVE_GROUP_DAILY, GROUP_CONTROL_REASON_USAGE, false);
        (void)r;
    }

    if (bundleHist != nullptr) {
        bundleHist->currentGroup_ = ACTIVE_GROUP_DAILY;
        int32_t r = history->SetAppGroup(input.bundleName, input.userId, input.uid, input.bootTs,
            ACTIVE_GROUP_ALIVE, GROUP_CONTROL_REASON_USAGE, false);
        (void)r;
    }

    if (bundleHist != nullptr) {
        int32_t r = history->SetAppGroup(input.bundleName, input.userId, input.uid, input.bootTs,
            ACTIVE_GROUP_FIXED, GROUP_CONTROL_REASON_TIMEOUT, true);
        (void)r;
    }
}

static void FuzzUserHistoryPersist(const std::shared_ptr<BundleActiveUserHistory>& history,
    const GroupFuzzInput& input)
{
    history->WriteDeviceDuration();

    history->WriteBundleUsage(input.userId);
    history->WriteBundleUsage(input.userId + MISSING_WRITE_USER_DELTA);

    history->PrintData(input.userId);
    history->PrintData(input.userId + MISSING_PRINT_USER_DELTA);

    (void)history->GetFirstUseTime(input.bundleName, input.userId);
    (void)history->GetFirstUseTime(input.bundleName + "_noexist_key", input.userId);
    (void)history->GetFirstUseTime(input.bundleName, input.userId + MISSING_FIRST_USE_USER_DELTA);

    history->OnBundleUninstalled(input.userId, input.bundleName, input.uid, 0);
    history->OnBundleUninstalled(input.userId, input.bundleName, input.uid, 1);
    (void)history->GetUsageHistoryForBundle(input.bundleName, FIXED_USER_ID,
        input.bootTs, true, FIXED_UID);
}

static void FuzzUserHistory(FuzzedDataProvider& fdp)
{
    auto& history = BundleActiveGroupController::GetInstance().bundleUserHistory_;
    if (history == nullptr) {
        return;
    }

    GroupFuzzInput input = BuildGroupFuzzInput(fdp);
    auto bundleHist = FuzzUserHistoryLookups(history, input);
    FuzzUserHistoryTimeAndLevel(history, input, bundleHist);
    FuzzUserHistoryReportUsage(history, input, bundleHist);
    FuzzUserHistorySetGroup(history, input, bundleHist);
    FuzzUserHistoryPersist(history, input);
}

static std::shared_ptr<BundleActiveGroupHandlerObject> BuildHandlerObject(const GroupFuzzInput& input)
{
    auto handlerObj = std::make_shared<BundleActiveGroupHandlerObject>();
    handlerObj->bundleName_ = input.bundleName;
    handlerObj->userId_ = input.userId;
    handlerObj->uid_ = input.uid;
    return handlerObj;
}

static void FuzzHandlerBeforeInit(BundleActiveGroupHandler& handler,
    const std::shared_ptr<BundleActiveGroupHandlerObject>& handlerObj, int64_t delayMs)
{
    auto emptyKey = handler.GetMsgKey(
        BundleActiveGroupHandler::MSG_CHECK_DEFAULT_BUNDLE_STATE, nullptr, delayMs);
    (void)emptyKey;

    auto validKey = handler.GetMsgKey(
        BundleActiveGroupHandler::MSG_CHECK_DEFAULT_BUNDLE_STATE, handlerObj, delayMs);
    (void)validKey;

    handler.SendEvent(BundleActiveGroupHandler::MSG_CHECK_DEFAULT_BUNDLE_STATE, handlerObj);
    handler.RemoveEvent(BundleActiveGroupHandler::MSG_CHECK_DEFAULT_BUNDLE_STATE);
    handler.SendCheckBundleMsg(
        BundleActiveGroupHandler::MSG_CHECK_NOTIFICATION_SEEN_BUNDLE_STATE, handlerObj, delayMs);
    handler.RemoveCheckBundleMsg("nonexistent_before_init");
    handler.PostTask([]() {});
    handler.PostSyncTask([]() {});
    handler.ProcessEvent(BundleActiveGroupHandler::MSG_CHECK_DEFAULT_BUNDLE_STATE, handlerObj);
}

static void FuzzHandlerSchedule(BundleActiveGroupHandler& handler,
    const std::shared_ptr<BundleActiveGroupHandlerObject>& handlerObj, int64_t delayMs)
{
    handler.SendEvent(BundleActiveGroupHandler::MSG_CHECK_DEFAULT_BUNDLE_STATE, handlerObj);
    handler.SendEvent(
        BundleActiveGroupHandler::MSG_CHECK_NOTIFICATION_SEEN_BUNDLE_STATE, handlerObj, delayMs);
    handler.SendEvent(
        BundleActiveGroupHandler::MSG_CHECK_SYSTEM_INTERACTIVE_BUNDLE_STATE, handlerObj, delayMs);
    handler.SendEvent(BundleActiveGroupHandler::MSG_ONE_TIME_CHECK_BUNDLE_STATE, handlerObj);
    handler.SendEvent(BundleActiveGroupHandler::MSG_CHECK_IDLE_STATE, handlerObj);

    handler.RemoveEvent(BundleActiveGroupHandler::MSG_CHECK_DEFAULT_BUNDLE_STATE);
    handler.RemoveEvent(BundleActiveGroupHandler::MSG_CHECK_DEFAULT_BUNDLE_STATE);
    handler.RemoveEvent(UNKNOWN_REMOVE_EVENT_ID);

    handler.SendCheckBundleMsg(
        BundleActiveGroupHandler::MSG_CHECK_NOTIFICATION_SEEN_BUNDLE_STATE, handlerObj, FIRST_CHECK_DELTA);
    handler.SendCheckBundleMsg(
        BundleActiveGroupHandler::MSG_CHECK_NOTIFICATION_SEEN_BUNDLE_STATE, nullptr, FIRST_CHECK_DELTA);
    handler.SendCheckBundleMsg(
        BundleActiveGroupHandler::MSG_CHECK_NOTIFICATION_SEEN_BUNDLE_STATE, handlerObj, FIRST_CHECK_DELTA);
}

static void FuzzHandlerProcess(BundleActiveGroupHandler& handler,
    const std::shared_ptr<BundleActiveGroupHandlerObject>& handlerObj)
{
    handler.PostTask([]() {});
    handler.PostSyncTask([]() {});

    handler.ProcessEvent(BundleActiveGroupHandler::MSG_CHECK_DEFAULT_BUNDLE_STATE, nullptr);
    handler.ProcessEvent(BundleActiveGroupHandler::MSG_CHECK_IDLE_STATE, nullptr);

    handler.ProcessEvent(BundleActiveGroupHandler::MSG_CHECK_DEFAULT_BUNDLE_STATE, handlerObj);
    handler.ProcessEvent(
        BundleActiveGroupHandler::MSG_CHECK_NOTIFICATION_SEEN_BUNDLE_STATE, handlerObj);
    handler.ProcessEvent(
        BundleActiveGroupHandler::MSG_CHECK_SYSTEM_INTERACTIVE_BUNDLE_STATE, handlerObj);
    handler.ProcessEvent(BundleActiveGroupHandler::MSG_ONE_TIME_CHECK_BUNDLE_STATE, handlerObj);
    handler.ProcessEvent(BundleActiveGroupHandler::MSG_CHECK_IDLE_STATE, handlerObj);
    handler.ProcessEvent(UNKNOWN_PROCESS_EVENT_ID, handlerObj);
}

static void FuzzGroupHandler(FuzzedDataProvider& fdp)
{
    GroupFuzzInput input = BuildGroupFuzzInput(fdp);
    int64_t delayMs = fdp.ConsumeIntegralInRange<int64_t>(0, FIVE_SECOND);
    auto handlerObj = BuildHandlerObject(input);

    {
        BundleActiveGroupHandler handler(true);
        FuzzHandlerBeforeInit(handler, handlerObj, delayMs);
        handler.Init();

        auto keyNotif = handler.GetMsgKey(
            BundleActiveGroupHandler::MSG_CHECK_NOTIFICATION_SEEN_BUNDLE_STATE, handlerObj, FIRST_CHECK_DELTA);
        (void)handler.GetMsgKey(BundleActiveGroupHandler::MSG_CHECK_IDLE_STATE, handlerObj, 0);

        FuzzHandlerSchedule(handler, handlerObj, delayMs);
        if (!keyNotif.empty()) {
            handler.RemoveCheckBundleMsg(keyNotif);
            handler.RemoveCheckBundleMsg(keyNotif);
        }
        handler.RemoveCheckBundleMsg("nonexistent_key_xyz");
        FuzzHandlerProcess(handler, handlerObj);

        handler.DeInit();
        handler.ProcessEvent(BundleActiveGroupHandler::MSG_CHECK_DEFAULT_BUNDLE_STATE, handlerObj);
        handler.SendEvent(BundleActiveGroupHandler::MSG_CHECK_DEFAULT_BUNDLE_STATE, handlerObj);
        handler.PostTask([]() {});
    }

    {
        BundleActiveGroupHandler handlerRelease(false);
        handlerRelease.Init();
        (void)handlerRelease.GetMsgKey(
            BundleActiveGroupHandler::MSG_CHECK_IDLE_STATE, handlerObj, 0);
        handlerRelease.ProcessEvent(BundleActiveGroupHandler::MSG_CHECK_IDLE_STATE, handlerObj);
        handlerRelease.DeInit();
    }
}

} // namespace

// ======= 模糊测试主入口 =======
bool BundleActiveGroupControllerFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return false;
    }
    GlobalInit();
    FuzzedDataProvider fdp(data, size);
    FuzzGroupController(fdp);
    FuzzUserHistory(fdp);
    FuzzGroupHandler(fdp);
    auto& ctrl = BundleActiveGroupController::GetInstance();
    ctrl.DeInit();
    ctrl.Init(true);
    if (ctrl.bundleUserHistory_ == nullptr) {
        ctrl.bundleUserHistory_ =
            std::make_shared<BundleActiveUserHistory>(FIXED_BOOT_TS, nullptr);
    }
    return true;
}

} // namespace DeviceUsageStats
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DeviceUsageStats::BundleActiveGroupControllerFuzzTest(data, size);
    return 0;
}
