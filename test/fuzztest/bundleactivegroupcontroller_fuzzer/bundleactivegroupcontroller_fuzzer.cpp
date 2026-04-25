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
constexpr int32_t FUZZ_CODE_MAX = 12;

/* -------------------------------------------------------
 * BundleActiveGroupController fuzz helpers
 * ------------------------------------------------------- */
bool GroupControllerInitFuzzTest(FuzzedDataProvider* fdp)
{
    bool debug = fdp->ConsumeBool();
    BundleActiveGroupController::GetInstance().Init(debug);
    return true;
}

bool GroupControllerReportEventFuzzTest(FuzzedDataProvider* fdp)
{
    BundleActiveEvent event;
    event.bundleName_     = fdp->ConsumeRandomLengthString();
    event.eventId_        = fdp->ConsumeIntegral<int32_t>();
    event.timeStamp_      = fdp->ConsumeIntegral<int64_t>();
    event.abilityName_    = fdp->ConsumeRandomLengthString();
    event.uid_            = fdp->ConsumeIntegral<int32_t>();
    int64_t bootTs        = fdp->ConsumeIntegral<int64_t>();
    int32_t userId        = fdp->ConsumeIntegral<int32_t>();
    BundleActiveGroupController::GetInstance().ReportEvent(event, bootTs, userId);
    return true;
}

bool GroupControllerCheckAndUpdateGroupFuzzTest(FuzzedDataProvider* fdp)
{
    std::string bundleName = fdp->ConsumeRandomLengthString();
    int32_t userId         = fdp->ConsumeIntegral<int32_t>();
    int32_t uid            = fdp->ConsumeIntegral<int32_t>();
    int64_t bootTs         = fdp->ConsumeIntegral<int64_t>();
    BundleActiveGroupController::GetInstance().CheckAndUpdateGroup(bundleName, userId, uid, bootTs);
    return true;
}

bool GroupControllerCheckEachBundleStateFuzzTest(FuzzedDataProvider* fdp)
{
    int32_t userId = fdp->ConsumeIntegral<int32_t>();
    BundleActiveGroupController::GetInstance().CheckEachBundleState(userId);
    return true;
}

bool GroupControllerCheckIdleStatsOneTimeFuzzTest(FuzzedDataProvider* /*fdp*/)
{
    BundleActiveGroupController::GetInstance().CheckIdleStatsOneTime();
    return true;
}

bool GroupControllerPeriodCheckFuzzTest(FuzzedDataProvider* fdp)
{
    int32_t userId = fdp->ConsumeIntegral<int32_t>();
    BundleActiveGroupController::GetInstance().PeriodCheckBundleState(userId);
    return true;
}

bool GroupControllerOnUserRemovedFuzzTest(FuzzedDataProvider* fdp)
{
    int32_t userId = fdp->ConsumeIntegral<int32_t>();
    BundleActiveGroupController::GetInstance().OnUserRemoved(userId);
    return true;
}

bool GroupControllerOnBundleUninstalledFuzzTest(FuzzedDataProvider* fdp)
{
    int32_t userId     = fdp->ConsumeIntegral<int32_t>();
    std::string bundle = fdp->ConsumeRandomLengthString();
    int32_t uid        = fdp->ConsumeIntegral<int32_t>();
    int32_t appIndex   = fdp->ConsumeIntegral<int32_t>();
    BundleActiveGroupController::GetInstance().OnBundleUninstalled(userId, bundle, uid, appIndex);
    return true;
}

bool GroupControllerOnScreenChangedFuzzTest(FuzzedDataProvider* fdp)
{
    bool isScreenOn = fdp->ConsumeBool();
    int64_t bootTs  = fdp->ConsumeIntegral<int64_t>();
    BundleActiveGroupController::GetInstance().OnScreenChanged(isScreenOn, bootTs);
    return true;
}

bool GroupControllerSetAppGroupFuzzTest(FuzzedDataProvider* fdp)
{
    std::string bundle = fdp->ConsumeRandomLengthString();
    int32_t userId     = fdp->ConsumeIntegral<int32_t>();
    int32_t newGroup   = fdp->ConsumeIntegral<int32_t>();
    uint32_t reason    = fdp->ConsumeIntegral<uint32_t>();
    int64_t bootTs     = fdp->ConsumeIntegral<int64_t>();
    bool isFlush       = fdp->ConsumeBool();
    BundleActiveGroupController::GetInstance().SetAppGroup(bundle, userId, newGroup, reason, bootTs, isFlush);
    return true;
}

bool GroupControllerIsBundleIdleFuzzTest(FuzzedDataProvider* fdp)
{
    std::string bundle = fdp->ConsumeRandomLengthString();
    int32_t userId     = fdp->ConsumeIntegral<int32_t>();
    BundleActiveGroupController::GetInstance().IsBundleIdle(bundle, userId);
    return true;
}

bool GroupControllerQueryAppGroupFuzzTest(FuzzedDataProvider* fdp)
{
    int32_t appGroup   = 0;
    std::string bundle = fdp->ConsumeRandomLengthString();
    int32_t userId     = fdp->ConsumeIntegral<int32_t>();
    BundleActiveGroupController::GetInstance().QueryAppGroup(appGroup, bundle, userId);
    return true;
}

bool GroupControllerMiscFuzzTest(FuzzedDataProvider* fdp)
{
    // IsBundleInstalled
    std::string bundle = fdp->ConsumeRandomLengthString();
    int32_t userId     = fdp->ConsumeIntegral<int32_t>();
    BundleActiveGroupController::GetInstance().IsBundleInstalled(bundle, userId);

    // IsScreenOn
    BundleActiveGroupController::GetInstance().IsScreenOn();

    // SetBundleGroupEnable / GetBundleGroupEnable
    bool enable = fdp->ConsumeBool();
    BundleActiveGroupController::GetInstance().SetBundleGroupEnable(enable);
    BundleActiveGroupController::GetInstance().GetBundleGroupEnable();

    // IsUsedOverOneWeek
    BundleActiveGroupController::GetInstance().IsUsedOverOneWeek(bundle, userId);

    // OnUserSwitched
    int32_t currentUser = fdp->ConsumeIntegral<int32_t>();
    BundleActiveGroupController::GetInstance().OnUserSwitched(userId, currentUser);

    // GetBundleGroupHandler
    BundleActiveGroupController::GetInstance().GetBundleGroupHandler();

    return true;
}

/* -------------------------------------------------------
 * Dispatch
 * ------------------------------------------------------- */
void FuzzGroupController(FuzzedDataProvider& fdp)
{
    int code = fdp.ConsumeIntegralInRange(FUZZ_CODE_0, FUZZ_CODE_MAX);
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
        case FUZZ_CODE_10:
            GroupControllerIsBundleIdleFuzzTest(&fdp);
            break;
        case FUZZ_CODE_11:
            GroupControllerQueryAppGroupFuzzTest(&fdp);
            break;
        case FUZZ_CODE_12:
            GroupControllerMiscFuzzTest(&fdp);
            break;
        default:
            break;
    }
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
