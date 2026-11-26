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

#include "bundleactiveeventlist_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>

#include "bundle_active_event.h"
#include "bundle_active_event_list.h"
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
constexpr int32_t FUZZ_CODE_7 = 7;
constexpr int32_t FUZZ_CODE_8 = 8;
constexpr int32_t FUZZ_CODE_9 = 9;
constexpr int32_t FUZZ_CODE_MAX = 9;
constexpr int32_t TIME_OFFSET_SHUTDOWN = 2;
constexpr int32_t TIME_OFFSET_STARTUP = 3;

// Magic numbers for FindBestIndex boundary test
constexpr int64_t K_TIME_OFFSET_BOUNDARY = 10;

// Magic numbers for timestamp offset calculations
constexpr int64_t K_TIMESTAMP_OFFSET1 = 1;
constexpr int32_t K_TIMESTAMP_MULTIPLIER10 = 10;
constexpr int32_t K_TIMESTAMP_MULTIPLIER5 = 5;
constexpr int32_t K_TIMESTAMP_MULTIPLIER7 = 7;
constexpr int64_t K_TIMESTAMP_OFFSET100 = 100;
constexpr int32_t K_TIMESTAMP_DIVIDER2 = 2;

/* Build a fuzzed BundleActiveEvent */
static BundleActiveEvent BuildFuzzEvent(FuzzedDataProvider* fdp)
{
    BundleActiveEvent event;
    event.bundleName_ = fdp->ConsumeRandomLengthString();
    event.moduleName_ = fdp->ConsumeRandomLengthString();
    event.abilityName_ = fdp->ConsumeRandomLengthString();
    event.abilityId_ = fdp->ConsumeRandomLengthString();
    event.eventId_ = fdp->ConsumeIntegral<int32_t>();
    event.timeStamp_ = fdp->ConsumeIntegral<int64_t>();
    event.uid_ = fdp->ConsumeIntegral<int32_t>();
    return event;
}

/* =============================================================
 *  Test 0: Default constructor + Size()
 * ============================================================= */
bool EventListCtorSizeFuzzTest(FuzzedDataProvider* fdp)
{
    BundleActiveEventList list;
    (void)list.Size();
    return true;
}

/* =============================================================
 *  Test 1: Clear() on empty and non-empty list
 * ============================================================= */
bool EventListClearFuzzTest(FuzzedDataProvider* fdp)
{
    BundleActiveEventList list;
    // Clear empty
    list.Clear();
    // Clear after adding events
    int32_t count = fdp->ConsumeIntegralInRange<int32_t>(1, 8);
    for (int32_t i = 0; i < count; ++i) {
        list.Insert(BuildFuzzEvent(fdp));
    }
    list.Clear();
    return true;
}

/* =============================================================
 *  Test 2: Insert at the end (sorted order)
 * ============================================================= */
bool EventListInsertSortedFuzzTest(FuzzedDataProvider* fdp)
{
    BundleActiveEventList list;
    int32_t count = fdp->ConsumeIntegralInRange<int32_t>(1, 10);
    int64_t baseTs = fdp->ConsumeIntegral<int64_t>();
    for (int32_t i = 0; i < count; ++i) {
        BundleActiveEvent ev = BuildFuzzEvent(fdp);
        ev.timeStamp_ = baseTs + i * K_TIMESTAMP_OFFSET1;  // monotonically increasing
        list.Insert(ev);
    }
    (void)list.Size();
    return true;
}

/* =============================================================
 *  Test 3: Insert at front/middle (unsorted order)
 * ============================================================= */
bool EventListInsertUnsortedFuzzTest(FuzzedDataProvider* fdp)
{
    BundleActiveEventList list;
    int32_t count = fdp->ConsumeIntegralInRange<int32_t>(1, 10);
    for (int32_t i = 0; i < count; ++i) {
        BundleActiveEvent ev = BuildFuzzEvent(fdp);
        // Random timestamps - may not be sorted
        list.Insert(ev);
    }
    (void)list.Size();
    return true;
}

/* =============================================================
 *  Test 4: FindBestIndex with various timestamps
 * ============================================================= */
