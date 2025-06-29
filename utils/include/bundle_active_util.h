/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef BUNDLE_ACTIVE_GROUP_UTIL_H
#define BUNDLE_ACTIVE_GROUP_UTIL_H

#include <string>
namespace OHOS {
namespace DeviceUsageStats {
class BundleActiveUtil {
public:
    BundleActiveUtil() = delete;
    ~BundleActiveUtil() = delete;
    static constexpr int32_t PERIOD_DAILY = 0;
    static constexpr int32_t PERIOD_WEEKLY = 1;
    static constexpr int32_t PERIOD_MONTHLY = 2;
    static constexpr int32_t PERIOD_YEARLY = 3;
    static std::string GetBundleUsageKey(const std::string& bundleName, const int32_t uid);
    static int64_t GetFFRTDelayTime(const int64_t& delayTime);
    static int64_t GetIntervalTypeStartTime(const int64_t& timeStamp, const int32_t& intervalType);
    static int32_t StringToInt32(const std::string& str);
    static int64_t StringToInt64(const std::string& str);
    static int64_t GetSystemTimeMs();
    static int64_t GetNowMicroTime();
    static int64_t GetSteadyTime();
    static uint64_t GetFolderOrFileSize(const std::string& path);
    static double GetDeviceValidSize(const std::string& path);
    static std::string GetPartitionName(const std::string& path);
    static double GetPercentOfAvailableUserSpace(const std::string& path);
};
}  // namespace DeviceUsageStats
}  // namespace OHOS
#endif  // BUNDLE_ACTIVE_GROUP_UTIL_H

