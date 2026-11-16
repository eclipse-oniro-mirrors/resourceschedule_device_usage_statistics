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

#include "bundle_active_event_filter.h"

namespace OHOS {
namespace DeviceUsageStats {
bool BundleActiveEventFilter::Marshalling(Parcel &parcel) const
{
    if (parcel.WriteInt32(userId_) &&
        parcel.WriteInt32(maxNum_) &&
        parcel.WriteInt64(beginTime_) &&
        parcel.WriteInt64(endTime_) &&
        parcel.WriteInt32Vector(eventIds_)) {
        return true;
    }
    return false;
}

BundleActiveEventFilter *BundleActiveEventFilter::Unmarshalling(Parcel &parcel)
{
    BundleActiveEventFilter *result = new (std::nothrow) BundleActiveEventFilter();
    if (result == nullptr) {
        return nullptr;
    }
    if (!parcel.ReadInt32(result->userId_) ||
        !parcel.ReadInt32(result->maxNum_) ||
        !parcel.ReadInt64(result->beginTime_) ||
        !parcel.ReadInt64(result->endTime_) ||
        !parcel.ReadInt32Vector(&result->eventIds_)) {
        delete result;
        return nullptr;
    }
    return result;
}
}  // namespace DeviceUsageStats
}  // namespace OHOS
