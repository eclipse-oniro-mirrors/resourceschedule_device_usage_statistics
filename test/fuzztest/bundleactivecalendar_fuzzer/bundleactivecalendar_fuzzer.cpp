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

#include "bundleactivecalendar_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>

#include "bundle_active_calendar.h"
#include "bundle_active_util.h"
#include "bundle_active_constant.h"
#include "bundle_active_period_stats.h"
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

// Magic numbers for calendar operation test ranges
constexpr int64_t kIncreaseValueMax = 100;

/* =============================================================
 *  Test 0: Default constructor + parameterized constructor
 * ============================================================= */
bool CalendarCtorFuzzTest(FuzzedDataProvider* fdp)
{
    BundleActiveCalendar defaultCal;
    int64_t ts = fdp->ConsumeIntegral<int64_t>();
    BundleActiveCalendar cal(ts);
    (void)defaultCal.GetMilliseconds();
    (void)cal.GetMilliseconds();
    return true;
}

/* =============================================================
 *  Test 1: ChangeToDebug() + SetMilliseconds/GetMilliseconds
 * ============================================================= */
bool CalendarDebugModeFuzzTest(FuzzedDataProvider* fdp)
{
    BundleActiveCalendar cal(fdp->ConsumeIntegral<int64_t>());
    cal.ChangeToDebug();
    int64_t ts = fdp->ConsumeIntegral<int64_t>();
    cal.SetMilliseconds(ts);
    (void)cal.GetMilliseconds();
    // ChangeToDebug sets debug_=true and uses debug time constants
    // All TruncateTo* operations will use: time_ -= time_ % periodMs
    return true;
}

/* =============================================================
 *  Test 2: TruncateToDay (debug mode: time -= time % ONE_DAY_TIME_DEBUG)
 * ============================================================= */
bool CalendarTruncateDayFuzzTest(FuzzedDataProvider* fdp)
{
    BundleActiveCalendar cal(fdp->ConsumeIntegral<int64_t>());
    cal.ChangeToDebug();
    int64_t ts = fdp->ConsumeIntegral<int64_t>();
    cal.SetMilliseconds(ts);
    cal.TruncateToDay();
    cal.TruncateToDay();  // call twice

    // Also test without debug
    BundleActiveCalendar cal2(fdp->ConsumeIntegral<int64_t>());
    cal2.SetMilliseconds(ts);
    cal2.TruncateToDay();
    (void)cal2.GetMilliseconds();
    return true;
}

/* =============================================================
 *  Test 3: TruncateToWeek/Month/Year (all TruncateTo* methods)
 * ============================================================= */
bool CalendarTruncateOtherFuzzTest(FuzzedDataProvider* fdp)
{
    BundleActiveCalendar cal(fdp->ConsumeIntegral<int64_t>());
    cal.ChangeToDebug();
    int64_t ts = fdp->ConsumeIntegral<int64_t>();
    cal.SetMilliseconds(ts);

    cal.TruncateToWeek();
    cal.TruncateToMonth();
    cal.TruncateToYear();

    // Also test without debug
    BundleActiveCalendar cal2(fdp->ConsumeIntegral<int64_t>());
    cal2.SetMilliseconds(ts);
    cal2.TruncateToWeek();
    cal2.TruncateToMonth();
    cal2.TruncateToYear();
    (void)cal2.GetMilliseconds();
    return true;
}

/* =============================================================
 *  Test 4: IncreaseDays/Weeks/Months/Years
 * ============================================================= */
bool CalendarIncreaseFuzzTest(FuzzedDataProvider* fdp)
{
    BundleActiveCalendar cal(fdp->ConsumeIntegral<int64_t>());
    int64_t val = fdp->ConsumeIntegral<int64_t>();
    int64_t negVal = fdp->ConsumeIntegral<int64_t>();

    cal.IncreaseDays(val);
    cal.IncreaseDays(negVal);
    cal.IncreaseWeeks(val);
    cal.IncreaseWeeks(negVal);
    cal.IncreaseMonths(val);
    cal.IncreaseMonths(negVal);
    cal.IncreaseYears(val);
    cal.IncreaseYears(negVal);
    (void)cal.GetMilliseconds();
    return true;
}

/* =============================================================
 *  Test 5: TruncateTo(int32_t intervalType) - switch coverage
 * ============================================================= */
bool CalendarTruncateToFuzzTest(FuzzedDataProvider* fdp)
{
    BundleActiveCalendar cal(fdp->ConsumeIntegral<int64_t>());
    cal.ChangeToDebug();
    int64_t ts = fdp->ConsumeIntegral<int64_t>();
    cal.SetMilliseconds(ts);

    // All four PERIOD_* cases
    cal.TruncateTo(BundleActivePeriodStats::PERIOD_DAILY);
    cal.TruncateTo(BundleActivePeriodStats::PERIOD_WEEKLY);
    cal.TruncateTo(BundleActivePeriodStats::PERIOD_MONTHLY);
    cal.TruncateTo(BundleActivePeriodStats::PERIOD_YEARLY);

    // Default case (invalid interval type)
    int32_t invalidType = fdp->ConsumeIntegral<int32_t>();
    // Filter to non-matching values
    if (invalidType < 0 || invalidType > BundleActivePeriodStats::PERIOD_YEARLY) {
        cal.TruncateTo(invalidType);
    }

    // Non-debug path
    BundleActiveCalendar cal2(fdp->ConsumeIntegral<int64_t>());
    cal2.SetMilliseconds(ts);
    cal2.TruncateTo(BundleActivePeriodStats::PERIOD_DAILY);
    cal2.TruncateTo(BundleActivePeriodStats::PERIOD_WEEKLY);
    cal2.TruncateTo(BundleActivePeriodStats::PERIOD_MONTHLY);
    cal2.TruncateTo(BundleActivePeriodStats::PERIOD_YEARLY);
    (void)cal2.GetMilliseconds();
    return true;
}

