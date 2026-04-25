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

#include "bundleactiveperiodstats_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>

#include "bundle_active_period_stats.h"
#include "bundle_active_event.h"
#include "bundle_active_event_stats.h"
#include "bundle_active_package_stats.h"
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
constexpr int32_t FUZZ_CODE_MAX = 8;

// Magic numbers for timestamp offsets
constexpr int64_t kTsOffset1 = 1;
constexpr int64_t kTsOffset2 = 2;
constexpr int64_t kTsOffset3 = 3;
constexpr int64_t kTsOffset4 = 4;

// PERIOD_* constants for PeriodStatsConstantsFuzzTest
constexpr int32_t kPeriodDaily = 0;
constexpr int32_t kPeriodWeekly = 1;
constexpr int32_t kPeriodMonthly = 2;
constexpr int32_t kPeriodYearly = 3;

// Magic number for event count in AddEvent test
constexpr int32_t kEventCountMax = 5;

/* Build a fuzzed BundleActiveEvent */
static BundleActiveEvent BuildFuzzEvent(FuzzedDataProvider* fdp)
{
    BundleActiveEvent event;
    event.bundleName_  = fdp->ConsumeRandomLengthString();
    event.moduleName_  = fdp->ConsumeRandomLengthString();
    event.abilityName_ = fdp->ConsumeRandomLengthString();
    event.eventId_     = fdp->ConsumeIntegral<int32_t>();
    event.timeStamp_   = fdp->ConsumeIntegral<int64_t>();
    event.uid_         = fdp->ConsumeIntegral<int32_t>();
    return event;
}

/* -------------------------------------------------------
 * BundleActivePeriodStats fuzz helpers
 * ------------------------------------------------------- */

/* Test constructors + GetOrCreateUsageStats */
bool PeriodStatsConstructFuzzTest(FuzzedDataProvider* fdp)
{
    // Default constructor
    BundleActivePeriodStats defaultStats;

    // Parameterized constructor
    int32_t userId    = fdp->ConsumeIntegral<int32_t>();
    int64_t beginTime = fdp->ConsumeIntegral<int64_t>();
    BundleActivePeriodStats stats(userId, beginTime);

    // GetOrCreateUsageStats
    std::string bundleName = fdp->ConsumeRandomLengthString();
    int32_t uid            = fdp->ConsumeIntegral<int32_t>();
    auto pkgStats = stats.GetOrCreateUsageStats(bundleName, uid);
    (void)pkgStats;

    // GetCachedString
    std::string cachedStr = fdp->ConsumeRandomLengthString();
    stats.GetCachedString(cachedStr);
    return true;
}

/* Test Update with various event types */
bool PeriodStatsUpdateFuzzTest(FuzzedDataProvider* fdp)
{
    int32_t userId    = fdp->ConsumeIntegral<int32_t>();
    int64_t beginTime = fdp->ConsumeIntegral<int64_t>();
    BundleActivePeriodStats stats(userId, beginTime);

    std::string bundleName      = fdp->ConsumeRandomLengthString();
    std::string longTimeTaskName = fdp->ConsumeRandomLengthString();
    int64_t timeStamp           = fdp->ConsumeIntegral<int64_t>();
    int32_t eventId             = fdp->ConsumeIntegral<int32_t>();
    std::string abilityId       = fdp->ConsumeRandomLengthString();
    int32_t uid                 = fdp->ConsumeIntegral<int32_t>();

    stats.Update(bundleName, longTimeTaskName, timeStamp, eventId, abilityId, uid);

    // Update again with a different event (ABILITY_FOREGROUND = 2, ABILITY_BACKGROUND = 3)
    stats.Update(bundleName, "", timeStamp + kTsOffset1, BundleActiveEvent::ABILITY_FOREGROUND, abilityId, uid);
    stats.Update(bundleName, "", timeStamp + kTsOffset2, BundleActiveEvent::ABILITY_BACKGROUND, abilityId, uid);
    stats.Update(bundleName, longTimeTaskName, timeStamp + kTsOffset3,
        BundleActiveEvent::LONG_TIME_TASK_STARTTED, abilityId, uid);
    stats.Update(bundleName, longTimeTaskName, timeStamp + kTsOffset4,
        BundleActiveEvent::LONG_TIME_TASK_ENDED, abilityId, uid);
    return true;
}

