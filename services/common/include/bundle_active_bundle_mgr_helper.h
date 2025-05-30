/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef BUNDLE_ACTIVE_BUNDLE_MGR_HELPER_H
#define BUNDLE_ACTIVE_BUNDLE_MGR_HELPER_H

#include "errors.h"
#include "bundle_mgr_interface.h"
#include "bundle_mgr_proxy.h"
#include "ipc_skeleton.h"
#include "iremote_object.h"
#include "singleton.h"
#include <unordered_set>
#include "ffrt.h"

namespace OHOS {
namespace DeviceUsageStats {
struct BundleActiveApplication {
    std::string bundleName;
    int32_t uid;
    bool isLauncherApp;
    BundleActiveApplication(std::string bundleName_, int32_t uid_, bool isLauncherApp_)
        : bundleName{std::move(bundleName_)}, uid{uid_}, isLauncherApp{isLauncherApp_} {}
};
class BundleActiveBundleMgrHelper : public DelayedSingleton<BundleActiveBundleMgrHelper> {
public:
    /**
     * @brief GetNameForUid by uid.
     *
     * @param uid .
     * @param bundleName .
     */
    void GetNameForUid(int32_t uid, std::string& bundleName);

    /**
     * @brief GetBundleInfo by bundleName、flag、userId.
     *
     * @param bundleName .
     * @param flag which type is AppExecFwk::ApplicationFlag.
     * @param bundleInfo.
     * @param userId which type is AppExecFwk::BundleInfo.
     */
    bool GetBundleInfo(const std::string &bundleName, const AppExecFwk::BundleFlag flag,
        AppExecFwk::BundleInfo &bundleInfo, int32_t userId);

    /**
     * @brief GetApplicationInfo, get bundleName by uid.
     *
     * @param appName .
     * @param flag which type is AppExecFwk::ApplicationFlag.
     * @param userId.
     * @param appInfo which type is AppExecFwk::ApplicationInfo.
     */
    bool GetApplicationInfo(const std::string &appName, const AppExecFwk::ApplicationFlag flag,
        const int userId, AppExecFwk::ApplicationInfo &appInfo);
    
    bool GetApplicationInfos(const AppExecFwk::ApplicationFlag flag,
        const int userId, std::vector<BundleActiveApplication> &bundleActiveApplicationInfos);

    bool IsLauncherApp(const std::string &bundleName, const int32_t userId);

    void InsertPackageUid(const std::string &bundleName, const int32_t userId);
    
    std::set<int32_t> GetPackageUidSet(const std::string &bundleName);

    void DeletePackageUid(const std::string &bundleName, const int32_t userId);

    void DeleteMemPackage(const std::string &bundleName);

    void InitLauncherAppMap();

private:
    bool Connect();
    void InitSystemEvent();
    std::map<std::string, bool> launcherAppMap_;
    std::map<std::string, std::set<int32_t>> packageContainUidMap_;
    bool isInitLauncherAppMap_ = false;
    sptr<AppExecFwk::IBundleMgr> bundleMgr_ = nullptr;
    ffrt::mutex connectionMutex_;
    ffrt::mutex packageContainUidMapMutex_;
    DECLARE_DELAYED_SINGLETON(BundleActiveBundleMgrHelper);
};
}  // namespace DeviceUsageStats
}  // namespace OHOS
#endif  // BUNDLE_ACTIVE_BUNDLE_MGR_HELPER_H

