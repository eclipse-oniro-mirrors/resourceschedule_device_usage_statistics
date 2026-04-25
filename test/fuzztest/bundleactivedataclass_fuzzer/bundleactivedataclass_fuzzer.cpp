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

#include "bundleactivedataclass_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>

#include "bundle_active_package_stats.h"
#include "bundle_active_event_stats.h"
#include "bundle_active_form_record.h"
#include "app_group_callback_info.h"
#include "bundle_active_high_frequency_period.h"
#include "bundle_active_event.h"
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
constexpr int32_t FUZZ_CODE_MAX = 7;

// Magic numbers for timestamp offsets in Update() tests
constexpr int64_t kTsOffset1 = 1;
constexpr int64_t kTsOffset2 = 2;
constexpr int64_t kTsOffset3 = 3;
constexpr int64_t kTsOffset4 = 4;
constexpr int64_t kTsOffset5 = 5;
constexpr int64_t kTsOffset6 = 6;

/* =============================================================
 *  BundleActivePackageStats
 * ============================================================= */
bool PackageStatsFuzzTest(FuzzedDataProvider* fdp)
{
    // Default constructor
    BundleActivePackageStats stats1;

    // Populate fields
    stats1.bundleName_               = fdp->ConsumeRandomLengthString();
    stats1.beginTimeStamp_           = fdp->ConsumeIntegral<int64_t>();
    stats1.endTimeStamp_             = fdp->ConsumeIntegral<int64_t>();
    stats1.lastTimeUsed_             = fdp->ConsumeIntegral<int64_t>();
    stats1.totalInFrontTime_         = fdp->ConsumeIntegral<int64_t>();
    stats1.lastContiniousTaskUsed_   = fdp->ConsumeIntegral<int64_t>();
    stats1.totalContiniousTaskUsedTime_ = fdp->ConsumeIntegral<int64_t>();
    stats1.startCount_               = fdp->ConsumeIntegral<int32_t>();
    stats1.bundleStartedCount_       = fdp->ConsumeIntegral<int32_t>();
    stats1.lastEvent_                = fdp->ConsumeIntegral<int32_t>();
    stats1.userId_                   = fdp->ConsumeIntegral<int32_t>();
    stats1.uid_                      = fdp->ConsumeIntegral<int32_t>();
    stats1.appIndex_                 = fdp->ConsumeIntegral<int32_t>();

    // Copy constructor
    BundleActivePackageStats stats2(stats1);

    // Update()
    std::string longTask  = fdp->ConsumeRandomLengthString();
    int64_t ts            = fdp->ConsumeIntegral<int64_t>();
    int32_t eventId       = fdp->ConsumeIntegral<int32_t>();
    std::string abilityId = fdp->ConsumeRandomLengthString();
    int32_t uid           = fdp->ConsumeIntegral<int32_t>();
    stats1.Update(longTask, ts, eventId, abilityId, uid);
    stats1.Update("", ts + kTsOffset1, BundleActiveEvent::ABILITY_FOREGROUND, abilityId, uid);
    stats1.Update("", ts + kTsOffset2, BundleActiveEvent::ABILITY_BACKGROUND, abilityId, uid);
    stats1.Update(longTask, ts + kTsOffset3, BundleActiveEvent::LONG_TIME_TASK_STARTTED, abilityId, uid);
    stats1.Update(longTask, ts + kTsOffset4, BundleActiveEvent::LONG_TIME_TASK_ENDED, abilityId, uid);

    // IncrementTimeUsed, IncrementServiceTimeUsed, IncrementBundleLaunchedCount
    stats1.IncrementTimeUsed(ts + kTsOffset5);
    stats1.IncrementServiceTimeUsed(ts + kTsOffset6);
    stats1.IncrementBundleLaunchedCount();

    // ToString
    stats1.ToString();

    // Marshalling / Unmarshalling
    Parcel parcel;
    stats1.Marshalling(parcel);
    BundleActivePackageStats* restored = BundleActivePackageStats::Unmarshalling(parcel);
    if (restored != nullptr) {
        restored->ToString();
        delete restored;
    }
    return true;
}

