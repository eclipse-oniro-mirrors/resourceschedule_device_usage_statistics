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

#include "bundleactivepackagestats_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>

#include "bundle_active_event.h"
#include "bundle_active_package_stats.h"

namespace OHOS {
namespace DeviceUsageStats {
namespace {
constexpr int32_t MAX_STRING_LENGTH = 128;
constexpr int32_t MAX_OPERATIONS = 30;
constexpr int32_t MAX_NUM = 100;

void FuzzPackageStatsUpdate(FuzzedDataProvider &provider)
{
    BundleActivePackageStats stats;
    stats.bundleName_ = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    stats.userId_ = provider.ConsumeIntegral<int32_t>();
    stats.uid_ = provider.ConsumeIntegral<int32_t>();

    int32_t operationCount = provider.ConsumeIntegralInRange<int32_t>(1, MAX_OPERATIONS);
    for (int32_t i = 0; i < operationCount; ++i) {
        std::string longTimeTaskName = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
        int64_t timeStamp = provider.ConsumeIntegral<int64_t>();
        int32_t eventId = provider.ConsumeIntegralInRange<int32_t>(
            BundleActiveEvent::ABILITY_FOREGROUND, BundleActiveEvent::SYSTEM_WAKEUP);
        std::string abilityId = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
        int32_t uid = provider.ConsumeIntegral<int32_t>();

        stats.Update(longTimeTaskName, timeStamp, eventId, abilityId, uid);
    }

    stats.ToString();
}

void FuzzPackageStatsTimeOperations(FuzzedDataProvider &provider)
{
    BundleActivePackageStats stats;
    stats.bundleName_ = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    stats.beginTimeStamp_ = provider.ConsumeIntegral<int64_t>();
    stats.endTimeStamp_ = provider.ConsumeIntegral<int64_t>();
    stats.lastTimeUsed_ = provider.ConsumeIntegral<int64_t>();

    int32_t operationCount = provider.ConsumeIntegralInRange<int32_t>(1, MAX_OPERATIONS);
    for (int32_t i = 0; i < operationCount; ++i) {
        int32_t operation = provider.ConsumeIntegralInRange<int32_t>(0, 2);
        enum class StatsOp : uint8_t {
        INCREMENT_TIME_USED = 0,     // 增加应用使用时间
        INCREMENT_SERVICE_TIME_USED, // 增加服务使用时间
        INCREMENT_LAUNCH_COUNT,      // 增加应用启动次数
        MAX_OP                       // 用于限制 Fuzzer 的随机范围
        };
        switch (static_cast<StatsOp>(operation)) {
            case StatsOp::INCREMENT_TIME_USED:
                stats.IncrementTimeUsed(provider.ConsumeIntegral<int64_t>());
                break;
            case StatsOp::INCREMENT_SERVICE_TIME_USED:
                stats.IncrementServiceTimeUsed(provider.ConsumeIntegral<int64_t>());
                break;
            case StatsOp::INCREMENT_LAUNCH_COUNT:
                stats.IncrementBundleLaunchedCount();
                break;
            default:
                break;
        }
    }
}

void FuzzPackageStatsAbilityTracking(FuzzedDataProvider &provider)
{
    BundleActivePackageStats stats;
    stats.bundleName_ = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);

    int32_t abilityCount = provider.ConsumeIntegralInRange<int32_t>(1, 20);
    for (int32_t i = 0; i < abilityCount; ++i) {
        std::string abilityId = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
        int64_t timeStamp = provider.ConsumeIntegral<int64_t>();
        int32_t eventId = provider.ConsumeIntegralInRange<int32_t>(
            BundleActiveEvent::ABILITY_FOREGROUND, BundleActiveEvent::ABILITY_STOP);

        stats.Update("", timeStamp, eventId, abilityId, provider.ConsumeIntegral<int32_t>());
    }
}

void FuzzPackageStatsLongTimeTaskTracking(FuzzedDataProvider &provider)
{
    BundleActivePackageStats stats;
    stats.bundleName_ = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);

