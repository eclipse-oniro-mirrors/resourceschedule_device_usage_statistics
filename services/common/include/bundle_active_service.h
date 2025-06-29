/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifndef BUNDLE_ACTIVE_SERVICE_H
#define BUNDLE_ACTIVE_SERVICE_H

#include "singleton.h"

#include "app_mgr_interface.h"
#include "ibundle_active_service.h"
#include "bundle_active_service_stub.h"
#include "bundle_active_core.h"
#include "bundle_active_report_handler.h"
#ifdef DEVICE_USAGES_STATISTICS_POWERMANGER_ENABLE
#include "shutdown/async_shutdown_callback_stub.h"
#include "bundle_active_power_state_callback_service.h"
#endif
#include "bundle_active_app_state_observer.h"
#include "bundle_active_continuous_task_observer.h"
#include "bundle_active_account_helper.h"
#include "bundle_active_high_frequency_period.h"
#include "file_ex.h"
#include "string_ex.h"
#include "system_ability.h"
#include "ffrt.h"

namespace OHOS {
namespace DeviceUsageStats {
class BundleActiveService : public SystemAbility, public BundleActiveServiceStub,
    public std::enable_shared_from_this<BundleActiveService> {
    DISALLOW_COPY_AND_MOVE(BundleActiveService);
    DECLARE_SYSTEM_ABILITY(BundleActiveService);
    DECLARE_DELAYED_SINGLETON(BundleActiveService);
public:
    using IBundleMgr = OHOS::AppExecFwk::IBundleMgr;
    using BundleInfo = OHOS::AppExecFwk::BundleInfo;
    using BundleFlag = OHOS::AppExecFwk::BundleFlag;
    using HapModuleInfo = OHOS::AppExecFwk::HapModuleInfo;
    using AbilityInfo = OHOS::AppExecFwk::AbilityInfo;
    using ApplicationInfo = OHOS::AppExecFwk::ApplicationInfo;
    using ApplicationFlag = OHOS::AppExecFwk::ApplicationFlag;
    using AbilityType = OHOS::AppExecFwk::AbilityType;

    /**
     * @brief ReportEvent, used to report event.
     *
     * @param event .
     * @param userId .
     * @return errCode.
     */
    ErrCode ReportEvent(const BundleActiveEvent& event, int32_t userId) override;

    /**
     * @brief IsBundleIdle, used to check whether specific bundle is idle.
     *
     * @param isBundleIdle The result of IsBundleIdle.
     * @param bundleName .
     * @param userId default userId is -1 for JS API, if other SAs call this API, they should explicit define userId.
     * @return errCode.
     */
    ErrCode IsBundleIdle(bool& isBundleIdle, const std::string& bundleName, int32_t userId) override;

        /**
     * @brief IsBundleUsePeriod, used to check whether specific bundle is use period.
     *
     * @param IsUsePeriod The result of IsBundleIdle.

     * @param bundleName the target of application's bundle name.
     * @param userId default userId is -1 for JS API, if other SAs call this API, they should explicit define userId.
     * @return errCode.
     */
    ErrCode IsBundleUsePeriod(bool& IsUsePeriod, const std::string& bundleName, int32_t userId) override;

    /**
     * @brief QueryBundleStatsInfoByInterval, query all bundle usage statistics in specific time span for calling user.
     *
     * @param packageStats .
     * @param intervalType .
     * @param beginTime .
     * @param endTime .
     * @param userId default userId is -1 for JS API, if other SAs call this API, they should explicit define userId.
     * @return errCode.
     */
    ErrCode QueryBundleStatsInfoByInterval(std::vector<BundleActivePackageStats>& packageStats,
        const int32_t intervalType, const int64_t beginTime, const int64_t endTime, int32_t userId) override;

    /**
     * @brief QueryBundleEvents, query all events in specific time span for calling user.
     *
     * @param bundleActiveEvents .
     * @param beginTime .
     * @param endTime .
     * @param userId default userId is -1 for JS API, if other SAs call this API, they should explicit define userId.
     * @return errCode.
     */
    ErrCode QueryBundleEvents(std::vector<BundleActiveEvent>& bundleActiveEvents, const int64_t beginTime,
        const int64_t endTime, int32_t userId) override;

    /**
     * @brief SetAppGroup, set specific bundle of specific user to a priority group.
     *
     * @param bundleName .
     * @param newGroup .
     * @param userId default userId is -1 for JS API, if other SAs call this API, they should explicit define userId.
     * @return errCode.
     */
    ErrCode SetAppGroup(const std::string& bundleName, int32_t newGroup, int32_t userId) override;

    /**
     * @brief QueryBundleStatsInfos, query bundle usage statistics in specific time span for calling bundle.
     *
     * @param bundleActivePackageStats, The result of QueryBundleStatsInfos.
     * @param intervalType .
     * @param beginTime .
     * @param endTime .
     * @return errCode.
     */
    ErrCode QueryBundleStatsInfos(std::vector<BundleActivePackageStats>& bundleActivePackageStats,
        const int32_t intervalType, const int64_t beginTime, const int64_t endTime) override;

    /**
     * @brief QueryCurrentBundleEvents, query bundle usage statistics in specific time span for calling bundle.
     *
     * @param bundleActiveEvents the std::vector<BundleActiveEvent>, as the result of QueryCurrentBundleEvents.
     * @param beginTime .
     * @param endTime .
     * @return errCode.
     */
    ErrCode QueryCurrentBundleEvents(std::vector<BundleActiveEvent>& bundleActiveEvents,
        const int64_t beginTime, const int64_t endTime) override;

    /**
     * @brief QueryAppGroup, query appGroup by bundleName and userId.
     *
     * @param appGroup as the result of QueryAppGroup.
     * @param bundleName .
     * @param userId default userId is -1 for JS API, if other SAs call this API, they should explicit define userId.
     * @return errCode.
     */
    ErrCode QueryAppGroup(int32_t& appGroup, const std::string& bundleName, int32_t userId) override;

    /**
     * @brief QueryModuleUsageRecords, query all from usage statistics in specific time span for calling user.
     *
     * @param maxNum .
     * @param results .
     * @param userId default userId is -1 for JS API, if other SAs call this API, they should explicit define userId.
     * @return errCode.
     */
    ErrCode QueryModuleUsageRecords(int32_t maxNum, std::vector<BundleActiveModuleRecord>& results,
        int32_t userId = -1) override;

    /**
     * @brief QueryDeviceEventStats, query all from event stats in specific time span for calling user.
     *
     * @param beginTime .
     * @param endTime .
     * @param eventStats .
     * @param userId default userId is -1 for JS API, if other SAs call this API, they should explicit define userId.
     * @return errCode.
     */
    ErrCode QueryDeviceEventStats(int64_t beginTime, int64_t endTime,
        std::vector<BundleActiveEventStats>& eventStats, int32_t userId) override;

    /**
     * @brief QueryNotificationEventStats, query all app notification number in specific time span for calling user.
     *
     * @param beginTime .
     * @param endTime .
     * @param eventStats .
     * @param userId default userId is -1 for JS API, if other SAs call this API, they should explicit define userId.
     * @return errCode.
     */
    ErrCode QueryNotificationEventStats(int64_t beginTime, int64_t endTime,
        std::vector<BundleActiveEventStats>& eventStats, int32_t userId) override;

    /**
     * @brief QueryHighFrequencyPeriodBundle, query the high-frequency usage period of the app in the past week.
     *
     * @param appFreqHours .
     * @param userId default userId is -1 for JS API, if other SAs call this API, they should explicit define userId.
     * @return errCode.
     */
    ErrCode QueryHighFrequencyPeriodBundle(
        std::vector<BundleActiveHighFrequencyPeriod>& appFreqHours, int32_t userId) override;

    /**
     * @brief QueryBundleTodayLatestUsedTime, query the latest use time of the application on the current day.
     *
     * @param latestUsedTime .
     * @param bundleName .
     * @param userId default userId is -1 for JS API, if other SAs call this API, they should explicit define userId.
     * @return errCode.
     */
    ErrCode QueryBundleTodayLatestUsedTime(
        int64_t& latestUsedTime, const std::string& bundleName, int32_t userId) override;

    /**
     * @brief BundleActiveService, default constructor.
     *
     * @param systemAbilityId .
     * @param runOnCreate .
     */
    BundleActiveService(const int32_t systemAbilityId, bool runOnCreate);

    /**
     * @brief RegisterAppGroupCallBack, observe bundle group change event.
     *
     * @param observer .
     * @return errCode.
     */
    int32_t RegisterAppGroupCallBack(const sptr<IAppGroupCallback> &observer) override;

    /**
     * @brief UnRegisterAppGroupCallBack, unobserve bundle group change event.
     *
     * @param observer .
     * @return errCode.
     */
    int32_t UnRegisterAppGroupCallBack(const sptr<IAppGroupCallback> &observer) override;

    int32_t Dump(int32_t fd, const std::vector<std::u16string> &args) override;

protected:
    /**
     * @brief The OnStart callback.
     */
    void OnStart() override;
    /**
     * @brief The OnStop callback.
     */
    void OnStop() override;

private:
    std::shared_ptr<BundleActiveCore> bundleActiveCore_;

    sptr<BundleActiveAppStateObserver> appStateObserver_;
#ifdef BGTASKMGR_ENABLE
    std::shared_ptr<BundleActiveContinuousTaskObserver> continuousTaskObserver_;
#endif
#ifdef DEVICE_USAGES_STATISTICS_POWERMANGER_ENABLE
    sptr<PowerMgr::IAsyncShutdownCallback> shutdownCallback_;
    sptr<PowerMgr::IPowerStateCallback> powerStateCallback_;
#endif
    bool ready_ {false};
    int32_t ConvertIntervalType(const int32_t intervalType);
    void InitNecessaryState();
    void InitService();
    ErrCode CheckBundleIsSystemAppAndHasPermission(const int32_t uid,
        OHOS::Security::AccessToken::AccessTokenID tokenId);
    ErrCode CheckSystemAppOrNativePermission(const int32_t uid, OHOS::Security::AccessToken::AccessTokenID tokenId);
    ErrCode CheckNativePermission(OHOS::Security::AccessToken::AccessTokenID tokenId);
    void InitAppStateSubscriber();
    void InitContinuousSubscriber();
    bool SubscribeAppState();
    bool SubscribeContinuousTask();
    OHOS::sptr<OHOS::AppExecFwk::IAppMgr> GetAppManagerInstance();
    void QueryModuleRecordInfos(BundleActiveModuleRecord& moduleRecord);
    void SerModuleProperties(const HapModuleInfo& hapModuleInfo,
    const ApplicationInfo& appInfo, const AbilityInfo& abilityInfo, BundleActiveModuleRecord& moduleRecord);
    void DumpUsage(std::string &result);
    bool AllowDump();
    int32_t ShellDump(const std::vector<std::string> &dumpOption, std::vector<std::string> &dumpInfo);
    int32_t DumpEvents(const std::vector<std::string> &dumpOption, std::vector<std::string> &dumpInfo);
    int32_t DumpPackageUsage(const std::vector<std::string> &dumpOption, std::vector<std::string> &dumpInfo);
    int32_t DumpModuleUsage(const std::vector<std::string> &dumpOption, std::vector<std::string> &dumpInfo);
    int32_t DumpHighFreqHourUsage(const std::vector<std::string> &dumpOption, std::vector<std::string> &dumpInfo);
    int32_t GetNameAndIndexForUid(int32_t uid);
    std::vector<BundleActivePackageStats> MergePackageStats(
        const std::vector<BundleActivePackageStats>& packageStats);
    void MergeSamePackageStats(BundleActivePackageStats &left, const BundleActivePackageStats &right);
};
}  // namespace DeviceUsageStats
}  // namespace OHOS
#endif  // BUNDLE_ACTIVE_SERVICE_H