bool EventListFindBestIndexFuzzTest(FuzzedDataProvider* fdp)
{
    BundleActiveEventList list;
    // Populate list first
    int32_t count = fdp->ConsumeIntegralInRange<int32_t>(1, 10);
    int64_t baseTs = fdp->ConsumeIntegral<int64_t>();
    for (int32_t i = 0; i < count; ++i) {
        BundleActiveEvent ev = BuildFuzzEvent(fdp);
        ev.timeStamp_ = baseTs + i * K_TIMESTAMP_MULTIPLIER10;
        list.Insert(ev);
    }
    // Now query FindBestIndex with various timestamps
    int32_t queries = fdp->ConsumeIntegralInRange<int32_t>(1, 5);
    for (int32_t i = 0; i < queries; ++i) {
        int64_t ts = fdp->ConsumeIntegral<int64_t>();
        int32_t idx = list.FindBestIndex(ts);
        (void)idx;
    }
    // Edge case: empty list
    BundleActiveEventList emptyList;
    (void)emptyList.FindBestIndex(baseTs);
    return true;
}

/* =============================================================
 *  Test 5: Merge two non-empty lists
 * ============================================================= */
bool EventListMergeFuzzTest(FuzzedDataProvider* fdp)
{
    BundleActiveEventList listA;
    BundleActiveEventList listB;

    int32_t countA = fdp->ConsumeIntegralInRange<int32_t>(0, 5);
    int32_t countB = fdp->ConsumeIntegralInRange<int32_t>(0, 5);
    int64_t baseTs = fdp->ConsumeIntegral<int64_t>();

    for (int32_t i = 0; i < countA; ++i) {
        BundleActiveEvent ev = BuildFuzzEvent(fdp);
        ev.timeStamp_ = baseTs + i * K_TIMESTAMP_MULTIPLIER5;
        listA.Insert(ev);
    }
    for (int32_t i = 0; i < countB; ++i) {
        BundleActiveEvent ev = BuildFuzzEvent(fdp);
        ev.timeStamp_ = baseTs + K_TIMESTAMP_OFFSET100 + i * K_TIMESTAMP_MULTIPLIER5;
        listB.Insert(ev);
    }

    // Merge A into empty list
    BundleActiveEventList merged;
    merged.Merge(listA);
    merged.Merge(listB);
    merged.Merge(merged);  // self-merge (edge case)
    (void)merged.Size();
    return true;
}

/* =============================================================
 *  Test 6: Complex mix - insert, find, merge, clear
 * ============================================================= */
bool EventListComplexFuzzTest(FuzzedDataProvider* fdp)
{
    BundleActiveEventList list;
    int64_t baseTs = fdp->ConsumeIntegral<int64_t>();

    int32_t count = fdp->ConsumeIntegralInRange<int32_t>(1, 8);
    for (int32_t i = 0; i < count; ++i) {
        BundleActiveEvent ev = BuildFuzzEvent(fdp);
        ev.timeStamp_ = baseTs + i * K_TIMESTAMP_MULTIPLIER7;
        list.Insert(ev);
    }

    (void)list.FindBestIndex(baseTs + count * K_TIMESTAMP_MULTIPLIER7 / K_TIMESTAMP_DIVIDER2);
    (void)list.Size();

    BundleActiveEventList other;
    int32_t otherCount = fdp->ConsumeIntegralInRange<int32_t>(1, 4);
    for (int32_t i = 0; i < otherCount; ++i) {
        other.Insert(BuildFuzzEvent(fdp));
    }
    list.Merge(other);

    list.Clear();
    (void)list.Size();
    return true;
}

/* =============================================================
 *  Test 7: Insert all BundleActiveEvent::SYSTEM_* events
 * ============================================================= */
bool EventListSystemEventsFuzzTest(FuzzedDataProvider* fdp)
{
    BundleActiveEventList list;
    int64_t baseTs = fdp->ConsumeIntegral<int64_t>();

    BundleActiveEvent ev1(BundleActiveEvent::SYSTEM_INTERACTIVE, baseTs);
    ev1.bundleName_ = fdp->ConsumeRandomLengthString();
    list.Insert(ev1);

    BundleActiveEvent ev2(BundleActiveEvent::USER_INTERACTIVE, baseTs + K_TIMESTAMP_OFFSET1);
    ev2.bundleName_ = fdp->ConsumeRandomLengthString();
    list.Insert(ev2);

    BundleActiveEvent ev3(BundleActiveEvent::SHUTDOWN, baseTs + TIME_OFFSET_SHUTDOWN);
    ev3.bundleName_ = fdp->ConsumeRandomLengthString();
    list.Insert(ev3);

    BundleActiveEvent ev4(BundleActiveEvent::STARTUP, baseTs + TIME_OFFSET_STARTUP);
    ev4.bundleName_ = fdp->ConsumeRandomLengthString();
    list.Insert(ev4);

    (void)list.Size();
    (void)list.FindBestIndex(baseTs);
    (void)list.FindBestIndex(baseTs + K_TIME_OFFSET_BOUNDARY);

    list.Clear();
    return true;
}