    int32_t taskCount = provider.ConsumeIntegralInRange<int32_t>(1, 20);
    for (int32_t i = 0; i < taskCount; ++i) {
        std::string taskName = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
        int64_t timeStamp = provider.ConsumeIntegral<int64_t>();
        int32_t eventId = provider.ConsumeIntegralInRange<int32_t>(
            BundleActiveEvent::LONG_TIME_TASK_STARTTED, BundleActiveEvent::LONG_TIME_TASK_ENDED);

        stats.Update(taskName, timeStamp, eventId, "", provider.ConsumeIntegral<int32_t>());
    }
}

void FuzzPackageStatsMixedScenario(FuzzedDataProvider &provider)
{
    BundleActivePackageStats stats;
    stats.bundleName_ = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    stats.userId_ = provider.ConsumeIntegral<int32_t>();
    stats.uid_ = provider.ConsumeIntegral<int32_t>();
    stats.appIndex_ = provider.ConsumeIntegral<int32_t>();

    int32_t operationCount = provider.ConsumeIntegralInRange<int32_t>(1, MAX_OPERATIONS);
    for (int32_t i = 0; i < operationCount; ++i) {
        int32_t scenario = provider.ConsumeIntegralInRange<int32_t>(0, 5);
        enum class StatsScenario : uint8_t {
        UPDATE_ABILITY_FOREGROUND = 0, // 更新 Ability 前台状态
        UPDATE_ABILITY_BACKGROUND,     // 更新 Ability 后台状态
        UPDATE_TASK_STARTED,           // 长时间任务开始
        UPDATE_TASK_ENDED,             // 长时间任务结束
        INCREMENT_TIME_USED,           // 增加使用时间
        INCREMENT_LAUNCH_COUNT,        // 增加启动次数
        MAX_SCENARIO                   // 用于限制 Fuzzer 的随机范围
    };
       switch (static_cast<StatsScenario>(scenario)) {
            case StatsScenario::UPDATE_ABILITY_FOREGROUND: {
                std::string abilityId = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
                stats.Update("", provider.ConsumeIntegral<int64_t>(),
                    BundleActiveEvent::ABILITY_FOREGROUND, abilityId,
                    provider.ConsumeIntegral<int32_t>());
                break;
            }
            case StatsScenario::UPDATE_ABILITY_BACKGROUND: {
                std::string abilityId = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
                stats.Update("", provider.ConsumeIntegral<int64_t>(),
                    BundleActiveEvent::ABILITY_BACKGROUND, abilityId,
                    provider.ConsumeIntegral<int32_t>());
                break;
            }
            case StatsScenario::UPDATE_TASK_STARTED: {
                std::string taskName = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
                stats.Update(taskName, provider.ConsumeIntegral<int64_t>(),
                    BundleActiveEvent::LONG_TIME_TASK_STARTTED, "",
                    provider.ConsumeIntegral<int32_t>());
                break;
            }
            case StatsScenario::UPDATE_TASK_ENDED: {
                std::string taskName = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
                stats.Update(taskName, provider.ConsumeIntegral<int64_t>(),
                    BundleActiveEvent::LONG_TIME_TASK_ENDED, "",
                    provider.ConsumeIntegral<int32_t>());
                break;
            }
            case StatsScenario::INCREMENT_TIME_USED:
                stats.IncrementTimeUsed(provider.ConsumeIntegral<int64_t>());
                break;
            case StatsScenario::INCREMENT_LAUNCH_COUNT:
                stats.IncrementBundleLaunchedCount();
                break;
            default:
                break;
        }
    }

    stats.ToString();
}

void FuzzPackageStatsCopyAndAssignment(FuzzedDataProvider &provider)
{
    BundleActivePackageStats stats1;
    stats1.bundleName_ = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    stats1.beginTimeStamp_ = provider.ConsumeIntegral<int64_t>();
    stats1.endTimeStamp_ = provider.ConsumeIntegral<int64_t>();
    stats1.lastTimeUsed_ = provider.ConsumeIntegral<int64_t>();
    stats1.totalInFrontTime_ = provider.ConsumeIntegral<int64_t>();
    stats1.userId_ = provider.ConsumeIntegral<int32_t>();

    int32_t abilityCount = provider.ConsumeIntegralInRange<int32_t>(0, 5);
    for (int32_t i = 0; i < abilityCount; ++i) {
        stats1.abilities_[provider.ConsumeRandomLengthString(MAX_STRING_LENGTH)] =
            provider.ConsumeIntegral<int32_t>();
    }

    BundleActivePackageStats stats2(stats1);
    stats2.IncrementTimeUsed(provider.ConsumeIntegral<int64_t>());

    BundleActivePackageStats stats3;
    stats3.bundleName_ = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    stats3.Update("", provider.ConsumeIntegral<int64_t>(),
        BundleActiveEvent::ABILITY_FOREGROUND,
        provider.ConsumeRandomLengthString(MAX_STRING_LENGTH),
        provider.ConsumeIntegral<int32_t>());
}

void FuzzPackageStatsEdgeCases(FuzzedDataProvider &provider)
{
    BundleActivePackageStats stats;
    stats.bundleName_ = "";
    stats.Update("", 0, BundleActiveEvent::ABILITY_FOREGROUND, "", 0);
    stats.IncrementTimeUsed(0);
    stats.IncrementServiceTimeUsed(0);
    stats.IncrementBundleLaunchedCount();

    stats.bundleName_ = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    stats.Update("", -1, BundleActiveEvent::ABILITY_FOREGROUND, "", -1);
    stats.IncrementTimeUsed(-1);
    stats.IncrementServiceTimeUsed(-1);

    stats.Update("", INT64_MAX, BundleActiveEvent::ABILITY_FOREGROUND, "", INT32_MAX);
    stats.IncrementTimeUsed(INT64_MAX);
    stats.IncrementServiceTimeUsed(INT64_MAX);

    for (int32_t i = 0; i < MAX_NUM; ++i) {
        stats.IncrementBundleLaunchedCount();
    }
}

enum FuzzCase : uint8_t {
    PACKAGE_STATS_UPDATE = 0,
    PACKAGE_STATS_TIME_OPS,
    PACKAGE_STATS_ABILITY_TRACKING,
    PACKAGE_STATS_TASK_TRACKING,
    PACKAGE_STATS_MIXED,
    PACKAGE_STATS_COPY,
    PACKAGE_STATS_EDGE_CASES,
    FUZZ_CASE_COUNT,
};

void DispatchFuzzCase(FuzzedDataProvider &provider)
{
    switch (provider.ConsumeIntegralInRange<uint8_t>(PACKAGE_STATS_UPDATE, FUZZ_CASE_COUNT - 1)) {
        case PACKAGE_STATS_UPDATE:
            FuzzPackageStatsUpdate(provider);
            break;
        case PACKAGE_STATS_TIME_OPS:
            FuzzPackageStatsTimeOperations(provider);
            break;
        case PACKAGE_STATS_ABILITY_TRACKING:
            FuzzPackageStatsAbilityTracking(provider);
            break;
        case PACKAGE_STATS_TASK_TRACKING:
            FuzzPackageStatsLongTimeTaskTracking(provider);
            break;
        case PACKAGE_STATS_MIXED:
            FuzzPackageStatsMixedScenario(provider);
            break;
        case PACKAGE_STATS_COPY:
            FuzzPackageStatsCopyAndAssignment(provider);
            break;
        case PACKAGE_STATS_EDGE_CASES:
            FuzzPackageStatsEdgeCases(provider);
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
