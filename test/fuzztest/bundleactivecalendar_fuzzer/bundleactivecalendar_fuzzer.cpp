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

#include <climits>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <fuzzer/FuzzedDataProvider.h>

#include "bundle_active_binary_search.h"
#include "bundle_active_calendar.h"
#include "bundle_active_event.h"
#include "bundle_active_event_stats.h"
#include "bundle_active_event_tracker.h"
#include "bundle_active_period_stats.h"
#include "bundle_active_util.h"

namespace OHOS {
namespace DeviceUsageStats {
namespace {

constexpr size_t MAX_OPS      = 24;
constexpr size_t MAX_NAME_LEN = 32;
constexpr size_t MAX_VEC_SIZE = 16;
constexpr int64_t MIN_FUZZ_TIME = 1;
constexpr int64_t MAX_FUZZ_TIME = 4102444800000LL; // 2100-01-01 00:00:00 UTC in ms.
constexpr int64_t MAX_FUZZ_SPAN = 1000000LL;
constexpr int64_t CALENDAR_MIN_INCREASE = -1000;
constexpr int64_t CALENDAR_MAX_INCREASE = 1000;
constexpr int64_t MAX_TRACKER_STEP = 10000;
constexpr int64_t MAX_BINARY_DELTA = 100;
constexpr int32_t TRACKER_ROUNDS = 4;
constexpr int32_t EVEN_DIVISOR = 2;
constexpr size_t SHORT_NAME_LEN = 16;
constexpr size_t TINY_NAME_LEN = 8;
constexpr int64_t EMPTY_SEARCH_TARGET = 42;
constexpr int64_t SINGLE_SEARCH_VALUE = 100;
constexpr int64_t SINGLE_SEARCH_LOW_TARGET = 50;
constexpr int64_t SINGLE_SEARCH_HIGH_TARGET = 200;
constexpr int64_t MULTI_SEARCH_FIRST = 10;
constexpr int64_t MULTI_SEARCH_SECOND = 20;
constexpr int64_t MULTI_SEARCH_THIRD = 30;
constexpr int64_t MULTI_SEARCH_FOURTH = 40;
constexpr int64_t MULTI_SEARCH_FIFTH = 50;
constexpr int64_t MULTI_SEARCH_MID_TARGET = 30;
constexpr int64_t MULTI_SEARCH_LEFT_TARGET = 20;
constexpr int64_t MULTI_SEARCH_RIGHT_TARGET = 40;
constexpr int64_t MULTI_SEARCH_HIGH_MISS = 999;
constexpr int64_t MULTI_SEARCH_LOW_MISS = 1;
constexpr int64_t MULTI_SEARCH_GAP_MISS = 25;

// IsBundleEvent() 返回 true 的全部 eventId
constexpr int32_t BUNDLE_EVENTS[] = {
    BundleActiveEvent::ABILITY_FOREGROUND,
    BundleActiveEvent::ABILITY_BACKGROUND,
    BundleActiveEvent::ABILITY_STOP,
    BundleActiveEvent::LONG_TIME_TASK_STARTTED,
    BundleActiveEvent::LONG_TIME_TASK_ENDED,
    BundleActiveEvent::END_OF_THE_DAY,
};
constexpr size_t BUNDLE_EVENTS_CNT = sizeof(BUNDLE_EVENTS) / sizeof(BUNDLE_EVENTS[0]);

static int64_t ConsumeTimeStamp(FuzzedDataProvider& fdp)
{
    return fdp.ConsumeIntegralInRange<int64_t>(MIN_FUZZ_TIME, MAX_FUZZ_TIME);
}

static int64_t AddTime(int64_t base, int64_t delta)
{
    if (delta > 0 && base > MAX_FUZZ_TIME - delta) {
        return MAX_FUZZ_TIME;
    }
    if (delta < 0 && base < MIN_FUZZ_TIME - delta) {
        return MIN_FUZZ_TIME;
    }
    return base + delta;
}

static void FuzzCalendar(FuzzedDataProvider& fdp)
{
    int64_t ts  = ConsumeTimeStamp(fdp);
    int64_t val = fdp.ConsumeIntegralInRange<int64_t>(CALENDAR_MIN_INCREASE, CALENDAR_MAX_INCREASE);

    // --- Release 模式：TruncateTo* 内部调用 GetIntervalTypeStartTime ---
    {
        BundleActiveCalendar cal(ts);

        // TruncateTo dispatcher 覆盖全部 4 个合法 intervalType
        cal.TruncateTo(BundleActivePeriodStats::PERIOD_DAILY);
        cal.TruncateTo(BundleActivePeriodStats::PERIOD_WEEKLY);
        cal.TruncateTo(BundleActivePeriodStats::PERIOD_MONTHLY);
        cal.TruncateTo(BundleActivePeriodStats::PERIOD_YEARLY);

        // 越界 intervalType → default 分支（空操作）
        cal.TruncateTo(BundleActivePeriodStats::PERIOD_BEST);
        cal.TruncateTo(-1);
        cal.TruncateTo(fdp.ConsumeIntegral<int32_t>());

        // 各 Increase* 方法
        cal.IncreaseDays(val);
        cal.IncreaseWeeks(val);
        cal.IncreaseMonths(val);
        cal.IncreaseYears(val);

        // SetMilliseconds + GetMilliseconds
        int64_t newTs = ConsumeTimeStamp(fdp);
        cal.SetMilliseconds(newTs);
        (void)cal.GetMilliseconds();
    }

    // --- Debug 模式：TruncateTo* 直接取模，不调用 localtime ---
    {
        BundleActiveCalendar calDebug(ts);
        calDebug.ChangeToDebug();

        calDebug.TruncateTo(BundleActivePeriodStats::PERIOD_DAILY);
        calDebug.TruncateTo(BundleActivePeriodStats::PERIOD_WEEKLY);
        calDebug.TruncateTo(BundleActivePeriodStats::PERIOD_MONTHLY);
        calDebug.TruncateTo(BundleActivePeriodStats::PERIOD_YEARLY);

        calDebug.IncreaseDays(val);
        calDebug.IncreaseWeeks(val);
        calDebug.IncreaseMonths(val);
        calDebug.IncreaseYears(val);

        (void)calDebug.GetMilliseconds();
    }

    // --- 以合法时间戳（当前系统时间）构造 Calendar，确保 TruncateTo 走到
    //     非零返回值路径（localtime 成功返回、mktime 成功的分支）---
    {
        int64_t nowMs = BundleActiveUtil::GetSystemTimeMs();
        BundleActiveCalendar calValid(nowMs);
        calValid.TruncateTo(BundleActivePeriodStats::PERIOD_DAILY);
        calValid.TruncateTo(BundleActivePeriodStats::PERIOD_WEEKLY);
        calValid.TruncateTo(BundleActivePeriodStats::PERIOD_MONTHLY);
        calValid.TruncateTo(BundleActivePeriodStats::PERIOD_YEARLY);
    }

    // --- 默认构造函数（时间戳为 0）---
    {
        BundleActiveCalendar calZero;
        calZero.TruncateTo(BundleActivePeriodStats::PERIOD_DAILY);
        (void)calZero.GetMilliseconds();
    }
}

static void FuzzUtilIntervals(FuzzedDataProvider& fdp)
{
    std::string bundleName = fdp.ConsumeRandomLengthString(MAX_NAME_LEN);
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    (void)BundleActiveUtil::GetBundleUsageKey(bundleName, uid);
    (void)BundleActiveUtil::GetFFRTDelayTime(fdp.ConsumeIntegralInRange<int64_t>(-MAX_FUZZ_SPAN, MAX_FUZZ_SPAN));

    (void)BundleActiveUtil::GetIntervalTypeStartTime(-1, BundleActiveUtil::PERIOD_DAILY);
    (void)BundleActiveUtil::GetIntervalTypeStartTime(0,  BundleActiveUtil::PERIOD_WEEKLY);

    int64_t ts = ConsumeTimeStamp(fdp);
    (void)BundleActiveUtil::GetIntervalTypeStartTime(ts, BundleActiveUtil::PERIOD_DAILY);
    (void)BundleActiveUtil::GetIntervalTypeStartTime(ts, BundleActiveUtil::PERIOD_WEEKLY);
    (void)BundleActiveUtil::GetIntervalTypeStartTime(ts, BundleActiveUtil::PERIOD_MONTHLY);
    (void)BundleActiveUtil::GetIntervalTypeStartTime(ts, BundleActiveUtil::PERIOD_YEARLY);

    int64_t nowMs = BundleActiveUtil::GetSystemTimeMs();
    (void)BundleActiveUtil::GetIntervalTypeStartTime(nowMs, BundleActiveUtil::PERIOD_DAILY);
    (void)BundleActiveUtil::GetIntervalTypeStartTime(nowMs, BundleActiveUtil::PERIOD_WEEKLY);
    (void)BundleActiveUtil::GetIntervalTypeStartTime(nowMs, BundleActiveUtil::PERIOD_MONTHLY);
    (void)BundleActiveUtil::GetIntervalTypeStartTime(nowMs, BundleActiveUtil::PERIOD_YEARLY);

    (void)BundleActiveUtil::GetIntervalTypeStartTime(nowMs, fdp.ConsumeIntegral<int32_t>());
}

static void FuzzUtilStringAndTime(FuzzedDataProvider& fdp)
{
    (void)BundleActiveUtil::StringToInt32("0");
    (void)BundleActiveUtil::StringToInt32("2147483647");
    (void)BundleActiveUtil::StringToInt32("2147483648");
    (void)BundleActiveUtil::StringToInt32("-2147483648");
    (void)BundleActiveUtil::StringToInt32("-2147483649");
    (void)BundleActiveUtil::StringToInt32("");
    (void)BundleActiveUtil::StringToInt32("abc");
    (void)BundleActiveUtil::StringToInt32("123abc");
    (void)BundleActiveUtil::StringToInt32("99999999999999999999");
    (void)BundleActiveUtil::StringToInt32("-99999999999999999999");
    (void)BundleActiveUtil::StringToInt32(fdp.ConsumeRandomLengthString(MAX_NAME_LEN));

    (void)BundleActiveUtil::StringToInt64("0");
    (void)BundleActiveUtil::StringToInt64("1234567890");
    (void)BundleActiveUtil::StringToInt64("");
    (void)BundleActiveUtil::StringToInt64("xyz");
    (void)BundleActiveUtil::StringToInt64("123 ");
    (void)BundleActiveUtil::StringToInt64("99999999999999999999");
    (void)BundleActiveUtil::StringToInt64(fdp.ConsumeRandomLengthString(MAX_NAME_LEN));

    (void)BundleActiveUtil::GetSystemTimeMs();
    (void)BundleActiveUtil::GetSteadyTime();
    (void)BundleActiveUtil::GetNowMicroTime();
}

static void FuzzUtilStorage(FuzzedDataProvider& fdp)
{
    (void)BundleActiveUtil::GetPartitionName("/data/local/tmp");
    (void)BundleActiveUtil::GetPartitionName("/data");
    (void)BundleActiveUtil::GetPartitionName("relative/path");
    (void)BundleActiveUtil::GetPartitionName("single");
    (void)BundleActiveUtil::GetPartitionName("");
    (void)BundleActiveUtil::GetPartitionName(fdp.ConsumeRandomLengthString(MAX_NAME_LEN));

    (void)BundleActiveUtil::GetFolderOrFileSize("/proc/self");
    (void)BundleActiveUtil::GetFolderOrFileSize("/proc/self/status");
    (void)BundleActiveUtil::GetFolderOrFileSize("/no_such_path_xyz");
    (void)BundleActiveUtil::GetFolderOrFileSize(fdp.ConsumeRandomLengthString(MAX_NAME_LEN));

    (void)BundleActiveUtil::GetDeviceValidSize("/data");
    (void)BundleActiveUtil::GetDeviceValidSize(fdp.ConsumeRandomLengthString(MAX_NAME_LEN));

    (void)BundleActiveUtil::GetPercentOfAvailableUserSpace("/data");
    (void)BundleActiveUtil::GetPercentOfAvailableUserSpace(fdp.ConsumeRandomLengthString(MAX_NAME_LEN));
}

static void FuzzUtil(FuzzedDataProvider& fdp)
{
    FuzzUtilIntervals(fdp);
    FuzzUtilStringAndTime(fdp);
    FuzzUtilStorage(fdp);
}

static void FuzzEventTracker(FuzzedDataProvider& fdp)
{
    BundleActiveEventTracker tracker;

    // CommitTime：curStartTime_=0 → no-op（if 不进入）
    tracker.CommitTime(ConsumeTimeStamp(fdp));

    // Update：首次调用
    //   !curStartTime_ → true → count_++
    //   CommitTime(ts1)：curStartTime_=0 → no-op
    //   curStartTime_ = ts1
    int64_t ts1 = ConsumeTimeStamp(fdp);
    tracker.Update(ts1);

    // CommitTime：curStartTime_=ts1 (!=0) → duration_ += ts2-ts1；curStartTime_=0
    int64_t ts2 = AddTime(ts1, fdp.ConsumeIntegralInRange<int64_t>(MIN_FUZZ_TIME, MAX_TRACKER_STEP));
    tracker.CommitTime(ts2);

    // Update 第二次：curStartTime_=0 → count_++；CommitTime no-op；curStartTime_=ts3
    int64_t ts3 = ConsumeTimeStamp(fdp);
    tracker.Update(ts3);

    // Update 第三次：curStartTime_=ts3 (!=0) → 不 count_++；
    //   CommitTime(ts4)：duration_ += ts4-ts3；curStartTime_=0；然后 curStartTime_=ts4
    int64_t ts4 = AddTime(ts3, fdp.ConsumeIntegralInRange<int64_t>(MIN_FUZZ_TIME, MAX_TRACKER_STEP));
    tracker.Update(ts4);

    // AddToEventStats：此时 count_ > 0，duration_ > 0 → 追加 BundleActiveEventStats
    {
        std::vector<BundleActiveEventStats> statsList;
        tracker.AddToEventStats(statsList,
            BundleActiveEvent::SCREEN_INTERACTIVE,
            ConsumeTimeStamp(fdp),
            ConsumeTimeStamp(fdp));
    }

    // 新建空 tracker → count_=0, duration_=0 → AddToEventStats 不追加（空路径）
    {
        BundleActiveEventTracker emptyTracker;
        std::vector<BundleActiveEventStats> emptyList;
        emptyTracker.AddToEventStats(emptyList,
            BundleActiveEvent::KEYGUARD_SHOWN,
            ConsumeTimeStamp(fdp),
            ConsumeTimeStamp(fdp));
    }

    // 随机驱动：交替 Update / CommitTime，覆盖更多路径组合
    size_t ops = fdp.ConsumeIntegralInRange<size_t>(1, MAX_OPS);
    for (size_t i = 0; i < ops && fdp.remaining_bytes() > 0; i++) {
        int64_t t = ConsumeTimeStamp(fdp);
        if (fdp.ConsumeBool()) {
            tracker.Update(t);
        } else {
            tracker.CommitTime(t);
        }
    }

    // 最终 AddToEventStats：保证经过随机驱动后 duration_ 可能有值
    {
        std::vector<BundleActiveEventStats> finalList;
        tracker.AddToEventStats(finalList,
            BundleActiveEvent::SCREEN_NON_INTERACTIVE,
            ConsumeTimeStamp(fdp),
            ConsumeTimeStamp(fdp));
    }
}

static BundleActivePeriodStats BuildPeriodStats(FuzzedDataProvider& fdp, std::string& bundleName, int32_t& uid)
{
    {
        BundleActivePeriodStats defaultStats;
        (void)defaultStats.GetCachedString("probe");
    }
    int32_t userId  = fdp.ConsumeIntegral<int32_t>();
    int64_t beginTs = ConsumeTimeStamp(fdp);
    BundleActivePeriodStats stats(userId, beginTs);

    bundleName = fdp.ConsumeRandomLengthString(MAX_NAME_LEN);
    uid = fdp.ConsumeIntegral<int32_t>();
    auto stat1 = stats.GetOrCreateUsageStats(bundleName, uid);
    (void)stat1;

    auto stat1Again = stats.GetOrCreateUsageStats(bundleName, uid);
    (void)stat1Again;

    std::string bundle2 = fdp.ConsumeRandomLengthString(MAX_NAME_LEN);
    int32_t uid2 = (uid == INT32_MAX) ? uid - static_cast<int32_t>(MIN_FUZZ_TIME) :
        uid + static_cast<int32_t>(MIN_FUZZ_TIME);
    auto stat2 = stats.GetOrCreateUsageStats(bundle2, uid2);
    (void)stat2;
    return stats;
}

static void FuzzPeriodStatsUpdates(BundleActivePeriodStats& stats, FuzzedDataProvider& fdp,
    const std::string& bundleName, int32_t uid)
{
    for (size_t i = 0; i < BUNDLE_EVENTS_CNT; i++) {
        int64_t t      = ConsumeTimeStamp(fdp);
        std::string ai = fdp.ConsumeRandomLengthString(SHORT_NAME_LEN);
        stats.Update(bundleName, "", t, BUNDLE_EVENTS[i], ai, uid);
    }

    stats.Update("", "", ConsumeTimeStamp(fdp),
        BundleActiveEvent::SHUTDOWN, "", uid);
    stats.Update("", "", ConsumeTimeStamp(fdp),
        BundleActiveEvent::FLUSH, "", uid);

    stats.Update(bundleName, "", ConsumeTimeStamp(fdp),
        BundleActiveEvent::SCREEN_INTERACTIVE, "", uid);
    stats.Update(bundleName, "", ConsumeTimeStamp(fdp),
        fdp.ConsumeIntegral<int32_t>(), "", uid);
}

static void FuzzPeriodStatsEvents(BundleActivePeriodStats& stats, FuzzedDataProvider& fdp,
    const std::string& bundleName)
{
    {
        BundleActiveEvent event;
        event.bundleName_ = fdp.ConsumeRandomLengthString(MAX_NAME_LEN);
        event.timeStamp_  = ConsumeTimeStamp(fdp);
        stats.AddEvent(event);
    }

    {
        BundleActiveEvent event;
        event.bundleName_ = bundleName;
        event.timeStamp_  = ConsumeTimeStamp(fdp);
        stats.AddEvent(event);
    }

    {
        BundleActiveEvent event;
        event.bundleName_                = bundleName;
        event.continuousTaskAbilityName_ = fdp.ConsumeRandomLengthString(SHORT_NAME_LEN);
        event.timeStamp_                 = ConsumeTimeStamp(fdp);
        stats.AddEvent(event);
    }
}

static void FuzzPeriodStatsTrackers(BundleActivePeriodStats& stats, FuzzedDataProvider& fdp)
{
    for (int i = 0; i < TRACKER_ROUNDS; i++) {
        int64_t t = ConsumeTimeStamp(fdp);
        if (i % EVEN_DIVISOR == 0) {
            stats.UpdateScreenInteractive(t);
        } else {
            stats.UpdateScreenNonInteractive(t);
        }
    }

    for (int i = 0; i < TRACKER_ROUNDS; i++) {
        int64_t t = ConsumeTimeStamp(fdp);
        if (i % EVEN_DIVISOR == 0) {
            stats.UpdateKeyguardShown(t);
        } else {
            stats.UpdateKeyguardHidden(t);
        }
    }

    stats.CommitTime(ConsumeTimeStamp(fdp));

    {
        std::vector<BundleActiveEventStats> eventStatsList;
        stats.AddEventStatsTo(eventStatsList);
    }

    // --- GetCachedString 直接测试两路径 ---
    std::string key = "unique_cache_key_42";
    (void)stats.GetCachedString(key);      // 新建 → insert
    (void)stats.GetCachedString(key);      // 命中 → return *it

    // --- 空 PeriodStats 的 AddEventStatsTo（4 个 tracker 均无数据 → 不追加）---
    {
        BundleActivePeriodStats emptyStats;
        std::vector<BundleActiveEventStats> emptyList;
        emptyStats.AddEventStatsTo(emptyList);
    }
}

static void FuzzPeriodStatsRandomOps(BundleActivePeriodStats& stats, FuzzedDataProvider& fdp, int32_t uid)
{
    size_t ops = fdp.ConsumeIntegralInRange<size_t>(1, MAX_OPS);
    for (size_t i = 0; i < ops && fdp.remaining_bytes() > 0; i++) {
        int64_t t  = ConsumeTimeStamp(fdp);
        int32_t ev = fdp.ConsumeIntegral<int32_t>();
        std::string bn = fdp.ConsumeRandomLengthString(SHORT_NAME_LEN);
        std::string ai = fdp.ConsumeRandomLengthString(TINY_NAME_LEN);
        stats.Update(bn, ai, t, ev, ai, uid);
    }
}

static void FuzzPeriodStats(FuzzedDataProvider& fdp)
{
    std::string bundleName;
    int32_t uid = 0;
    BundleActivePeriodStats stats = BuildPeriodStats(fdp, bundleName, uid);
    FuzzPeriodStatsUpdates(stats, fdp, bundleName, uid);
    FuzzPeriodStatsEvents(stats, fdp, bundleName);
    FuzzPeriodStatsTrackers(stats, fdp);
    FuzzPeriodStatsRandomOps(stats, fdp, uid);
}

static void FuzzBinarySearchFixedCases(BundleActiveBinarySearch& searcher)
{
    {
        std::vector<int64_t> empty;
        (void)searcher.BinarySearch(empty, EMPTY_SEARCH_TARGET);
    }

    {
        std::vector<int64_t> arr = {SINGLE_SEARCH_VALUE};
        (void)searcher.BinarySearch(arr, SINGLE_SEARCH_VALUE);
    }

    {
        std::vector<int64_t> arr = {SINGLE_SEARCH_VALUE};
        (void)searcher.BinarySearch(arr, SINGLE_SEARCH_LOW_TARGET);
    }

    {
        std::vector<int64_t> arr = {SINGLE_SEARCH_VALUE};
        (void)searcher.BinarySearch(arr, SINGLE_SEARCH_HIGH_TARGET);
    }

    // 五元素数组：覆盖 low/high 各收缩方向
    {
        std::vector<int64_t> arr = {
            MULTI_SEARCH_FIRST,
            MULTI_SEARCH_SECOND,
            MULTI_SEARCH_THIRD,
            MULTI_SEARCH_FOURTH,
            MULTI_SEARCH_FIFTH,
        };

        (void)searcher.BinarySearch(arr, MULTI_SEARCH_MID_TARGET);
        (void)searcher.BinarySearch(arr, MULTI_SEARCH_LEFT_TARGET);
        (void)searcher.BinarySearch(arr, MULTI_SEARCH_RIGHT_TARGET);
        (void)searcher.BinarySearch(arr, MULTI_SEARCH_HIGH_MISS);
        (void)searcher.BinarySearch(arr, MULTI_SEARCH_LOW_MISS);
        (void)searcher.BinarySearch(arr, MULTI_SEARCH_GAP_MISS);
    }
}

static void FuzzBinarySearchRandomCases(BundleActiveBinarySearch& searcher, FuzzedDataProvider& fdp)
{
    {
        size_t cnt = fdp.ConsumeIntegralInRange<size_t>(0, MAX_VEC_SIZE);
        std::vector<int64_t> arr;
        arr.reserve(cnt);
        int64_t prev = 0;
        for (size_t i = 0; i < cnt && fdp.remaining_bytes() > 0; i++) {
            int64_t delta = fdp.ConsumeIntegralInRange<int64_t>(0, MAX_BINARY_DELTA);
            prev += delta;
            arr.emplace_back(prev);
        }
        int64_t target = fdp.ConsumeIntegralInRange<int64_t>(0, MAX_FUZZ_SPAN);
        (void)searcher.BinarySearch(arr, target);

        // 若数组非空，再用数组内真实元素作为目标，确保命中分支被覆盖
        if (!arr.empty()) {
            size_t idx = fdp.ConsumeIntegralInRange<size_t>(0, arr.size() - 1);
            (void)searcher.BinarySearch(arr, arr[idx]);
        }
    }
}

static void FuzzBinarySearch(FuzzedDataProvider& fdp)
{
    auto& searcher = *DelayedSingleton<BundleActiveBinarySearch>::GetInstance();
    FuzzBinarySearchFixedCases(searcher);
    FuzzBinarySearchRandomCases(searcher, fdp);
}

} // namespace

// ======= 模糊测试主入口 =======
bool BundleActiveCalendarFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return false;
    }
    FuzzedDataProvider fdp(data, size);

    FuzzCalendar(fdp);
    FuzzUtil(fdp);
    FuzzEventTracker(fdp);
    FuzzPeriodStats(fdp);
    FuzzBinarySearch(fdp);

    return true;
}

} // namespace DeviceUsageStats
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DeviceUsageStats::BundleActiveCalendarFuzzTest(data, size);
    return 0;
}
