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

#include "bundleactiveutil_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>
#include <string>
#include <vector>

#include "bundle_active_util.h"

namespace OHOS {
namespace DeviceUsageStats {
namespace {
constexpr size_t MAX_STRING_LENGTH = 128;
constexpr int32_t MAX_NUM = 32;
constexpr size_t MAX_PATH_SUFFIX_LENGTH = 64;
constexpr int64_t MIN_TIMESTAMP = -86400000LL;
constexpr int64_t MAX_TIMESTAMP = 4102444800000LL;
constexpr int32_t MIN_INTERVAL_TYPE = -4;
constexpr int32_t MAX_INTERVAL_TYPE = 8;

std::string MakeNonExistingPath(FuzzedDataProvider &provider)
{
    static const std::vector<std::string> pathPrefixes = {
        "/data/local/tmp/nonexistent_bundle_active_",
        "/tmp/nonexistent_bundle_active_",
        "relative_nonexistent_bundle_active_",
    };
    size_t prefixIndex = provider.ConsumeIntegralInRange<size_t>(0, pathPrefixes.size() - 1);
    std::string pathSuffix = provider.ConsumeRandomLengthString(MAX_PATH_SUFFIX_LENGTH);
    for (char &character : pathSuffix) {
        if (character == '/' || character == '\0') {
            character = '_';
        }
    }
    return pathPrefixes[prefixIndex] + pathSuffix;
}

void FuzzNumericHelpers(FuzzedDataProvider &provider)
{
    std::string rawString = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    std::string numericString = std::to_string(provider.ConsumeIntegral<int64_t>());
    if (provider.ConsumeBool()) {
        numericString += provider.ConsumeRandomLengthString(MAX_NUM);
    }

    (void)BundleActiveUtil::StringToInt32(rawString);
    (void)BundleActiveUtil::StringToInt64(rawString);
    (void)BundleActiveUtil::StringToInt32(numericString);
    (void)BundleActiveUtil::StringToInt64(numericString);
    (void)BundleActiveUtil::GetBundleUsageKey(rawString, provider.ConsumeIntegral<int32_t>());
    int64_t delayTime = provider.ConsumeIntegralInRange<int64_t>(-MAX_TIMESTAMP, MAX_TIMESTAMP);
    (void)BundleActiveUtil::GetFFRTDelayTime(delayTime);
}

void FuzzTimeHelpers(FuzzedDataProvider &provider)
{
    int64_t timeStamp = provider.ConsumeIntegralInRange<int64_t>(MIN_TIMESTAMP, MAX_TIMESTAMP);
    int32_t intervalType = provider.ConsumeIntegralInRange<int32_t>(MIN_INTERVAL_TYPE, MAX_INTERVAL_TYPE);

    (void)BundleActiveUtil::GetIntervalTypeStartTime(timeStamp, intervalType);
    (void)BundleActiveUtil::GetSystemTimeMs();
    (void)BundleActiveUtil::GetNowMicroTime();
}

void FuzzPathHelpers(FuzzedDataProvider &provider)
{
    std::string nonExistingPath = MakeNonExistingPath(provider);
    (void)BundleActiveUtil::GetPartitionName(nonExistingPath);
    (void)BundleActiveUtil::GetFolderOrFileSize(nonExistingPath);
    (void)BundleActiveUtil::GetDeviceValidSize(nonExistingPath);
    (void)BundleActiveUtil::GetPercentOfAvailableUserSpace(nonExistingPath);
}
}

bool BundleActiveUtilFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return false;
    }

    FuzzedDataProvider provider(data, size);
    FuzzNumericHelpers(provider);
    if (provider.remaining_bytes() > 0) {
        FuzzTimeHelpers(provider);
    }
    if (provider.remaining_bytes() > 0) {
        FuzzPathHelpers(provider);
    }
    return true;
}
}  // namespace DeviceUsageStats
}  // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    (void)OHOS::DeviceUsageStats::BundleActiveUtilFuzzTest(data, size);
    return 0;
}
