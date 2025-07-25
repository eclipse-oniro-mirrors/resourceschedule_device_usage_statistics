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

#include <string>

#include <gtest/gtest.h>
#include "system_ability_definition.h"

#include "bundle_active_client.h"
#include "bundle_active_event.h"
#include "app_group_callback_stub.h"
#include "bundle_active_group_map.h"
#include "app_group_callback_info.h"
#include "bundle_active_form_record.h"
#include "bundle_active_event_stats.h"
#include "bundle_active_module_record.h"
#include "bundle_active_package_stats.h"
#include "app_group_callback_proxy.h"
#include "iapp_group_callback.h"
#include "bundle_active_log.h"
#include "accesstoken_kit.h"
#include "token_setproc.h"
#include "nativetoken_kit.h"
#include "bundle_active_test_util.h"
#include "bundle_active_high_frequency_period.h"

using namespace testing::ext;

namespace OHOS {
namespace DeviceUsageStats {
using namespace Security::AccessToken;
#ifdef __aarch64__
static std::string g_defaultBundleName = "com.huawei.hmos.camera";
#else
static std::string g_defaultBundleName = "com.ohos.camera";
#endif
static std::string g_defaultMoudleName = "defaultmodulename";
static std::string g_defaultFormName = "defaultformname";
static int32_t g_defaultDimension = 4;
static int64_t g_defaultFormId = 1;
static int32_t g_defaultUserId = 0;
static int32_t g_commonUserid = 100;
static int32_t DEFAULT_ERRCODE = 0;
static int64_t g_largeNum = 20000000000000;
static int32_t g_defaultGroup = 10;
static std::vector<int32_t> GROUP_TYPE {10, 20, 30, 40, 50};
static sptr<IAppGroupCallback> observer = nullptr;

class DeviceUsageStatisticsTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DeviceUsageStatisticsTest::SetUpTestCase(void)
{
    static const char *perms[] = {
        "ohos.permission.BUNDLE_ACTIVE_INFO",
    };
    uint64_t tokenId;
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 1,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "DeviceUsageStatisticsTest",
        .aplStr = "system_core",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    AccessTokenKit::ReloadNativeTokenInfo();
    BundleActiveTestUtil::TestInit();
}

void DeviceUsageStatisticsTest::TearDownTestCase(void)
{
    BundleActiveTestUtil::TestDeInit();
}

void DeviceUsageStatisticsTest::SetUp(void)
{
}

void DeviceUsageStatisticsTest::TearDown(void)
{
}

class TestAppGroupChangeCallback : public AppGroupCallbackStub {
public:
    TestAppGroupChangeCallback() = default;
    virtual ~TestAppGroupChangeCallback() = default;
    ErrCode OnAppGroupChanged(const AppGroupCallbackInfo &appGroupCallbackInfo) override;
};

ErrCode TestAppGroupChangeCallback::OnAppGroupChanged(const AppGroupCallbackInfo &appGroupCallbackInfo)
{
    BUNDLE_ACTIVE_LOGI("TestAppGroupChangeCallback::OnAppGroupChanged!");
    MessageParcel data;
    if (!appGroupCallbackInfo.Marshalling(data)) {
        BUNDLE_ACTIVE_LOGE("Marshalling fail");
    }
    appGroupCallbackInfo.Unmarshalling(data);
    return ERR_OK;
}

/*
 * @tc.name: DeviceUsageStatisticsTest_GetServiceObject_001
 * @tc.desc: get service object
 * @tc.type: FUNC
 * @tc.require: SR000GGTO8 AR000GH6PK
 */
HWTEST_F(DeviceUsageStatisticsTest, DeviceUsageStatisticsTest_GetServiceObject_001,
    Function | MediumTest | TestSize.Level0)
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(systemAbilityManager, nullptr);

