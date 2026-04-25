/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include <memory>

#include "bundle_active_calendar.h"
#include "bundle_active_event_list.h"
#include "bundle_active_period_stats.h"
#include "bundle_active_stats_combiner.h"

namespace OHOS {
namespace DeviceUsageStats {
namespace {
constexpr int32_t MAX_EVENTS = 50;
constexpr int32_t MAX_STATS = 20;

void FuzzBundleActiveCalendar(FuzzedDataProvider &provider)
{
    int64_t timeStamp = provider.ConsumeIntegral<int64_t>();
    BundleActiveCalendar calendar(timeStamp);

    if (provider.ConsumeBool()) {
        calendar.ChangeToDebug();
    }

    int32_t operation = provider.ConsumeIntegralInRange<int32_t>(0, 11);
    switch (operation) {
        case 0:
            calendar.TruncateToDay();
            break;
        case 1:
            calendar.TruncateToWeek();
            break;
        case 2:
            calendar.TruncateToMonth();
            break;
        case 3:
            calendar.TruncateToYear();
            break;
        case 4:
            calendar.IncreaseDays(provider.ConsumeIntegral<int64_t>());
            break;
        case 5:
            calendar.IncreaseWeeks(provider.ConsumeIntegral<int64_t>());
            break;
        case 6:
            calendar.IncreaseMonths(provider.ConsumeIntegral<int64_t>());
            break;
        case 7:
            calendar.IncreaseYears(provider.ConsumeIntegral<int64_t>());
            break;
        case 8:
            calendar.SetMilliseconds(provider.ConsumeIntegral<int64_t>());
            break;
        case 9:
            calendar.GetMilliseconds();
            break;
        case 10:
            calendar.TruncateTo(provider.ConsumeIntegralInRange<int32_t>(0, 4));
            break;
        case 11: {
            BundleActiveCalendar calendar2(provider.ConsumeIntegral<int64_t>());
            calendar2.TruncateToDay();
            calendar.IncreaseDays(1);
            break;
        }
        default:
            break;
    }
}

void FuzzBundleActiveEventList(FuzzedDataProvider &provider)
{
    BundleActiveEventList eventList;

    int32_t eventCount = provider.ConsumeIntegralInRange<int32_t>(0, MAX_EVENTS);
    for (int32_t i = 0; i < eventCount; ++i) {
        BundleActiveEvent event;
        event.bundleName_ = provider.ConsumeRandomLengthString(64);
        event.timeStamp_ = provider.ConsumeIntegral<int64_t>();
        event.eventId_ = provider.ConsumeIntegralInRange<int32_t>(
            BundleActiveEvent::ABILITY_FOREGROUND, BundleActiveEvent::SYSTEM_WAKEUP);
        eventList.Insert(event);
    }

    eventList.Size();
    eventList.FindBestIndex(provider.ConsumeIntegral<int64_t>());

    BundleActiveEventList eventList2;
    int32_t event2Count = provider.ConsumeIntegralInRange<int32_t>(0, MAX_EVENTS);
    for (int32_t i = 0; i < event2Count; ++i) {
        BundleActiveEvent event;
        event.bundleName_ = provider.ConsumeRandomLengthString(64);
        event.timeStamp_ = provider.ConsumeIntegral<int64_t>();
        event.eventId_ = provider.ConsumeIntegralInRange<int32_t>(
            BundleActiveEvent::ABILITY_FOREGROUND, BundleActiveEvent::SYSTEM_WAKEUP);
        eventList2.Insert(event);
    }

    eventList.Merge(eventList2);
    eventList.Clear();
}

void FuzzBundleActiveStatsCombinerPackageStats(FuzzedDataProvider &provider)
{
    BundleActiveStatsCombiner<BundleActivePackageStats> combiner;
    auto periodStats = std::make_shared<BundleActivePeriodStats>();
    periodStats->beginTime_ = provider.ConsumeIntegral<int64_t>();
    periodStats->endTime_ = provider.ConsumeIntegral<int64_t>();
    periodStats->userId_ = provider.ConsumeIntegral<int32_t>();

    int32_t statsCount = provider.ConsumeIntegralInRange<int32_t>(0, MAX_STATS);
    for (int32_t i = 0; i < statsCount; ++i) {
        std::string bundleName = provider.ConsumeRandomLengthString(64);
        if (bundleName.empty()) {
            bundleName = "com.example.test";
        }
        auto packageStats = std::make_shared<BundleActivePackageStats>();
        packageStats->bundleName_ = bundleName;
        packageStats->beginTimeStamp_ = provider.ConsumeIntegral<int64_t>();
        packageStats->endTimeStamp_ = provider.ConsumeIntegral<int64_t>();
        packageStats->lastTimeUsed_ = provider.ConsumeIntegral<int64_t>();
        packageStats->totalInFrontTime_ = provider.ConsumeIntegral<int64_t>();
        packageStats->startCount_ = provider.ConsumeIntegral<int32_t>();
        periodStats->bundleStats_[bundleName] = packageStats;
    }

    std::vector<BundleActivePackageStats> result;
    int64_t beginTime = provider.ConsumeIntegral<int64_t>();
    combiner.combine(periodStats, result, beginTime);
}

void FuzzBundleActiveStatsCombinerEvent(FuzzedDataProvider &provider)
{
    BundleActiveStatsCombiner<BundleActiveEvent> combiner;
    auto periodStats = std::make_shared<BundleActivePeriodStats>();
    periodStats->beginTime_ = provider.ConsumeIntegral<int64_t>();
    periodStats->endTime_ = provider.ConsumeIntegral<int64_t>();
    periodStats->userId_ = provider.ConsumeIntegral<int32_t>();

    int32_t eventCount = provider.ConsumeIntegralInRange<int32_t>(0, MAX_STATS);
    for (int32_t i = 0; i < eventCount; ++i) {
        BundleActiveEvent event;
        event.bundleName_ = provider.ConsumeRandomLengthString(64);
        if (event.bundleName_.empty()) {
            event.bundleName_ = "com.example.test";
        }
        event.timeStamp_ = provider.ConsumeIntegral<int64_t>();
        event.eventId_ = provider.ConsumeIntegralInRange<int32_t>(
            BundleActiveEvent::ABILITY_FOREGROUND, BundleActiveEvent::SYSTEM_WAKEUP);
        periodStats->events_.Insert(event);
    }

    std::vector<BundleActiveEvent> result;
    int64_t beginTime = provider.ConsumeIntegral<int64_t>();
    combiner.combine(periodStats, result, beginTime);
}

void FuzzBundleActivePeriodStats(FuzzedDataProvider &provider)
{
    BundleActivePeriodStats periodStats;
    periodStats.userId_ = provider.ConsumeIntegral<int32_t>();
    periodStats.beginTime_ = provider.ConsumeIntegral<int64_t>();
    periodStats.endTime_ = provider.ConsumeIntegral<int64_t>();
    periodStats.lastTimeSaved_ = provider.ConsumeIntegral<int64_t>();

    int32_t statsCount = provider.ConsumeIntegralInRange<int32_t>(0, MAX_STATS);
    for (int32_t i = 0; i < statsCount; ++i) {
        std::string bundleName = provider.ConsumeRandomLengthString(64);
        if (bundleName.empty()) {
            bundleName = "com.example.test";
        }
        auto packageStats = std::make_shared<BundleActivePackageStats>();
        packageStats->bundleName_ = bundleName;
        packageStats->beginTimeStamp_ = provider.ConsumeIntegral<int64_t>();
        packageStats->endTimeStamp_ = provider.ConsumeIntegral<int64_t>();
        packageStats->lastTimeUsed_ = provider.ConsumeIntegral<int64_t>();
        packageStats->totalInFrontTime_ = provider.ConsumeIntegral<int64_t>();
        periodStats.bundleStats_[bundleName] = packageStats;
    }
}

void FuzzBundleActiveEventStaticMethods(FuzzedDataProvider &provider)
{
    int32_t eventId = provider.ConsumeIntegralInRange<int32_t>(
        BundleActiveEvent::DEFAULT_EVENT_ID, BundleActiveEvent::SYSTEM_WAKEUP);
    BundleActiveEvent::IsBundleEvent(eventId);
    BundleActiveEvent::IsAppStateEvent(eventId);

    BundleActiveEvent event1(
        provider.ConsumeIntegral<int32_t>(),
        provider.ConsumeIntegral<int64_t>());

    BundleActiveEvent event2(
        provider.ConsumeIntegral<int32_t>(),
        provider.ConsumeRandomLengthString(64),
        provider.ConsumeIntegral<int32_t>());

    BundleActiveEvent event3(
        provider.ConsumeRandomLengthString(64),
        provider.ConsumeRandomLengthString(64),
        provider.ConsumeIntegral<int32_t>());

    BundleActiveEvent event4(
        provider.ConsumeRandomLengthString(64),
        provider.ConsumeRandomLengthString(64),
        provider.ConsumeRandomLengthString(64),
        provider.ConsumeRandomLengthString(64),
        provider.ConsumeIntegral<int32_t>());

    BundleActiveEvent event5(
        provider.ConsumeRandomLengthString(64),
        provider.ConsumeRandomLengthString(64),
        provider.ConsumeRandomLengthString(64),
        provider.ConsumeIntegral<int32_t>(),
        provider.ConsumeIntegral<int64_t>(),
        provider.ConsumeIntegral<int32_t>(),
        provider.ConsumeIntegral<int32_t>());

    event1.ToString();
    event2.ToString();
    event3.ToString();
}

void FuzzBundleActiveFormRecordOperations(FuzzedDataProvider &provider)
{
    BundleActiveFormRecord record1(
        provider.ConsumeRandomLengthString(64),
        provider.ConsumeIntegral<int32_t>(),
        provider.ConsumeIntegral<int64_t>(),
        provider.ConsumeIntegral<int64_t>(),
        provider.ConsumeIntegral<int32_t>(),
        provider.ConsumeIntegral<int32_t>());

    BundleActiveFormRecord record2(
        provider.ConsumeRandomLengthString(64),
        provider.ConsumeIntegral<int32_t>(),
        provider.ConsumeIntegral<int64_t>(),
        provider.ConsumeIntegral<int64_t>(),
        provider.ConsumeIntegral<int32_t>(),
        provider.ConsumeIntegral<int32_t>());

    (void)(record1 == record2);
    (void)(record1 == provider.ConsumeIntegral<int64_t>());
    BundleActiveFormRecord::cmp(record1, record2);

    record1.UpdateFormRecord(provider.ConsumeIntegral<int64_t>());
}

void FuzzBundleActiveModuleRecordOperations(FuzzedDataProvider &provider)
{
    BundleActiveModuleRecord record1;
    record1.bundleName_ = provider.ConsumeRandomLengthString(64);
    record1.moduleName_ = provider.ConsumeRandomLengthString(64);
    record1.lastModuleUsedTime_ = provider.ConsumeIntegral<int64_t>();
    record1.launchedCount_ = provider.ConsumeIntegral<int32_t>();

    BundleActiveModuleRecord record2;
    record2.bundleName_ = provider.ConsumeRandomLengthString(64);
    record2.moduleName_ = provider.ConsumeRandomLengthString(64);
    record2.lastModuleUsedTime_ = provider.ConsumeIntegral<int64_t>();
    record2.launchedCount_ = provider.ConsumeIntegral<int32_t>();

    BundleActiveModuleRecord::cmp(record1, record2);

    record1.AddOrUpdateOneFormRecord(
        provider.ConsumeRandomLengthString(64),
        provider.ConsumeIntegral<int32_t>(),
        provider.ConsumeIntegral<int64_t>(),
        provider.ConsumeIntegral<int64_t>(),
        provider.ConsumeIntegral<int32_t>());

    record1.RemoveOneFormRecord(
        provider.ConsumeRandomLengthString(64),
        provider.ConsumeIntegral<int32_t>(),
        provider.ConsumeIntegral<int64_t>());

    record1.UpdateModuleRecord(provider.ConsumeIntegral<int64_t>());
}

enum FuzzCase : uint8_t {
    CALENDAR = 0,
    EVENT_LIST,
    STATS_COMBINER_PACKAGE,
    STATS_COMBINER_EVENT,
    PERIOD_STATS,
    EVENT_STATIC_METHODS,
    FORM_RECORD_OPS,
    MODULE_RECORD_OPS,
    FUZZ_CASE_COUNT,
};

void DispatchFuzzCase(FuzzedDataProvider &provider)
{
    switch (provider.ConsumeIntegralInRange<uint8_t>(CALENDAR, FUZZ_CASE_COUNT - 1)) {
        case CALENDAR:
            FuzzBundleActiveCalendar(provider);
            break;
        case EVENT_LIST:
            FuzzBundleActiveEventList(provider);
            break;
        case STATS_COMBINER_PACKAGE:
            FuzzBundleActiveStatsCombinerPackageStats(provider);
            break;
        case STATS_COMBINER_EVENT:
            FuzzBundleActiveStatsCombinerEvent(provider);
            break;
        case PERIOD_STATS:
            FuzzBundleActivePeriodStats(provider);
            break;
        case EVENT_STATIC_METHODS:
            FuzzBundleActiveEventStaticMethods(provider);
            break;
        case FORM_RECORD_OPS:
            FuzzBundleActiveFormRecordOperations(provider);
            break;
        case MODULE_RECORD_OPS:
            FuzzBundleActiveModuleRecordOperations(provider);
            break;
        default:
            break;
    }
}
} // namespace
} // namespace DeviceUsageStats
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return 0;
    }
    FuzzedDataProvider provider(data, size);
    OHOS::DeviceUsageStats::DispatchFuzzCase(provider);
    return 0;
}