/* =============================================================
 *  Test 8: Edge cases - extreme timestamps, zero uid
 * ============================================================= */
bool EventListEdgeCasesFuzzTest(FuzzedDataProvider* fdp)
{
    BundleActiveEventList list;

    // Insert with zero values
    BundleActiveEvent zeroEv;
    zeroEv.timeStamp_ = 0;
    zeroEv.uid_ = 0;
    zeroEv.eventId_ = 0;
    list.Insert(zeroEv);

    // Insert with very large timestamp
    BundleActiveEvent largeTsEv;
    largeTsEv.timeStamp_ = INT64_MAX;
    largeTsEv.uid_ = INT32_MAX;
    list.Insert(largeTsEv);

    // Insert with negative timestamp
    BundleActiveEvent negTsEv;
    negTsEv.timeStamp_ = -1;
    negTsEv.uid_ = -1;
    list.Insert(negTsEv);

    // Insert with very long string (stress test memory)
    BundleActiveEvent longStrEv;
    longStrEv.bundleName_ = fdp->ConsumeRandomLengthString();
    longStrEv.abilityName_ = fdp->ConsumeRandomLengthString();
    longStrEv.abilityId_ = fdp->ConsumeRandomLengthString();
    longStrEv.timeStamp_ = INT64_MIN;
    list.Insert(longStrEv);

    (void)list.Size();
    (void)list.FindBestIndex(0);
    (void)list.FindBestIndex(INT64_MAX);
    (void)list.FindBestIndex(-1);
    list.Clear();
    (void)list.Size();
    return true;
}

/* =============================================================
 *  Test 9: Stress - many inserts, FindBestIndex on boundary
 * ============================================================= */
bool EventListStressFuzzTest(FuzzedDataProvider* fdp)
{
    BundleActiveEventList list;
    int64_t baseTs = fdp->ConsumeIntegral<int64_t>();

    int32_t count = fdp->ConsumeIntegralInRange<int32_t>(5, 20);
    for (int32_t i = 0; i < count; ++i) {
        BundleActiveEvent ev = BuildFuzzEvent(fdp);
        ev.timeStamp_ = baseTs + i * K_TIMESTAMP_OFFSET1;
        list.Insert(ev);
    }

    // Query at exact boundary positions
    for (int32_t i = 0; i <= count; ++i) {
        (void)list.FindBestIndex(baseTs + i * K_TIMESTAMP_OFFSET1);
    }

    BundleActiveEventList merged;
    merged.Merge(list);
    (void)merged.Size();
    return true;
}

/* -------------------------------------------------------
 * Dispatch
 * ------------------------------------------------------- */
void FuzzEventList(FuzzedDataProvider& fdp)
{
    int code = fdp.ConsumeIntegralInRange(FUZZ_CODE_0, FUZZ_CODE_MAX);
    switch (code) {
        case FUZZ_CODE_0:
            EventListCtorSizeFuzzTest(&fdp);
            break;
        case FUZZ_CODE_1:
            EventListClearFuzzTest(&fdp);
            break;
        case FUZZ_CODE_2:
            EventListInsertSortedFuzzTest(&fdp);
            break;
        case FUZZ_CODE_3:
            EventListInsertUnsortedFuzzTest(&fdp);
            break;
        case FUZZ_CODE_4:
            EventListFindBestIndexFuzzTest(&fdp);
            break;
        case FUZZ_CODE_5:
            EventListMergeFuzzTest(&fdp);
            break;
        case FUZZ_CODE_6:
            EventListComplexFuzzTest(&fdp);
            break;
        case FUZZ_CODE_7:
            EventListSystemEventsFuzzTest(&fdp);
            break;
        case FUZZ_CODE_8:
            EventListEdgeCasesFuzzTest(&fdp);
            break;
        case FUZZ_CODE_9:
            EventListStressFuzzTest(&fdp);
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
    OHOS::DeviceUsageStats::FuzzEventList(fdp);
    return 0;
}