/* =============================================================
 *  BundleActiveEventStats
 * ============================================================= */
bool EventStatsFuzzTest(FuzzedDataProvider* fdp)
{
    // Default constructor
    BundleActiveEventStats es1;
    es1.eventId_        = fdp->ConsumeIntegral<int32_t>();
    es1.beginTimeStamp_ = fdp->ConsumeIntegral<int64_t>();
    es1.endTimeStamp_   = fdp->ConsumeIntegral<int64_t>();
    es1.lastEventTime_  = fdp->ConsumeIntegral<int64_t>();
    es1.totalTime_      = fdp->ConsumeIntegral<int64_t>();
    es1.count_          = fdp->ConsumeIntegral<int32_t>();
    es1.uid_            = fdp->ConsumeIntegral<int32_t>();
    es1.name_           = fdp->ConsumeRandomLengthString();

    // Copy constructor
    BundleActiveEventStats es2(es1);

    // Getters
    es1.GetEventId();
    es1.GetFirstTimeStamp();
    es1.GetLastTimeStamp();
    es1.GetLastEventTime();
    es1.GetTotalTime();
    es1.GetCount();

    // add()
    es1.add(es2);

    // Marshalling / Unmarshalling
    Parcel parcel;
    es1.Marshalling(parcel);
    BundleActiveEventStats* restored = BundleActiveEventStats::Unmarshalling(parcel);
    if (restored != nullptr) {
        delete restored;
    }
    return true;
}

/* =============================================================
 *  BundleActiveFormRecord
 * ============================================================= */
bool FormRecordFuzzTest(FuzzedDataProvider* fdp)
{
    // Default constructor
    BundleActiveFormRecord fr1;

    // Parameterized constructor
    std::string formName    = fdp->ConsumeRandomLengthString();
    int32_t formDimension   = fdp->ConsumeIntegral<int32_t>();
    int64_t formId          = fdp->ConsumeIntegral<int64_t>();
    int64_t timeStamp       = fdp->ConsumeIntegral<int64_t>();
    int32_t userId          = fdp->ConsumeIntegral<int32_t>();
    int32_t uid             = fdp->ConsumeIntegral<int32_t>();
    BundleActiveFormRecord fr2(formName, formDimension, formId, timeStamp, userId, uid);

    // Copy constructor
    BundleActiveFormRecord fr3(fr2);

    // UpdateFormRecord
    int64_t newTs = fdp->ConsumeIntegral<int64_t>();
    fr2.UpdateFormRecord(newTs);

    // operator== (by record)
    bool eq1 = (fr2 == fr3);
    (void)eq1;

    // operator== (by id)
    int64_t testId = fdp->ConsumeIntegral<int64_t>();
    bool eq2 = (fr2 == testId);
    (void)eq2;

    // cmp
    BundleActiveFormRecord frA;
    BundleActiveFormRecord frB;
    frA.count_ = fdp->ConsumeIntegral<int32_t>();
    frB.count_ = fdp->ConsumeIntegral<int32_t>();
    bool cmpResult = BundleActiveFormRecord::cmp(frA, frB);
    (void)cmpResult;

    // ToString
    fr2.ToString();

    // Marshalling / Unmarshalling
    Parcel parcel;
    fr2.Marshalling(parcel);
    BundleActiveFormRecord* restored = BundleActiveFormRecord::Unmarshalling(parcel);
    if (restored != nullptr) {
        delete restored;
    }
    return true;
}

/* =============================================================
 *  AppGroupCallbackInfo
 * ============================================================= */
