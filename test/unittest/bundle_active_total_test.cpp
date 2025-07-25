/*
 * Copyright (c) 2022-2024  Huawei Device Co., Ltd.
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

#include <string>

#include <gtest/gtest.h>
#include "system_ability_definition.h"

#include "bundle_active_power_state_callback_service.h"
#include "bundle_active_power_state_callback_service.h"
#include "bundle_active_service_stub.h"
#include "bundle_active_core.h"
#include "bundle_active_continuous_task_observer.h"
#include "bundle_active_bundle_mgr_helper.h"
#include "bundle_active_app_state_observer.h"
#include "bundle_active_usage_database.h"
#include "bundle_active_account_helper.h"
#include "bundle_active_power_state_callback_service.h"
#include "bundle_active_open_callback.h"
#include "bundle_active_shutdown_callback_service.h"
#include "bundle_active_binary_search.h"
#include "bundle_active_debug_mode.h"
#include "bundle_active_report_controller.h"
#include "bundle_active_test_util.h"
#include "bundle_active_event_reporter.h"
#include "file_ex.h"

using namespace testing::ext;

namespace OHOS {
namespace DeviceUsageStats {

class BundleActiveTotalTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void BundleActiveTotalTest::SetUpTestCase(void)
{
    BundleActiveTestUtil::TestInit();
}

void BundleActiveTotalTest::TearDownTestCase(void)
{
    BundleActiveTestUtil::TestDeInit();
}

void BundleActiveTotalTest::SetUp(void)
{
}

void BundleActiveTotalTest::TearDown(void)
{
}

/*
 * @tc.name: BundleActiveAccountHelperTest_001
 * @tc.desc: test the interface of bundle_active_account_helper
 * @tc.type: FUNC
 * @tc.require: DTS2023121404861
 */
HWTEST_F(BundleActiveTotalTest, BundleActiveAccountHelperTest_001, Function | MediumTest | TestSize.Level0)
{
    std::vector<int32_t> activatedOsAccountIds;

    BundleActiveAccountHelper::GetActiveUserId(activatedOsAccountIds);

    EXPECT_NE(activatedOsAccountIds.size(), 0);
}

/*
 * @tc.name: BundleActiveContinuousTaskObserverTest_001
 * @tc.desc: test the interface of bundle_active_continuous_task_observer
 * @tc.type: FUNC
 * @tc.require: DTS2023121404861
 */
HWTEST_F(BundleActiveTotalTest, BundleActiveContinuousTaskObserverTest_001, Function | MediumTest | TestSize.Level0)
{
#ifdef BGTASKMGR_ENABLE
#ifdef OS_ACCOUNT_PART_ENABLED
    BundleActiveReportController::GetInstance().Init();
    BundleActiveContinuousTaskObserver test;

    std::shared_ptr<OHOS::BackgroundTaskMgr::ContinuousTaskCallbackInfo> continuousTaskCallbackInfo;
    test.OnContinuousTaskStart(continuousTaskCallbackInfo);

    test.ReportContinuousTaskEvent(continuousTaskCallbackInfo, true);
    test.ReportContinuousTaskEvent(continuousTaskCallbackInfo, false);
    SUCCEED();

    test.GetBundleMgr();
    test.bundleMgr_ = nullptr;
    EXPECT_TRUE(test.GetBundleMgr());
#endif
#endif
}

/*
 * @tc.name: BundleActiveBundleMgrHelperTest_001
 * @tc.desc: test the interface of bundle_active_bundle_mgr_helper
 * @tc.type: FUNC
 * @tc.require: DTS2023121404861
 */
