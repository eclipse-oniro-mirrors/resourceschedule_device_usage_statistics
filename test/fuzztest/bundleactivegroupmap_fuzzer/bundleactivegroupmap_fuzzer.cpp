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
#include "bundleactivegroupmap_fuzzer.h"

#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>
#include <map>

#include "bundle_active_group_map.h"

namespace OHOS {
namespace DeviceUsageStats {
namespace {
constexpr size_t MAX_MUTATION_COUNT = 32;
constexpr int32_t MIN_GROUP_VALUE = -100;
constexpr int32_t MAX_GROUP_VALUE = 100;

void FuzzGroupIntervalMap(FuzzedDataProvider &provider)
{
    std::map<int32_t, int64_t> groupIntervalMap = DeviceUsageStatsGroupMap::groupIntervalMap_;
    size_t mutationCount = provider.ConsumeIntegralInRange<size_t>(0, MAX_MUTATION_COUNT);
    for (size_t index = 0; index < mutationCount && provider.remaining_bytes() > 0; ++index) {
        int32_t group = provider.ConsumeIntegralInRange<int32_t>(MIN_GROUP_VALUE, MAX_GROUP_VALUE);
        if (provider.ConsumeBool()) {
            groupIntervalMap[group] = provider.ConsumeIntegral<int64_t>();
        } else {
            (void)groupIntervalMap.erase(group);
        }
    }

    int32_t queryGroup = provider.ConsumeIntegralInRange<int32_t>(MIN_GROUP_VALUE, MAX_GROUP_VALUE);
    auto queryIter = groupIntervalMap.find(queryGroup);
    if (queryIter != groupIntervalMap.end()) {
        (void)queryIter->second;
    }

    for (const auto &entry : groupIntervalMap) {
        (void)(entry.first <= DeviceUsageStatsGroupConst::ACTIVE_GROUP_NEVER);
        (void)(entry.second >= 0);
    }
}
}

bool BundleActiveGroupMapFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return false;
    }

    FuzzedDataProvider provider(data, size);
    FuzzGroupIntervalMap(provider);
    return true;
}
}  // namespace DeviceUsageStats
}  // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    (void)OHOS::DeviceUsageStats::BundleActiveGroupMapFuzzTest(data, size);
    return 0;
}
