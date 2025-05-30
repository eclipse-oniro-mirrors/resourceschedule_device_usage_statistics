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

#ifndef FOUNDATION_RESOURCESCHEDULE_DEVICE_USAGE_STATISTICS_APP_GROUP_OBSERVER_NAPI_H
#define FOUNDATION_RESOURCESCHEDULE_DEVICE_USAGE_STATISTICS_APP_GROUP_OBSERVER_NAPI_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "app_group_callback_stub.h"
#include "app_group_callback_info.h"

namespace OHOS {
namespace DeviceUsageStats {
static const int8_t NO_ERROR = 0;
class AppGroupObserver : public AppGroupCallbackStub {
public:
    AppGroupObserver() =default;

    ~AppGroupObserver();

    ErrCode OnAppGroupChanged(const AppGroupCallbackInfo &appGroupCallbackInfo) override;

    void SetCallbackInfo(const napi_env &env, const napi_ref &ref);

private:
    struct BundleGroupCallbackInfo {
        napi_env env = nullptr;
        napi_ref ref = nullptr;
    };
    BundleGroupCallbackInfo bundleGroupCallbackInfo_;
};

struct BundleActiveGroupObserverInfo {
    napi_ref ref = nullptr;
    sptr<AppGroupObserver> callback = nullptr;
};
}  // namespace DeviceUsageStats
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_DEVICE_USAGE_STATISTICS_APP_GROUP_OBSERVER_NAPI_H