    sptr<IRemoteObject> remoteObject =
        systemAbilityManager->GetSystemAbility(DEVICE_USAGE_STATISTICS_SYS_ABILITY_ID);
    EXPECT_NE(remoteObject, nullptr);
}

/*
 * @tc.name: DeviceUsageStatisticsTest_ReportEvent_001
 * @tc.desc: report a mock event
 * @tc.type: FUNC
 * @tc.require: SR000GGTO7 SR000GU31B AR000GH6PJ AR000GU380
 */
HWTEST_F(DeviceUsageStatisticsTest, DeviceUsageStatisticsTest_ReportEvent_001, Function | MediumTest | TestSize.Level0)
{
    BundleActiveEvent eventA(g_defaultBundleName, g_defaultMoudleName, g_defaultFormName,
        g_defaultDimension, g_defaultFormId, BundleActiveEvent::FORM_IS_CLICKED);
    BundleActiveClient::GetInstance().ReportEvent(eventA, g_defaultUserId);
    BundleActiveEvent eventB(g_defaultBundleName, g_defaultMoudleName, g_defaultFormName,
        g_defaultDimension, g_defaultFormId, BundleActiveEvent::FORM_IS_REMOVED);
    BundleActiveClient::GetInstance().ReportEvent(eventB, g_defaultUserId);
    EXPECT_TRUE(BundleActiveClient::GetInstance().bundleActiveProxy_ != nullptr);
}

/*
 * @tc.name: DeviceUsageStatisticsTest_QueryBundleEvents_001
 * @tc.desc: QueryBundleEvents
 * @tc.type: FUNC
 * @tc.require: SR000GGTO6 AR000GH6PH
 */
HWTEST_F(DeviceUsageStatisticsTest, DeviceUsageStatisticsTest_QueryBundleEvents_001,
    Function | MediumTest | TestSize.Level0)
{
    std::vector<BundleActiveEvent> result;
    BundleActiveClient::GetInstance().QueryBundleEvents(result, 0, g_largeNum, 100);
    EXPECT_EQ(result.size() > 0, true);
    EXPECT_NE(BundleActiveClient::GetInstance().QueryBundleEvents(result, g_largeNum, g_largeNum, 100), 0);
}

/*
 * @tc.name: DeviceUsageStatisticsTest_QueryCurrentBundleEvents_001
 * @tc.desc: QueryCurrentBundleEvents
 * @tc.type: FUNC
 * @tc.require: SR000GGTO4 AR000GH6PF
 */
HWTEST_F(DeviceUsageStatisticsTest,
    DeviceUsageStatisticsTest_QueryCurrentBundleEvents_001, Function | MediumTest | TestSize.Level0)
{
    std::vector<BundleActiveEvent> result;
    BundleActiveClient::GetInstance().QueryCurrentBundleEvents(result, 0, g_largeNum);
    EXPECT_EQ(result.size(), 0);
}

/*
 * @tc.name: DeviceUsageStatisticsTest_QueryPackagesStats_001
 * @tc.desc: querypackagestats
 * @tc.type: FUNC
 * @tc.require: SR000GGTO3 AR000GH6PD
 */
HWTEST_F(DeviceUsageStatisticsTest, DeviceUsageStatisticsTest_QueryPackagesStats_001,
    Function | MediumTest | TestSize.Level0)
{
    std::vector<BundleActivePackageStats> result;
    BundleActiveClient::GetInstance().QueryBundleStatsInfoByInterval(result, 4, 0, g_largeNum);
    EXPECT_EQ(result.size(), 0);
    EXPECT_NE(BundleActiveClient::GetInstance().QueryBundleStatsInfoByInterval(result, 4, g_largeNum, g_largeNum), 0);
}

/*
 * @tc.name: DeviceUsageStatisticsTest_QueryBundleStatsInfos_001
 * @tc.desc: QueryBundleStatsInfos
 * @tc.type: FUNC
 * @tc.require: issuesI5QJD9
 */
HWTEST_F(DeviceUsageStatisticsTest, DeviceUsageStatisticsTest_QueryBundleStatsInfos_001,
    Function | MediumTest | TestSize.Level0)
{
    std::vector<BundleActivePackageStats> result;
    BundleActiveClient::GetInstance().QueryBundleStatsInfos(result, 4, 0, g_largeNum);
    EXPECT_EQ(result.size(), 0);
}

/*
 * @tc.name: DeviceUsageStatisticsTest_IsBundleIdle_001
 * @tc.desc: isbundleidle
 * @tc.type: FUNC
 * @tc.require: SR000GGTO5 AR000GH6PG
 */
HWTEST_F(DeviceUsageStatisticsTest, DeviceUsageStatisticsTest_IsBundleIdle_001,
    Function | MediumTest | TestSize.Level0)
{
    bool result = false;
    int32_t errCode = BundleActiveClient::GetInstance().IsBundleIdle(result, g_defaultBundleName, g_defaultUserId);
    EXPECT_EQ(errCode, 0);
}

/*
 * @tc.name: DeviceUsageStatisticsTest_IsBundleUsePeriod_001
 * @tc.desc: IsBundleUsePeriod
 * @tc.type: FUNC
 * @tc.require: issueIBB2G4
 */
HWTEST_F(DeviceUsageStatisticsTest, DeviceUsageStatisticsTest_IsBundleUsePeriod_001,
    Function | MediumTest | TestSize.Level0)
{
    bool result = false;
    int32_t errCode = BundleActiveClient::GetInstance().IsBundleUsePeriod(result, g_defaultBundleName, g_defaultUserId);
    EXPECT_EQ(errCode, 0);
}

/*
 * @tc.name: DeviceUsageStatisticsTest_QueryModuleUsageRecords_001
 * @tc.desc: QueryModuleUsageRecords
 * @tc.type: FUNC
 * @tc.require: SR000GU2T1 AR000GU37U
 */
HWTEST_F(DeviceUsageStatisticsTest,
    DeviceUsageStatisticsTest_QueryModuleUsageRecords_001, Function | MediumTest | TestSize.Level0)
{
    int32_t maxNum = 1;
    BundleActiveEvent eventA(g_defaultBundleName, g_defaultMoudleName, g_defaultFormName,
        g_defaultDimension, g_defaultFormId, BundleActiveEvent::FORM_IS_CLICKED);
    BundleActiveClient::GetInstance().ReportEvent(eventA, g_defaultUserId);
    std::vector<BundleActiveModuleRecord> results;
    int32_t errCode = BundleActiveClient::GetInstance().QueryModuleUsageRecords(maxNum, results, g_defaultUserId);
    EXPECT_EQ(errCode, 0);
    EXPECT_EQ(results.size(), 0);

    results.clear();
    maxNum = 0;
    errCode = BundleActiveClient::GetInstance().QueryModuleUsageRecords(maxNum, results, g_defaultUserId);
    EXPECT_NE(errCode, 0);

    results.clear();
    maxNum = 1001;
    errCode = BundleActiveClient::GetInstance().QueryModuleUsageRecords(maxNum, results, g_defaultUserId);
    EXPECT_NE(errCode, 0);
}

/*
 * @tc.name: DeviceUsageStatisticsTest_RegisterAppGroupCallBack_001
 * @tc.desc: RegisterAppGroupCallBack
 * @tc.type: FUNC
 * @tc.require: SR000H0HAQ AR000H0ROE
 */
HWTEST_F(DeviceUsageStatisticsTest,
    DeviceUsageStatisticsTest_RegisterAppGroupCallBack_001, Function | MediumTest | TestSize.Level0)
{
    if (!observer) {
        BUNDLE_ACTIVE_LOGI("RegisterAppGroupCallBack construct observer!");
        observer = new (std::nothrow) TestAppGroupChangeCallback();
    }
    ASSERT_NE(observer, nullptr);
    int32_t result = BundleActiveClient::GetInstance().RegisterAppGroupCallBack(observer);
    EXPECT_EQ(result, DEFAULT_ERRCODE);
}

/*
 * @tc.name: DeviceUsageStatisticsTest_SetBundleGroup_001
 * @tc.desc: setbundlename
 * @tc.type: FUNC
 * @tc.require: SR000H0HAQ AR000H0ROE
 */
HWTEST_F(DeviceUsageStatisticsTest, DeviceUsageStatisticsTest_SetAppGroup_001, Function | MediumTest | TestSize.Level0)
{
    int32_t result = 0;
    BundleActiveClient::GetInstance().QueryAppGroup(result, g_defaultBundleName, g_commonUserid);
    g_defaultGroup = (result == g_defaultGroup) ? (result + 10) : g_defaultGroup;
    result = BundleActiveClient::GetInstance().SetAppGroup(g_defaultBundleName, g_defaultGroup, g_commonUserid);
    EXPECT_EQ(result, DEFAULT_ERRCODE);

    result = BundleActiveClient::GetInstance().SetAppGroup(g_defaultBundleName, g_defaultGroup, -1);
    EXPECT_NE(result, DEFAULT_ERRCODE);
}

/*
 * @tc.name: DeviceUsageStatisticsTest_QueryAppGroup_001
 * @tc.desc: QueryAppGroup, no bundleName
 * @tc.type: FUNC
 * @tc.require: SR000H0HAQ AR000H0ROE
 */
HWTEST_F(DeviceUsageStatisticsTest, DeviceUsageStatisticsTest_QueryAppGroup_001,
    Function | MediumTest | TestSize.Level0)
{
    BundleActiveClient::GetInstance().SetAppGroup(g_defaultBundleName, g_defaultGroup, g_commonUserid);
    int32_t result = 0;
    BundleActiveClient::GetInstance().QueryAppGroup(result, g_defaultBundleName, g_commonUserid);
    EXPECT_NE(result, DEFAULT_ERRCODE);
}

/*
 * @tc.name: DeviceUsageStatisticsTest_UnRegisterAppGroupCallBack_001
 * @tc.desc: UnRegisterAppGroupCallBack
 * @tc.type: FUNC
 * @tc.require: SR000H0HAQ AR000H0ROE
 */
HWTEST_F(DeviceUsageStatisticsTest, DeviceUsageStatisticsTest_UnRegisterAppGroupCallBack_001,
    Function | MediumTest | TestSize.Level0)
{
    if (!observer) {
        BUNDLE_ACTIVE_LOGI("observer has been delete");
    }
    ASSERT_NE(observer, nullptr);
    int32_t result = BundleActiveClient::GetInstance().UnRegisterAppGroupCallBack(observer);
    observer = nullptr;
    EXPECT_EQ(result, DEFAULT_ERRCODE);
}

/*
 * @tc.name: DeviceUsageStatisticsTest_QueryDeviceEventStats_001
 * @tc.desc: QueryDeviceEventStats
 * @tc.type: FUNC
 * @tc.require: SR000H0H9H AR000H0ROG
 */
HWTEST_F(DeviceUsageStatisticsTest,
    DeviceUsageStatisticsTest_QueryDeviceEventStats_001, Function | MediumTest | TestSize.Level0)
{
    std::vector<BundleActiveEventStats> eventStats;
    int32_t errCode = BundleActiveClient::GetInstance().QueryDeviceEventStats(0, g_largeNum, eventStats);
    EXPECT_EQ(errCode, 0);
    errCode = BundleActiveClient::GetInstance().QueryDeviceEventStats(0, g_largeNum, eventStats, g_commonUserid);
    EXPECT_EQ(errCode, 0);
}

/*
 * @tc.name: DeviceUsageStatisticsTest_QueryNotificationEventStats_001
 * @tc.desc: QueryNotificationEventStats
 * @tc.type: FUNC
 * @tc.require: SR000H0H7D AR000H0RR6
 */
HWTEST_F(DeviceUsageStatisticsTest, DeviceUsageStatisticsTest_QueryNotificationEventStats_001, Function
    | MediumTest | TestSize.Level0)
{
    std::vector<BundleActiveEventStats> eventStats;
    int32_t errCode = BundleActiveClient::GetInstance().QueryNotificationEventStats(0, g_largeNum, eventStats);
    EXPECT_EQ(errCode, 0);
    errCode = BundleActiveClient::GetInstance().QueryNotificationEventStats(0, g_largeNum, eventStats, g_commonUserid);
    EXPECT_EQ(errCode, 0);
}

/*
 * @tc.name: DeviceUsageStatisticsTest_BundleActiveGroupMap_001
 * @tc.desc: BundleActiveGroupMap
 * @tc.type: FUNC
 * @tc.require: SR000H0G4F AR000H2US8
 */
HWTEST_F(DeviceUsageStatisticsTest, DeviceUsageStatisticsTest_BundleActiveGroupMap_001,
    Function | MediumTest | TestSize.Level0)
{
    int64_t minInterval = DeviceUsageStatsGroupMap::groupIntervalMap_
        .at(DeviceUsageStatsGroupConst::ACTIVE_GROUP_FORCED_SET);
    EXPECT_EQ(minInterval, 0);
    minInterval = DeviceUsageStatsGroupMap::groupIntervalMap_
        .at(DeviceUsageStatsGroupConst::ACTIVE_GROUP_ALIVE);
    EXPECT_EQ(minInterval, DeviceUsageStatsGroupConst::TWO_HOUR);
    minInterval = DeviceUsageStatsGroupMap::groupIntervalMap_
        .at(DeviceUsageStatsGroupConst::ACTIVE_GROUP_DAILY);
    EXPECT_EQ(minInterval, 2 * DeviceUsageStatsGroupConst::TWO_HOUR);
    minInterval = DeviceUsageStatsGroupMap::groupIntervalMap_
        .at(DeviceUsageStatsGroupConst::ACTIVE_GROUP_FIXED);
    EXPECT_EQ(minInterval, DeviceUsageStatsGroupConst::TWENTY_FOUR_HOUR);
    minInterval = DeviceUsageStatsGroupMap::groupIntervalMap_
        .at(DeviceUsageStatsGroupConst::ACTIVE_GROUP_RARE);
    EXPECT_EQ(minInterval, DeviceUsageStatsGroupConst::FOURTY_EIGHT_HOUR);
}

/*
 * @tc.name: DeviceUsageStatisticsTest_DeathRecipient_001
 * @tc.desc: DeathRecipient_001
 * @tc.type: FUNC
 * @tc.require: issuesI5SOZY
 */
HWTEST_F(DeviceUsageStatisticsTest, DeviceUsageStatisticsTest_DeathRecipient_001,
    Function | MediumTest | TestSize.Level0)
{
    auto deathTest = std::make_shared<BundleActiveClient::BundleActiveClientDeathRecipient>();
    deathTest->AddObserver(observer);
    deathTest->RemoveObserver();
    deathTest->OnServiceDiedInner();

    deathTest->observer_ = new (std::nothrow) TestAppGroupChangeCallback();
    deathTest->OnServiceDiedInner();
    EXPECT_TRUE(deathTest != nullptr);
    deathTest->OnRemoteDied(nullptr);
}

/*
 * @tc.name: DeviceUsageStatisticsTest_AppGroupCallbackInfo_001
 * @tc.desc: AppGroupCallbackInfo_001
 * @tc.type: FUNC
 * @tc.require: issuesI5SOZY
 */
HWTEST_F(DeviceUsageStatisticsTest, DeviceUsageStatisticsTest_AppGroupCallbackInfo_001,
    Function | MediumTest | TestSize.Level0)
{
    int32_t oldGroup = 60;
    int32_t newGroup = 10;
    uint32_t changeReason = 1;
    auto appGroupCallbackInfo =
        std::make_shared<AppGroupCallbackInfo>(g_commonUserid, oldGroup, newGroup, changeReason, g_defaultBundleName);

    MessageParcel data;
    EXPECT_TRUE(appGroupCallbackInfo->Marshalling(data));
    auto appGroupCallback = appGroupCallbackInfo->Unmarshalling(data);
    EXPECT_TRUE(appGroupCallback != nullptr);

    EXPECT_EQ(appGroupCallback->GetUserId(), g_commonUserid);
    EXPECT_EQ(appGroupCallback->GetOldGroup(), oldGroup);
    EXPECT_EQ(appGroupCallback->GetNewGroup(), newGroup);
    EXPECT_EQ(appGroupCallback->GetChangeReason(), changeReason);
    EXPECT_EQ(appGroupCallback->GetBundleName(), g_defaultBundleName);
    delete appGroupCallback;
}

/*
 * @tc.name: DeviceUsageStatisticsTest_BundleActiveEventStat_001
 * @tc.desc: BundleActiveEventStats_001
 * @tc.type: FUNC
 * @tc.require: issuesI5SOZY
 */
HWTEST_F(DeviceUsageStatisticsTest, DeviceUsageStatisticsTest_BundleActiveEventStat_001,
    Function | MediumTest | TestSize.Level0)
{
    auto bundleActiveEventStats = std::make_shared<BundleActiveEventStats>();
    bundleActiveEventStats->eventId_ = g_commonUserid;
    bundleActiveEventStats->beginTimeStamp_ = 0;
    bundleActiveEventStats->endTimeStamp_ = g_largeNum;
    bundleActiveEventStats->lastEventTime_ = g_largeNum;
    bundleActiveEventStats->totalTime_ = g_largeNum;
    bundleActiveEventStats->count_ = 1;

    MessageParcel data;
    EXPECT_TRUE(bundleActiveEventStats->Marshalling(data));
    auto tempEventStats = bundleActiveEventStats->Unmarshalling(data);
    EXPECT_TRUE(tempEventStats != nullptr);

    auto bundleActiveEvent = std::make_shared<BundleActiveEvent>();
    EXPECT_TRUE(bundleActiveEvent->Marshalling(data));
    auto tempEvent = bundleActiveEvent->Unmarshalling(data);
    EXPECT_TRUE(tempEvent != nullptr);

    EXPECT_EQ(tempEventStats->GetEventId(), g_commonUserid);
    EXPECT_EQ(tempEventStats->GetFirstTimeStamp(), 0);
    EXPECT_EQ(tempEventStats->GetLastTimeStamp(), g_largeNum);
    EXPECT_EQ(tempEventStats->GetLastEventTime(), g_largeNum);
    EXPECT_EQ(tempEventStats->GetTotalTime(), g_largeNum);
    EXPECT_EQ(tempEventStats->GetCount(), 1);
}

/*
 * @tc.name: DeviceUsageStatisticsTest_FormRecord_001
 * @tc.desc: BundleActiveFormRecord_001
 * @tc.type: FUNC
 * @tc.require: issuesI5SOZY
 */
HWTEST_F(DeviceUsageStatisticsTest, DeviceUsageStatisticsTest_FormRecord_001, Function | MediumTest | TestSize.Level0)
{
    auto bundleActiveFormRecord = std::make_shared<BundleActiveFormRecord>();
    EXPECT_NE(bundleActiveFormRecord->ToString(), " ");

    MessageParcel data;
    EXPECT_TRUE(bundleActiveFormRecord->Marshalling(data));
    EXPECT_TRUE(bundleActiveFormRecord->Unmarshalling(data) != nullptr);

    BundleActiveFormRecord bundleActiveFormRecordA;
    bundleActiveFormRecordA.count_ = 2;
    BundleActiveFormRecord bundleActiveFormRecordB;
    bundleActiveFormRecordB.count_ = 1;
    EXPECT_TRUE(bundleActiveFormRecord->cmp(bundleActiveFormRecordA, bundleActiveFormRecordB));
}

/*
 * @tc.name: DeviceUsageStatisticsTest_BundleActiveEvent_001
 * @tc.desc: BundleActiveEvent_001
 * @tc.type: FUNC
 * @tc.require: issuesI5SOZY
 */
HWTEST_F(DeviceUsageStatisticsTest, DeviceUsageStatisticsTest_BundleActiveEvent_001,
    Function | MediumTest | TestSize.Level0)
{
    BundleActiveEvent bundleActiveEvent;
    BundleActiveEvent bundleActiveEventA;
    EXPECT_NE(bundleActiveEvent.ToString(), " ");
    bundleActiveEvent = bundleActiveEventA;
}

/*
 * @tc.name: DeviceUsageStatisticsTest_PackageStats_001
 * @tc.desc: BundleActivePackageStats_001
 * @tc.type: FUNC
 * @tc.require: issuesI5SOZY
 */
HWTEST_F(DeviceUsageStatisticsTest, DeviceUsageStatisticsTest_PackageStats_001, Function | MediumTest | TestSize.Level0)
{
    auto bundleActivePackageStats = std::make_shared<BundleActivePackageStats>();
    bundleActivePackageStats->IncrementBundleLaunchedCount();
    EXPECT_NE(bundleActivePackageStats->ToString(), " ");

    MessageParcel data;
    EXPECT_TRUE(bundleActivePackageStats->Marshalling(data));
    EXPECT_TRUE(bundleActivePackageStats->Unmarshalling(data) != nullptr);
}

/*
 * @tc.name: DeviceUsageStatisticsTest_ModuleRecord_001
 * @tc.desc: BundleActiveModuleRecord_001
 * @tc.type: FUNC
 * @tc.require: issuesI5SOZY
 */
HWTEST_F(DeviceUsageStatisticsTest, DeviceUsageStatisticsTest_ModuleRecord_001, Function | MediumTest | TestSize.Level0)
{
    auto bundleActiveModuleRecord = std::make_shared<BundleActiveModuleRecord>();
    EXPECT_NE(bundleActiveModuleRecord->ToString(), " ");

    MessageParcel data;
    EXPECT_TRUE(bundleActiveModuleRecord->Marshalling(data));
    EXPECT_TRUE(bundleActiveModuleRecord->Unmarshalling(data) != nullptr);

    BundleActiveModuleRecord bundleActiveModuleRecordA;
    bundleActiveModuleRecordA.lastModuleUsedTime_ = 2;
    BundleActiveModuleRecord bundleActiveModuleRecordB;
    bundleActiveModuleRecordB.lastModuleUsedTime_ = 1;
    EXPECT_TRUE(bundleActiveModuleRecord->cmp(bundleActiveModuleRecordA, bundleActiveModuleRecordB));

    BundleActiveModuleRecord bundleActiveModuleRecordC;
    bundleActiveModuleRecordC.lastModuleUsedTime_ = 2;
    bundleActiveModuleRecord->UpdateModuleRecord(1000);
    SUCCEED();
}

/*
 * @tc.name: DeviceUsageStatisticsTest_AppGroupCallbackProxy_001
 * @tc.desc: AppGroupCallbackProxy_001
 * @tc.type: FUNC
 * @tc.require: issuesI5SOZY
 */
HWTEST_F(DeviceUsageStatisticsTest, DeviceUsageStatisticsTest_AppGroupCallbackProxy_001,
    Function | MediumTest | TestSize.Level0)
{
    int32_t oldGroup = 60;
    int32_t newGroup = 10;
    uint32_t changeReason = 1;
    AppGroupCallbackInfo appGroupCallbackInfo(g_commonUserid, oldGroup, newGroup, changeReason, g_defaultBundleName);

    auto appGroupCallbackProxy = std::make_shared<AppGroupCallbackProxy>(nullptr);
    appGroupCallbackProxy->OnAppGroupChanged(appGroupCallbackInfo);
    EXPECT_NE(appGroupCallbackProxy, nullptr);
}

/*
 * @tc.name: DeviceUsageStatisticsTest_AppGroupCallbackStub_001
 * @tc.desc: AppGroupCallbackStub_001
 * @tc.type: FUNC
 * @tc.require: issuesI5SOZY
 */
HWTEST_F(DeviceUsageStatisticsTest, DeviceUsageStatisticsTest_AppGroupCallbackStub_001,
    Function | MediumTest | TestSize.Level0)
{
    int32_t oldGroup = 60;
    int32_t newGroup = 10;
    uint32_t changeReason = 1;
    AppGroupCallbackInfo appGroupCallbackInfo(g_commonUserid, oldGroup, newGroup, changeReason, g_defaultBundleName);

    auto appGroupCallbackStub = std::make_shared<TestAppGroupChangeCallback>();
    appGroupCallbackStub->OnAppGroupChanged(appGroupCallbackInfo);
    MessageParcel data1;
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(appGroupCallbackStub->OnRemoteRequest(1, data1, reply, option), 1);
    data1.WriteInterfaceToken(AppGroupCallbackStub::GetDescriptor());
    EXPECT_EQ(appGroupCallbackStub->OnRemoteRequest(1, data1, reply, option), 5);
}

/*
 * @tc.name: DeviceUsageStatisticsTest_QueryHighFrequencyPeriodBundle_001
 * @tc.desc: QueryHighFrequencyPeriodBundle
 * @tc.type: FUNC
 * @tc.require: SR20250319441801 AR20250322520501
 */
HWTEST_F(DeviceUsageStatisticsTest, DeviceUsageStatisticsTest_QueryHighFrequencyPeriodBundle_001,
    Function | MediumTest | TestSize.Level0)
{
    std::vector<BundleActiveHighFrequencyPeriod> appFreqHours;
    ErrCode ret = BundleActiveClient::GetInstance().QueryHighFrequencyPeriodBundle(
        appFreqHours, g_defaultFormId);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(appFreqHours.size(), 0);
    ret = BundleActiveClient::GetInstance().QueryHighFrequencyPeriodBundle(
        appFreqHours, g_commonUserid);
    EXPECT_EQ(ret, 0);
}

/*
 * @tc.name: DeviceUsageStatisticsTest__QueryBundleTodayLatestUsedTime_001
 * @tc.desc: _QueryBundleTodayLatestUsedTime
 * @tc.type: FUNC
 * @tc.require: SR20250319441801 AR20250322520501
 */
HWTEST_F(DeviceUsageStatisticsTest, DeviceUsageStatisticsTest_QueryBundleTodayLatestUsedTime_001,
    Function | MediumTest | TestSize.Level0)
{
    int64_t latestUsedTime = 0;
    ErrCode ret = BundleActiveClient::GetInstance().QueryBundleTodayLatestUsedTime(
        latestUsedTime, g_defaultFormName, g_commonUserid);
    EXPECT_NE(ret, 0);
    EXPECT_EQ(latestUsedTime, 0);
}

/*
 * @tc.name: DeviceUsageStatisticsTest__BundleActiveHighFrequencyPeriod_001
 * @tc.desc: BundleActiveHighFrequencyPeriod
 * @tc.type: FUNC
 * @tc.require: SR20250319441801 AR20250322520501
 */
HWTEST_F(DeviceUsageStatisticsTest, DeviceUsageStatisticsTest_BundleActiveHighFrequencyPeriod_001,
    Function | MediumTest | TestSize.Level0)
{
    BundleActiveHighFrequencyPeriod originalBundle;
    std::vector<int32_t> highFreqHour;
    highFreqHour.push_back(1);
    originalBundle.bundleName_ = g_defaultFormName;
    originalBundle.highFreqHours_ = highFreqHour;
    EXPECT_EQ(originalBundle.bundleName_, g_defaultFormName);

    BundleActiveHighFrequencyPeriod copy(originalBundle);
    EXPECT_EQ(copy.bundleName_, originalBundle.bundleName_);

    BundleActiveHighFrequencyPeriod withPara(g_defaultFormName, highFreqHour);
    EXPECT_EQ(withPara.bundleName_, g_defaultFormName);
}

/*
 * @tc.name: DeviceUsageStatisticsTest__BundleActiveHighFrequencyPeriod_002
 * @tc.desc: BundleActiveHighFrequencyPeriod
 * @tc.type: FUNC
 * @tc.require: SR20250319441801 AR20250322520501
 */
HWTEST_F(DeviceUsageStatisticsTest, DeviceUsageStatisticsTest_BundleActiveHighFrequencyPeriod_002,
    Function | MediumTest | TestSize.Level0)
{
    BundleActiveHighFrequencyPeriod testShall;
    MessageParcel parcel1;
    EXPECT_TRUE(testShall.Marshalling(parcel1));
    EXPECT_NE(testShall.Unmarshalling(parcel1), nullptr);

    MessageParcel parcel2;
    testShall.bundleName_ = g_defaultFormName;
    testShall.highFreqHours_.push_back(1);
    EXPECT_TRUE(testShall.Marshalling(parcel2));
    EXPECT_NE(testShall.Unmarshalling(parcel2), nullptr);

    MessageParcel parcel3;
    parcel3.WriteString(g_defaultFormName);
    parcel3.WriteUint32(3);
    parcel3.WriteInt32(1);
    parcel3.WriteInt32(2);
    EXPECT_EQ(testShall.Unmarshalling(parcel3), nullptr);

    MessageParcel parcel4;
    parcel4.WriteString(g_defaultFormName);
    parcel4.WriteUint32(2);
    parcel4.WriteInt32(1);
    parcel4.WriteInt32(2);
    EXPECT_NE(testShall.Unmarshalling(parcel4), nullptr);

    MessageParcel parcel5;
    parcel5.WriteString(g_defaultFormName);
    parcel5.WriteUint32(25);
    parcel5.WriteInt32(1);
    parcel5.WriteInt32(2);
    EXPECT_EQ(testShall.Unmarshalling(parcel5), nullptr);

    MessageParcel parcel6;
    testShall.highFreqHours_.resize(25);
    EXPECT_FALSE(testShall.Marshalling(parcel6));
}

/*
 * @tc.name: DeviceUsageStatisticsTest__BundleActiveHighFrequencyPeriod_003
 * @tc.desc: BundleActiveHighFrequencyPeriod
 * @tc.type: FUNC
 * @tc.require: SR20250319441801 AR20250322520501
 */
HWTEST_F(DeviceUsageStatisticsTest, DeviceUsageStatisticsTest_BundleActiveHighFrequencyPeriod_003,
    Function | MediumTest | TestSize.Level0)
{
    BundleActiveHighFrequencyPeriod originalBundle;
    EXPECT_EQ(originalBundle.ToString(), "bundle name is , highFreqHours_ is \n");

    originalBundle.bundleName_ = "test";
    originalBundle.highFreqHours_.push_back(1);
    EXPECT_EQ(originalBundle.ToString(), "bundle name is test, highFreqHours_ is 1\n");

    originalBundle.highFreqHours_.push_back(2);
    EXPECT_EQ(originalBundle.ToString(), "bundle name is test, highFreqHours_ is 1, 2\n");
}
}  // namespace DeviceUsageStats
}  // namespace OHOS