HWTEST_F(BundleActiveTotalTest, BundleActiveBundleMgrHelperTest_001, Function | MediumTest | TestSize.Level0)
{
    AppExecFwk::ApplicationFlag flag = AppExecFwk::GET_BASIC_APPLICATION_INFO;
    AppExecFwk::ApplicationInfo appInfo;
    AppExecFwk::BundleInfo bundleInfo;
    BundleActiveBundleMgrHelper test;

    std::string string = "test";
    test.GetNameForUid(0, string);
    test.GetApplicationInfo(string, flag, 0, appInfo);
    test.GetBundleInfo(string, AppExecFwk::BundleFlag::GET_BUNDLE_WITH_EXTENSION_INFO, bundleInfo, 0);
    SUCCEED();

    test.bundleMgr_ = nullptr;
    test.GetNameForUid(0, string);
    bool result = test.GetApplicationInfo(string, flag, 0, appInfo);
    EXPECT_FALSE(result);
    test.GetBundleInfo(string, AppExecFwk::BundleFlag::GET_BUNDLE_WITH_EXTENSION_INFO, bundleInfo, 0);
    SUCCEED();
}

/*
 * @tc.name: BundleActiveBundleMgrHelperTest_002
 * @tc.desc: test GetApplicationInfos
 * @tc.type: FUNC
 * @tc.require: issuesI9SQ6G
 */
HWTEST_F(BundleActiveTotalTest, BundleActiveBundleMgrHelperTest_002, Function | MediumTest | TestSize.Level0)
{
    AppExecFwk::ApplicationFlag flag = AppExecFwk::GET_BASIC_APPLICATION_INFO;
    std::vector<BundleActiveApplication> appInfos;
    bool result = BundleActiveBundleMgrHelper::GetInstance()->GetApplicationInfos(flag, 0, appInfos);
    EXPECT_TRUE(result);
}

/*
 * @tc.name: BundleActiveBundleMgrHelperTest_003
 * @tc.desc: test is launcherApp
 * @tc.type: FUNC
 * @tc.require: issuesI9SQ6G
 */
HWTEST_F(BundleActiveTotalTest, BundleActiveBundleMgrHelperTest_003, Function | MediumTest | TestSize.Level0)
{
    BundleActiveBundleMgrHelper::GetInstance()->InitLauncherAppMap();
    BundleActiveBundleMgrHelper::GetInstance()->IsLauncherApp("test", 0);
    BundleActiveBundleMgrHelper::GetInstance()->launcherAppMap_["test"] = false;
    BundleActiveBundleMgrHelper::GetInstance()->IsLauncherApp("test", 0);
    EXPECT_TRUE(BundleActiveBundleMgrHelper::GetInstance()->isInitLauncherAppMap_);
}

/*
 * @tc.name: BundleActiveBundleMgrHelperTest_004
 * @tc.desc: test is packageContainUidMap
 * @tc.type: FUNC
 * @tc.require: issuesI9SQ6G
 */
HWTEST_F(BundleActiveTotalTest, BundleActiveBundleMgrHelperTest_004, Function | MediumTest | TestSize.Level0)
{
    BundleActiveBundleMgrHelper::GetInstance()->packageContainUidMap_.clear();
    BundleActiveBundleMgrHelper::GetInstance()->InsertPackageUid("test", 0);
    EXPECT_NE(BundleActiveBundleMgrHelper::GetInstance()->packageContainUidMap_.size(), 0);
    BundleActiveBundleMgrHelper::GetInstance()->DeletePackageUid("test", 0);
    EXPECT_EQ(BundleActiveBundleMgrHelper::GetInstance()->packageContainUidMap_.size(), 0);
    BundleActiveBundleMgrHelper::GetInstance()->InsertPackageUid("test", 0);
    auto uidSet = BundleActiveBundleMgrHelper::GetInstance()->GetPackageUidSet("test");
    EXPECT_NE(uidSet.size(), 0);
    BundleActiveBundleMgrHelper::GetInstance()->DeleteMemPackage("test");
    uidSet = BundleActiveBundleMgrHelper::GetInstance()->GetPackageUidSet("test");
    EXPECT_EQ(uidSet.size(), 0);
}

