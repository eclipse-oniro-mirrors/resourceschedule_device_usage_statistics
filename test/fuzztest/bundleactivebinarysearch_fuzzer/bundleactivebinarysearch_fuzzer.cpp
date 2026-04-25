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

#include "bundleactivebinarysearch_fuzzer.h"

#include <algorithm>
#include <fuzzer/FuzzedDataProvider.h>
#include <vector>

#include "bundle_active_binary_search.h"

namespace OHOS {
namespace DeviceUsageStats {
namespace {
constexpr size_t MAX_VECTOR_SIZE = 128;
constexpr size_t MAX_TARGET_COUNT = 8;
constexpr int64_t MAX_STEP = 1000000;
constexpr int64_t MIN_BASE_VALUE = -1000000000;
constexpr int64_t MAX_BASE_VALUE = 1000000000;

std::vector<int64_t> BuildSearchTable(FuzzedDataProvider &provider)
{
    size_t tableSize = provider.ConsumeIntegralInRange<size_t>(0, MAX_VECTOR_SIZE);
    std::vector<int64_t> searchTable;
    searchTable.reserve(tableSize);

    int64_t currentValue = provider.ConsumeIntegralInRange<int64_t>(MIN_BASE_VALUE, MAX_BASE_VALUE);
    for (size_t index = 0; index < tableSize && provider.remaining_bytes() > 0; ++index) {
        if (provider.ConsumeBool()) {
            currentValue += provider.ConsumeIntegralInRange<int64_t>(0, MAX_STEP);
            searchTable.emplace_back(currentValue);
        } else {
            searchTable.emplace_back(provider.ConsumeIntegral<int64_t>());
        }
    }

    if (provider.ConsumeBool()) {
        std::sort(searchTable.begin(), searchTable.end());
    }
    return searchTable;
}

void FuzzBinarySearch(FuzzedDataProvider &provider)
{
    std::vector<int64_t> searchTable = BuildSearchTable(provider);
    BundleActiveBinarySearch binarySearch;

    (void)binarySearch.BinarySearch(searchTable, provider.ConsumeIntegral<int64_t>());
    if (!searchTable.empty()) {
        size_t targetCount = provider.ConsumeIntegralInRange<size_t>(1, MAX_TARGET_COUNT);
        for (size_t index = 0; index < targetCount && provider.remaining_bytes() > 0; ++index) {
            size_t targetIndex = provider.ConsumeIntegralInRange<size_t>(0, searchTable.size() - 1);
            (void)binarySearch.BinarySearch(searchTable, searchTable[targetIndex]);
        }
    }
}
}

bool BundleActiveBinarySearchFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return false;
    }

    FuzzedDataProvider provider(data, size);
    FuzzBinarySearch(provider);
    return true;
}
}  // namespace DeviceUsageStats
}  // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    (void)OHOS::DeviceUsageStats::BundleActiveBinarySearchFuzzTest(data, size);
    return 0;
}
