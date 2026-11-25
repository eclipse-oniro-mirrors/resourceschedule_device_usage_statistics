/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "bundle_active_usage_database.h"
#include "bundle_active_util.h"
#include "bundle_active_constant.h"

using namespace testing::ext;

namespace OHOS {
namespace DeviceUsageStats {
class BundleActiveUsageDatabaseTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void BundleActiveUsageDatabaseTest::SetUpTestCase(void)
{
}

void BundleActiveUsageDatabaseTest::TearDownTestCase(void)
{
}

void BundleActiveUsageDatabaseTest::SetUp(void)
{
}

void BundleActiveUsageDatabaseTest::TearDown(void)
{
}

/*
 * @tc.name: BundleActiveUsageDatabaseTest_OnPackageUninstalled_001
 * @tc.desc: OnPackageUninstalled
 * @tc.type: FUNC
 * @tc.require: issuesI5SOZY
 */
HWTEST_F(BundleActiveUsageDatabaseTest, BundleActiveUsageDatabaseTest_OnPackageUninstalled_001,
    Function | MediumTest | TestSize.Level0)
{
    auto database = std::make_shared<BundleActiveUsageDatabase>();
    int32_t userId = 0;
    std::string bundleName = "defaultBundleName";
    database->sortedTableArray_ = {{1}};
    database->eventTableName_ = "defaultBundleName";
    database->bundleHistoryTableName_ = "defaultBundleName";
    database->moduleRecordsTableName_ = "defaultBundleName";
    database->formRecordsTableName_ = "defaultBundleName";

    database->OnPackageUninstalled(userId, bundleName, 0, 0);
    EXPECT_NE(database, nullptr);
}

/*
 * @tc.name: BundleActiveUsageDatabaseTest_GetOptimalIntervalType_001
 * @tc.desc: GetOptimalIntervalType
 * @tc.type: FUNC
 * @tc.require: issuesI5SOZY
 */
HWTEST_F(BundleActiveUsageDatabaseTest, BundleActiveUsageDatabaseTest_GetOptimalIntervalType_001,
    Function | MediumTest | TestSize.Level0)
{
    auto database = std::make_shared<BundleActiveUsageDatabase>();
    int64_t beginTime = 0;
    int64_t endTime = 2000000000000000;
    database->sortedTableArray_ = {{0}, {1}};
    database->GetOptimalIntervalType(beginTime, endTime);
    EXPECT_NE(database, nullptr);
}

/*
 * @tc.name: BundleActiveUsageDatabaseTest_NearIndexOnOrBeforeCurrentTime_001
 * @tc.desc: NearIndexOnOrBeforeCurrentTime
 * @tc.type: FUNC
 * @tc.require: issuesI5SOZY
 */
HWTEST_F(BundleActiveUsageDatabaseTest, BundleActiveUsageDatabaseTest_NearIndexOnOrBeforeCurrentTime_001,
    Function | MediumTest | TestSize.Level0)
{
    auto database = std::make_shared<BundleActiveUsageDatabase>();
    int64_t currentTime = 0;
    vector<int64_t> sortedTableArray = {0};
    database->NearIndexOnOrBeforeCurrentTime(currentTime, sortedTableArray);
    EXPECT_NE(database, nullptr);
}

/*
 * @tc.name: BundleActiveUsageDatabaseTest_CheckDatabaseVersion_001
 * @tc.desc: CheckDatabaseVersion
 * @tc.type: FUNC
 * @tc.require: issuesI9Q9ZJ
 */
HWTEST_F(BundleActiveUsageDatabaseTest, BundleActiveUsageDatabaseTest_CheckDatabaseVersion_001,
    Function | MediumTest | TestSize.Level0)
{
    auto database = std::make_shared<BundleActiveUsageDatabase>();
    database->CheckDatabaseVersion();
    EXPECT_NE(database, nullptr);
}

/*
 * @tc.name: BundleActiveUsageDatabaseTest_GetOldDbVersion_001
 * @tc.desc: GetOldDbVersion
 * @tc.type: FUNC
 * @tc.require: issuesI9Q9ZJ
 */
HWTEST_F(BundleActiveUsageDatabaseTest, BundleActiveUsageDatabaseTest_GetOldDbVersion_001,
    Function | MediumTest | TestSize.Level0)
{
    auto database = std::make_shared<BundleActiveUsageDatabase>();
    database->GetOldDbVersion();
    EXPECT_NE(database, nullptr);
}

/*
 * @tc.name: BundleActiveUsageDatabaseTest_GetVersionByFileInput_001
 * @tc.desc: GetVersionByFileInput
 * @tc.type: FUNC
 * @tc.require: issuesI9Q9ZJ
 */
HWTEST_F(BundleActiveUsageDatabaseTest, BundleActiveUsageDatabaseTest_GetVersionByFileInput_001,
    Function | MediumTest | TestSize.Level0)
{
    auto database = std::make_shared<BundleActiveUsageDatabase>();
    std::string fileInput = "";
    database->GetVersionByFileInput(fileInput);
    fileInput = "aa123";
    int32_t result = database->GetVersionByFileInput(fileInput);
    EXPECT_EQ(result, 123);
}

/*
 * @tc.name: BundleActiveUsageDatabaseTest_UpgradeDatabase_001
 * @tc.desc: UpgradeDatabase
 * @tc.type: FUNC
 * @tc.require: issuesI9Q9ZJ
 */
HWTEST_F(BundleActiveUsageDatabaseTest, BundleActiveUsageDatabaseTest_UpgradeDatabase_001,
    Function | MediumTest | TestSize.Level0)
{
    auto database = std::make_shared<BundleActiveUsageDatabase>();
    database->currentVersion_ = 0;
    int32_t oldVersion = 1;
    int32_t curVersion = 2;
    database->UpgradeDatabase(oldVersion, curVersion);

    oldVersion = 2;
    curVersion = 3;
    database->UpgradeDatabase(oldVersion, curVersion);
    EXPECT_EQ(database->currentVersion_, curVersion);
}

/*
 * @tc.name: BundleActiveUsageDatabaseTest_UpgradeDatabase_001
 * @tc.desc: UpgradeDatabase
 * @tc.type: FUNC
 * @tc.require: issuesI9Q9ZJ
 */
HWTEST_F(BundleActiveUsageDatabaseTest, BundleActiveUsageDatabaseTest_UpgradeDatabase_002,
    Function | MediumTest | TestSize.Level0)
{
    auto database = std::make_shared<BundleActiveUsageDatabase>();
    database->currentVersion_ = 0;
    int32_t oldVersion = 1;
    int32_t curVersion = 3;
    database->UpgradeDatabase(oldVersion, curVersion);
    EXPECT_EQ(database->currentVersion_, curVersion);
    database->currentVersion_ = 0;
    oldVersion = 2;
    curVersion = 3;
    database->UpgradeDatabase(oldVersion, curVersion);
    EXPECT_EQ(database->currentVersion_, curVersion);
    oldVersion = 3;
    database->UpgradeDatabase(oldVersion, curVersion);
    EXPECT_EQ(database->currentVersion_, curVersion);
}

/*
 * @tc.name: BundleActiveUsageDatabaseTest_AddRdbColumn_001
 * @tc.desc: AddRdbColumn
 * @tc.type: FUNC
 * @tc.require: issuesI9Q9ZJ
 */
HWTEST_F(BundleActiveUsageDatabaseTest, BundleActiveUsageDatabaseTest_AddRdbColumn_001,
    Function | MediumTest | TestSize.Level0)
{
    auto database = std::make_shared<BundleActiveUsageDatabase>();
    std::shared_ptr<NativeRdb::RdbStore> store = database->GetBundleActiveRdbStore(0);
    database->AddRdbColumn(store, "test", "testColumn", "INTERGER");
    EXPECT_NE(database, nullptr);
}

/*
 * @tc.name: BundleActiveUsageDatabaseTest_UpdateOldDataUid_001
 * @tc.desc: UpdateOldDataUid
 * @tc.type: FUNC
 * @tc.require: issuesI9Q9ZJ
 */
HWTEST_F(BundleActiveUsageDatabaseTest, BundleActiveUsageDatabaseTest_UpdateOldDataUid_001,
    Function | MediumTest | TestSize.Level0)
{
    std::map<std::string, int32_t> bundleNameUidMap;
    bundleNameUidMap["111"] = 111;
    bundleNameUidMap["222"] = 222;
    auto database = std::make_shared<BundleActiveUsageDatabase>();
    std::shared_ptr<NativeRdb::RdbStore> store = database->GetBundleActiveRdbStore(0);
    database->UpdateOldDataUid(store, "test", 100, bundleNameUidMap);
    EXPECT_NE(database, nullptr);
}

/*
 * @tc.name: BundleActiveUsageDatabaseTest_CreateRecordTable_001
 * @tc.desc: CreateRecordTable
 * @tc.type: FUNC
 * @tc.require: issuesI9Q9ZJ
 */
HWTEST_F(BundleActiveUsageDatabaseTest, BundleActiveUsageDatabaseTest_CreateRecordTable_001,
    Function | MediumTest | TestSize.Level0)
{
    int64_t timeStamp = 10000;
    auto database = std::make_shared<BundleActiveUsageDatabase>();
    database->moduleRecordsTableName_ = "unknownTableName";
    database->formRecordsTableName_ = "unknownTableName";
    database->CreateRecordTable(timeStamp);;
    EXPECT_NE(database, nullptr);
}

/*
 * @tc.name: BundleActiveUsageDatabaseTest_GetQuerySqlCommand_001
 * @tc.desc: GetQuerySqlCommand
 * @tc.type: FUNC
 * @tc.require: issuesI9Q9ZJ
 */
 HWTEST_F(BundleActiveUsageDatabaseTest, BundleActiveUsageDatabaseTest_GetQuerySqlCommand_001,
    Function | MediumTest | TestSize.Level0)
{
    std::vector<std::string> queryCondition;
    std::string queryPackageSql;
    auto database = std::make_shared<BundleActiveUsageDatabase>();
    database->sortedTableArray_ = {{0}, {1}, {2}};
    database->GetQuerySqlCommand(0, 1, 2, 0, 0, 1, 10, queryCondition, queryPackageSql);
    EXPECT_TRUE(!queryPackageSql.empty());
}

/*
 * @tc.name: BundleActiveUtil_001
 * @tc.desc: CreateRecordTable
 * @tc.type: FUNC
 * @tc.require: issuesI9Q9ZJ
 */
 HWTEST_F(BundleActiveUsageDatabaseTest, BundleActiveUtil_001,
    Function | MediumTest | TestSize.Level0)
{
    int32_t result = BundleActiveUtil::StringToInt32("11");
    EXPECT_EQ(result, 11);
    int64_t resultInt64 = BundleActiveUtil::StringToInt64("11");
    EXPECT_EQ(resultInt64, 11);
}

/*
 * @tc.name: BundleActiveUsageDatabaseTest_SupportFirstUseTime_001
 * @tc.desc: SupportFirstUseTime
 * @tc.type: FUNC
 * @tc.require: issuesICCZ27
 */
HWTEST_F(BundleActiveUsageDatabaseTest, SupportFirstUseTime_001,
    Function | MediumTest | TestSize.Level0)
{
    auto database = std::make_shared<BundleActiveUsageDatabase>();

    database->SupportFirstUseTime();
    EXPECT_NE(database, nullptr);
}

/*
 * @tc.name: BundleActiveUsageDatabaseTest_UpdateFirstUseTime_002
 * @tc.desc: UpdateFirstUseTime
 * @tc.type: FUNC
 * @tc.require: issuesICCZ27
 */
HWTEST_F(BundleActiveUsageDatabaseTest, UpdateFirstUseTime_002,
    Function | MediumTest | TestSize.Level0)
{
    auto database = std::make_shared<BundleActiveUsageDatabase>();
    auto rdb = database->GetBundleActiveRdbStore(APP_GROUP_DATABASE_INDEX);
    database->UpdateFirstUseTime(rdb, BUNDLE_HISTORY_LOG_TABLE, 100);
    EXPECT_NE(database, nullptr);
}

/*
 * @tc.name: BundleActiveUsageDatabaseTest_DeleteExcessiveEventTableData_001
 * @tc.desc: DeleteExcessiveEventTableData
 * @tc.type: FUNC
 * @tc.require: issuesIC2FBU
 */
HWTEST_F(BundleActiveUsageDatabaseTest, BundleActiveUsageDatabaseTest_DeleteExcessiveEventTableData_001,
    Function | MediumTest | TestSize.Level0)
{
    auto database = std::make_shared<BundleActiveUsageDatabase>();
    database->DeleteExcessiveEventTableData(0);
    database->eventTableName_ = "a123";
    database->DeleteExcessiveEventTableData(0);
    database->eventBeginTime_  = 0;
    database->DeleteExcessiveEventTableData(0);
    database->eventBeginTime_  = MAX_END_TIME;
    database->DeleteExcessiveEventTableData(0);
    database->InitDatabaseTableInfo(100);
    database->DeleteExcessiveEventTableData(0);
    EXPECT_EQ(database->eventTableName_, "DeviceEventLog100");
}

/*
 * @tc.name: GetFolderOrFileSize_001
 * @tc.desc: BundleActiveUtil Test
 * @tc.type: FUNC
 * @tc.require: issuesIC2FBU
 */
HWTEST_F(BundleActiveUsageDatabaseTest, GetFolderOrFileSize_001,
    Function | MediumTest | TestSize.Level0)
{
    EXPECT_EQ(BundleActiveUtil::GetFolderOrFileSize(""), 0);
    EXPECT_TRUE(BundleActiveUtil::GetFolderOrFileSize(BUNDLE_ACTIVE_DATABASE_DIR) > 0);
}

/*
 * @tc.name: GetPartitionName_001
 * @tc.desc: BundleActiveUtil Test
 * @tc.type: FUNC
 * @tc.require: issuesIC2FBU
 */
HWTEST_F(BundleActiveUsageDatabaseTest, GetPartitionName_001,
    Function | MediumTest | TestSize.Level0)
{
    EXPECT_EQ(BundleActiveUtil::GetPartitionName(""), "/");
    EXPECT_EQ(BundleActiveUtil::GetPartitionName(BUNDLE_ACTIVE_DATABASE_DIR), "/data");
    EXPECT_EQ(BundleActiveUtil::GetPartitionName("/data"), "/data");
}

/*
 * @tc.name: GetDeviceValidSize_001
 * @tc.desc: BundleActiveUtil Test
 * @tc.type: FUNC
 * @tc.require: issuesIC2FBU
 */
HWTEST_F(BundleActiveUsageDatabaseTest, GetDeviceValidSize_001,
    Function | MediumTest | TestSize.Level0)
{
    EXPECT_TRUE(BundleActiveUtil::GetDeviceValidSize("") >= 0);
    EXPECT_TRUE(BundleActiveUtil::GetDeviceValidSize(BUNDLE_ACTIVE_DATABASE_DIR) >= 0);
}

/*
 * @tc.name: GetPercentOfAvailableUserSpace_001
 * @tc.desc: BundleActiveUtil Test
 * @tc.type: FUNC
 * @tc.require: issuesIC2FBU
 */
HWTEST_F(BundleActiveUsageDatabaseTest, GetPercentOfAvailableUserSpace_001,
    Function | MediumTest | TestSize.Level0)
{
    EXPECT_TRUE(BundleActiveUtil::GetPercentOfAvailableUserSpace("") >= 0);
    EXPECT_TRUE(BundleActiveUtil::GetPercentOfAvailableUserSpace(BUNDLE_ACTIVE_DATABASE_DIR) >= 0);
}

namespace {
const int64_t ONE_HOUR_MILLIS = 3600 * 1000LL;
const int32_t TEST_EVENT_ROWS_PER_DAY = 2;

std::string MakeEventTableName(const int64_t tableTime)
{
    return EVENT_LOG_TABLE + std::to_string(tableTime);
}

void DropEventTable(const std::shared_ptr<NativeRdb::RdbStore>& rdbStore, const int64_t tableTime)
{
    if (rdbStore == nullptr) {
        return;
    }
    rdbStore->ExecuteSql("DROP TABLE IF EXISTS " + MakeEventTableName(tableTime));
    rdbStore->ExecuteSql("DROP INDEX IF EXISTS " + EVENT_LOG_TABLE_INDEX_PREFIX + std::to_string(tableTime));
}

void InsertEventRows(const std::shared_ptr<NativeRdb::RdbStore>& rdbStore, const std::string& tableName,
    const int64_t dayOffset, const int32_t rowCount)
{
    for (int32_t i = 0; i < rowCount; ++i) {
        const int64_t timeStamp = dayOffset * ONE_DAY_TIME + ONE_HOUR_MILLIS + i;
        const std::string insertSql = "insert into " + tableName + " (userId, bundleName, eventId, timeStamp, "
            "abilityId, uid) values (100, 'com.ohos.tdd', 1, " + std::to_string(timeStamp) +
            ", 'TddAbility', 200)";
        EXPECT_EQ(rdbStore->ExecuteSql(insertSql), NativeRdb::E_OK);
    }
}

int64_t CountEventRows(const std::shared_ptr<NativeRdb::RdbStore>& rdbStore, const std::string& tableName,
    const std::string& condition = "")
{
    std::string querySql = "select count(*) as cnt from " + tableName;
    if (!condition.empty()) {
        querySql += " where " + condition;
    }
    std::shared_ptr<NativeRdb::ResultSet> resultSet = rdbStore->QueryByStep(querySql);
    if (resultSet == nullptr) {
        return -1;
    }
    int32_t columnIndex = -1;
    if (resultSet->GetColumnIndex("cnt", columnIndex) != NativeRdb::E_OK) {
        resultSet->Close();
        return -1;
    }
    int64_t rowCount = 0;
    if (resultSet->GoToNextRow() == NativeRdb::E_OK) {
        resultSet->GetLong(columnIndex, rowCount);
    }
    resultSet->Close();
    return rowCount;
}
}

/*
 * @tc.name: BundleActiveUsageDatabaseTest_GetDistinctEventDays_001
 * @tc.desc: GetDistinctEventDays returns empty when event table name is unknown
 * @tc.type: FUNC
 * @tc.require: issuesIC2FBU
 */
HWTEST_F(BundleActiveUsageDatabaseTest, BundleActiveUsageDatabaseTest_GetDistinctEventDays_001,
    Function | MediumTest | TestSize.Level0)
{
    auto database = std::make_shared<BundleActiveUsageDatabase>();
    std::vector<int64_t> distinctDays = database->GetDistinctEventDays();
    EXPECT_TRUE(distinctDays.empty());
}

/*
 * @tc.name: BundleActiveUsageDatabaseTest_GetDistinctEventDays_002
 * @tc.desc: GetDistinctEventDays returns empty when event table name cannot be parsed
 * @tc.type: FUNC
 * @tc.require: issuesIC2FBU
 */
HWTEST_F(BundleActiveUsageDatabaseTest, BundleActiveUsageDatabaseTest_GetDistinctEventDays_002,
    Function | MediumTest | TestSize.Level0)
{
    auto database = std::make_shared<BundleActiveUsageDatabase>();
    database->eventTableName_ = "";
    EXPECT_TRUE(database->GetDistinctEventDays().empty());
}

/*
 * @tc.name: BundleActiveUsageDatabaseTest_GetDistinctEventDays_003
 * @tc.desc: GetDistinctEventDays returns empty for a missing table or a table without data
 * @tc.type: FUNC
 * @tc.require: issuesIC2FBU
 */
HWTEST_F(BundleActiveUsageDatabaseTest, BundleActiveUsageDatabaseTest_GetDistinctEventDays_003,
    Function | MediumTest | TestSize.Level0)
{
    const int64_t tableTime = 910001;
    auto database = std::make_shared<BundleActiveUsageDatabase>();
    std::shared_ptr<NativeRdb::RdbStore> rdbStore = database->GetBundleActiveRdbStore(EVENT_DATABASE_INDEX);
    ASSERT_NE(rdbStore, nullptr);
    DropEventTable(rdbStore, tableTime);
    database->eventTableName_ = MakeEventTableName(tableTime + 100);
    EXPECT_TRUE(database->GetDistinctEventDays().empty());
    EXPECT_EQ(database->CreateEventLogTable(EVENT_DATABASE_INDEX, tableTime), BUNDLE_ACTIVE_SUCCESS);
    EXPECT_EQ(CountEventRows(rdbStore, database->eventTableName_, ""), 0);
    EXPECT_TRUE(database->GetDistinctEventDays().empty());
    DropEventTable(rdbStore, tableTime);
}

/*
 * @tc.name: BundleActiveUsageDatabaseTest_GetDistinctEventDays_004
 * @tc.desc: GetDistinctEventDays returns distinct day buckets in descending order
 * @tc.type: FUNC
 * @tc.require: issuesIC2FBU
 */
HWTEST_F(BundleActiveUsageDatabaseTest, BundleActiveUsageDatabaseTest_GetDistinctEventDays_004,
    Function | MediumTest | TestSize.Level0)
{
    const int64_t tableTime = 910002;
    auto database = std::make_shared<BundleActiveUsageDatabase>();
    std::shared_ptr<NativeRdb::RdbStore> rdbStore = database->GetBundleActiveRdbStore(EVENT_DATABASE_INDEX);
    ASSERT_NE(rdbStore, nullptr);
    DropEventTable(rdbStore, tableTime);
    EXPECT_EQ(database->CreateEventLogTable(EVENT_DATABASE_INDEX, tableTime), BUNDLE_ACTIVE_SUCCESS);
    EXPECT_EQ(database->eventTableName_, MakeEventTableName(tableTime));
    for (int64_t day = 0; day <= NUM_DAY_ONE_WEEK; ++day) {
        InsertEventRows(rdbStore, database->eventTableName_, day, TEST_EVENT_ROWS_PER_DAY);
    }
    InsertEventRows(rdbStore, database->eventTableName_, 5, TEST_EVENT_ROWS_PER_DAY);
    EXPECT_EQ(CountEventRows(rdbStore, database->eventTableName_, ""),
        (NUM_DAY_ONE_WEEK + 1) * TEST_EVENT_ROWS_PER_DAY + TEST_EVENT_ROWS_PER_DAY);
    std::vector<int64_t> distinctDays = database->GetDistinctEventDays();
    ASSERT_EQ(static_cast<int32_t>(distinctDays.size()), NUM_DAY_ONE_WEEK + 1);
    EXPECT_EQ(distinctDays.at(0), NUM_DAY_ONE_WEEK);
    EXPECT_EQ(distinctDays.at(NUM_DAY_ONE_WEEK), 0);
    for (size_t i = 1; i < distinctDays.size(); ++i) {
        EXPECT_GT(distinctDays.at(i - 1), distinctDays.at(i));
    }
    DropEventTable(rdbStore, tableTime);
}

/*
 * @tc.name: BundleActiveUsageDatabaseTest_CalcEventDeleteTimePoint_001
 * @tc.desc: CalcEventDeleteTimePoint returns 0 when keepDays is not positive
 * @tc.type: FUNC
 * @tc.require: issuesIC2FBU
 */
HWTEST_F(BundleActiveUsageDatabaseTest, BundleActiveUsageDatabaseTest_CalcEventDeleteTimePoint_001,
    Function | MediumTest | TestSize.Level0)
{
    auto database = std::make_shared<BundleActiveUsageDatabase>();
    EXPECT_EQ(database->CalcEventDeleteTimePoint(0, 1000), 0);
    EXPECT_EQ(database->CalcEventDeleteTimePoint(-1, 1000), 0);
}

/*
 * @tc.name: BundleActiveUsageDatabaseTest_CalcEventDeleteTimePoint_002
 * @tc.desc: CalcEventDeleteTimePoint returns 0 when eventTableTime is not positive
 * @tc.type: FUNC
 * @tc.require: issuesIC2FBU
 */
HWTEST_F(BundleActiveUsageDatabaseTest, BundleActiveUsageDatabaseTest_CalcEventDeleteTimePoint_002,
    Function | MediumTest | TestSize.Level0)
{
    auto database = std::make_shared<BundleActiveUsageDatabase>();
    EXPECT_EQ(database->CalcEventDeleteTimePoint(NUM_DAY_ONE_WEEK, 0), 0);
    EXPECT_EQ(database->CalcEventDeleteTimePoint(NUM_DAY_ONE_WEEK, -1), 0);
}

/*
 * @tc.name: BundleActiveUsageDatabaseTest_CalcEventDeleteTimePoint_003
 * @tc.desc: CalcEventDeleteTimePoint returns 0 when distinct days are fewer than keepDays
 * @tc.type: FUNC
 * @tc.require: issuesIC2FBU
 */
HWTEST_F(BundleActiveUsageDatabaseTest, BundleActiveUsageDatabaseTest_CalcEventDeleteTimePoint_003,
    Function | MediumTest | TestSize.Level0)
{
    const int64_t tableTime = 910003;
    auto database = std::make_shared<BundleActiveUsageDatabase>();
    std::shared_ptr<NativeRdb::RdbStore> rdbStore = database->GetBundleActiveRdbStore(EVENT_DATABASE_INDEX);
    ASSERT_NE(rdbStore, nullptr);
    DropEventTable(rdbStore, tableTime);
    EXPECT_EQ(database->CreateEventLogTable(EVENT_DATABASE_INDEX, tableTime), BUNDLE_ACTIVE_SUCCESS);
    for (int64_t day = 0; day < 3; ++day) {
        InsertEventRows(rdbStore, database->eventTableName_, day, TEST_EVENT_ROWS_PER_DAY);
    }
    EXPECT_EQ(CountEventRows(rdbStore, database->eventTableName_, ""), 3 * TEST_EVENT_ROWS_PER_DAY);
    EXPECT_EQ(database->CalcEventDeleteTimePoint(NUM_DAY_ONE_WEEK, tableTime), 0);
    DropEventTable(rdbStore, tableTime);
}

/*
 * @tc.name: BundleActiveUsageDatabaseTest_CalcEventDeleteTimePoint_004
 * @tc.desc: CalcEventDeleteTimePoint returns 0 when distinct days equal keepDays
 * @tc.type: FUNC
 * @tc.require: issuesIC2FBU
 */
HWTEST_F(BundleActiveUsageDatabaseTest, BundleActiveUsageDatabaseTest_CalcEventDeleteTimePoint_004,
    Function | MediumTest | TestSize.Level0)
{
    const int64_t tableTime = 910004;
    auto database = std::make_shared<BundleActiveUsageDatabase>();
    std::shared_ptr<NativeRdb::RdbStore> rdbStore = database->GetBundleActiveRdbStore(EVENT_DATABASE_INDEX);
    ASSERT_NE(rdbStore, nullptr);
    DropEventTable(rdbStore, tableTime);
    EXPECT_EQ(database->CreateEventLogTable(EVENT_DATABASE_INDEX, tableTime), BUNDLE_ACTIVE_SUCCESS);
    for (int64_t day = 0; day < NUM_DAY_ONE_WEEK; ++day) {
        InsertEventRows(rdbStore, database->eventTableName_, day, TEST_EVENT_ROWS_PER_DAY);
    }
    EXPECT_EQ(CountEventRows(rdbStore, database->eventTableName_, ""),
        NUM_DAY_ONE_WEEK * TEST_EVENT_ROWS_PER_DAY);
    EXPECT_EQ(database->CalcEventDeleteTimePoint(NUM_DAY_ONE_WEEK, tableTime), 0);
    DropEventTable(rdbStore, tableTime);
}

/*
 * @tc.name: BundleActiveUsageDatabaseTest_CalcEventDeleteTimePoint_005
 * @tc.desc: CalcEventDeleteTimePoint keeps the newest 7 days when 8 distinct days exist
 * @tc.type: FUNC
 * @tc.require: issuesIC2FBU
 */
HWTEST_F(BundleActiveUsageDatabaseTest, BundleActiveUsageDatabaseTest_CalcEventDeleteTimePoint_005,
    Function | MediumTest | TestSize.Level0)
{
    const int64_t tableTime = 910005;
    auto database = std::make_shared<BundleActiveUsageDatabase>();
    std::shared_ptr<NativeRdb::RdbStore> rdbStore = database->GetBundleActiveRdbStore(EVENT_DATABASE_INDEX);
    ASSERT_NE(rdbStore, nullptr);
    DropEventTable(rdbStore, tableTime);
    EXPECT_EQ(database->CreateEventLogTable(EVENT_DATABASE_INDEX, tableTime), BUNDLE_ACTIVE_SUCCESS);
    for (int64_t day = 0; day <= NUM_DAY_ONE_WEEK; ++day) {
        InsertEventRows(rdbStore, database->eventTableName_, day, TEST_EVENT_ROWS_PER_DAY);
    }
    EXPECT_EQ(CountEventRows(rdbStore, database->eventTableName_, ""),
        (NUM_DAY_ONE_WEEK + 1) * TEST_EVENT_ROWS_PER_DAY);
    EXPECT_EQ(database->CalcEventDeleteTimePoint(NUM_DAY_ONE_WEEK, tableTime), ONE_DAY_TIME);
    DropEventTable(rdbStore, tableTime);
}

/*
 * @tc.name: BundleActiveUsageDatabaseTest_CalcEventDeleteTimePoint_006
 * @tc.desc: CalcEventDeleteTimePoint skips days without data when counting retention days
 * @tc.type: FUNC
 * @tc.require: issuesIC2FBU
 */
HWTEST_F(BundleActiveUsageDatabaseTest, BundleActiveUsageDatabaseTest_CalcEventDeleteTimePoint_006,
    Function | MediumTest | TestSize.Level0)
{
    const int64_t tableTime = 910006;
    const std::vector<int64_t> daysWithData = {0, 2, 3, 4, 5, 6, 7, 8};
    auto database = std::make_shared<BundleActiveUsageDatabase>();
    std::shared_ptr<NativeRdb::RdbStore> rdbStore = database->GetBundleActiveRdbStore(EVENT_DATABASE_INDEX);
    ASSERT_NE(rdbStore, nullptr);
    DropEventTable(rdbStore, tableTime);
    EXPECT_EQ(database->CreateEventLogTable(EVENT_DATABASE_INDEX, tableTime), BUNDLE_ACTIVE_SUCCESS);
    for (int64_t day : daysWithData) {
        InsertEventRows(rdbStore, database->eventTableName_, day, TEST_EVENT_ROWS_PER_DAY);
    }
    EXPECT_EQ(CountEventRows(rdbStore, database->eventTableName_, ""),
        static_cast<int64_t>(daysWithData.size()) * TEST_EVENT_ROWS_PER_DAY);
    EXPECT_EQ(database->CalcEventDeleteTimePoint(NUM_DAY_ONE_WEEK, tableTime), 2 * ONE_DAY_TIME);
    DropEventTable(rdbStore, tableTime);
}

/*
 * @tc.name: BundleActiveUsageDatabaseTest_CalcEventDeleteTimePoint_007
 * @tc.desc: CalcEventDeleteTimePoint keeps only the newest day when keepDays is 1
 * @tc.type: FUNC
 * @tc.require: issuesIC2FBU
 */
HWTEST_F(BundleActiveUsageDatabaseTest, BundleActiveUsageDatabaseTest_CalcEventDeleteTimePoint_007,
    Function | MediumTest | TestSize.Level0)
{
    const int64_t tableTime = 910007;
    auto database = std::make_shared<BundleActiveUsageDatabase>();
    std::shared_ptr<NativeRdb::RdbStore> rdbStore = database->GetBundleActiveRdbStore(EVENT_DATABASE_INDEX);
    ASSERT_NE(rdbStore, nullptr);
    DropEventTable(rdbStore, tableTime);
    EXPECT_EQ(database->CreateEventLogTable(EVENT_DATABASE_INDEX, tableTime), BUNDLE_ACTIVE_SUCCESS);
    for (int64_t day = 0; day <= NUM_DAY_ONE_WEEK; ++day) {
        InsertEventRows(rdbStore, database->eventTableName_, day, TEST_EVENT_ROWS_PER_DAY);
    }
    EXPECT_EQ(CountEventRows(rdbStore, database->eventTableName_, ""),
        (NUM_DAY_ONE_WEEK + 1) * TEST_EVENT_ROWS_PER_DAY);
    EXPECT_EQ(database->CalcEventDeleteTimePoint(1, tableTime), NUM_DAY_ONE_WEEK * ONE_DAY_TIME);
    DropEventTable(rdbStore, tableTime);
}

/*
 * @tc.name: BundleActiveUsageDatabaseTest_DeleteExcessiveTableData_001
 * @tc.desc: DeleteExcessiveTableData keeps data when eventBeginTime_ is not initialized
 * @tc.type: FUNC
 * @tc.require: issuesIC2FBU
 */
HWTEST_F(BundleActiveUsageDatabaseTest, BundleActiveUsageDatabaseTest_DeleteExcessiveTableData_001,
    Function | MediumTest | TestSize.Level0)
{
    const int64_t tableTime = 910008;
    auto database = std::make_shared<BundleActiveUsageDatabase>();
    database->DeleteExcessiveTableData(EVENT_DATABASE_INDEX);
    std::shared_ptr<NativeRdb::RdbStore> rdbStore = database->GetBundleActiveRdbStore(EVENT_DATABASE_INDEX);
    ASSERT_NE(rdbStore, nullptr);
    DropEventTable(rdbStore, tableTime);
    EXPECT_EQ(database->CreateEventLogTable(EVENT_DATABASE_INDEX, tableTime), BUNDLE_ACTIVE_SUCCESS);
    for (int64_t day = 0; day <= NUM_DAY_ONE_WEEK; ++day) {
        InsertEventRows(rdbStore, database->eventTableName_, day, TEST_EVENT_ROWS_PER_DAY);
    }
    const int64_t rowsBefore = CountEventRows(rdbStore, database->eventTableName_, "");
    EXPECT_EQ(rowsBefore, (NUM_DAY_ONE_WEEK + 1) * TEST_EVENT_ROWS_PER_DAY);
    EXPECT_EQ(database->eventBeginTime_, EVENT_BEGIN_TIME_INITIAL_VALUE);
    database->DeleteExcessiveTableData(EVENT_DATABASE_INDEX);
    EXPECT_EQ(CountEventRows(rdbStore, database->eventTableName_, ""), rowsBefore);
    DropEventTable(rdbStore, tableTime);
}

/*
 * @tc.name: BundleActiveUsageDatabaseTest_DeleteExcessiveTableData_002
 * @tc.desc: DeleteExcessiveTableData removes events older than the newest 7 days
 * @tc.type: FUNC
 * @tc.require: issuesIC2FBU
 */
HWTEST_F(BundleActiveUsageDatabaseTest, BundleActiveUsageDatabaseTest_DeleteExcessiveTableData_002,
    Function | MediumTest | TestSize.Level0)
{
    const int64_t tableTime = 910009;
    auto database = std::make_shared<BundleActiveUsageDatabase>();
    std::shared_ptr<NativeRdb::RdbStore> rdbStore = database->GetBundleActiveRdbStore(EVENT_DATABASE_INDEX);
    ASSERT_NE(rdbStore, nullptr);
    DropEventTable(rdbStore, tableTime);
    EXPECT_EQ(database->CreateEventLogTable(EVENT_DATABASE_INDEX, tableTime), BUNDLE_ACTIVE_SUCCESS);
    for (int64_t day = 0; day <= NUM_DAY_ONE_WEEK; ++day) {
        InsertEventRows(rdbStore, database->eventTableName_, day, TEST_EVENT_ROWS_PER_DAY);
    }
    EXPECT_EQ(CountEventRows(rdbStore, database->eventTableName_, ""),
        (NUM_DAY_ONE_WEEK + 1) * TEST_EVENT_ROWS_PER_DAY);
    database->eventBeginTime_ = tableTime + (NUM_DAY_ONE_WEEK + 1) * ONE_DAY_TIME;
    database->DeleteExcessiveTableData(EVENT_DATABASE_INDEX);
    EXPECT_EQ(CountEventRows(rdbStore, database->eventTableName_,
        "timeStamp < " + std::to_string(ONE_DAY_TIME)), 0);
    EXPECT_EQ(CountEventRows(rdbStore, database->eventTableName_,
        "timeStamp >= " + std::to_string(ONE_DAY_TIME)), NUM_DAY_ONE_WEEK * TEST_EVENT_ROWS_PER_DAY);
    EXPECT_EQ(CountEventRows(rdbStore, database->eventTableName_, ""),
        NUM_DAY_ONE_WEEK * TEST_EVENT_ROWS_PER_DAY);
    DropEventTable(rdbStore, tableTime);
}

/*
 * @tc.name: BundleActiveUsageDatabaseTest_DeleteExcessiveTableData_003
 * @tc.desc: DeleteExcessiveTableData skips days without data and keeps 7 days with data
 * @tc.type: FUNC
 * @tc.require: issuesIC2FBU
 */
HWTEST_F(BundleActiveUsageDatabaseTest, BundleActiveUsageDatabaseTest_DeleteExcessiveTableData_003,
    Function | MediumTest | TestSize.Level0)
{
    const int64_t tableTime = 910011;
    const std::vector<int64_t> daysWithData = {0, 2, 3, 4, 5, 6, 7, 8};
    auto database = std::make_shared<BundleActiveUsageDatabase>();
    std::shared_ptr<NativeRdb::RdbStore> rdbStore = database->GetBundleActiveRdbStore(EVENT_DATABASE_INDEX);
    ASSERT_NE(rdbStore, nullptr);
    DropEventTable(rdbStore, tableTime);
    EXPECT_EQ(database->CreateEventLogTable(EVENT_DATABASE_INDEX, tableTime), BUNDLE_ACTIVE_SUCCESS);
    for (int64_t day : daysWithData) {
        InsertEventRows(rdbStore, database->eventTableName_, day, TEST_EVENT_ROWS_PER_DAY);
    }
    EXPECT_EQ(CountEventRows(rdbStore, database->eventTableName_, ""),
        static_cast<int64_t>(daysWithData.size()) * TEST_EVENT_ROWS_PER_DAY);
    database->eventBeginTime_ = tableTime + (NUM_DAY_ONE_WEEK + 2) * ONE_DAY_TIME;
    database->DeleteExcessiveTableData(EVENT_DATABASE_INDEX);
    EXPECT_EQ(CountEventRows(rdbStore, database->eventTableName_,
        "timeStamp < " + std::to_string(ONE_DAY_TIME)), 0);
    EXPECT_EQ(CountEventRows(rdbStore, database->eventTableName_,
        "timeStamp >= " + std::to_string(2 * ONE_DAY_TIME)), NUM_DAY_ONE_WEEK * TEST_EVENT_ROWS_PER_DAY);
    EXPECT_EQ(CountEventRows(rdbStore, database->eventTableName_, ""),
        NUM_DAY_ONE_WEEK * TEST_EVENT_ROWS_PER_DAY);
    DropEventTable(rdbStore, tableTime);
}
}  // namespace DeviceUsageStats
}  // namespace OHOS
