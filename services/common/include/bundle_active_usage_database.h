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

#ifndef BUNDLE_ACTIVE_USAGE_DATABASE_H
#define BUNDLE_ACTIVE_USAGE_DATABASE_H

#ifdef DEVICE_USAGE_UNIT_TEST
#define WEAK_FUNC __attribute__((weak))
#else
#define WEAK_FUNC
#endif

#include <vector>
#include <string>
#include <mutex>

#include "rdb_errno.h"
#include "rdb_helper.h"
#include "rdb_open_callback.h"
#include "rdb_store.h"
#include "rdb_store_config.h"
#include "result_set.h"
#include "values_bucket.h"
#include "ffrt.h"

#include "bundle_active_period_stats.h"
#include "bundle_active_calendar.h"
#include "bundle_active_package_history.h"
#include "bundle_active_module_record.h"

namespace OHOS {
namespace DeviceUsageStats {
class BundleActiveUsageDatabase {
public:
    BundleActiveUsageDatabase();
    ~BundleActiveUsageDatabase();
    void InitDatabaseTableInfo(int64_t currentTime);
    void InitUsageGroupDatabase(const int32_t databaseType, const bool forModuleRecords);
    void UpdateBundleUsageData(int32_t databaseType, BundleActivePeriodStats &stats);
    void UpdateEventData(int32_t databaseType, BundleActivePeriodStats &stats);
    std::shared_ptr<BundleActivePeriodStats> GetCurrentUsageData(int32_t databaseType, int32_t userId);
    void RenewTableTime(int64_t timeDiffMillis);
    int32_t GetOptimalIntervalType(int64_t beginTime, int64_t endTime);
    void RemoveOldData(int64_t currentTime);
    std::vector<BundleActivePackageStats> QueryDatabaseUsageStats(int32_t databaseType, int64_t beginTime,
        int64_t endTime, int32_t userId, const std::string& bundleName = "");
    std::vector<BundleActiveEvent> QueryDatabaseEvents(int64_t beginTime, int64_t endTime, int32_t userId,
        std::string bundleName);
    void PutDurationData(int64_t bootBasedDuration, int64_t screenOnDuration);
    std::pair<int64_t, int64_t> GetDurationData();
    void PutBundleHistoryData(int32_t userId, std::shared_ptr<std::map<std::string,
        std::shared_ptr<BundleActivePackageHistory>>> userHistory);
    std::shared_ptr<std::map<std::string, std::shared_ptr<BundleActivePackageHistory>>>
        GetBundleHistoryData(int32_t userId);
    void OnPackageUninstalled(const int32_t userId, const std::string& bundleName,
        const int32_t uid, const int32_t appIndex);
    void ChangeToDebug();
    void UpdateModuleData(const int32_t userId,
        std::map<std::string, std::shared_ptr<BundleActiveModuleRecord>>& moduleRecords, const int64_t timeStamp);
    void RemoveFormData(const int32_t userId, const std::string bundleName,
        const std::string moduleName, const std::string formName, const int32_t formDimension,
        const int64_t formId, const int32_t uid);
    void LoadModuleData(const int32_t userId, std::map<std::string,
        std::shared_ptr<BundleActiveModuleRecord>>& moduleRecords);
    void LoadFormData(const int32_t userId, std::map<std::string,
        std::shared_ptr<BundleActiveModuleRecord>>& moduleRecords);
    void QueryDeviceEventStats(int32_t eventId, int64_t beginTime, int64_t endTime,
        std::map<std::string, BundleActiveEventStats>& eventStats, int32_t userId);
    void QueryNotificationEventStats(int32_t eventId, int64_t beginTime, int64_t endTime,
        std::map<std::string, BundleActiveEventStats>& notificationEventStats, int32_t userId);
    void DeleteExcessiveEventTableData(int32_t deleteDays);

private:
    void CheckDatabaseVersion();
    void HandleTableInfo(uint32_t databaseType);
    std::shared_ptr<NativeRdb::RdbStore> GetBundleActiveRdbStore(uint32_t databaseType);
    int64_t ParseStartTime(const std::string &tableName);
    int32_t NearIndexOnOrAfterCurrentTime(int64_t currentTime, std::vector<int64_t> &sortedTableArray);
    int32_t NearIndexOnOrBeforeCurrentTime(int64_t currentTime, std::vector<int64_t> &sortedTableArray);
    int32_t RenameTableName(uint32_t databaseType, int64_t tableOldTime, int64_t tableNewTime);
    std::string GetTableIndexSql(uint32_t databaseType, int64_t tableTime, bool createFlag,
        int32_t indexFlag = BUNDLE_ACTIVE_DB_INDEX_NORMAL);
    int32_t SetNewIndexWhenTimeChanged(uint32_t databaseType, int64_t tableOldTime,
        int64_t tableNewTime, std::shared_ptr<NativeRdb::RdbStore> rdbStore);
    void FlushPackageInfo(uint32_t databaseType, const BundleActivePeriodStats &stats);
    void FlushEventInfo(uint32_t databaseType, BundleActivePeriodStats &stats);
    void DeleteExcessiveTableData(uint32_t databaseType);
    int32_t DeleteInvalidTable(uint32_t databaseType, int64_t tableTimeMillis);
    std::unique_ptr<std::vector<int64_t>> GetOverdueTableCreateTime(uint32_t databaseType,
        int64_t currentTimeMillis);
    int32_t CreatePackageLogTable(uint32_t databaseType, int64_t currentTimeMillis);
    int32_t CreateEventLogTable(uint32_t databaseType, int64_t currentTimeMillis);
    int32_t CreateDurationTable(uint32_t databaseType);
    int32_t CreateBundleHistoryTable(uint32_t databaseType);
    int32_t CreateModuleRecordTable(uint32_t databaseType, int64_t timeStamp);
    int32_t CreateFormRecordTable(uint32_t databaseType, int64_t timeStamp);
    std::shared_ptr<NativeRdb::ResultSet> QueryStatsInfoByStep(uint32_t databaseType,
        const std::string &sql, const std::vector<std::string> &selectionArgs);
    void DeleteUninstalledInfo(const int32_t userId, const std::string& bundleName, const int32_t uid,
        const std::string& tableName, uint32_t databaseType, const int32_t appIndex);
    int32_t CreateDatabasePath();
    void CheckDatabaseFile(uint32_t databaseType);
    void UpdateFormData(const int32_t userId, const std::string bundleName,
        const std::string moduleName, const BundleActiveFormRecord& formRecord,
        std::shared_ptr<NativeRdb::RdbStore> rdbStore,
        std::vector<NativeRdb::ValuesBucket>& formValueBuckets);
    void BatchInsert(std::shared_ptr<NativeRdb::RdbStore> rdbStore,
        std::vector<NativeRdb::ValuesBucket>& rawContactValues, const std::string& tableName);
    int32_t JudgeQueryCondition(const int64_t beginTime, const int64_t endTime, const int64_t eventTableTime);
    std::string GetSystemEventName(const int32_t userId);
    int32_t ExecuteRenameTableName(std::string tablePrefix, int64_t tableOldTime, int64_t tableNewTime,
        std::shared_ptr<NativeRdb::RdbStore> rdbStore);
    void GetQuerySqlCommand(const int64_t beginTime,
        const int64_t endTime, const int32_t databaseType,
        const int32_t index, const int32_t startIndex, const int32_t endIndex, const int32_t userId,
        std::vector<std::string> &queryCondition, std::string &queryPackageSql);
    void GetCurrentBundleStats(int64_t currentPackageTime,
        std::shared_ptr<NativeRdb::ResultSet>& bundleActiveResult,
        std::shared_ptr<BundleActivePeriodStats>& intervalStats);
    bool GetDbIndex(const int64_t beginTime, const int64_t endTime,
        const int32_t databaseType, int32_t &startIndex, int32_t &endIndex);

private:
    int32_t GetOldDbVersion();
    int32_t GetVersionByFileInput(const std::string& FileVersionInput);
    void CreateRecordTable(const int64_t timeStamp);
    void HandleAllTableName(const uint32_t databaseType, std::vector<std::vector<std::string>>& allTableName);
    void UpgradeDatabase(const int32_t oldVersion, const int32_t curVersion);
    void UpdateOldDataUid(const std::shared_ptr<NativeRdb::RdbStore> store, const std::string& tableName,
        const int32_t userId, std::map<std::string, int32_t>& bundleNameUidMap);
    void SupportAppTwin();
    void AddRdbColumn(const std::shared_ptr<NativeRdb::RdbStore> store, const std::string& tableName,
        const std::string& columnName, const std::string& columnType);
    void SupportFirstUseTime();
    void UpdateFirstUseTime(const std::shared_ptr<NativeRdb::RdbStore> store,
        const std::string& tableName, const int32_t userId);
    std::map<std::string, int64_t> GetAllBundleFirstUseTime(const int32_t userId);
    std::vector<std::string> databaseFiles_;
    std::vector<std::vector<int64_t>> sortedTableArray_;
    std::map<std::string, std::shared_ptr<NativeRdb::RdbStore>> bundleActiveRdbStoreCache_;
    std::shared_ptr<BundleActiveCalendar> calendar_;
    void CheckDatabaseFileAndTable();
    std::string eventTableName_;
    std::string durationTableName_;
    std::string bundleHistoryTableName_;
    std::string moduleRecordsTableName_;
    std::string formRecordsTableName_;
    std::string versionFile_;
    uint32_t currentVersion_;
    ffrt::mutex databaseMutex_;
    std::int64_t eventBeginTime_;
    bool debugDatabase_;
};
}  // namespace DeviceUsageStats
}  // namespace OHOS
#endif  // BUNDLE_ACTIVE_USAGE_DATABASE_H

