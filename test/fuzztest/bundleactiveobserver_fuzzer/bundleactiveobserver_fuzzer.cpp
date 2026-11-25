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
#include "fuzzer/FuzzedDataProvider.h"

namespace OHOS {
namespace DeviceUsageStats {
    bool g_isInited = false;
    bool DoSomethingInterestingWithMyAPI(FuzzedDataProvider* provider)
    {
        DelayedSingleton<FuzztestHelper>::GetInstance()->NativeTokenGet();
        if (!g_isInited) {
            DelayedSingleton<BundleActiveService>::GetInstance()->InitService();
            g_isInited = true;
        }
        sptr<IAppGroupCallback> appGroupCallback = nullptr;
        DelayedSingleton<BundleActiveService>::GetInstance()->RegisterAppGroupCallBack(appGroupCallback);
        DelayedSingleton<BundleActiveService>::GetInstance()->UnRegisterAppGroupCallBack(appGroupCallback);
        bool result = false;
        int32_t userId = provider->ConsumeIntegral<int32_t>();
        std::string inputBundlName = provider->ConsumeRandomLengthString();
        DelayedSingleton<BundleActiveService>::GetInstance()->IsBundleIdle(result, inputBundlName, userId);
        DelayedSingleton<BundleActiveService>::GetInstance()->IsBundleUsePeriod(result, inputBundlName, userId);
        DelayedSingleton<BundleActiveConfigReader>::GetInstance()->GetApplicationUsePeriodicallyConfig();
        DelayedSingleton<BundleActiveConfigReader>::GetInstance()->GetAppHighFrequencyPeriodThresholdConfig();
        DelayedSingleton<BundleActiveConfigReader>::GetInstance()->LoadConfigFile(inputBundlName.c_str());
        return true;
    }

    bool BundleActiveServiceDumpFuzzTest(FuzzedDataProvider* provider)
    {
        int32_t fd = provider->ConsumeIntegral<int32_t>();
        std::vector<std::u16string> args;
        args = {to_utf16(provider->ConsumeRandomLengthString())};
        DelayedSingleton<BundleActiveService>::GetInstance()->Dump(fd, args);
        DelayedSingleton<BundleActiveService>::GetInstance()->AllowDump();
        std::vector<std::string> dumpOption = {
            provider->ConsumeRandomLengthString(), provider->ConsumeRandomLengthString(),
            provider->ConsumeRandomLengthString(), provider->ConsumeRandomLengthString(),
            provider->ConsumeRandomLengthString(), provider->ConsumeRandomLengthString()};
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
} // namespace DeviceUsageStats
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    OHOS::DeviceUsageStats::DoSomethingInterestingWithMyAPI(&fdp);
    OHOS::DeviceUsageStats::BundleActiveServiceDumpFuzzTest(&fdp);
    return 0;
}

