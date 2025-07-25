/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "unistd.h"
#include "ffrt.h"
#include "bundle_active_log.h"
#include "bundle_active_client.h"

namespace OHOS {
namespace DeviceUsageStats {
namespace {
    static const int32_t DELAY_TIME = 5000;
    static const int32_t SLEEP_TIME_SECOND = 1;
    static const int32_t MS_TO_US = 1000;
}
ffrt::recursive_mutex mutex_;
BundleActiveClient& BundleActiveClient::GetInstance()
{
    static BundleActiveClient instance;
    return instance;
}

ErrCode BundleActiveClient::GetBundleActiveProxy()
{
    if (bundleActiveProxy_ != nullptr) {
        return ERR_OK;
    }
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!samgr) {
        BUNDLE_ACTIVE_LOGE("Failed to get SystemAbilityManager.");
        return ERR_GET_SYSTEM_ABILITY_MANAGER_FAILED;
    }

    sptr<IRemoteObject> object = samgr->GetSystemAbility(DEVICE_USAGE_STATISTICS_SYS_ABILITY_ID);
    if (!object) {
        BUNDLE_ACTIVE_LOGE("Failed to get SystemAbility[1907] .");
        return ERR_GET_SYSTEM_ABILITY_FAILED;
    }

    bundleActiveProxy_ = iface_cast<IBundleActiveService>(object);
    if (!bundleActiveProxy_) {
        BUNDLE_ACTIVE_LOGE("Failed to get BundleActiveProxy.");
        return ERR_REMOTE_OBJECT_IF_CAST_FAILED;
    }
    if (!recipient_) {
        recipient_ = new (std::nothrow) BundleActiveClientDeathRecipient();
    }
    if (recipient_) {
        bundleActiveProxy_->AsObject()->AddDeathRecipient(recipient_);
    }
    return ERR_OK;
}

ErrCode BundleActiveClient::ReportEvent(BundleActiveEvent event, const int32_t userId)
{
    BUNDLE_ACTIVE_LOGD("BundleActiveClient::ReportEvent called");
    std::lock_guard<ffrt::recursive_mutex> lock(mutex_);
    ErrCode ret = GetBundleActiveProxy();
    if (ret != ERR_OK) {
        return ret;
    }
    return bundleActiveProxy_->ReportEvent(event, userId);
}

ErrCode BundleActiveClient::IsBundleIdle(bool& isBundleIdle, const std::string& bundleName, int32_t userId)
{
    std::lock_guard<ffrt::recursive_mutex> lock(mutex_);
    ErrCode ret = GetBundleActiveProxy();
    if (ret != ERR_OK) {
        return ret;
    }
    return bundleActiveProxy_->IsBundleIdle(isBundleIdle, bundleName, userId);
}

ErrCode BundleActiveClient::IsBundleUsePeriod(bool& IsUsePeriod, const std::string& bundleName, int32_t userId)
{
    std::lock_guard<ffrt::recursive_mutex> lock(mutex_);
    ErrCode ret = GetBundleActiveProxy();
    if (ret != ERR_OK) {
        return ret;
    }
    return bundleActiveProxy_->IsBundleUsePeriod(IsUsePeriod, bundleName, userId);
}

ErrCode BundleActiveClient::QueryBundleStatsInfoByInterval(std::vector<BundleActivePackageStats>& PackageStats,
    const int32_t intervalType, const int64_t beginTime, const int64_t endTime, int32_t userId)
{
    std::lock_guard<ffrt::recursive_mutex> lock(mutex_);
    ErrCode ret = GetBundleActiveProxy();
    if (ret != ERR_OK) {
        return ret;
    }
    return bundleActiveProxy_->QueryBundleStatsInfoByInterval(PackageStats, intervalType, beginTime, endTime, userId);
}

ErrCode BundleActiveClient::QueryBundleTodayLatestUsedTime(
    int64_t& latestUsedTime, const std::string& bundleName, int32_t userId)
{
    std::lock_guard<ffrt::recursive_mutex> lock(mutex_);
    ErrCode ret = GetBundleActiveProxy();
    if (ret != ERR_OK) {
        return ret;
    }
    return bundleActiveProxy_->QueryBundleTodayLatestUsedTime(latestUsedTime, bundleName, userId);
}

ErrCode BundleActiveClient::QueryHighFrequencyPeriodBundle(
    std::vector<BundleActiveHighFrequencyPeriod>& appFreqHours, int32_t userId)
{
    std::lock_guard<ffrt::recursive_mutex> lock(mutex_);
    ErrCode ret = GetBundleActiveProxy();
    if (ret != ERR_OK) {
        return ret;
    }
    auto err = bundleActiveProxy_->QueryHighFrequencyPeriodBundle(appFreqHours, userId);
    BUNDLE_ACTIVE_LOGI("QueryHighFrequencyPeriodBundle appFreqHour is %{public}zu", appFreqHours.size());
    return err;
}

ErrCode BundleActiveClient::QueryBundleEvents(std::vector<BundleActiveEvent>& bundleActiveEvents,
    const int64_t beginTime, const int64_t endTime, int32_t userId)
{
    std::lock_guard<ffrt::recursive_mutex> lock(mutex_);
    ErrCode ret = GetBundleActiveProxy();
    if (ret != ERR_OK) {
        return ret;
    }
    auto err = bundleActiveProxy_->QueryBundleEvents(bundleActiveEvents, beginTime, endTime, userId);
    BUNDLE_ACTIVE_LOGI("QueryBundleEvents bundleActiveEvents is %{public}zu", bundleActiveEvents.size());
    return err;
}

ErrCode BundleActiveClient::SetAppGroup(std::string bundleName, const int32_t newGroup, int32_t userId)
{
    std::lock_guard<ffrt::recursive_mutex> lock(mutex_);
    ErrCode ret = GetBundleActiveProxy();
    if (ret != ERR_OK) {
        return ret;
    }
    return bundleActiveProxy_->SetAppGroup(bundleName, newGroup, userId);
}

ErrCode BundleActiveClient::QueryBundleStatsInfos(std::vector<BundleActivePackageStats>& bundleActivePackageStats,
    const int32_t intervalType, const int64_t beginTime, const int64_t endTime)
{
    std::lock_guard<ffrt::recursive_mutex> lock(mutex_);
    ErrCode ret = GetBundleActiveProxy();
    if (ret != ERR_OK) {
        return ret;
    }
    return bundleActiveProxy_->QueryBundleStatsInfos(bundleActivePackageStats, intervalType, beginTime, endTime);
}

ErrCode BundleActiveClient::QueryCurrentBundleEvents(std::vector<BundleActiveEvent>& bundleActiveEvents,
    const int64_t beginTime, const int64_t endTime)
{
    std::lock_guard<ffrt::recursive_mutex> lock(mutex_);
    ErrCode ret = GetBundleActiveProxy();
    if (ret != ERR_OK) {
        return ret;
    }
    return bundleActiveProxy_->QueryCurrentBundleEvents(bundleActiveEvents, beginTime, endTime);
}

ErrCode BundleActiveClient::QueryAppGroup(int32_t& appGroup, std::string& bundleName, const int32_t userId)
{
    std::lock_guard<ffrt::recursive_mutex> lock(mutex_);
    ErrCode ret = GetBundleActiveProxy();
    if (ret != ERR_OK) {
        return ret;
    }
    return bundleActiveProxy_->QueryAppGroup(appGroup, bundleName, userId);
}

ErrCode BundleActiveClient::QueryModuleUsageRecords(int32_t maxNum, std::vector<BundleActiveModuleRecord>& results,
    int32_t userId)
{
    if (maxNum <= 0 || maxNum > MAXNUM_UP_LIMIT) {
        BUNDLE_ACTIVE_LOGI("maxNum is illegal, maxNum is %{public}d", maxNum);
        return ERR_MAX_RECORDS_NUM_BIGER_THEN_ONE_THOUSAND;
    }
    std::lock_guard<ffrt::recursive_mutex> lock(mutex_);
    ErrCode ret = GetBundleActiveProxy();
    if (ret != ERR_OK) {
        return ret;
    }
    return bundleActiveProxy_->QueryModuleUsageRecords(maxNum, results, userId);
}

ErrCode BundleActiveClient::RegisterAppGroupCallBack(const sptr<IAppGroupCallback> &observer)
{
    std::lock_guard<ffrt::recursive_mutex> lock(mutex_);
    ErrCode ret = GetBundleActiveProxy();
    if (ret != ERR_OK) {
        return ret;
    }
    ret = bundleActiveProxy_->RegisterAppGroupCallBack(observer);
    if (recipient_ && ret == ERR_OK) {
        recipient_->AddObserver(observer);
    }
    return ret;
}

ErrCode BundleActiveClient::UnRegisterAppGroupCallBack(const sptr<IAppGroupCallback> &observer)
{
    std::lock_guard<ffrt::recursive_mutex> lock(mutex_);
    ErrCode ret = GetBundleActiveProxy();
    if (ret != ERR_OK) {
        return ret;
    }
    ret = bundleActiveProxy_->UnRegisterAppGroupCallBack(observer);
    if (recipient_ && ret == ERR_OK) {
        recipient_->RemoveObserver();
    }
    return ret;
}

ErrCode BundleActiveClient::QueryDeviceEventStats(int64_t beginTime, int64_t endTime,
    std::vector<BundleActiveEventStats>& eventStats, int32_t userId)
{
    std::lock_guard<ffrt::recursive_mutex> lock(mutex_);
    ErrCode ret = GetBundleActiveProxy();
    if (ret != ERR_OK) {
        return ret;
    }
    return bundleActiveProxy_->QueryDeviceEventStats(beginTime, endTime, eventStats, userId);
}

ErrCode BundleActiveClient::QueryNotificationEventStats(int64_t beginTime, int64_t endTime,
    std::vector<BundleActiveEventStats>& eventStats, int32_t userId)
{
    std::lock_guard<ffrt::recursive_mutex> lock(mutex_);
    ErrCode ret = GetBundleActiveProxy();
    if (ret != ERR_OK) {
        return ret;
    }
    return bundleActiveProxy_->QueryNotificationEventStats(beginTime, endTime, eventStats, userId);
}

void BundleActiveClient::BundleActiveClientDeathRecipient::AddObserver(const sptr<IAppGroupCallback> &observer)
{
    std::lock_guard<ffrt::recursive_mutex> lock(mutex_);
    if (observer) {
        observer_ = observer;
    }
}

void BundleActiveClient::BundleActiveClientDeathRecipient::RemoveObserver()
{
    std::lock_guard<ffrt::recursive_mutex> lock(mutex_);
    if (observer_) {
        observer_ = nullptr;
    }
}

void BundleActiveClient::BundleActiveClientDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &object)
{
    (void)object;
    std::lock_guard<ffrt::recursive_mutex> lock(mutex_);
    BundleActiveClient::GetInstance().bundleActiveProxy_ = nullptr;
    ffrt::submit([this]() {
        this->OnServiceDiedInner();
    }, {}, {}, ffrt::task_attr().delay(DELAY_TIME * MS_TO_US));
}

void BundleActiveClient::BundleActiveClientDeathRecipient::OnServiceDiedInner()
{
    std::lock_guard<ffrt::recursive_mutex> lock(mutex_);
    while (BundleActiveClient::GetInstance().GetBundleActiveProxy() != ERR_OK) {
        sleep(SLEEP_TIME_SECOND);
    }
    if (observer_) {
        BundleActiveClient::GetInstance().RegisterAppGroupCallBack(observer_);
    }
}
}  // namespace DeviceUsageStats
}  // namespace OHOS