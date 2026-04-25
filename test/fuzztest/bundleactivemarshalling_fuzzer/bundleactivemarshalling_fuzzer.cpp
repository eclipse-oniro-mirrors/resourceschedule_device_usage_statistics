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

#include "bundleactivemarshalling_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>
#include <memory>

#include "app_group_callback_info.h"
#include "bundle_active_event.h"
#include "bundle_active_event_stats.h"
#include "bundle_active_form_record.h"
#include "bundle_active_module_record.h"
#include "bundle_active_package_stats.h"
#include "parcel.h"

namespace OHOS {
namespace DeviceUsageStats {
namespace {
constexpr size_t MAX_PARCEL_SIZE = 4096;
constexpr int32_t MAX_STRING_LENGTH = 256;
constexpr int32_t MAX_COLLECTION_SIZE = 10;

void FuzzBundleActiveEventMarshalling(FuzzedDataProvider &provider)
{
    BundleActiveEvent event;
    event.bundleName_ = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    event.continuousTaskAbilityName_ = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    event.abilityName_ = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    event.abilityId_ = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    event.moduleName_ = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    event.formName_ = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    event.formDimension_ = provider.ConsumeIntegral<int32_t>();
    event.formId_ = provider.ConsumeIntegral<int64_t>();
    event.timeStamp_ = provider.ConsumeIntegral<int64_t>();
    event.eventId_ = provider.ConsumeIntegralInRange<int32_t>(
        BundleActiveEvent::ABILITY_FOREGROUND, BundleActiveEvent::SYSTEM_WAKEUP);
    event.uid_ = provider.ConsumeIntegral<int32_t>();

    Parcel parcel;
    if (event.Marshalling(parcel)) {
        BundleActiveEvent *unmarshalled = BundleActiveEvent::Unmarshalling(parcel);
        if (unmarshalled != nullptr) {
            delete unmarshalled;
        }
    }
}

void FuzzBundleActiveEventUnmarshalling(FuzzedDataProvider &provider)
{
    size_t dataSize = provider.ConsumeIntegralInRange<size_t>(0, MAX_PARCEL_SIZE);
    std::vector<uint8_t> rawData = provider.ConsumeBytes<uint8_t>(dataSize);

    Parcel parcel;
    parcel.WriteBuffer(rawData.data(), rawData.size());
    parcel.RewindRead(0);

    BundleActiveEvent *event = BundleActiveEvent::Unmarshalling(parcel);
    if (event != nullptr) {
        event->ToString();
        event->PrintEvent(provider.ConsumeBool());
        delete event;
    }
}

void FuzzBundleActivePackageStatsMarshalling(FuzzedDataProvider &provider)
{
    BundleActivePackageStats stats;
    stats.bundleName_ = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    stats.beginTimeStamp_ = provider.ConsumeIntegral<int64_t>();
    stats.endTimeStamp_ = provider.ConsumeIntegral<int64_t>();
    stats.lastTimeUsed_ = provider.ConsumeIntegral<int64_t>();
    stats.totalInFrontTime_ = provider.ConsumeIntegral<int64_t>();
    stats.lastContiniousTaskUsed_ = provider.ConsumeIntegral<int64_t>();
    stats.totalContiniousTaskUsedTime_ = provider.ConsumeIntegral<int64_t>();
    stats.startCount_ = provider.ConsumeIntegral<int32_t>();
    stats.bundleStartedCount_ = provider.ConsumeIntegral<int32_t>();
    stats.lastEvent_ = provider.ConsumeIntegral<int32_t>();
    stats.userId_ = provider.ConsumeIntegral<int32_t>();
    stats.uid_ = provider.ConsumeIntegral<int32_t>();
    stats.appIndex_ = provider.ConsumeIntegral<int32_t>();

    int32_t abilityCount = provider.ConsumeIntegralInRange<int32_t>(0, MAX_COLLECTION_SIZE);
    for (int32_t i = 0; i < abilityCount; ++i) {
        std::string abilityId = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
        int32_t lastEvent = provider.ConsumeIntegral<int32_t>();
        stats.abilities_[abilityId] = lastEvent;
    }

    int32_t taskCount = provider.ConsumeIntegralInRange<int32_t>(0, MAX_COLLECTION_SIZE);
    for (int32_t i = 0; i < taskCount; ++i) {
        std::string taskName = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
        int32_t lastEvent = provider.ConsumeIntegral<int32_t>();
        stats.longTimeTasks_[taskName] = lastEvent;
    }

    Parcel parcel;
    if (stats.Marshalling(parcel)) {
        BundleActivePackageStats *unmarshalled = BundleActivePackageStats::Unmarshalling(parcel);
        if (unmarshalled != nullptr) {
            unmarshalled->ToString();
            delete unmarshalled;
        }
    }
}

void FuzzBundleActivePackageStatsUnmarshalling(FuzzedDataProvider &provider)
{
    size_t dataSize = provider.ConsumeIntegralInRange<size_t>(0, MAX_PARCEL_SIZE);
    std::vector<uint8_t> rawData = provider.ConsumeBytes<uint8_t>(dataSize);

    Parcel parcel;
    parcel.WriteBuffer(rawData.data(), rawData.size());
    parcel.RewindRead(0);

    BundleActivePackageStats *stats = BundleActivePackageStats::Unmarshalling(parcel);
    if (stats != nullptr) {
        stats->ToString();
        stats->IncrementBundleLaunchedCount();
        stats->IncrementTimeUsed(provider.ConsumeIntegral<int64_t>());
        stats->IncrementServiceTimeUsed(provider.ConsumeIntegral<int64_t>());
        delete stats;
    }
}

void FuzzBundleActiveEventStatsMarshalling(FuzzedDataProvider &provider)
{
    BundleActiveEventStats stats;
    stats.eventId_ = provider.ConsumeIntegral<int32_t>();
    stats.beginTimeStamp_ = provider.ConsumeIntegral<int64_t>();
    stats.endTimeStamp_ = provider.ConsumeIntegral<int64_t>();
    stats.lastEventTime_ = provider.ConsumeIntegral<int64_t>();
    stats.totalTime_ = provider.ConsumeIntegral<int64_t>();
    stats.count_ = provider.ConsumeIntegral<int32_t>();
    stats.uid_ = provider.ConsumeIntegral<int32_t>();
    stats.name_ = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);