/*
 * @tc.name: BundleActiveAppStateObserverTest_001
 * @tc.desc: test the interface of bundle_active_app_state_observer
 * @tc.type: FUNC
 * @tc.require: DTS2023121404861
 */
HWTEST_F(BundleActiveTotalTest, BundleActiveAppStateObserverTest_001, Function | MediumTest | TestSize.Level0)
{
    std::shared_ptr<BundleActiveCore> bundleActiveCore = std::make_shared<BundleActiveCore>();
    BundleActiveReportController::GetInstance().Init(bundleActiveCore);
    AbilityStateData abilityStateData;
    abilityStateData.abilityType = 1;
    BundleActiveAppStateObserver test;
    test.OnAbilityStateChanged(abilityStateData);

    abilityStateData.abilityState = static_cast<int32_t>(AppExecFwk::AbilityState::ABILITY_STATE_FOREGROUND);
    test.OnAbilityStateChanged(abilityStateData);
    EXPECT_TRUE(BundleActiveReportController::GetInstance().GetBundleReportHandler() != nullptr);

    abilityStateData.abilityState = static_cast<int32_t>(AppExecFwk::AbilityState::ABILITY_STATE_BACKGROUND);
    test.OnAbilityStateChanged(abilityStateData);
    SUCCEED();

    abilityStateData.abilityState = static_cast<int32_t>(AppExecFwk::AbilityState::ABILITY_STATE_TERMINATED);
    test.OnAbilityStateChanged(abilityStateData);
    SUCCEED();
}

/*
 * @tc.name: BundleActiveUsageDatabaseTest_001
 * @tc.desc: test the interface of bundle_active_usage_database
 * @tc.type: FUNC
 * @tc.require: DTS2023121404861
 */
HWTEST_F(BundleActiveTotalTest, BundleActiveUsageDatabaseTest_001, Function | MediumTest | TestSize.Level0)
{
    BundleActiveUsageDatabase test;
    test.InitDatabaseTableInfo(test.ParseStartTime(test.eventTableName_)-1);
    EXPECT_FALSE(test.debugDatabase_);
}

/*
 * @tc.name: BundleActiveUsageDatabaseTest_002
 * @tc.desc: test the interface of bundle_active_usage_database
 * @tc.type: FUNC
 * @tc.require: DTS2023121404861
 */
HWTEST_F(BundleActiveTotalTest, BundleActiveUsageDatabaseTest_002, Function | MediumTest | TestSize.Level0)
{
    BundleActiveUsageDatabase test;
    EXPECT_FALSE(test.debugDatabase_);
    test.debugDatabase_ = true;
    test.DeleteExcessiveTableData(100);
    test.DeleteExcessiveTableData(0);
    SUCCEED();
}

/*
 * @tc.name: BundleActiveUsageDatabaseTest_003
 * @tc.desc: test the interface of bundle_active_usage_database
 * @tc.type: FUNC
 * @tc.require: DTS2023121404861
 */
HWTEST_F(BundleActiveTotalTest, BundleActiveUsageDatabaseTest_003, Function | MediumTest | TestSize.Level0)
{
    BundleActiveUsageDatabase test;
    auto result = test.GetOverdueTableCreateTime(100, 0);
    EXPECT_EQ(result, nullptr);
}

/*
 * @tc.name: BundleActiveUsageDatabaseTest_005
 * @tc.desc: test the interface of bundle_active_usage_database
 * @tc.type: FUNC
 * @tc.require: DTS2023121404861
 */
HWTEST_F(BundleActiveTotalTest, BundleActiveUsageDatabaseTest_005, Function | MediumTest | TestSize.Level0)
{
    BundleActiveUsageDatabase test;
    test.bundleHistoryTableName_ = "unknownTableName";
    std::shared_ptr<map<string, std::shared_ptr<BundleActivePackageHistory>>> userHistory;
    test.PutBundleHistoryData(0, userHistory);
    test.GetTableIndexSql(0, 0, true, 0);
    test.GetTableIndexSql(5, 0, true, 0);
    test.GetTableIndexSql(5, 0, false, 0);
    test.GetTableIndexSql(5, 0, false, 1);
    test.GetTableIndexSql(5, 0, false, 2);
    std::string result = test.GetTableIndexSql(0, 0, false, 0);
    EXPECT_TRUE(result != "");
}

