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

#include "bundleactiveobserver_fuzzer.h"

#include "accesstoken_kit.h"
#include "app_mgr_interface.h"

#include "system_ability_definition.h"
#include "iservice_registry.h"
#include "bundle_active_service.h"
#include "iapp_group_callback.h"
#include "bundle_active_config_reader.h"

#ifndef EOK
#define EOK 0
#endif

namespace OHOS {
namespace DeviceUsageStats {
    constexpr uint32_t U32_AT_SIZE = 4;
    bool g_isInited = false;

    const char* G_DATA = nullptr;
    size_t g_size = 0;
    size_t g_pos;

    template<class T>
    T GetData()
    {
        T object {};
        size_t objectSize = sizeof(object);
        if (G_DATA == nullptr || objectSize > g_size - g_pos) {
            return object;
        }
        if (memcpy_s(&object, objectSize, G_DATA + g_pos, objectSize) != EOK) {
            return {};
        }
        g_pos += objectSize;
        return object;
    }

    bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
    {
        G_DATA = data;
        g_size = size;
        DelayedSingleton<FuzztestHelper>::GetInstance()->NativeTokenGet();
        if (!g_isInited) {
            DelayedSingleton<BundleActiveService>::GetInstance()->InitService();
            g_isInited = true;
        }
        sptr<IAppGroupCallback> appGroupCallback = nullptr;
        DelayedSingleton<BundleActiveService>::GetInstance()->RegisterAppGroupCallBack(appGroupCallback);
        DelayedSingleton<BundleActiveService>::GetInstance()->UnRegisterAppGroupCallBack(appGroupCallback);
        bool result = false;
        int32_t userId = GetData<int32_t>();
        std::string inputBundlName(data + g_pos);
        DelayedSingleton<BundleActiveService>::GetInstance()->IsBundleIdle(result, inputBundlName, userId);
        DelayedSingleton<BundleActiveService>::GetInstance()->IsBundleUsePeriod(result, inputBundlName, userId);
        DelayedSingleton<BundleActiveConfigReader>::GetInstance()->GetApplicationUsePeriodicallyConfig();
        DelayedSingleton<BundleActiveConfigReader>::GetInstance()->GetAppHighFrequencyPeriodThresholdConfig();
        DelayedSingleton<BundleActiveConfigReader>::GetInstance()->LoadConfigFile(inputBundlName.c_str());
        return true;
    }

    bool BundleActiveServiceDumpFuzzTest(const char* data, size_t size)
    {
        G_DATA = data;
        g_size = size;
        int32_t fd = GetData<int32_t>();
        std::vector<std::u16string> args;
        args = {to_utf16(std::to_string(GetData<uint32_t>()))};
        DelayedSingleton<BundleActiveService>::GetInstance()->Dump(fd, args);
        DelayedSingleton<BundleActiveService>::GetInstance()->AllowDump();
        static constexpr int32_t DUMP_SIZE_OPTIONS[] = {3, 4, 5, 6, 7};
        int32_t optSize = DUMP_SIZE_OPTIONS[GetData<uint8_t>() % 5];
        static const std::string DUMP_CMDS[] = {"Events", "PackageUsage", "ModuleUsage", "HighFreqHourUsage"};
        uint8_t cmdIdx = GetData<uint8_t>() % 5;
        std::vector<std::string> dumpOption;
        dumpOption.reserve(optSize);
        dumpOption.push_back(std::to_string(GetData<uint32_t>()));
        dumpOption.push_back(cmdIdx < U32_AT_SIZE ? DUMP_CMDS[cmdIdx] : std::to_string(GetData<uint32_t>()));
        for (int32_t i = 2; i < optSize; ++i) {
            dumpOption.push_back(std::to_string(GetData<uint32_t>()));
        }

        std::vector<std::string> dumpInfo;
        DelayedSingleton<BundleActiveService>::GetInstance()->ShellDump(dumpOption, dumpInfo);
        dumpInfo.clear();
        DelayedSingleton<BundleActiveService>::GetInstance()->DumpEvents(dumpOption, dumpInfo);
        dumpInfo.clear();
        DelayedSingleton<BundleActiveService>::GetInstance()->DumpPackageUsage(dumpOption, dumpInfo);
        dumpInfo.clear();
        DelayedSingleton<BundleActiveService>::GetInstance()->DumpModuleUsage(dumpOption, dumpInfo);
        dumpInfo.clear();
        DelayedSingleton<BundleActiveService>::GetInstance()->DumpHighFreqHourUsage(dumpOption, dumpInfo);
        std::string result;
        DelayedSingleton<BundleActiveService>::GetInstance()->DumpUsage(result);
        return true;
    }

    bool SetGPos()
    {
        g_pos = 0;
        return true;
    }
} // namespace DeviceUsageStats
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        return 0;
    }

    if (size < OHOS::DeviceUsageStats::U32_AT_SIZE) {
        return 0;
    }
    char* ch = (char *)malloc(size + 1);
    if (ch == nullptr) {
        return 0;
    }

    (void)memset_s(ch, size + 1, 0x00, size + 1);
    if (memcpy_s(ch, size, data, size) != EOK) {
        free(ch);
        ch = nullptr;
        return 0;
    }

    OHOS::DeviceUsageStats::SetGPos();
    OHOS::DeviceUsageStats::DoSomethingInterestingWithMyAPI(ch, size);
    OHOS::DeviceUsageStats::BundleActiveServiceDumpFuzzTest(ch, size);
    free(ch);
    ch = nullptr;
    return 0;
}