    Parcel parcel;
    if (stats.Marshalling(parcel)) {
        BundleActiveEventStats *unmarshalled = BundleActiveEventStats::Unmarshalling(parcel);
        if (unmarshalled != nullptr) {
            unmarshalled->GetEventId();
            unmarshalled->GetFirstTimeStamp();
            unmarshalled->GetLastTimeStamp();
            unmarshalled->GetLastEventTime();
            unmarshalled->GetTotalTime();
            unmarshalled->GetCount();

            BundleActiveEventStats other;
            unmarshalled->add(other);
            delete unmarshalled;
        }
    }
}

void FuzzBundleActiveEventStatsUnmarshalling(FuzzedDataProvider &provider)
{
    size_t dataSize = provider.ConsumeIntegralInRange<size_t>(0, MAX_PARCEL_SIZE);
    std::vector<uint8_t> rawData = provider.ConsumeBytes<uint8_t>(dataSize);

    Parcel parcel;
    parcel.WriteBuffer(rawData.data(), rawData.size());
    parcel.RewindRead(0);

    BundleActiveEventStats *stats = BundleActiveEventStats::Unmarshalling(parcel);
    if (stats != nullptr) {
        delete stats;
    }
}

void FuzzBundleActiveFormRecordMarshalling(FuzzedDataProvider &provider)
{
    BundleActiveFormRecord record;
    record.formName_ = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    record.formDimension_ = provider.ConsumeIntegral<int32_t>();
    record.formId_ = provider.ConsumeIntegral<int64_t>();
    record.formLastUsedTime_ = provider.ConsumeIntegral<int64_t>();
    record.count_ = provider.ConsumeIntegral<int32_t>();
    record.userId_ = provider.ConsumeIntegral<int32_t>();
    record.uid_ = provider.ConsumeIntegral<int32_t>();

    Parcel parcel;
    if (record.Marshalling(parcel)) {
        BundleActiveFormRecord *unmarshalled = BundleActiveFormRecord::Unmarshalling(parcel);
        if (unmarshalled != nullptr) {
            unmarshalled->ToString();
            unmarshalled->UpdateFormRecord(provider.ConsumeIntegral<int64_t>());
            delete unmarshalled;
        }
    }
}

void FuzzBundleActiveFormRecordUnmarshalling(FuzzedDataProvider &provider)
{
    size_t dataSize = provider.ConsumeIntegralInRange<size_t>(0, MAX_PARCEL_SIZE);
    std::vector<uint8_t> rawData = provider.ConsumeBytes<uint8_t>(dataSize);

    Parcel parcel;
    parcel.WriteBuffer(rawData.data(), rawData.size());
    parcel.RewindRead(0);

    BundleActiveFormRecord *record = BundleActiveFormRecord::Unmarshalling(parcel);
    if (record != nullptr) {
        delete record;
    }
}

void FuzzBundleActiveModuleRecordMarshalling(FuzzedDataProvider &provider)
{
    BundleActiveModuleRecord record;
    record.deviceId_ = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    record.bundleName_ = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    record.moduleName_ = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    record.abilityName_ = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    record.appLabelId_ = provider.ConsumeIntegral<uint32_t>();
    record.labelId_ = provider.ConsumeIntegral<uint32_t>();
    record.descriptionId_ = provider.ConsumeIntegral<uint32_t>();
    record.abilityLableId_ = provider.ConsumeIntegral<uint32_t>();
    record.abilityDescriptionId_ = provider.ConsumeIntegral<uint32_t>();
    record.abilityIconId_ = provider.ConsumeIntegral<uint32_t>();
    record.launchedCount_ = provider.ConsumeIntegral<int32_t>();
    record.lastModuleUsedTime_ = provider.ConsumeIntegral<int64_t>();
    record.userId_ = provider.ConsumeIntegral<int32_t>();
    record.uid_ = provider.ConsumeIntegral<int32_t>();
    record.removed_ = provider.ConsumeBool();
    record.installFreeSupported_ = provider.ConsumeBool();
    record.isNewAdded_ = provider.ConsumeBool();

    int32_t formCount = provider.ConsumeIntegralInRange<int32_t>(0, MAX_COLLECTION_SIZE);
    for (int32_t i = 0; i < formCount; ++i) {
        BundleActiveFormRecord formRecord;
        formRecord.formName_ = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
        formRecord.formDimension_ = provider.ConsumeIntegral<int32_t>();
        formRecord.formId_ = provider.ConsumeIntegral<int64_t>();
        formRecord.formLastUsedTime_ = provider.ConsumeIntegral<int64_t>();
        formRecord.count_ = provider.ConsumeIntegral<int32_t>();
        record.formRecords_.push_back(formRecord);
    }

    Parcel parcel;
    if (record.Marshalling(parcel)) {
        BundleActiveModuleRecord *unmarshalled = BundleActiveModuleRecord::Unmarshalling(parcel);
        if (unmarshalled != nullptr) {
            unmarshalled->ToString();
            unmarshalled->UpdateModuleRecord(provider.ConsumeIntegral<int64_t>());
            unmarshalled->AddOrUpdateOneFormRecord(
                provider.ConsumeRandomLengthString(MAX_STRING_LENGTH),
                provider.ConsumeIntegral<int32_t>(),
                provider.ConsumeIntegral<int64_t>(),
                provider.ConsumeIntegral<int64_t>(),
                provider.ConsumeIntegral<int32_t>());
            delete unmarshalled;
        }
    }
}

void FuzzBundleActiveModuleRecordUnmarshalling(FuzzedDataProvider &provider)
{
    size_t dataSize = provider.ConsumeIntegralInRange<size_t>(0, MAX_PARCEL_SIZE);
    std::vector<uint8_t> rawData = provider.ConsumeBytes<uint8_t>(dataSize);

    Parcel parcel;
    parcel.WriteBuffer(rawData.data(), rawData.size());
    parcel.RewindRead(0);

    BundleActiveModuleRecord *record = BundleActiveModuleRecord::Unmarshalling(parcel);
    if (record != nullptr) {
        delete record;
    }
}

void FuzzAppGroupCallbackInfoMarshalling(FuzzedDataProvider &provider)
{
    int32_t userId = provider.ConsumeIntegral<int32_t>();
    int32_t oldGroup = provider.ConsumeIntegral<int32_t>();
    int32_t newGroup = provider.ConsumeIntegral<int32_t>();
    uint32_t changeReason = provider.ConsumeIntegral<uint32_t>();
    std::string bundleName = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);

