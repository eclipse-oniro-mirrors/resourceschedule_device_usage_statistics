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

#include "bundleactivevalueobject_fuzzer.h"

#include <climits>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <fuzzer/FuzzedDataProvider.h>

#include "app_group_callback_info.h"
#include "bundle_active_event.h"
#include "bundle_active_event_stats.h"
#include "bundle_active_form_record.h"
#include "bundle_active_high_frequency_period.h"
#include "bundle_active_package_stats.h"
#include "message_parcel.h"

namespace OHOS {
namespace DeviceUsageStats {
namespace {

// ======= 全局常量 =======
constexpr size_t MAX_OPS        = 32;
constexpr size_t MAX_NAME_LEN   = 32;
constexpr size_t MAX_ABILITY_IDS = 4;
constexpr size_t MAX_TASK_NAMES  = 4;
constexpr int32_t MAX_HOUR_NUM   = 24;
constexpr int64_t MIN_FUZZ_TIME  = 1;
constexpr int64_t MAX_FUZZ_TIME  = 4102444800000LL; // 2100-01-01 00:00:00 UTC in ms.
constexpr int64_t MAX_FUZZ_SPAN  = 1000000LL;
constexpr int64_t MIDDLE_TIME_MARGIN = 16;
constexpr int64_t ONE_STEP_DELTA = 1;
constexpr int64_t BACKWARD_STEP_DELTA = -1;
constexpr int64_t ABILITY_FOREGROUND_AGAIN_DELTA = 2;
constexpr int64_t ABILITY_BACKGROUND_DELTA = 3;
constexpr int64_t ABILITY_REFOREGROUND_DELTA = 4;
constexpr int64_t TASK_RESTART_DELTA = 5;
constexpr int64_t TASK_END_DELTA = 6;
constexpr int64_t TASK_START_AGAIN_DELTA = 7;
constexpr int64_t BATCH_EVENT_DELTA = 8;
constexpr uint8_t ACTION_ABILITY_RANDOM = 0;
constexpr uint8_t ACTION_ABILITY_EXISTING = 1;
constexpr uint8_t ACTION_TASK = 2;
constexpr uint8_t ACTION_BATCH = 3;
constexpr uint8_t ACTION_RANDOM = 4;
constexpr uint8_t ACTION_COUNT = 5;
constexpr uint32_t TRUNCATED_HOUR_COUNT = 5;
constexpr size_t RAW_DATA_MAX_LEN = 64;

// BundleActivePackageStats::UpdateAbility 所需的 ability 事件 ID
constexpr int32_t ABILITY_EVENTS[] = {
    BundleActiveEvent::ABILITY_FOREGROUND,
    BundleActiveEvent::ABILITY_BACKGROUND,
    BundleActiveEvent::ABILITY_STOP,
};
constexpr size_t ABILITY_EVENTS_CNT = sizeof(ABILITY_EVENTS) / sizeof(ABILITY_EVENTS[0]);

// BundleActivePackageStats::UpdateLongTimeTask 所需的任务事件 ID
constexpr int32_t TASK_EVENTS[] = {
    BundleActiveEvent::LONG_TIME_TASK_STARTTED,
    BundleActiveEvent::LONG_TIME_TASK_ENDED,
};
constexpr size_t TASK_EVENTS_CNT = sizeof(TASK_EVENTS) / sizeof(TASK_EVENTS[0]);

// 触发 HasFrontAbility/AnyLongTimeTaskStarted 检查的批量事件 ID
constexpr int32_t BATCH_EVENTS[] = {
    BundleActiveEvent::END_OF_THE_DAY,
    BundleActiveEvent::SHUTDOWN,
    BundleActiveEvent::FLUSH,
};
constexpr size_t BATCH_EVENTS_CNT = sizeof(BATCH_EVENTS) / sizeof(BATCH_EVENTS[0]);

static int64_t ConsumeTimeStamp(FuzzedDataProvider& fdp)
{
    return fdp.ConsumeIntegralInRange<int64_t>(MIN_FUZZ_TIME, MAX_FUZZ_TIME);
}

static int64_t ConsumeMiddleTimeStamp(FuzzedDataProvider& fdp)
{
    return fdp.ConsumeIntegralInRange<int64_t>(
        MIN_FUZZ_TIME + MIDDLE_TIME_MARGIN, MAX_FUZZ_TIME - MIDDLE_TIME_MARGIN);
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

// ======= 工具函数：Parcel 序列化往返测试 =======
template <typename T>
static void RoundTripParcel(T& obj)
{
    MessageParcel parcel;
    if (obj.Marshalling(parcel)) {
        parcel.RewindRead(0);
        std::unique_ptr<T> restored(T::Unmarshalling(parcel));
        (void)restored;
    }
}

// 写入随机字节，驱动 Unmarshalling 的防御性分支
template <typename T>
static void FuzzRawUnmarshalling(FuzzedDataProvider& fdp)
{
    MessageParcel rawParcel;
    std::string rawData = fdp.ConsumeRandomLengthString(RAW_DATA_MAX_LEN);
    rawParcel.WriteBuffer(rawData.data(), rawData.size());
    rawParcel.RewindRead(0);
    std::unique_ptr<T> obj(T::Unmarshalling(rawParcel));
    (void)obj;
}

// ============================================================
// Section 1: BundleActivePackageStats 状态机 + Parcel
// 覆盖目标：UpdateAbility、UpdateLongTimeTask、Update、
//           HasFrontAbility、AnyLongTimeTaskStarted、
//           IncrementTimeUsed、IncrementServiceTimeUsed、
//           IncrementBundleLaunchedCount、
//           Marshalling、Unmarshalling、ToString、拷贝构造
// ============================================================

static BundleActivePackageStats BuildPackageStats(FuzzedDataProvider& fdp)
{
    BundleActivePackageStats stats;
    stats.bundleName_                = fdp.ConsumeRandomLengthString(MAX_NAME_LEN);
    stats.beginTimeStamp_            = ConsumeTimeStamp(fdp);
    stats.endTimeStamp_              = ConsumeTimeStamp(fdp);
    stats.lastTimeUsed_              = ConsumeTimeStamp(fdp);
    stats.totalInFrontTime_          = fdp.ConsumeIntegralInRange<int64_t>(0, MAX_FUZZ_SPAN);
    stats.lastContiniousTaskUsed_    = ConsumeTimeStamp(fdp);
    stats.totalContiniousTaskUsedTime_ = fdp.ConsumeIntegralInRange<int64_t>(0, MAX_FUZZ_SPAN);
    stats.startCount_                = fdp.ConsumeIntegral<int32_t>();
    stats.bundleStartedCount_        = fdp.ConsumeIntegral<int32_t>();
    stats.lastEvent_                 = fdp.ConsumeIntegral<int32_t>();
    stats.userId_                    = fdp.ConsumeIntegral<int32_t>();
    stats.uid_                       = fdp.ConsumeIntegral<int32_t>();
    stats.appIndex_                  = fdp.ConsumeIntegral<int32_t>();
    return stats;
}

static std::vector<std::string> BuildNamePool(FuzzedDataProvider& fdp, size_t maxCount)
{
    size_t count = fdp.ConsumeIntegralInRange<size_t>(1, maxCount);
    std::vector<std::string> names;
    names.reserve(count);
    for (size_t i = 0; i < count; i++) {
        names.emplace_back(fdp.ConsumeRandomLengthString(MAX_NAME_LEN));
    }
    return names;
}

static void PrewarmPackageStats(BundleActivePackageStats& stats, FuzzedDataProvider& fdp,
    const std::vector<std::string>& abilityIds, const std::vector<std::string>& taskNames)
{
    int64_t prewarmTs = ConsumeMiddleTimeStamp(fdp);
    stats.Update("", prewarmTs,
        BundleActiveEvent::ABILITY_FOREGROUND, abilityIds[0], stats.uid_);

    int64_t prewarmTs2 = AddTime(prewarmTs, ONE_STEP_DELTA);
    stats.Update(taskNames[0], prewarmTs2,
        BundleActiveEvent::LONG_TIME_TASK_STARTTED, "", stats.uid_);

    stats.Update("", AddTime(prewarmTs, ABILITY_FOREGROUND_AGAIN_DELTA),
        BundleActiveEvent::ABILITY_FOREGROUND, abilityIds[0], stats.uid_);
    stats.Update("", AddTime(prewarmTs, ABILITY_BACKGROUND_DELTA),
        BundleActiveEvent::ABILITY_BACKGROUND, abilityIds[0], stats.uid_);
    stats.Update("", AddTime(prewarmTs, ABILITY_REFOREGROUND_DELTA),
        BundleActiveEvent::ABILITY_FOREGROUND, abilityIds[0], stats.uid_);
    stats.Update(taskNames[0], AddTime(prewarmTs, TASK_RESTART_DELTA),
        BundleActiveEvent::LONG_TIME_TASK_STARTTED, "", stats.uid_);
    stats.Update(taskNames[0], AddTime(prewarmTs, TASK_END_DELTA),
        BundleActiveEvent::LONG_TIME_TASK_ENDED, "", stats.uid_);
    stats.Update(taskNames[0], AddTime(prewarmTs, TASK_START_AGAIN_DELTA),
        BundleActiveEvent::LONG_TIME_TASK_STARTTED, "", stats.uid_);
    stats.Update("", AddTime(prewarmTs, BATCH_EVENT_DELTA),
        BundleActiveEvent::END_OF_THE_DAY, "", stats.uid_);
}

static void DrivePackageStatsOps(BundleActivePackageStats& stats, FuzzedDataProvider& fdp,
    const std::vector<std::string>& abilityIds, const std::vector<std::string>& taskNames)
{
    size_t ops = fdp.ConsumeIntegralInRange<size_t>(1, MAX_OPS);
    for (size_t i = 0; i < ops && fdp.remaining_bytes() > 0; i++) {
        int64_t ts  = ConsumeTimeStamp(fdp);
        int32_t uid = fdp.ConsumeIntegral<int32_t>();
        uint8_t action = fdp.ConsumeIntegral<uint8_t>() % ACTION_COUNT;

        switch (action) {
            case ACTION_ABILITY_RANDOM: {
                size_t evIdx = fdp.ConsumeIntegral<uint8_t>() % ABILITY_EVENTS_CNT;
                size_t aIdx  = fdp.ConsumeIntegral<uint8_t>() % abilityIds.size();
                stats.Update("", ts, ABILITY_EVENTS[evIdx], abilityIds[aIdx], uid);
                break;
            }
            case ACTION_ABILITY_EXISTING: {
                size_t evIdx = fdp.ConsumeIntegral<uint8_t>() % ABILITY_EVENTS_CNT;
                stats.Update("", ts, ABILITY_EVENTS[evIdx], abilityIds[0], uid);
                break;
            }
            case ACTION_TASK: {
                size_t evIdx = fdp.ConsumeIntegral<uint8_t>() % TASK_EVENTS_CNT;
                size_t tIdx  = fdp.ConsumeIntegral<uint8_t>() % taskNames.size();
                stats.Update(taskNames[tIdx], ts, TASK_EVENTS[evIdx], "", uid);
                break;
            }
            case ACTION_BATCH: {
                size_t evIdx = fdp.ConsumeIntegral<uint8_t>() % BATCH_EVENTS_CNT;
                stats.Update("", ts, BATCH_EVENTS[evIdx], "", uid);
                break;
            }
            case ACTION_RANDOM: {
                stats.Update(
                    fdp.ConsumeRandomLengthString(MAX_NAME_LEN),
                    ts,
                    fdp.ConsumeIntegral<int32_t>(),
                    fdp.ConsumeRandomLengthString(MAX_NAME_LEN),
                    uid);
                break;
            }
            default:
                break;
        }
    }
}

static void FuzzPackageStatsStateMachine(FuzzedDataProvider& fdp)
{
    BundleActivePackageStats stats = BuildPackageStats(fdp);
    std::vector<std::string> abilityIds = BuildNamePool(fdp, MAX_ABILITY_IDS);
    std::vector<std::string> taskNames = BuildNamePool(fdp, MAX_TASK_NAMES);

    PrewarmPackageStats(stats, fdp, abilityIds, taskNames);
    DrivePackageStatsOps(stats, fdp, abilityIds, taskNames);

    stats.IncrementBundleLaunchedCount();
    (void)stats.ToString();

    BundleActivePackageStats statsCopy(stats);
    (void)statsCopy.ToString();
}

static void FuzzPackageStatsParcel(FuzzedDataProvider& fdp)
{
    // 构造含非空 abilities_ 状态的对象，确保 Marshalling 序列化有实际内容
    BundleActivePackageStats stats = BuildPackageStats(fdp);
    std::string abilityId = fdp.ConsumeRandomLengthString(MAX_NAME_LEN);
    int64_t ts = ConsumeTimeStamp(fdp);
    stats.Update("", ts, BundleActiveEvent::ABILITY_FOREGROUND, abilityId, stats.uid_);

    // 正常 Marshalling + Unmarshalling 往返
    RoundTripParcel(stats);

    // 向 Parcel 写入随机字节，驱动 Unmarshalling 防御分支（如 ReadString 失败）
    FuzzRawUnmarshalling<BundleActivePackageStats>(fdp);
}

// ============================================================
// Section 2: BundleActiveEventStats
// 覆盖目标：默认/拷贝构造、全部 getter、add()（同 ID 聚合两个子分支
//           + 异 ID 早返回）、Marshalling、Unmarshalling
// ============================================================

static void FuzzEventStats(FuzzedDataProvider& fdp)
{
    // 默认构造并填充字段
    BundleActiveEventStats stats;
    stats.eventId_        = fdp.ConsumeIntegral<int32_t>();
    stats.beginTimeStamp_ = ConsumeMiddleTimeStamp(fdp);
    stats.endTimeStamp_   = ConsumeTimeStamp(fdp);
    stats.lastEventTime_  = ConsumeTimeStamp(fdp);
    stats.totalTime_      = fdp.ConsumeIntegralInRange<int64_t>(0, MAX_FUZZ_SPAN);
    stats.count_          = fdp.ConsumeIntegral<int32_t>();
    stats.name_           = fdp.ConsumeRandomLengthString(MAX_NAME_LEN);
    stats.uid_            = fdp.ConsumeIntegral<int32_t>();

    // 覆盖全部 getter
    (void)stats.GetEventId();
    (void)stats.GetFirstTimeStamp();
    (void)stats.GetLastTimeStamp();
    (void)stats.GetLastEventTime();
    (void)stats.GetTotalTime();
    (void)stats.GetCount();

    // 拷贝构造
    BundleActiveEventStats statsCopy(stats);

    // add()：同 ID，right.beginTimeStamp_ > beginTimeStamp_
    // → 进入 if 分支，更新 lastEventTime_，然后聚合 totalTime_/count_
    BundleActiveEventStats rightNewer;
    rightNewer.eventId_        = stats.eventId_;
    rightNewer.beginTimeStamp_ = AddTime(stats.beginTimeStamp_, ONE_STEP_DELTA);
    rightNewer.endTimeStamp_   = ConsumeTimeStamp(fdp);
    rightNewer.lastEventTime_  = ConsumeTimeStamp(fdp);
    rightNewer.totalTime_      = fdp.ConsumeIntegralInRange<int64_t>(0, MAX_FUZZ_SPAN);
    rightNewer.count_          = fdp.ConsumeIntegral<int32_t>();
    statsCopy.add(rightNewer);

    // add()：同 ID，right.beginTimeStamp_ <= beginTimeStamp_
    // → 不进入 if 分支，直接聚合 totalTime_/count_
    BundleActiveEventStats rightOlder;
    rightOlder.eventId_        = stats.eventId_;
    rightOlder.beginTimeStamp_ = AddTime(stats.beginTimeStamp_, BACKWARD_STEP_DELTA);
    rightOlder.endTimeStamp_   = ConsumeTimeStamp(fdp);
    rightOlder.lastEventTime_  = ConsumeTimeStamp(fdp);
    rightOlder.totalTime_      = fdp.ConsumeIntegralInRange<int64_t>(0, MAX_FUZZ_SPAN);
    rightOlder.count_          = fdp.ConsumeIntegral<int32_t>();
    statsCopy.add(rightOlder);

    // add()：不同 ID → 早返回
    BundleActiveEventStats diffId;
    diffId.eventId_ = (stats.eventId_ == INT32_MAX) ? stats.eventId_ - 1 : stats.eventId_ + 1;
    statsCopy.add(diffId);

    // 正常往返序列化
    RoundTripParcel(stats);

    // 随机字节 Unmarshalling 防御测试
    FuzzRawUnmarshalling<BundleActiveEventStats>(fdp);
}

// ============================================================
// Section 3: BundleActiveFormRecord
// 覆盖目标：默认/参数/拷贝构造、UpdateFormRecord（新旧时间戳两个分支）、
//           operator==（两重载）、cmp（两方向）、ToString、
//           Marshalling、Unmarshalling
// ============================================================

static void FuzzFormRecord(FuzzedDataProvider& fdp)
{
    // 默认构造
    BundleActiveFormRecord defaultRec;
    (void)defaultRec.ToString();

    // 参数构造
    std::string formName  = fdp.ConsumeRandomLengthString(MAX_NAME_LEN);
    int32_t formDimension = fdp.ConsumeIntegral<int32_t>();
    int64_t formId        = fdp.ConsumeIntegralInRange<int64_t>(0, MAX_FUZZ_TIME);
    int64_t timeStamp     = ConsumeMiddleTimeStamp(fdp);
    int32_t userId        = fdp.ConsumeIntegral<int32_t>();
    int32_t uid           = fdp.ConsumeIntegral<int32_t>();
    BundleActiveFormRecord rec(formName, formDimension, formId, timeStamp, userId, uid);

    // 拷贝构造
    BundleActiveFormRecord recCopy(rec);

    // UpdateFormRecord：旧时间戳（timeStamp - 1 < formLastUsedTime_）→ 不更新
    rec.UpdateFormRecord(AddTime(timeStamp, BACKWARD_STEP_DELTA));

    // UpdateFormRecord：新时间戳（timeStamp + 1 > formLastUsedTime_）→ 更新 count_++
    rec.UpdateFormRecord(AddTime(timeStamp, ONE_STEP_DELTA));

    // 再次更新，确保 count_ 累积路径多次执行
    rec.UpdateFormRecord(AddTime(timeStamp, ABILITY_FOREGROUND_AGAIN_DELTA));

    // operator==(const BundleActiveFormRecord&)
    (void)(rec == recCopy);

    // operator==(int64_t)
    (void)(rec == formId);
    (void)(rec == AddTime(formId, ONE_STEP_DELTA));

    // cmp：rec.count_ > recCopy.count_ → 返回 true
    (void)BundleActiveFormRecord::cmp(rec, recCopy);

    // cmp：recCopy.count_ <= rec.count_ → 返回 false
    (void)BundleActiveFormRecord::cmp(recCopy, rec);

    // ToString
    (void)rec.ToString();

    // 正常往返序列化
    RoundTripParcel(rec);

    // 随机字节 Unmarshalling 防御测试
    FuzzRawUnmarshalling<BundleActiveFormRecord>(fdp);
}

// ============================================================
// Section 4: AppGroupCallbackInfo
// 覆盖目标：默认/参数构造、全部 getter、
//           Marshalling、Unmarshalling（含字段写入失败防御分支）
// ============================================================

static void FuzzAppGroupCallbackInfo(FuzzedDataProvider& fdp)
{
    // 默认构造
    AppGroupCallbackInfo defaultInfo;
    (void)defaultInfo.GetUserId();

    // 参数构造
    int32_t  userId       = fdp.ConsumeIntegral<int32_t>();
    int32_t  oldGroup     = fdp.ConsumeIntegral<int32_t>();
    int32_t  newGroup     = fdp.ConsumeIntegral<int32_t>();
    uint32_t changeReason = fdp.ConsumeIntegral<uint32_t>();
    std::string bundleName = fdp.ConsumeRandomLengthString(MAX_NAME_LEN);
    AppGroupCallbackInfo info(userId, oldGroup, newGroup, changeReason, bundleName);

    // 全部 getter 覆盖
    (void)info.GetUserId();
    (void)info.GetOldGroup();
    (void)info.GetNewGroup();
    (void)info.GetChangeReason();
    (void)info.GetBundleName();

    // 正常往返序列化（同时覆盖 Unmarshalling 中逐字段 Read 路径）
    MessageParcel parcel;
    if (info.Marshalling(parcel)) {
        parcel.RewindRead(0);
        std::unique_ptr<AppGroupCallbackInfo> restored(
            AppGroupCallbackInfo::Unmarshalling(parcel));
        if (restored != nullptr) {
            (void)restored->GetUserId();
            (void)restored->GetBundleName();
        }
    }

    // 随机字节 Unmarshalling：覆盖 new 失败及字段读取异常分支
    FuzzRawUnmarshalling<AppGroupCallbackInfo>(fdp);
}

static void FuzzHighFrequencyPeriodParcel(const std::string& bundleName, FuzzedDataProvider& fdp)
{
    size_t validCount = fdp.ConsumeIntegralInRange<size_t>(0, static_cast<size_t>(MAX_HOUR_NUM));
    std::vector<int32_t> validHours;
    validHours.reserve(validCount);
    for (size_t i = 0; i < validCount && fdp.remaining_bytes() > 0; i++) {
        validHours.emplace_back(fdp.ConsumeIntegral<int32_t>());
    }
    BundleActiveHighFrequencyPeriod validPeriod(bundleName, validHours);
    RoundTripParcel(validPeriod);
}

static void FuzzHighFrequencyPeriodBadParcel(const std::string& bundleName)
{
    std::vector<int32_t> overflowHours(static_cast<size_t>(MAX_HOUR_NUM) + ONE_STEP_DELTA, 0);
    BundleActiveHighFrequencyPeriod overflowPeriod(bundleName, overflowHours);
    MessageParcel parcel;
    bool result = overflowPeriod.Marshalling(parcel);
    (void)result;

    MessageParcel overflowParcel;
    overflowParcel.WriteString(bundleName);
    overflowParcel.WriteUint32(static_cast<uint32_t>(MAX_HOUR_NUM) + ONE_STEP_DELTA);
    overflowParcel.RewindRead(0);
    std::unique_ptr<BundleActiveHighFrequencyPeriod> obj(
        BundleActiveHighFrequencyPeriod::Unmarshalling(overflowParcel));
    (void)obj;
}

static void FuzzHighFrequencyPeriodTruncatedParcel(const std::string& bundleName)
{
    MessageParcel truncParcel;
    truncParcel.WriteString(bundleName);
    truncParcel.WriteUint32(TRUNCATED_HOUR_COUNT);
    truncParcel.WriteInt32(ONE_STEP_DELTA);
    truncParcel.RewindRead(0);
    std::unique_ptr<BundleActiveHighFrequencyPeriod> obj(
        BundleActiveHighFrequencyPeriod::Unmarshalling(truncParcel));
    (void)obj;
}

// ============================================================
// Section 5: BundleActiveHighFrequencyPeriod
// 覆盖目标：默认/参数/拷贝构造、ToString、
//           Marshalling（hours<=24 成功路径 + hours>24 溢出保护路径）、
//           Unmarshalling（正常往返 + size>MAX_HOUR_NUM 保护分支
//                        + 中途 ReadInt32 失败分支 + 随机字节）
// ============================================================

static void FuzzHighFrequencyPeriod(FuzzedDataProvider& fdp)
{
    // 默认构造
    BundleActiveHighFrequencyPeriod defaultPeriod;
    (void)defaultPeriod.ToString();

    std::string bundleName = fdp.ConsumeRandomLengthString(MAX_NAME_LEN);

    // 参数构造（hours 数量完全随机，含超出 24 的情况）
    size_t hourCount = fdp.ConsumeIntegralInRange<size_t>(0,
        static_cast<size_t>(MAX_HOUR_NUM) + MAX_TASK_NAMES);
    std::vector<int32_t> hours;
    hours.reserve(hourCount);
    for (size_t i = 0; i < hourCount && fdp.remaining_bytes() > 0; i++) {
        hours.emplace_back(fdp.ConsumeIntegral<int32_t>());
    }
    BundleActiveHighFrequencyPeriod period(bundleName, hours);

    // 拷贝构造
    BundleActiveHighFrequencyPeriod periodCopy(period);

    // ToString（含空 highFreqHours_ 和非空两种情况）
    (void)period.ToString();
    (void)periodCopy.ToString();

    FuzzHighFrequencyPeriodParcel(bundleName, fdp);
    FuzzHighFrequencyPeriodBadParcel(bundleName);
    FuzzHighFrequencyPeriodTruncatedParcel(bundleName);

    // --- 随机字节防御测试 ---
    FuzzRawUnmarshalling<BundleActiveHighFrequencyPeriod>(fdp);
}

} // namespace

// ======= 模糊测试主入口 =======
bool BundleActiveValueObjectFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return false;
    }
    FuzzedDataProvider fdp(data, size);

    FuzzPackageStatsStateMachine(fdp);
    FuzzPackageStatsParcel(fdp);
    FuzzEventStats(fdp);
    FuzzFormRecord(fdp);
    FuzzAppGroupCallbackInfo(fdp);
    FuzzHighFrequencyPeriod(fdp);

    return true;
}

} // namespace DeviceUsageStats
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DeviceUsageStats::BundleActiveValueObjectFuzzTest(data, size);
    return 0;
}
