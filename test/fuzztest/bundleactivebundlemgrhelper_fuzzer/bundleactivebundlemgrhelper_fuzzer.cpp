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

#include "bundleactivebundlemgrhelper_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>

#include "bundle_active_bundle_mgr_helper.h"
#include "bundle_active_group_controller.h"
#include "fuzztest_helper.h"

namespace OHOS {
namespace DeviceUsageStats {

constexpr int32_t FUZZ_CODE_0  = 0;  // GetNameForUid
constexpr int32_t FUZZ_CODE_1  = 1;  // GetApplicationInfo (bundle not installed path)
constexpr int32_t FUZZ_CODE_2  = 2;  // GetApplicationInfo (bundle installed path)
constexpr int32_t FUZZ_CODE_3  = 3;  // GetApplicationInfos (empty result path)
constexpr int32_t FUZZ_CODE_4  = 4;  // GetApplicationInfos (non-empty result path)
constexpr int32_t FUZZ_CODE_5  = 5;  // GetBundleInfo
constexpr int32_t FUZZ_CODE_6  = 6;  // IsLauncherApp (cache miss + connect fail)
constexpr int32_t FUZZ_CODE_7  = 7;  // IsLauncherApp (cache hit path)
constexpr int32_t FUZZ_CODE_8  = 8;  // IsLauncherApp (system event names: lock/unlock/sleep/wakeup)
constexpr int32_t FUZZ_CODE_9  = 9;  // InsertPackageUid + GetPackageUidSet
constexpr int32_t FUZZ_CODE_10 = 10; // DeletePackageUid (remove uid, erase bundle)
constexpr int32_t FUZZ_CODE_11 = 11; // DeleteMemPackage
constexpr int32_t FUZZ_CODE_12 = 12; // InitLauncherAppMap
constexpr int32_t FUZZ_CODE_13 = 13; // Connect failure path
constexpr int32_t FUZZ_CODE_MAX = 13;

// Magic numbers for random range boundaries
constexpr int32_t kFlagCodeRangeMax = 3;
constexpr int32_t kBundleFlagCodeRangeMax = 2;

// Magic number for loop count in Connect failure path test
constexpr int32_t kConnectRetryCount = 3;

// Magic numbers for bundle name cache test
constexpr int32_t kBundleCacheCountMax = 5;
constexpr int32_t kUidInsertCountMax = 3;

// Magic number for invalid user ID in system event test
constexpr int32_t kInvalidUserId = -1;

/* -------------------------------------------------------
 * Helper: ensure BundleActiveGroupController is initialized
 * (which indirectly initializes BundleActiveBundleMgrHelper via its dependencies)
 * ------------------------------------------------------- */
static void EnsureGroupControllerInit()
{
    auto handler = BundleActiveGroupController::GetInstance().GetBundleGroupHandler();
    if (!handler) {
        BundleActiveGroupController::GetInstance().Init(false);
    }
}

/* -------------------------------------------------------
 * FUZZ_CODE_0: GetNameForUid
 * Covers: Connect() fail path -> early return
 *         Connect() success path -> bundleMgr_->GetNameForUid
 * ------------------------------------------------------- */
bool BundleMgrGetNameForUidFuzzTest(FuzzedDataProvider* fdp)
{
    int32_t uid = fdp->ConsumeIntegral<int32_t>();
    std::string bundleName;
    BundleActiveBundleMgrHelper::GetInstance()->GetNameForUid(uid, bundleName);
    return true;
}

/* -------------------------------------------------------
 * FUZZ_CODE_1: GetApplicationInfo (bundle not installed)
 * Covers: Connect() fail -> return false
 *         Connect() success -> bundle not found -> return false
 * ------------------------------------------------------- */
bool BundleMgrGetApplicationInfoNotInstalledFuzzTest(FuzzedDataProvider* fdp)
{
    std::string appName = fdp->ConsumeRandomLengthString();
    int32_t userId = fdp->ConsumeIntegral<int32_t>();

    AppExecFwk::ApplicationInfo appInfo;
    bool result = BundleActiveBundleMgrHelper::GetInstance()->GetApplicationInfo(
        appName, AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, appInfo);
    (void) result;  // unused but exercises the path
    return true;
}

/* -------------------------------------------------------
 * FUZZ_CODE_2: GetApplicationInfo (various flags)
 * Covers: GetApplicationInfo with different ApplicationFlag values
 * ------------------------------------------------------- */
bool BundleMgrGetApplicationInfoVariousFlagsFuzzTest(FuzzedDataProvider* fdp)
{
    std::string appName = fdp->ConsumeRandomLengthString();
    int32_t userId = fdp->ConsumeIntegral<int32_t>();
    int32_t flagCode = fdp->ConsumeIntegralInRange(0, kFlagCodeRangeMax);

    AppExecFwk::ApplicationFlag flag;
    switch (flagCode) {
        case 0:
            flag = AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO;
            break;
        case 1:
            flag = AppExecFwk::ApplicationFlag::GET_APPLICATION_INFO_WITH_PERMISSION;
            break;
        default:
            flag = AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO;
            break;
    }

    AppExecFwk::ApplicationInfo appInfo;
    BundleActiveBundleMgrHelper::GetInstance()->GetApplicationInfo(
        appName, flag, userId, appInfo);
    return true;
}

/* -------------------------------------------------------
 * FUZZ_CODE_3: GetApplicationInfos (empty result)
 * Covers: Connect() fail -> return false
 *         Connect() success -> appInfos empty -> return true
 * ------------------------------------------------------- */
bool BundleMgrGetApplicationInfosEmptyFuzzTest(FuzzedDataProvider* fdp)
{
    int32_t userId = fdp->ConsumeIntegral<int32_t>();

    std::vector<BundleActiveApplication> appInfos;
    BundleActiveBundleMgrHelper::GetInstance()->GetApplicationInfos(
        AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, appInfos);
    return true;
}

/* -------------------------------------------------------
 * FUZZ_CODE_4: GetApplicationInfos (various user IDs)
 * Covers: different userId -> possibly different app list
 * ------------------------------------------------------- */
bool BundleMgrGetApplicationInfosVariousUserFuzzTest(FuzzedDataProvider* fdp)
{
    int32_t userId = fdp->ConsumeIntegralInRange(-10, 100);

    std::vector<BundleActiveApplication> appInfos;
    BundleActiveBundleMgrHelper::GetInstance()->GetApplicationInfos(
        AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, appInfos);
    return true;
}

/* -------------------------------------------------------
 * FUZZ_CODE_5: GetBundleInfo
 * Covers: Connect() fail -> return false
 *         Connect() success -> bundle not found -> return false
 *         Connect() success -> bundle found -> return true
 * ------------------------------------------------------- */
bool BundleMgrGetBundleInfoFuzzTest(FuzzedDataProvider* fdp)
{
    std::string bundleName = fdp->ConsumeRandomLengthString();
    int32_t userId = fdp->ConsumeIntegral<int32_t>();
    int32_t flagCode = fdp->ConsumeIntegralInRange(0, kBundleFlagCodeRangeMax);

    AppExecFwk::BundleFlag flag;
    switch (flagCode) {
        case 0:
            flag = AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT;
            break;
        case 1:
            flag = AppExecFwk::BundleFlag::GET_BUNDLE_WITH_EXTENSION_INFO;
            break;
        default:
            flag = AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT;
            break;
    }

    AppExecFwk::BundleInfo bundleInfo;
    BundleActiveBundleMgrHelper::GetInstance()->GetBundleInfo(
        bundleName, flag, bundleInfo, userId);
    return true;
}

/* -------------------------------------------------------
 * FUZZ_CODE_6: IsLauncherApp (cache miss + connect fail)
 * Covers: !isInitLauncherAppMap_ -> InitLauncherAppMap()
 *         InitLauncherAppMap() -> GetApplicationInfos fail -> return false
 *         Connect() fail in GetApplicationInfo -> return false
 * ------------------------------------------------------- */
bool BundleMgrIsLauncherAppCacheMissFuzzTest(FuzzedDataProvider* fdp)
{
    std::string bundleName = fdp->ConsumeRandomLengthString();
    int32_t userId = fdp->ConsumeIntegral<int32_t>();

    BundleActiveBundleMgrHelper::GetInstance()->IsLauncherApp(bundleName, userId);
    return true;
}

/* -------------------------------------------------------
 * FUZZ_CODE_7: IsLauncherApp (cache hit)
 * Covers: isInitLauncherAppMap_ == true
 *         launcherAppMap_.find(bundleName) != end() -> return cached value
 *         Multiple calls to fill cache first, then call to hit cache
 * ------------------------------------------------------- */
bool BundleMgrIsLauncherAppCacheHitFuzzTest(FuzzedDataProvider* fdp)
{
    EnsureGroupControllerInit();

    // Pre-fill cache with several bundle names
    std::vector<std::string> bundleNames;
    int count = fdp->ConsumeIntegralInRange(1, kBundleCacheCountMax);
    for (int i = 0; i < count; ++i) {
        bundleNames.push_back(fdp->ConsumeRandomLengthString());
    }

    int32_t userId = fdp->ConsumeIntegral<int32_t>();

    // First pass: trigger cache misses (fill cache)
    for (const auto& name : bundleNames) {
        BundleActiveBundleMgrHelper::GetInstance()->IsLauncherApp(name, userId);
    }

    // Second pass: all should hit cache
    for (const auto& name : bundleNames) {
        BundleActiveBundleMgrHelper::GetInstance()->IsLauncherApp(name, userId);
    }

    // Also test a name that's not in cache
    std::string newName = fdp->ConsumeRandomLengthString();
    BundleActiveBundleMgrHelper::GetInstance()->IsLauncherApp(newName, userId);
    return true;
}

/* -------------------------------------------------------
 * FUZZ_CODE_8: IsLauncherApp (system event bundle names)
 * Covers: System event bundle names that are pre-inserted in InitSystemEvent:
 *         OPERATION_SYSTEM_LOCK, OPERATION_SYSTEM_UNLOCK,
 *         OPERATION_SYSTEM_SLEEP, OPERATION_SYSTEM_WAKEUP
 *         -> launcherAppMap_[name] = false -> return false
 * ------------------------------------------------------- */
bool BundleMgrIsLauncherAppSystemEventFuzzTest(FuzzedDataProvider* /*fdp*/)
{
    // These are the system event bundle names defined in bundle_active_constant.h
    // and inserted in InitSystemEvent()
    BundleActiveBundleMgrHelper::GetInstance()->IsLauncherApp("com.system_operation.lock", kInvalidUserId);
    BundleActiveBundleMgrHelper::GetInstance()->IsLauncherApp("com.system_operation.unlock", kInvalidUserId);
    BundleActiveBundleMgrHelper::GetInstance()->IsLauncherApp("com.system_operation.sleep", kInvalidUserId);
    BundleActiveBundleMgrHelper::GetInstance()->IsLauncherApp("com.system_operation.wakeup", kInvalidUserId);
    return true;
}

/* -------------------------------------------------------
 * FUZZ_CODE_9: InsertPackageUid + GetPackageUidSet
 * Covers: InsertPackageUid -> packageContainUidMap_[bundleName].insert(uid)
 *         GetPackageUidSet -> find -> return set / return empty set
 * ------------------------------------------------------- */
bool BundleMgrInsertAndGetUidSetFuzzTest(FuzzedDataProvider* fdp)
{
    int32_t uid = fdp->ConsumeIntegral<int32_t>();
    std::string bundleName = fdp->ConsumeRandomLengthString();

    // Insert
    BundleActiveBundleMgrHelper::GetInstance()->InsertPackageUid(bundleName, uid);

    // Get (existing)
    std::set<int32_t> uidSet = BundleActiveBundleMgrHelper::GetInstance()->GetPackageUidSet(bundleName);

    // Insert more UIDs
    int moreCount = fdp->ConsumeIntegralInRange(0, kUidInsertCountMax);
    for (int i = 0; i < moreCount; ++i) {
        int32_t anotherUid = fdp->ConsumeIntegral<int32_t>();
        BundleActiveBundleMgrHelper::GetInstance()->InsertPackageUid(bundleName, anotherUid);
    }

    // Get again
    BundleActiveBundleMgrHelper::GetInstance()->GetPackageUidSet(bundleName);

    // Get non-existent bundle
    std::string otherBundle = fdp->ConsumeRandomLengthString();
    BundleActiveBundleMgrHelper::GetInstance()->GetPackageUidSet(otherBundle);

    return true;
}

/* -------------------------------------------------------
 * FUZZ_CODE_10: DeletePackageUid
 * Covers: DeletePackageUid -> erase uid from set
 *         If set becomes empty -> erase bundleName from map
 * ------------------------------------------------------- */
bool BundleMgrDeletePackageUidFuzzTest(FuzzedDataProvider* fdp)
{
    std::string bundleName = fdp->ConsumeRandomLengthString();
    int32_t userId = fdp->ConsumeIntegral<int32_t>();

    // Insert several UIDs first
    int uidCount = fdp->ConsumeIntegralInRange(1, kBundleCacheCountMax);
    for (int i = 0; i < uidCount; ++i) {
        BundleActiveBundleMgrHelper::GetInstance()->InsertPackageUid(bundleName, i);
    }

    // Delete all UIDs (should erase bundle from map)
    for (int i = 0; i < uidCount; ++i) {
        BundleActiveBundleMgrHelper::GetInstance()->DeletePackageUid(bundleName, i);
    }

    // Delete from non-existent bundle (no-op)
    std::string otherBundle = fdp->ConsumeRandomLengthString();
    BundleActiveBundleMgrHelper::GetInstance()->DeletePackageUid(otherBundle,
        fdp->ConsumeIntegral<int32_t>());
    return true;
}

/* -------------------------------------------------------
 * FUZZ_CODE_11: DeleteMemPackage
 * Covers: DeleteMemPackage -> erase bundle from packageContainUidMap_
 * ------------------------------------------------------- */
bool BundleMgrDeleteMemPackageFuzzTest(FuzzedDataProvider* fdp)
{
    std::string bundleName = fdp->ConsumeRandomLengthString();
    int32_t userId = fdp->ConsumeIntegral<int32_t>();

    // Insert UID to create entry
    BundleActiveBundleMgrHelper::GetInstance()->InsertPackageUid(bundleName, userId);

    // Delete memory package
    BundleActiveBundleMgrHelper::GetInstance()->DeleteMemPackage(bundleName);

    // Delete non-existent package (no-op)
    std::string otherBundle = fdp->ConsumeRandomLengthString();
    BundleActiveBundleMgrHelper::GetInstance()->DeleteMemPackage(otherBundle);

    return true;
}

/* -------------------------------------------------------
 * FUZZ_CODE_12: InitLauncherAppMap
 * Covers: InitLauncherAppMap -> isInitLauncherAppMap_ = true
 *         InitSystemEvent() -> pre-populates system event names
 *         GetApplicationInfos -> fills launcherAppMap_ with real apps
 *         (only affects internal state, no direct return value)
 * ------------------------------------------------------- */
bool BundleMgrInitLauncherAppMapFuzzTest(FuzzedDataProvider* fdp)
{
    (void) fdp;
    // InitLauncherAppMap is called internally by IsLauncherApp when !isInitLauncherAppMap_
    // Here we explicitly call it to exercise the code path
    BundleActiveBundleMgrHelper::GetInstance()->InitLauncherAppMap();

    // Call again (should be a no-op since isInitLauncherAppMap_ = true)
    BundleActiveBundleMgrHelper::GetInstance()->InitLauncherAppMap();
    return true;
}

/* -------------------------------------------------------
 * FUZZ_CODE_13: Connect failure path
 * Covers: Connect() with all failure branches:
 *         - bundleMgr_ != nullptr -> return true (already connected)
 *         - systemAbilityManager == nullptr -> return false
 *         - remoteObject == nullptr -> return false
 *         - bundleMgr_ == nullptr after iface_cast -> return false
 * ------------------------------------------------------- */
bool BundleMgrConnectFailureFuzzTest(FuzzedDataProvider* fdp)
{
    (void) fdp;
    // The Connect() method is called internally by all other methods.
    // Calling various methods repeatedly exercises different Connect() paths:
    // 1. First call: bundleMgr_ == nullptr -> try to connect -> fail or succeed
    // 2. Subsequent calls: bundleMgr_ != nullptr -> return true immediately
    EnsureGroupControllerInit();

    // Call multiple times to hit "already connected" path
    std::string bundleName;
    for (int i = 0; i < kConnectRetryCount; ++i) {
        BundleActiveBundleMgrHelper::GetInstance()->GetNameForUid(0, bundleName);
    }

    return true;
}

/* -------------------------------------------------------
 * Dispatch
 * ------------------------------------------------------- */
void FuzzBundleMgrHelper(FuzzedDataProvider& fdp)
{
    int code = fdp.ConsumeIntegralInRange(FUZZ_CODE_0, FUZZ_CODE_MAX);
    switch (code) {
        case FUZZ_CODE_0:
            BundleMgrGetNameForUidFuzzTest(&fdp);
            break;
        case FUZZ_CODE_1:
            BundleMgrGetApplicationInfoNotInstalledFuzzTest(&fdp);
            break;
        case FUZZ_CODE_2:
            BundleMgrGetApplicationInfoVariousFlagsFuzzTest(&fdp);
            break;
        case FUZZ_CODE_3:
            BundleMgrGetApplicationInfosEmptyFuzzTest(&fdp);
            break;
        case FUZZ_CODE_4:
            BundleMgrGetApplicationInfosVariousUserFuzzTest(&fdp);
            break;
        case FUZZ_CODE_5:
            BundleMgrGetBundleInfoFuzzTest(&fdp);
            break;
        case FUZZ_CODE_6:
            BundleMgrIsLauncherAppCacheMissFuzzTest(&fdp);
            break;
        case FUZZ_CODE_7:
            BundleMgrIsLauncherAppCacheHitFuzzTest(&fdp);
            break;
        case FUZZ_CODE_8:
            BundleMgrIsLauncherAppSystemEventFuzzTest(&fdp);
            break;
        case FUZZ_CODE_9:
            BundleMgrInsertAndGetUidSetFuzzTest(&fdp);
            break;
        case FUZZ_CODE_10:
            BundleMgrDeletePackageUidFuzzTest(&fdp);
            break;
        case FUZZ_CODE_11:
            BundleMgrDeleteMemPackageFuzzTest(&fdp);
            break;
        case FUZZ_CODE_12:
            BundleMgrInitLauncherAppMapFuzzTest(&fdp);
            break;
        case FUZZ_CODE_13:
            BundleMgrConnectFailureFuzzTest(&fdp);
            break;
        default:
            break;
    }
}

} // namespace DeviceUsageStats
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Set native token permission */
    OHOS::DelayedSingleton<OHOS::DeviceUsageStats::FuzztestHelper>::GetInstance()->NativeTokenGet();

    FuzzedDataProvider fdp(data, size);
    OHOS::DeviceUsageStats::FuzzBundleMgrHelper(fdp);
    return 0;
}