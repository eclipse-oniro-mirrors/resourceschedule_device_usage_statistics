/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <fuzzer/FuzzedDataProvider.h>

#include "bundle_active_group_controller.h"
#include "bundle_active_user_history.h"
#include "bundle_active_package_history.h"
#include "bundle_active_event.h"
#include "bundle_active_core.h"
#include "bundle_active_group_handler.h"
#include "bundle_active_group_common.h"
#include "fuzztest_helper.h"

namespace OHOS {
namespace DeviceUsageStats {

constexpr int32_t FUZZ_CODE_0  = 0;
constexpr int32_t FUZZ_CODE_1  = 1;
constexpr int32_t FUZZ_CODE_2  = 2;
constexpr int32_t FUZZ_CODE_3  = 3;
constexpr int32_t FUZZ_CODE_4  = 4;
constexpr int32_t FUZZ_CODE_5  = 5;
constexpr int32_t FUZZ_CODE_6  = 6;
constexpr int32_t FUZZ_CODE_7  = 7;
constexpr int32_t FUZZ_CODE_8  = 8;
constexpr int32_t FUZZ_CODE_9  = 9;
constexpr int32_t FUZZ_CODE_10 = 10;
constexpr int32_t FUZZ_CODE_11 = 11;
constexpr int32_t FUZZ_CODE_12 = 12;
constexpr int32_t FUZZ_CODE_13 = 13;
constexpr int32_t FUZZ_CODE_14 = 14;
constexpr int32_t FUZZ_CODE_15 = 15;
constexpr int32_t FUZZ_CODE_16 = 16;
constexpr int32_t FUZZ_CODE_17 = 17;
constexpr int32_t FUZZ_CODE_18 = 18;
constexpr int32_t FUZZ_CODE_MAX = 18;

// Magic numbers used for testing undefined event IDs
constexpr int32_t K_UNMATCHED_EVENT_ID = 9999;

// Magic number for default delay in SendEvent/SendCheckBundleMsg
constexpr uint32_t K_DEFAULT_DELAY = 0;

/* -------------------------------------------------------
 * Init helper: ensure controller + handler are ready
 * Called at the start of every test that needs bundleUserHistory_
 * ------------------------------------------------------- */
static void EnsureInitialized()
{
    BundleActiveGroupController& ctrl = BundleActiveGroupController::GetInstance();
    if (!ctrl.GetBundleGroupHandler()) {
        ctrl.Init(false);
    }
}

/* -------------------------------------------------------
 * BundleActiveGroupController fuzz helpers
 * ------------------------------------------------------- */

/* FUZZ_CODE_0: Init */
bool GroupControllerInitFuzzTest(FuzzedDataProvider* fdp)
{
    bool debug = fdp->ConsumeBool();
    BundleActiveGroupController::GetInstance().Init(debug);
    return true;
}

/* FUZZ_CODE_14: DeInit (and double-DeInit guard) */
bool GroupControllerDeInitFuzzTest(FuzzedDataProvider* fdp)
{
    bool doInitFirst = fdp->ConsumeBool();
    if (doInitFirst) {
        BundleActiveGroupController::GetInstance().Init(false);
    }
    BundleActiveGroupController::GetInstance().DeInit();
    return true;
}

/* FUZZ_CODE_1: ReportEvent with random eventId (early exit paths) */
bool GroupControllerReportEventFuzzTest(FuzzedDataProvider* fdp)
{
    BundleActiveEvent event;
    event.bundleName_  = fdp->ConsumeRandomLengthString();
    event.eventId_     = fdp->ConsumeIntegral<int32_t>();
    event.timeStamp_   = fdp->ConsumeIntegral<int64_t>();
    event.abilityName_ = fdp->ConsumeRandomLengthString();
    event.uid_        = fdp->ConsumeIntegral<int32_t>();
    int64_t bootTs    = fdp->ConsumeIntegral<int64_t>();
    int32_t userId    = fdp->ConsumeIntegral<int32_t>();

    // Test bundleGroupEnable_=false early exit path
    BundleActiveGroupController::GetInstance().SetBundleGroupEnable(false);
    BundleActiveGroupController::GetInstance().ReportEvent(event, bootTs, userId);

    // Test normal path (bundleGroupEnable_=true)
    BundleActiveGroupController::GetInstance().SetBundleGroupEnable(true);
    BundleActiveGroupController::GetInstance().ReportEvent(event, bootTs, userId);
    return true;
}

/* FUZZ_CODE_13: ReportEvent with specific eventIds to hit all switch branches */
bool GroupControllerReportEventSpecificIdsFuzzTest(FuzzedDataProvider* fdp)
{
    std::string bundleName = fdp->ConsumeRandomLengthString();
    int64_t bootTs         = fdp->ConsumeIntegral<int64_t>();
    int32_t userId         = fdp->ConsumeIntegral<int32_t>();
    int32_t uid            = fdp->ConsumeIntegral<int32_t>();

    EnsureInitialized();

    // Case: NOTIFICATION_SEEN -> triggers MSG_CHECK_NOTIFICATION_SEEN_BUNDLE_STATE path
    BundleActiveEvent ev1;
    ev1.eventId_    = BundleActiveEvent::NOTIFICATION_SEEN;
    ev1.bundleName_ = bundleName;
    ev1.uid_        = uid;
    BundleActiveGroupController::GetInstance().ReportEvent(ev1, bootTs, userId);

    // Case: SYSTEM_INTERACTIVE -> triggers MSG_CHECK_SYSTEM_INTERACTIVE_BUNDLE_STATE path
    BundleActiveEvent ev2;
    ev2.eventId_    = BundleActiveEvent::SYSTEM_INTERACTIVE;
    ev2.bundleName_ = bundleName;
    ev2.uid_        = uid;
    BundleActiveGroupController::GetInstance().ReportEvent(ev2, bootTs, userId);

    // Case: USER_INTERACTIVE -> default branch (ACTIVE_GROUP_ALIVE, MSG_CHECK_DEFAULT_BUNDLE_STATE)
    BundleActiveEvent ev3;
    ev3.eventId_    = BundleActiveEvent::USER_INTERACTIVE;
    ev3.bundleName_ = bundleName;
    ev3.uid_        = uid;
    BundleActiveGroupController::GetInstance().ReportEvent(ev3, bootTs, userId);

    // Case: ABILITY_FOREGROUND -> default branch (no special handler message)
    BundleActiveEvent ev4;
    ev4.eventId_    = BundleActiveEvent::ABILITY_FOREGROUND;
    ev4.bundleName_ = bundleName;
    ev4.uid_        = uid;
    BundleActiveGroupController::GetInstance().ReportEvent(ev4, bootTs, userId);

    // Case: ABILITY_BACKGROUND -> default branch
    BundleActiveEvent ev5;
    ev5.eventId_    = BundleActiveEvent::ABILITY_BACKGROUND;
    ev5.bundleName_ = bundleName;
    ev5.uid_        = uid;
    BundleActiveGroupController::GetInstance().ReportEvent(ev5, bootTs, userId);

    // Case: LONG_TIME_TASK_STARTTED -> default branch
    BundleActiveEvent ev6;
    ev6.eventId_    = BundleActiveEvent::LONG_TIME_TASK_STARTTED;
    ev6.bundleName_ = bundleName;
    ev6.uid_        = uid;
    BundleActiveGroupController::GetInstance().ReportEvent(ev6, bootTs, userId);

    // Case: unmatched eventId -> early return (item == eventIdMatchReason_.end())
    BundleActiveEvent ev7;
    ev7.eventId_    = K_UNMATCHED_EVENT_ID;  // not in eventIdMatchReason_
    ev7.bundleName_ = bundleName;
    ev7.uid_        = uid;
    BundleActiveGroupController::GetInstance().ReportEvent(ev7, bootTs, userId);
    return true;
}

/* FUZZ_CODE_2: CheckAndUpdateGroup */
bool GroupControllerCheckAndUpdateGroupFuzzTest(FuzzedDataProvider* fdp)
{
    std::string bundleName = fdp->ConsumeRandomLengthString();
    int32_t userId    = fdp->ConsumeIntegral<int32_t>();
    int32_t uid      = fdp->ConsumeIntegral<int32_t>();
    int64_t bootTs   = fdp->ConsumeIntegral<int64_t>();
    BundleActiveGroupController::GetInstance().CheckAndUpdateGroup(bundleName, userId, uid, bootTs);
    return true;
}

/* FUZZ_CODE_3: CheckEachBundleState */
bool GroupControllerCheckEachBundleStateFuzzTest(FuzzedDataProvider* fdp)
{
    int32_t userId = fdp->ConsumeIntegral<int32_t>();
    BundleActiveGroupController::GetInstance().CheckEachBundleState(userId);
    return true;
}

/* FUZZ_CODE_4: CheckIdleStatsOneTime */
bool GroupControllerCheckIdleStatsOneTimeFuzzTest(FuzzedDataProvider* /*fdp*/)
{
    BundleActiveGroupController::GetInstance().CheckIdleStatsOneTime();
    return true;
}

/* FUZZ_CODE_5: PeriodCheckBundleState */
bool GroupControllerPeriodCheckFuzzTest(FuzzedDataProvider* fdp)
{
    int32_t userId = fdp->ConsumeIntegral<int32_t>();
    BundleActiveGroupController::GetInstance().PeriodCheckBundleState(userId);
    return true;
}

/* FUZZ_CODE_6: OnUserRemoved */
bool GroupControllerOnUserRemovedFuzzTest(FuzzedDataProvider* fdp)
{
    int32_t userId = fdp->ConsumeIntegral<int32_t>();
    BundleActiveGroupController::GetInstance().OnUserRemoved(userId);
    return true;
}

/* FUZZ_CODE_7: OnBundleUninstalled */
bool GroupControllerOnBundleUninstalledFuzzTest(FuzzedDataProvider* fdp)
{
    int32_t userId   = fdp->ConsumeIntegral<int32_t>();
    std::string bundle = fdp->ConsumeRandomLengthString();
    int32_t uid      = fdp->ConsumeIntegral<int32_t>();
    int32_t appIndex = fdp->ConsumeIntegral<int32_t>();
    BundleActiveGroupController::GetInstance().OnBundleUninstalled(userId, bundle, uid, appIndex);
    return true;
}

/* FUZZ_CODE_8: OnScreenChanged */
bool GroupControllerOnScreenChangedFuzzTest(FuzzedDataProvider* fdp)
{
    bool isScreenOn = fdp->ConsumeBool();
    int64_t bootTs  = fdp->ConsumeIntegral<int64_t>();
    BundleActiveGroupController::GetInstance().OnScreenChanged(isScreenOn, bootTs);
    return true;
}

/* FUZZ_CODE_9: SetAppGroup */
bool GroupControllerSetAppGroupFuzzTest(FuzzedDataProvider* fdp)
{
    std::string bundle = fdp->ConsumeRandomLengthString();
    int32_t userId   = fdp->ConsumeIntegral<int32_t>();
    int32_t newGroup = fdp->ConsumeIntegral<int32_t>();
    uint32_t reason  = fdp->ConsumeIntegral<uint32_t>();
    int64_t bootTs   = fdp->ConsumeIntegral<int64_t>();
    bool isFlush     = fdp->ConsumeBool();
    BundleActiveGroupController::GetInstance().SetAppGroup(bundle, userId, newGroup, reason, bootTs, isFlush);
    return true;
}

/* FUZZ_CODE_10: IsBundleIdle */
bool GroupControllerIsBundleIdleFuzzTest(FuzzedDataProvider* fdp)
{
    std::string bundle = fdp->ConsumeRandomLengthString();
    int32_t userId   = fdp->ConsumeIntegral<int32_t>();
    BundleActiveGroupController::GetInstance().IsBundleIdle(bundle, userId);
    return true;
}

/* FUZZ_CODE_11: QueryAppGroup */
bool GroupControllerQueryAppGroupFuzzTest(FuzzedDataProvider* fdp)
{
    int32_t appGroup = 0;
    std::string bundle = fdp->ConsumeRandomLengthString();
    int32_t userId   = fdp->ConsumeIntegral<int32_t>();
    BundleActiveGroupController::GetInstance().QueryAppGroup(appGroup, bundle, userId);

    // Also test with empty bundle name -> ERR_NO_APP_GROUP_INFO_IN_DATABASE path
    appGroup = 0;
    BundleActiveGroupController::GetInstance().QueryAppGroup(appGroup, "", userId);
    return true;
}

/* FUZZ_CODE_12: Misc */
bool GroupControllerMiscFuzzTest(FuzzedDataProvider* fdp)
{
    std::string bundle = fdp->ConsumeRandomLengthString();
    int32_t userId     = fdp->ConsumeIntegral<int32_t>();

    BundleActiveGroupController::GetInstance().IsBundleInstalled(bundle, userId);
    BundleActiveGroupController::GetInstance().IsScreenOn();
    bool enable = fdp->ConsumeBool();
    BundleActiveGroupController::GetInstance().SetBundleGroupEnable(enable);
    BundleActiveGroupController::GetInstance().GetBundleGroupEnable();
    BundleActiveGroupController::GetInstance().IsUsedOverOneWeek(bundle, userId);

    int32_t currentUser = fdp->ConsumeIntegral<int32_t>();
    BundleActiveGroupController::GetInstance().OnUserSwitched(userId, currentUser);
    BundleActiveGroupController::GetInstance().GetBundleGroupHandler();
    return true;
}

/* FUZZ_CODE_15: ShutDown */
bool GroupControllerShutDownFuzzTest(FuzzedDataProvider* fdp)
{
    int64_t bootTs = fdp->ConsumeIntegral<int64_t>();
    int32_t userId = fdp->ConsumeIntegral<int32_t>();
    BundleActiveGroupController::GetInstance().ShutDown(bootTs, userId);
    return true;
}

/* FUZZ_CODE_16: BundleActiveGroupHandler ProcessEvent coverage
 * Triggers ProcessEvent via SendEvent/SendCheckBundleMsg for each message type */
bool GroupControllerHandlerProcessEventFuzzTest(FuzzedDataProvider* fdp)
{
    EnsureInitialized();

    int32_t userId    = fdp->ConsumeIntegral<int32_t>();
    std::string bundle = fdp->ConsumeRandomLengthString();
    int32_t uid       = fdp->ConsumeIntegral<int32_t>();

    // Build handler object
    BundleActiveGroupHandlerObject handlerObj;
    handlerObj.userId_    = userId;
    handlerObj.bundleName_ = bundle;
    handlerObj.uid_       = uid;
    std::shared_ptr<BundleActiveGroupHandlerObject> handlerObjPtr =
        std::make_shared<BundleActiveGroupHandlerObject>(handlerObj);

    auto handler = BundleActiveGroupController::GetInstance()
        .GetBundleGroupHandler();
    if (!handler) {
        return true;
    }

    // Trigger MSG_ONE_TIME_CHECK_BUNDLE_STATE via SendEvent (triggers ProcessEvent -> MSG_ONE_TIME_CHECK_BUNDLE_STATE branch)
    handler->SendEvent(BundleActiveGroupHandler::MSG_ONE_TIME_CHECK_BUNDLE_STATE, handlerObjPtr, K_DEFAULT_DELAY);

    // Trigger MSG_CHECK_IDLE_STATE via SendEvent (triggers ProcessEvent -> MSG_CHECK_IDLE_STATE branch)
    handler->SendEvent(BundleActiveGroupHandler::MSG_CHECK_IDLE_STATE, handlerObjPtr, K_DEFAULT_DELAY);

    // Trigger MSG_CHECK_DEFAULT_BUNDLE_STATE via SendEvent
    handler->SendEvent(BundleActiveGroupHandler::MSG_CHECK_DEFAULT_BUNDLE_STATE, handlerObjPtr, K_DEFAULT_DELAY);

    // Trigger MSG_CHECK_NOTIFICATION_SEEN_BUNDLE_STATE via SendCheckBundleMsg
    handler->SendCheckBundleMsg(BundleActiveGroupHandler::MSG_CHECK_NOTIFICATION_SEEN_BUNDLE_STATE,
        handlerObjPtr, K_DEFAULT_DELAY);

    // Trigger MSG_CHECK_SYSTEM_INTERACTIVE_BUNDLE_STATE via SendCheckBundleMsg
    handler->SendCheckBundleMsg(BundleActiveGroupHandler::MSG_CHECK_SYSTEM_INTERACTIVE_BUNDLE_STATE,
        handlerObjPtr, K_DEFAULT_DELAY);

    // Trigger default case in ProcessEvent (unknown eventId)
    handler->SendEvent(K_UNMATCHED_EVENT_ID, handlerObjPtr, K_DEFAULT_DELAY);

    // Test RemoveEvent (removes pending tasks for a message type)
    handler->RemoveEvent(BundleActiveGroupHandler::MSG_CHECK_DEFAULT_BUNDLE_STATE);
    handler->RemoveEvent(BundleActiveGroupHandler::MSG_CHECK_IDLE_STATE);
    handler->RemoveEvent(BundleActiveGroupHandler::MSG_CHECK_NOTIFICATION_SEEN_BUNDLE_STATE);
    handler->RemoveEvent(BundleActiveGroupHandler::MSG_CHECK_SYSTEM_INTERACTIVE_BUNDLE_STATE);
    handler->RemoveEvent(K_UNMATCHED_EVENT_ID);  // RemoveEvent for non-existent event

    // Test RemoveCheckBundleMsg
    std::string msgKey = handler->GetMsgKey(BundleActiveGroupHandler::MSG_CHECK_DEFAULT_BUNDLE_STATE,
        handlerObjPtr, K_DEFAULT_DELAY);
    handler->RemoveCheckBundleMsg(msgKey);
    handler->RemoveCheckBundleMsg("non_existent_key");  // no-op path

    // Test GetMsgKey with nullptr -> returns ""
    std::shared_ptr<BundleActiveGroupHandlerObject> nullHandlerObj = nullptr;
    std::string emptyKey = handler->GetMsgKey(
        BundleActiveGroupHandler::MSG_CHECK_DEFAULT_BUNDLE_STATE, nullHandlerObj, K_DEFAULT_DELAY);
    (void) emptyKey;  // unused but exercises the nullptr check

    return true;
}

/* FUZZ_CODE_17: RestoreDurationToDatabase and RestoreToDatabase */
bool GroupControllerRestoreToDatabaseFuzzTest(FuzzedDataProvider* fdp)
{
    int32_t userId = fdp->ConsumeIntegral<int32_t>();
    BundleActiveGroupController::GetInstance().RestoreToDatabase(userId);
    BundleActiveGroupController::GetInstance().RestoreDurationToDatabase();
    return true;
}

/* FUZZ_CODE_18: IsUsedOverOneWeek detailed branches */
bool GroupControllerIsUsedOverOneWeekFuzzTest(FuzzedDataProvider* fdp)
{
    std::string bundle = fdp->ConsumeRandomLengthString();
    int32_t userId = fdp->ConsumeIntegral<int32_t>();
    BundleActiveGroupController::GetInstance().IsUsedOverOneWeek(bundle, userId);
    return true;
}

/* -------------------------------------------------------
 * Dispatch helpers (split to satisfy G.FUD.05 line/complexity limits)
 * ------------------------------------------------------- */
static void FuzzGroupControllerPart1(int code, FuzzedDataProvider& fdp)
{
    switch (code) {
        case FUZZ_CODE_0:
            GroupControllerInitFuzzTest(&fdp);
            break;
        case FUZZ_CODE_1:
            GroupControllerReportEventFuzzTest(&fdp);
            break;
        case FUZZ_CODE_2:
            GroupControllerCheckAndUpdateGroupFuzzTest(&fdp);
            break;
        case FUZZ_CODE_3:
            GroupControllerCheckEachBundleStateFuzzTest(&fdp);
            break;
        case FUZZ_CODE_4:
            GroupControllerCheckIdleStatsOneTimeFuzzTest(&fdp);
            break;
        case FUZZ_CODE_5:
            GroupControllerPeriodCheckFuzzTest(&fdp);
            break;
        case FUZZ_CODE_6:
            GroupControllerOnUserRemovedFuzzTest(&fdp);
            break;
        case FUZZ_CODE_7:
            GroupControllerOnBundleUninstalledFuzzTest(&fdp);
            break;
        case FUZZ_CODE_8:
            GroupControllerOnScreenChangedFuzzTest(&fdp);
            break;
        case FUZZ_CODE_9:
            GroupControllerSetAppGroupFuzzTest(&fdp);
            break;
        default:
            break;
    }
}

static void FuzzGroupControllerPart2(int code, FuzzedDataProvider& fdp)
{
    switch (code) {
        case FUZZ_CODE_10:
            GroupControllerIsBundleIdleFuzzTest(&fdp);
            break;
        case FUZZ_CODE_11:
            GroupControllerQueryAppGroupFuzzTest(&fdp);
            break;
        case FUZZ_CODE_12:
            GroupControllerMiscFuzzTest(&fdp);
            break;
        case FUZZ_CODE_13:
            GroupControllerReportEventSpecificIdsFuzzTest(&fdp);
            break;
        case FUZZ_CODE_14:
            GroupControllerDeInitFuzzTest(&fdp);
            break;
        case FUZZ_CODE_15:
            GroupControllerShutDownFuzzTest(&fdp);
            break;
        case FUZZ_CODE_16:
            GroupControllerHandlerProcessEventFuzzTest(&fdp);
            break;
        case FUZZ_CODE_17:
            GroupControllerRestoreToDatabaseFuzzTest(&fdp);
            break;
        case FUZZ_CODE_18:
            GroupControllerIsUsedOverOneWeekFuzzTest(&fdp);
            break;
        default:
            break;
    }
}

/* -------------------------------------------------------
 * Dispatch
 * ------------------------------------------------------- */
void FuzzGroupController(FuzzedDataProvider& fdp)
{
    int code = fdp.ConsumeIntegralInRange(FUZZ_CODE_0, FUZZ_CODE_MAX);
    FuzzGroupControllerPart1(code, fdp);
    FuzzGroupControllerPart2(code, fdp);
}

} // namespace DeviceUsageStats
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Set native token permission */
    OHOS::DelayedSingleton<OHOS::DeviceUsageStats::FuzztestHelper>::GetInstance()->NativeTokenGet();

    FuzzedDataProvider fdp(data, size);
    OHOS::DeviceUsageStats::FuzzGroupController(fdp);
    return 0;
}
