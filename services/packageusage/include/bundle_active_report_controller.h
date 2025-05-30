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

#ifndef BUNDLE_ACTIVE_REPORT_CONTROLLER_H
#define BUNDLE_ACTIVE_REPORT_CONTROLLER_H

#include "bundle_active_report_handler.h"
#include "bundle_active_core.h"
#include "ffrt.h"

namespace OHOS {
namespace DeviceUsageStats {
class BundleActiveReportController {
public:
    static BundleActiveReportController& GetInstance();
    void Init(const std::shared_ptr<BundleActiveCore>& bundleActiveCore);
    void DeInit();
    std::shared_ptr<BundleActiveReportHandler> GetBundleReportHandler();
private:
    BundleActiveReportController() = default;
    ~BundleActiveReportController() = default;
    std::shared_ptr<BundleActiveReportHandler> activeReportHandler_;
    bool isInit_ = false;
    ffrt::mutex mutex_;
};
}  // namespace DeviceUsageStats
}  // namespace OHOS
#endif  // BUNDLE_ACTIVE_REPORT_CONTROLLER_H
 
 