/* Test AddEvent */
bool PeriodStatsAddEventFuzzTest(FuzzedDataProvider* fdp)
{
    int32_t userId    = fdp->ConsumeIntegral<int32_t>();
    int64_t beginTime = fdp->ConsumeIntegral<int64_t>();
    BundleActivePeriodStats stats(userId, beginTime);

    BundleActiveEvent event = BuildFuzzEvent(fdp);
    stats.AddEvent(event);

    // Add multiple events
    int32_t count = fdp->ConsumeIntegralInRange<int32_t>(0, kEventCountMax);
    for (int32_t i = 0; i < count; ++i) {
        BundleActiveEvent ev = BuildFuzzEvent(fdp);
        stats.AddEvent(ev);
    }
    return true;
}

/* Test UpdateScreen* and UpdateKeyguard* */
bool PeriodStatsScreenAndKeyguardFuzzTest(FuzzedDataProvider* fdp)
{
    int32_t userId    = fdp->ConsumeIntegral<int32_t>();
    int64_t beginTime = fdp->ConsumeIntegral<int64_t>();
    BundleActivePeriodStats stats(userId, beginTime);

    int64_t ts1 = fdp->ConsumeIntegral<int64_t>();
    int64_t ts2 = fdp->ConsumeIntegral<int64_t>();
    int64_t ts3 = fdp->ConsumeIntegral<int64_t>();
    int64_t ts4 = fdp->ConsumeIntegral<int64_t>();
    int64_t ts5 = fdp->ConsumeIntegral<int64_t>();

    stats.UpdateScreenInteractive(ts1);
    stats.UpdateScreenNonInteractive(ts2);
    stats.UpdateKeyguardShown(ts3);
    stats.UpdateKeyguardHidden(ts4);
    stats.CommitTime(ts5);
    return true;
}

/* Test AddEventStatsTo */
bool PeriodStatsAddEventStatsToFuzzTest(FuzzedDataProvider* fdp)
{
    int32_t userId    = fdp->ConsumeIntegral<int32_t>();
    int64_t beginTime = fdp->ConsumeIntegral<int64_t>();
    BundleActivePeriodStats stats(userId, beginTime);

    // Populate trackers by calling screen/keyguard methods first
    int64_t ts = fdp->ConsumeIntegral<int64_t>();
    stats.UpdateScreenInteractive(ts);
    stats.UpdateScreenNonInteractive(ts + kTsOffset1);
    stats.UpdateKeyguardShown(ts + kTsOffset2);
    stats.UpdateKeyguardHidden(ts + kTsOffset3);
    stats.CommitTime(ts + kTsOffset4);

    std::vector<BundleActiveEventStats> eventStatsList;
    stats.AddEventStatsTo(eventStatsList);
    return true;
}

/* Stress: repeat Updates + AddEvents to trigger packetNamesCache_ */
bool PeriodStatsStressFuzzTest(FuzzedDataProvider* fdp)
{
    int32_t userId    = fdp->ConsumeIntegral<int32_t>();
    int64_t beginTime = fdp->ConsumeIntegral<int64_t>();
    BundleActivePeriodStats stats(userId, beginTime);

    int32_t iterations = fdp->ConsumeIntegralInRange<int32_t>(0, 10);
    for (int32_t i = 0; i < iterations; ++i) {
        std::string bundle = fdp->ConsumeRandomLengthString();
        int64_t ts         = fdp->ConsumeIntegral<int64_t>();
        int32_t evId       = fdp->ConsumeIntegral<int32_t>();
        int32_t uid        = fdp->ConsumeIntegral<int32_t>();
        stats.Update(bundle, "", ts, evId, "", uid);

        BundleActiveEvent event = BuildFuzzEvent(fdp);
        stats.AddEvent(event);
    }
    return true;
}

