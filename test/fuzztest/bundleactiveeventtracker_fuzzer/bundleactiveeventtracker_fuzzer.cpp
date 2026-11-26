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

#include "bundleactiveeventtracker_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>

#include "bundle_active_constant.h"
#include "bundle_active_event.h"
#include "bundle_active_event_stats.h"
#include "bundle_active_event_tracker.h"
#include "fuzztest_helper.h"

namespace OHOS {
namespace DeviceUsageStats {

constexpr int32_t FUZZ_CODE_0 = 0;
constexpr int32_t FUZZ_CODE_1 = 1;
constexpr int32_t FUZZ_CODE_2 = 2;
constexpr int32_t FUZZ_CODE_3 = 3;
constexpr int32_t FUZZ_CODE_4 = 4;
constexpr int32_t FUZZ_CODE_5 = 5;
constexpr int32_t FUZZ_CODE_6 = 6;
constexpr int32_t FUZZ_CODE_MAX = 6;
constexpr int32_t K_CYCLE_DURATION_MS = 100;  // 每个时间（毫秒）
constexpr int32_t K_OFFSET_MS = 50;          // 时间偏移量（毫秒）

// Magic number for empty tracker test time range
constexpr int64_t K_EMPTY_TRACKER_TIME_RANGE = 1000;

/* =============================================================
 *  Test 0: Default constructor (curStartTime_=0 path)
 * ============================================================= */
bool TrackerCtorFuzzTest(FuzzedDataProvider* /*fdp*/)
{
    BundleActiveEventTracker tracker;
    // Fields initialized: curStartTime_=0, lastEventTime_=0, duration_=0, count_=0
    return true;
}

/* =============================================================
 *  Test 1: Update() with curStartTime_=0 (first call, count++ path)
 * ============================================================= */
bool TrackerUpdateFirstCallFuzzTest(FuzzedDataProvider* fdp)
{
    BundleActiveEventTracker tracker;
    int64_t ts = fdp->ConsumeIntegral<int64_t>();
    tracker.Update(ts);
    // After first call: curStartTime_=ts, count_=1, lastEventTime_=ts, duration_=0
    return true;
}

/* =============================================================
 *  Test 2: Update() called twice (second call, curStartTime_!=0 path)
 * ============================================================= */
bool TrackerUpdateSecondCallFuzzTest(FuzzedDataProvider* fdp)
{
    BundleActiveEventTracker tracker;
    int64_t ts1 = fdp->ConsumeIntegral<int64_t>();
    int64_t ts2 = fdp->ConsumeIntegral<int64_t>();
    tracker.Update(ts1);
    tracker.Update(ts2);
    // duration_ += ts2 - ts1, curStartTime_ = ts2
    return true;
}

/* =============================================================
 *  Test 3: CommitTime() with curStartTime_=0 (no-op branch)
 * ============================================================= */
bool TrackerCommitTimeNoOpFuzzTest(FuzzedDataProvider* fdp)
{
    BundleActiveEventTracker tracker;
    int64_t ts = fdp->ConsumeIntegral<int64_t>();
    // curStartTime_=0, CommitTime should do nothing
    tracker.CommitTime(ts);
    return true;
}

/* =============================================================
 *  Test 4: Update() then CommitTime() (curStartTime_!=0 branch)
 * ============================================================= */
bool TrackerUpdateThenCommitFuzzTest(FuzzedDataProvider* fdp)
{
    BundleActiveEventTracker tracker;
    int64_t ts1 = fdp->ConsumeIntegral<int64_t>();
    int64_t ts2 = fdp->ConsumeIntegral<int64_t>();
    tracker.Update(ts1);
    tracker.CommitTime(ts2);
    // duration_ += ts2 - ts1, curStartTime_ = 0
    return true;
}

/* =============================================================
 *  Test 5: AddToEventStats with non-zero count/duration
 * ============================================================= */
bool TrackerAddToEventStatsFuzzTest(FuzzedDataProvider* fdp)
{
    BundleActiveEventTracker tracker;
    int64_t ts1 = fdp->ConsumeIntegral<int64_t>();
    int64_t ts2 = fdp->ConsumeIntegral<int64_t>();
    int64_t beginTime = fdp->ConsumeIntegral<int64_t>();
    int64_t endTime = fdp->ConsumeIntegral<int64_t>();
    int32_t eventId = fdp->ConsumeIntegral<int32_t>();

    tracker.Update(ts1);
    tracker.CommitTime(ts2);

    // AddToEventStats only adds entry if count_ || duration_ != 0
    std::vector<BundleActiveEventStats> statsList;
    tracker.AddToEventStats(statsList, eventId, beginTime, endTime);
    (void)statsList;
    return true;
}

/* =============================================================
 *  Test 6: Stress - repeated Update/Commit cycles, AddToEventStats
 * ============================================================= */
bool TrackerStressFuzzTest(FuzzedDataProvider* fdp)
{
    BundleActiveEventTracker tracker;
    int32_t cycles = fdp->ConsumeIntegralInRange<int32_t>(1, 10);
    int64_t baseTs = fdp->ConsumeIntegral<int64_t>();

    for (int32_t i = 0; i < cycles; ++i) {
        tracker.Update(baseTs + i * K_CYCLE_DURATION_MS);
        tracker.CommitTime(baseTs + i * K_CYCLE_DURATION_MS + K_OFFSET_MS);
    }

    std::vector<BundleActiveEventStats> statsList;
    tracker.AddToEventStats(statsList, BundleActiveEvent::SCREEN_INTERACTIVE,
                            baseTs, baseTs + cycles * K_CYCLE_DURATION_MS);

    // Empty tracker - should not add entry
    BundleActiveEventTracker emptyTracker;
    std::vector<BundleActiveEventStats> emptyStatsList;
    emptyTracker.AddToEventStats(emptyStatsList, BundleActiveEvent::SCREEN_NON_INTERACTIVE,
                                 baseTs, baseTs + K_EMPTY_TRACKER_TIME_RANGE);
    (void)emptyStatsList;
    return true;
}

/* -------------------------------------------------------
 * Dispatch
 * ------------------------------------------------------- */
void FuzzEventTracker(FuzzedDataProvider& fdp)
{
    int code = fdp.ConsumeIntegralInRange(FUZZ_CODE_0, FUZZ_CODE_MAX);
    switch (code) {
        case FUZZ_CODE_0:
            TrackerCtorFuzzTest(&fdp);
            break;
        case FUZZ_CODE_1:
            TrackerUpdateFirstCallFuzzTest(&fdp);
            break;
        case FUZZ_CODE_2:
            TrackerUpdateSecondCallFuzzTest(&fdp);
            break;
        case FUZZ_CODE_3:
            TrackerCommitTimeNoOpFuzzTest(&fdp);
            break;
        case FUZZ_CODE_4:
            TrackerUpdateThenCommitFuzzTest(&fdp);
            break;
        case FUZZ_CODE_5:
            TrackerAddToEventStatsFuzzTest(&fdp);
            break;
        case FUZZ_CODE_6:
            TrackerStressFuzzTest(&fdp);
            break;
        default:
            break;
    }
}

}  // namespace DeviceUsageStats
}  // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DelayedSingleton<OHOS::DeviceUsageStats::FuzztestHelper>::GetInstance()->NativeTokenGet();

    FuzzedDataProvider fdp(data, size);
    OHOS::DeviceUsageStats::FuzzEventTracker(fdp);
    return 0;
}
