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

#include "bundleactivehighfrequencyperiod_fuzzer.h"

#include <algorithm>
#include <fuzzer/FuzzedDataProvider.h>
#include <memory>
#include <string>
#include <vector>

#include "bundle_active_high_frequency_period.h"
#include "parcel.h"

namespace OHOS {
namespace DeviceUsageStats {
namespace {
constexpr int32_t MIN_FUZZ_HOUR = -48;
constexpr int32_t MAX_FUZZ_HOUR = 72;
constexpr size_t MAX_BUNDLE_NAME_LENGTH = 256;
constexpr size_t MAX_HOUR_VECTOR_SIZE = 32;
constexpr size_t MAX_RAW_PARCEL_SIZE = 4096;

std::vector<int32_t> BuildHighFrequencyHours(FuzzedDataProvider &provider)
{
    size_t hourCount = provider.ConsumeIntegralInRange<size_t>(0, MAX_HOUR_VECTOR_SIZE);
    std::vector<int32_t> highFrequencyHours;
    highFrequencyHours.reserve(hourCount);
    for (size_t index = 0; index < hourCount && provider.remaining_bytes() > 0; ++index) {
        if (provider.ConsumeBool()) {
            highFrequencyHours.emplace_back(provider.ConsumeIntegralInRange<int32_t>(MIN_FUZZ_HOUR, MAX_FUZZ_HOUR));
        } else {
            highFrequencyHours.emplace_back(provider.ConsumeIntegral<int32_t>());
        }
    }
    return highFrequencyHours;
}

void FuzzHighFrequencyPeriodRoundTrip(FuzzedDataProvider &provider)
{
    std::string bundleName = provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH);
    std::vector<int32_t> highFrequencyHours = BuildHighFrequencyHours(provider);
    BundleActiveHighFrequencyPeriod period(bundleName, highFrequencyHours);
    BundleActiveHighFrequencyPeriod copiedPeriod(period);

    (void)period.ToString();
    (void)copiedPeriod.ToString();

    Parcel parcel;
    if (period.Marshalling(parcel)) {
        parcel.RewindRead(0);
        std::unique_ptr<BundleActiveHighFrequencyPeriod> restoredPeriod(
            BundleActiveHighFrequencyPeriod::Unmarshalling(parcel));
        if (restoredPeriod != nullptr) {
            (void)restoredPeriod->ToString();
        }
    }
}

void FuzzHighFrequencyPeriodManualParcel(FuzzedDataProvider &provider)
{
    Parcel parcel;
    (void)parcel.WriteString(provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH));
    uint32_t declaredHourCount = provider.ConsumeIntegralInRange<uint32_t>(0, MAX_HOUR_VECTOR_SIZE + 16);
    (void)parcel.WriteUint32(declaredHourCount);

    size_t writtenHourCount = provider.ConsumeIntegralInRange<size_t>(
        0, std::min<size_t>(declaredHourCount, MAX_HOUR_VECTOR_SIZE));
    for (size_t index = 0; index < writtenHourCount && provider.remaining_bytes() > 0; ++index) {
        (void)parcel.WriteInt32(provider.ConsumeIntegral<int32_t>());
    }

    parcel.RewindRead(0);
    std::unique_ptr<BundleActiveHighFrequencyPeriod> period(BundleActiveHighFrequencyPeriod::Unmarshalling(parcel));
    if (period != nullptr) {
        (void)period->ToString();
    }
}

void FuzzHighFrequencyPeriodRawParcel(FuzzedDataProvider &provider)
{
    size_t rawDataSize = provider.ConsumeIntegralInRange<size_t>(0, MAX_RAW_PARCEL_SIZE);
    std::vector<uint8_t> rawData = provider.ConsumeBytes<uint8_t>(rawDataSize);

    Parcel parcel;
    if (!rawData.empty()) {
        (void)parcel.WriteBuffer(rawData.data(), rawData.size());
    }
    parcel.RewindRead(0);

    std::unique_ptr<BundleActiveHighFrequencyPeriod> period(BundleActiveHighFrequencyPeriod::Unmarshalling(parcel));
    if (period != nullptr) {
        (void)period->ToString();
    }
}
}

bool BundleActiveHighFrequencyPeriodFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return false;
    }

    FuzzedDataProvider provider(data, size);
    FuzzHighFrequencyPeriodRoundTrip(provider);
    if (provider.remaining_bytes() > 0) {
        FuzzHighFrequencyPeriodManualParcel(provider);
    }
    if (provider.remaining_bytes() > 0) {
        FuzzHighFrequencyPeriodRawParcel(provider);
    }
    return true;
}
}  // namespace DeviceUsageStats
}  // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    (void)OHOS::DeviceUsageStats::BundleActiveHighFrequencyPeriodFuzzTest(data, size);
    return 0;
}