/* Test BundleActivePeriodStats with pre-populated bundleStats_ (Marshalling path via PackageStats) */
bool PeriodStatsBundleStatsMarshallingFuzzTest(FuzzedDataProvider* fdp)
{
    int32_t userId    = fdp->ConsumeIntegral<int32_t>();
    int64_t beginTime = fdp->ConsumeIntegral<int64_t>();
    BundleActivePeriodStats stats(userId, beginTime);

    // Create and manipulate package stats
    std::string bundleName = fdp->ConsumeRandomLengthString();
    int32_t uid            = fdp->ConsumeIntegral<int32_t>();
    auto pkgStats = stats.GetOrCreateUsageStats(bundleName, uid);
    if (pkgStats != nullptr) {
        pkgStats->bundleName_    = bundleName;
        pkgStats->beginTimeStamp_ = beginTime;
        pkgStats->endTimeStamp_   = fdp->ConsumeIntegral<int64_t>();
        pkgStats->lastTimeUsed_   = fdp->ConsumeIntegral<int64_t>();
        pkgStats->totalInFrontTime_ = fdp->ConsumeIntegral<int64_t>();
        pkgStats->startCount_     = fdp->ConsumeIntegral<int32_t>();

        // Marshalling / Unmarshalling
        Parcel parcel;
        pkgStats->Marshalling(parcel);
        BundleActivePackageStats* restored = BundleActivePackageStats::Unmarshalling(parcel);
        if (restored != nullptr) {
            restored->ToString();
            delete restored;
        }
    }
    return true;
}

/* Test PERIOD_* constants coverage */
bool PeriodStatsConstantsFuzzTest(FuzzedDataProvider* fdp)
{
    // Exercise all PERIOD_* paths through Stats construction
    int64_t ts = fdp->ConsumeIntegral<int64_t>();
    {
        BundleActivePeriodStats daily(kPeriodDaily, ts);
        daily.UpdateScreenInteractive(ts);
        daily.CommitTime(ts + kTsOffset1);
    }
    {
        BundleActivePeriodStats weekly(kPeriodWeekly, ts);
        weekly.UpdateKeyguardShown(ts);
        weekly.CommitTime(ts + kTsOffset1);
    }
    {
        BundleActivePeriodStats monthly(kPeriodMonthly, ts);
        monthly.UpdateScreenNonInteractive(ts);
        monthly.CommitTime(ts + kTsOffset1);
    }
    {
        BundleActivePeriodStats yearly(kPeriodYearly, ts);
        yearly.UpdateKeyguardHidden(ts);
        yearly.CommitTime(ts + kTsOffset1);
    }
    return true;
}

/* -------------------------------------------------------
 * Dispatch
 * ------------------------------------------------------- */
void FuzzPeriodStats(FuzzedDataProvider& fdp)
{
    int code = fdp.ConsumeIntegralInRange(FUZZ_CODE_0, FUZZ_CODE_MAX);
    switch (code) {
        case FUZZ_CODE_0:
            PeriodStatsConstructFuzzTest(&fdp);
            break;
        case FUZZ_CODE_1:
            PeriodStatsUpdateFuzzTest(&fdp);
            break;
        case FUZZ_CODE_2:
            PeriodStatsAddEventFuzzTest(&fdp);
            break;
        case FUZZ_CODE_3:
            PeriodStatsScreenAndKeyguardFuzzTest(&fdp);
            break;
        case FUZZ_CODE_4:
            PeriodStatsAddEventStatsToFuzzTest(&fdp);
            break;
        case FUZZ_CODE_5:
            PeriodStatsStressFuzzTest(&fdp);
            break;
        case FUZZ_CODE_6:
            PeriodStatsBundleStatsMarshallingFuzzTest(&fdp);
            break;
        case FUZZ_CODE_7:
            PeriodStatsConstantsFuzzTest(&fdp);
            break;
        case FUZZ_CODE_8:
            // Chain: construct -> update -> add events -> screen changes -> stats
            PeriodStatsUpdateFuzzTest(&fdp);
            PeriodStatsScreenAndKeyguardFuzzTest(&fdp);
            PeriodStatsAddEventStatsToFuzzTest(&fdp);
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
    OHOS::DelayedSingleton<OHOS::DeviceUsageStats::FuzztestHelper>::GetInstance()->NativeTokenGet();

    FuzzedDataProvider fdp(data, size);
    OHOS::DeviceUsageStats::FuzzPeriodStats(fdp);
    return 0;
}