bool AppGroupCallbackInfoFuzzTest(FuzzedDataProvider* fdp)
{
    // Default constructor
    AppGroupCallbackInfo info1;

    // Parameterized constructor
    int32_t userId      = fdp->ConsumeIntegral<int32_t>();
    int32_t oldGroup    = fdp->ConsumeIntegral<int32_t>();
    int32_t newGroup    = fdp->ConsumeIntegral<int32_t>();
    uint32_t reason     = fdp->ConsumeIntegral<uint32_t>();
    std::string bundle  = fdp->ConsumeRandomLengthString();
    AppGroupCallbackInfo info2(userId, oldGroup, newGroup, reason, bundle);

    // Getters
    info2.GetUserId();
    info2.GetOldGroup();
    info2.GetNewGroup();
    info2.GetChangeReason();
    info2.GetBundleName();

    // Marshalling / Unmarshalling
    Parcel parcel;
    info2.Marshalling(parcel);
    AppGroupCallbackInfo* restored = AppGroupCallbackInfo::Unmarshalling(parcel);
    if (restored != nullptr) {
        restored->GetBundleName();
        delete restored;
    }
    return true;
}

/* =============================================================
 *  BundleActiveHighFrequencyPeriod
 * ============================================================= */
bool HighFrequencyPeriodFuzzTest(FuzzedDataProvider* fdp)
{
    // Default constructor
    BundleActiveHighFrequencyPeriod hfp1;

    // Parameterized constructor
    std::string bundleName = fdp->ConsumeRandomLengthString();
    int32_t vecSize        = fdp->ConsumeIntegralInRange<int32_t>(0, 24);
    std::vector<int32_t> hours;
    for (int32_t i = 0; i < vecSize; ++i) {
        hours.push_back(fdp->ConsumeIntegral<int32_t>());
    }
    BundleActiveHighFrequencyPeriod hfp2(bundleName, hours);

    // Copy constructor
    BundleActiveHighFrequencyPeriod hfp3(hfp2);

    // ToString
    hfp2.ToString();
    hfp1.ToString();

    // Marshalling / Unmarshalling
    Parcel parcel;
    hfp2.Marshalling(parcel);
    BundleActiveHighFrequencyPeriod* restored = BundleActiveHighFrequencyPeriod::Unmarshalling(parcel);
    if (restored != nullptr) {
        restored->ToString();
        delete restored;
    }

    // Empty case
    BundleActiveHighFrequencyPeriod hfpEmpty("", {});
    Parcel emptyParcel;
    hfpEmpty.Marshalling(emptyParcel);
    BundleActiveHighFrequencyPeriod* restoredEmpty = BundleActiveHighFrequencyPeriod::Unmarshalling(emptyParcel);
    if (restoredEmpty != nullptr) {
        delete restoredEmpty;
    }
    return true;
}

/* =============================================================
 *  BundleActiveEvent constructors and Marshalling
 * ============================================================= */
bool EventMarshallingFuzzTest(FuzzedDataProvider* fdp)
{
    // Default constructor
    BundleActiveEvent ev1;

    // Constructor: (eventId, timeStamp)
    int32_t eventId = fdp->ConsumeIntegral<int32_t>();
    int64_t ts = fdp->ConsumeIntegral<int64_t>();
    BundleActiveEvent ev2(eventId, ts);

    // Constructor: (eventId, bundleName, uid)
    std::string bundleName = fdp->ConsumeRandomLengthString();
    int32_t uid = fdp->ConsumeIntegral<int32_t>();
    BundleActiveEvent ev3(eventId, bundleName, uid);

    // Constructor: (bundleName, continuousTaskAbilityName, uid)
    std::string abilityName = fdp->ConsumeRandomLengthString();
    BundleActiveEvent ev4(bundleName, abilityName, uid);

    // Constructor: (bundleName, abilityName, abilityId, moduleName, uid)
    std::string abilityId = fdp->ConsumeRandomLengthString();
    std::string moduleName = fdp->ConsumeRandomLengthString();
    BundleActiveEvent ev5(bundleName, abilityName, abilityId, moduleName, uid);

    // Constructor: (bundleName, moduleName, formName, formDim, formId, eventId, uid)
    std::string formName = fdp->ConsumeRandomLengthString();
    int32_t formDimension = fdp->ConsumeIntegral<int32_t>();
    int64_t formId = fdp->ConsumeIntegral<int64_t>();
    BundleActiveEvent ev6(bundleName, moduleName, formName, formDimension, formId, eventId, uid);

    // Copy constructor
    BundleActiveEvent ev7(ev2);

    // ToString
    ev2.ToString();
    ev3.ToString();
    ev6.ToString();

    // operator=
    BundleActiveEvent ev8;
    ev8 = ev2;

    // IsBundleEvent, IsAppStateEvent
    (void)BundleActiveEvent::IsBundleEvent(eventId);
    (void)BundleActiveEvent::IsAppStateEvent(eventId);

    // PrintEvent
    bool debug = fdp->ConsumeBool();
    ev2.PrintEvent(debug);

    // Marshalling / Unmarshalling
    Parcel parcel;
    ev2.Marshalling(parcel);
    BundleActiveEvent* restored = BundleActiveEvent::Unmarshalling(parcel);
    if (restored != nullptr) {
        restored->ToString();
        delete restored;
    }

    return true;
}

