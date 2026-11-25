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

#ifndef BUNDLE_ACTIVE_EVENT_FILTER_H
#define BUNDLE_ACTIVE_EVENT_FILTER_H

#include <cstdint>
#include <vector>

#include "parcel.h"

namespace OHOS {
namespace DeviceUsageStats {
class BundleActiveEventFilter : public Parcelable {
public:
    // default userId is -1 for JS API, if other SAs call this API, they should explicit define userId.
    int32_t userId_ {-1};
    // max number of query events result.
    int32_t maxNum_ {1000};
    int64_t beginTime_ {0};
    int64_t endTime_ {0};
    // if eventIds_ is empty, all events will be queried.
    std::vector<int32_t> eventIds_;

    /*
    * function: Marshalling, mashalling filter object to parcel.
    * parameters: parcel
    * return: result of mashalling, true means successful, flase means failed.
    */
    virtual bool Marshalling(Parcel &parcel) const override;

    /*
    * function: Unmarshalling, unmashalling filter object from parcel.
    * parameters: parcel
    * return: point to a BundleActiveEventFilter.
    */
    static BundleActiveEventFilter *Unmarshalling(Parcel &parcel);
};
}  // namespace DeviceUsageStats
}  // namespace OHOS
#endif  // BUNDLE_ACTIVE_EVENT_FILTER_H