    AppGroupCallbackInfo info(userId, oldGroup, newGroup, changeReason, bundleName);

    Parcel parcel;
    if (info.Marshalling(parcel)) {
        AppGroupCallbackInfo *unmarshalled = AppGroupCallbackInfo::Unmarshalling(parcel);
        if (unmarshalled != nullptr) {
            unmarshalled->GetUserId();
            unmarshalled->GetOldGroup();
            unmarshalled->GetNewGroup();
            unmarshalled->GetChangeReason();
            unmarshalled->GetBundleName();
            delete unmarshalled;
        }
    }
}

void FuzzAppGroupCallbackInfoUnmarshalling(FuzzedDataProvider &provider)
{
    size_t dataSize = provider.ConsumeIntegralInRange<size_t>(0, MAX_PARCEL_SIZE);
    std::vector<uint8_t> rawData = provider.ConsumeBytes<uint8_t>(dataSize);

    Parcel parcel;
    parcel.WriteBuffer(rawData.data(), rawData.size());
    parcel.RewindRead(0);

    AppGroupCallbackInfo *info = AppGroupCallbackInfo::Unmarshalling(parcel);
    if (info != nullptr) {
        delete info;
    }
}

enum FuzzCase : uint8_t {
    EVENT_MARSHALLING = 0,
    EVENT_UNMARSHALLING,
    PACKAGE_STATS_MARSHALLING,
    PACKAGE_STATS_UNMARSHALLING,
    EVENT_STATS_MARSHALLING,
    EVENT_STATS_UNMARSHALLING,
    FORM_RECORD_MARSHALLING,
    FORM_RECORD_UNMARSHALLING,
    MODULE_RECORD_MARSHALLING,
    MODULE_RECORD_UNMARSHALLING,
    APP_GROUP_INFO_MARSHALLING,
    APP_GROUP_INFO_UNMARSHALLING,
    FUZZ_CASE_COUNT,
};

void DispatchFuzzCase(FuzzedDataProvider &provider)
{
    switch (provider.ConsumeIntegralInRange<uint8_t>(EVENT_MARSHALLING, FUZZ_CASE_COUNT - 1)) {
        case EVENT_MARSHALLING:
            FuzzBundleActiveEventMarshalling(provider);
            break;
        case EVENT_UNMARSHALLING:
            FuzzBundleActiveEventUnmarshalling(provider);
            break;
        case PACKAGE_STATS_MARSHALLING:
            FuzzBundleActivePackageStatsMarshalling(provider);
            break;
        case PACKAGE_STATS_UNMARSHALLING:
            FuzzBundleActivePackageStatsUnmarshalling(provider);
            break;
        case EVENT_STATS_MARSHALLING:
            FuzzBundleActiveEventStatsMarshalling(provider);
            break;
        case EVENT_STATS_UNMARSHALLING:
            FuzzBundleActiveEventStatsUnmarshalling(provider);
            break;
        case FORM_RECORD_MARSHALLING:
            FuzzBundleActiveFormRecordMarshalling(provider);
            break;
        case FORM_RECORD_UNMARSHALLING:
            FuzzBundleActiveFormRecordUnmarshalling(provider);
            break;
        case MODULE_RECORD_MARSHALLING:
            FuzzBundleActiveModuleRecordMarshalling(provider);
            break;
        case MODULE_RECORD_UNMARSHALLING:
            FuzzBundleActiveModuleRecordUnmarshalling(provider);
            break;
        case APP_GROUP_INFO_MARSHALLING:
            FuzzAppGroupCallbackInfoMarshalling(provider);
            break;
        case APP_GROUP_INFO_UNMARSHALLING:
            FuzzAppGroupCallbackInfoUnmarshalling(provider);
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