/* =============================================================
 *  Test 6: IncreaseDays + TruncateToDay (chained operations)
 * ============================================================= */
bool CalendarChainOperationsFuzzTest(FuzzedDataProvider* fdp)
{
    BundleActiveCalendar cal(fdp->ConsumeIntegral<int64_t>());
    cal.ChangeToDebug();
    int64_t ts = fdp->ConsumeIntegral<int64_t>();
    cal.SetMilliseconds(ts);

    // Chain: IncreaseDays -> TruncateToDay -> IncreaseWeeks -> TruncateToWeek
    int64_t incVal = fdp->ConsumeIntegralInRange<int64_t>(1, kIncreaseValueMax);
    cal.IncreaseDays(incVal);
    (void)cal.GetMilliseconds();
    cal.TruncateToDay();
    (void)cal.GetMilliseconds();
    cal.IncreaseWeeks(incVal);
    (void)cal.GetMilliseconds();
    cal.TruncateToWeek();
    (void)cal.GetMilliseconds();

    // Non-debug path
    BundleActiveCalendar cal2(fdp->ConsumeIntegral<int64_t>());
    cal2.SetMilliseconds(ts);
    cal2.IncreaseMonths(incVal);
    (void)cal2.GetMilliseconds();
    cal2.TruncateToMonth();
    (void)cal2.GetMilliseconds();
    return true;
}

/* =============================================================
 *  Test 7: Large and extreme values
 * ============================================================= */
bool CalendarEdgeCasesFuzzTest(FuzzedDataProvider* fdp)
{
    BundleActiveCalendar cal(0);
    cal.SetMilliseconds(0);
    (void)cal.GetMilliseconds();

    cal.SetMilliseconds(INT64_MAX);
    cal.TruncateToDay();
    cal.TruncateToWeek();
    cal.TruncateToMonth();
    cal.TruncateToYear();
    cal.IncreaseDays(1);
    cal.IncreaseWeeks(1);
    cal.IncreaseMonths(1);
    cal.IncreaseYears(1);
    (void)cal.GetMilliseconds();

    // Zero Increase
    cal.IncreaseDays(0);
    cal.IncreaseWeeks(0);
    cal.IncreaseMonths(0);
    cal.IncreaseYears(0);

    return true;
}

/* =============================================================
 *  Test 8: Complex - Build timestamp then do all operations
 * ============================================================= */
bool CalendarComplexFuzzTest(FuzzedDataProvider* fdp)
{
    int64_t baseTs = fdp->ConsumeIntegral<int64_t>();

    // Normal mode
    BundleActiveCalendar normalCal(baseTs);
    normalCal.SetMilliseconds(baseTs);
    normalCal.IncreaseDays(fdp->ConsumeIntegral<int64_t>());
    normalCal.TruncateToDay();
    normalCal.IncreaseWeeks(fdp->ConsumeIntegral<int64_t>());
    normalCal.TruncateToWeek();
    normalCal.IncreaseMonths(fdp->ConsumeIntegral<int64_t>());
    normalCal.TruncateToMonth();
    normalCal.IncreaseYears(fdp->ConsumeIntegral<int64_t>());
    normalCal.TruncateToYear();
    normalCal.SetMilliseconds(normalCal.GetMilliseconds());
    (void)normalCal.GetMilliseconds();

    // Debug mode
    BundleActiveCalendar debugCal(baseTs);
    debugCal.ChangeToDebug();
    debugCal.SetMilliseconds(baseTs);
    debugCal.IncreaseDays(fdp->ConsumeIntegral<int64_t>());
    debugCal.TruncateToDay();
    debugCal.IncreaseWeeks(fdp->ConsumeIntegral<int64_t>());
    debugCal.TruncateToWeek();
    debugCal.IncreaseMonths(fdp->ConsumeIntegral<int64_t>());
    debugCal.TruncateToMonth();
    debugCal.IncreaseYears(fdp->ConsumeIntegral<int64_t>());
    debugCal.TruncateToYear();
    (void)debugCal.GetMilliseconds();
    return true;
}

/* -------------------------------------------------------
 * Dispatch
 * ------------------------------------------------------- */
void FuzzCalendar(FuzzedDataProvider& fdp)
{
    int code = fdp.ConsumeIntegralInRange(FUZZ_CODE_0, FUZZ_CODE_MAX);
    switch (code) {
        case FUZZ_CODE_0:
            CalendarCtorFuzzTest(&fdp);
            break;
        case FUZZ_CODE_1:
            CalendarDebugModeFuzzTest(&fdp);
            break;
        case FUZZ_CODE_2:
            CalendarTruncateDayFuzzTest(&fdp);
            break;
        case FUZZ_CODE_3:
            CalendarTruncateOtherFuzzTest(&fdp);
            break;
        case FUZZ_CODE_4:
            CalendarIncreaseFuzzTest(&fdp);
            break;
        case FUZZ_CODE_5:
            CalendarTruncateToFuzzTest(&fdp);
            break;
        case FUZZ_CODE_6:
            CalendarChainOperationsFuzzTest(&fdp);
            break;
        case FUZZ_CODE_7:
            CalendarEdgeCasesFuzzTest(&fdp);
            break;
        case FUZZ_CODE_8:
            CalendarComplexFuzzTest(&fdp);
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
    OHOS::DeviceUsageStats::FuzzCalendar(fdp);
    return 0;
}