/* =============================================================
 *  Stress: marshal/unmarshal in a loop
 * ============================================================= */
bool DataClassStressFuzzTest(FuzzedDataProvider* fdp)
{
    int32_t iterations = fdp->ConsumeIntegralInRange<int32_t>(0, 5);
    for (int32_t i = 0; i < iterations; ++i) {
        // PackageStats round-trip
        {
            BundleActivePackageStats ps;
            ps.bundleName_     = fdp->ConsumeRandomLengthString();
            ps.lastTimeUsed_   = fdp->ConsumeIntegral<int64_t>();
            ps.totalInFrontTime_ = fdp->ConsumeIntegral<int64_t>();
            Parcel p;
            ps.Marshalling(p);
            auto* r = BundleActivePackageStats::Unmarshalling(p);
            if (r) delete r;
        }
        // EventStats round-trip
        {
            BundleActiveEventStats es;
            es.eventId_   = fdp->ConsumeIntegral<int32_t>();
            es.totalTime_ = fdp->ConsumeIntegral<int64_t>();
            es.count_     = fdp->ConsumeIntegral<int32_t>();
            Parcel p;
            es.Marshalling(p);
            auto* r = BundleActiveEventStats::Unmarshalling(p);
            if (r) delete r;
        }
        // FormRecord round-trip
        {
            BundleActiveFormRecord fr(fdp->ConsumeRandomLengthString(),
                fdp->ConsumeIntegral<int32_t>(),
                fdp->ConsumeIntegral<int64_t>(),
                fdp->ConsumeIntegral<int64_t>(),
                fdp->ConsumeIntegral<int32_t>(),
                fdp->ConsumeIntegral<int32_t>());
            Parcel p;
            fr.Marshalling(p);
            auto* r = BundleActiveFormRecord::Unmarshalling(p);
            if (r) delete r;
        }
    }
    return true;
}

/* -------------------------------------------------------
 * Dispatch
 * ------------------------------------------------------- */
void FuzzDataClass(FuzzedDataProvider& fdp)
{
    int code = fdp.ConsumeIntegralInRange(FUZZ_CODE_0, FUZZ_CODE_MAX);
    switch (code) {
        case FUZZ_CODE_0:
            PackageStatsFuzzTest(&fdp);
            break;
        case FUZZ_CODE_1:
            EventStatsFuzzTest(&fdp);
            break;
        case FUZZ_CODE_2:
            FormRecordFuzzTest(&fdp);
            break;
        case FUZZ_CODE_3:
            AppGroupCallbackInfoFuzzTest(&fdp);
            break;
        case FUZZ_CODE_4:
            HighFrequencyPeriodFuzzTest(&fdp);
            break;
        case FUZZ_CODE_5:
            EventMarshallingFuzzTest(&fdp);
            break;
        case FUZZ_CODE_6:
            DataClassStressFuzzTest(&fdp);
            break;
        case FUZZ_CODE_7:
            // Chain all in sequence for cross-type coverage
            PackageStatsFuzzTest(&fdp);
            EventStatsFuzzTest(&fdp);
            FormRecordFuzzTest(&fdp);
            AppGroupCallbackInfoFuzzTest(&fdp);
            HighFrequencyPeriodFuzzTest(&fdp);
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
    OHOS::DeviceUsageStats::FuzzDataClass(fdp);
    return 0;
}
