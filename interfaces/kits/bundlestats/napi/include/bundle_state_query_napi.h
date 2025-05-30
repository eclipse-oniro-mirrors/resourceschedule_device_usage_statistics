/*
 * Copyright (c) 2022  Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_RESOURCESCHEDULE_DEVICE_USAGE_STATISTICS_BUNDLE_STATE_QUERY_NAPI_H
#define FOUNDATION_RESOURCESCHEDULE_DEVICE_USAGE_STATISTICS_BUNDLE_STATE_QUERY_NAPI_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace DeviceUsageStats {
    napi_value IsIdleState(napi_env env, napi_callback_info info);
    napi_value IsIdleStateSync(napi_env env, napi_callback_info info);
    napi_value QueryCurrentBundleEvents(napi_env env, napi_callback_info info);
    napi_value QueryBundleEvents(napi_env env, napi_callback_info info);
    napi_value QueryBundleStatsInfoByInterval(napi_env env, napi_callback_info info);
    napi_value QueryBundleStatsInfos(napi_env env, napi_callback_info info);
    napi_value QueryModuleUsageRecords(napi_env env, napi_callback_info info);
    napi_value QueryDeviceEventStats(napi_env env, napi_callback_info info);
    napi_value QueryNotificationEventStats(napi_env env, napi_callback_info info);
    napi_value QueryAppStatsInfos(napi_env env, napi_callback_info info);
    napi_value QueryLastUseTime(napi_env env, napi_callback_info info);
}  // namespace DeviceUsageStats
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_DEVICE_USAGE_STATISTICS_BUNDLE_STATE_QUERY_NAPI_H