/*
 * @tc.name: BundleActiveUsageDatabaseTest_006
 * @tc.desc: test the interface of bundle_active_usage_database
 * @tc.type: FUNC
 * @tc.require: DTS2023121404861
 */
HWTEST_F(BundleActiveTotalTest, BundleActiveUsageDatabaseTest_006, Function | MediumTest | TestSize.Level0)
{
    BundleActiveUsageDatabase test;
    std::shared_ptr<NativeRdb::RdbStore> rdbStore;
    int32_t result = test.SetNewIndexWhenTimeChanged(5, 0, 0, rdbStore);
    EXPECT_EQ(result, -1);
    test.RenameTableName(0, 0, 0);
    test.ExecuteRenameTableName("test", 0, 0, rdbStore);
    SUCCEED();
}

/*
 * @tc.name: BundleActiveUsageDatabaseTest_007
 * @tc.desc: test the interface of bundle_active_usage_database
 * @tc.type: FUNC
 * @tc.require: DTS2023121404861
 */
HWTEST_F(BundleActiveTotalTest, BundleActiveUsageDatabaseTest_007, Function | MediumTest | TestSize.Level0)
{
    BundleActiveUsageDatabase test;
    std::vector<NativeRdb::ValuesBucket> formValueBuckets;
    std::shared_ptr<NativeRdb::RdbStore> rdbStore;
    BundleActiveFormRecord formRecord;
    test.UpdateFormData(0, "test", "test", formRecord, rdbStore, formValueBuckets);
    std::string result = test.GetSystemEventName(0);
    EXPECT_EQ(result, "");
    test.JudgeQueryCondition(0, 0, 1);
    SUCCEED();
}

/*
 * @tc.name: BundleActivePowerStateCallbackServiceTest_001
 * @tc.desc: test the interface of bundle_active_usage_database
 * @tc.type: FUNC
 * @tc.require: DTS2023121404861
 */
HWTEST_F(BundleActiveTotalTest, BundleActivePowerStateCallbackServiceTest_001, Function | MediumTest | TestSize.Level0)
{
#ifdef DEVICE_USAGES_STATISTICS_POWERMANGER_ENABLE
    BundleActivePowerStateCallbackService test1(nullptr);

    std::shared_ptr<BundleActiveCore> bundleActiveCore = std::make_shared<BundleActiveCore>();
    BundleActivePowerStateCallbackService test2(bundleActiveCore);
    
    test2.OnPowerStateChanged(PowerState::AWAKE);
    SUCCEED();
    test2.OnPowerStateChanged(PowerState::SLEEP);
    EXPECT_TRUE(test2.bundleActiveCore_ != nullptr);
#endif
}

/*
 * @tc.name: BundleActiveBinarySearchTest_001
 * @tc.desc: test the interface of bundle_active_binary_search
 * @tc.type: FUNC
 * @tc.require: DTS2023121404861
 */
HWTEST_F(BundleActiveTotalTest, BundleActiveBinarySearchTest_001, Function | MediumTest | TestSize.Level0)
{
    std::vector<int64_t> tableNameArray;
    BundleActiveBinarySearch test;
    int32_t result = test.BinarySearch(tableNameArray, 0);
    EXPECT_EQ(result, -1);
    result = test.BinarySearch(tableNameArray, -100);
    EXPECT_EQ(result, -1);
    tableNameArray.push_back(-1);
    tableNameArray.push_back(0);
    tableNameArray.push_back(1);
    result = test.BinarySearch(tableNameArray, 0);
    EXPECT_EQ(result, 1);
}
}
}