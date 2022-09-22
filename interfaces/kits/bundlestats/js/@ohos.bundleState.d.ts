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

import { AsyncCallback , Callback} from './basic';

/**
 * Provides methods for managing bundle usage statistics,
 * including the methods for querying bundle usage information and state data.
 *
 * <p>You can use the methods defined in this class to query
 * the usage history and states of bundles in a specified period.
 * The system stores the query result in a {@link BundleStateInfo} or {@link BundleActiveState} instance and
 * then returns it to you.
 *
 * @since 7
 */
declare namespace bundleState {

    /**
     * @since 7
     * @syscap SystemCapability.ResourceSchedule.UsageStatistics.App
     */
    interface BundleStateInfo {
        /**
         * the identifier of BundleStateInfo.
         */
        id: number;
        /**
         * the total duration, in milliseconds.
         */
        abilityInFgTotalTime?: number;
        /**
         * the last time when the application was accessed, in milliseconds.
         */
        abilityPrevAccessTime?: number;
        /**
         * the last time when the application was visible in the foreground, in milliseconds.
         */
        abilityPrevSeenTime?: number;
        /**
         * the total duration, in milliseconds.
         */
        abilitySeenTotalTime?: number;
        /**
         * the bundle name of the application.
         */
        bundleName?: string;
        /**
         * the total duration, in milliseconds.
         */
        fgAbilityAccessTotalTime?: number;
        /**
         * the last time when the foreground application was accessed, in milliseconds.
         */
        fgAbilityPrevAccessTime?: number;
        /**
         * the time of the first bundle usage record in this {@code BundleActiveInfo} object,
         * in milliseconds.
         */
        infosBeginTime?: number;
        /**
         * the time of the last bundle usage record in this {@code BundleActiveInfo} object,
         * in milliseconds.
         */
        infosEndTime?: number;

        /**
         * Merges a specified {@link BundleActiveInfo} object with this {@link BundleActiveInfo} object.
         * The bundle name of both objects must be the same.
         *
         * @since 7
         * @syscap SystemCapability.ResourceSchedule.UsageStatistics.App
         * @param toMerge Indicates the {@link BundleActiveInfo} object to merge.
         * if the bundle names of the two {@link BundleActiveInfo} objects are different.
         */
        merge(toMerge: BundleStateInfo): void;
    }

    /**
     * @since 9
     * @syscap SystemCapability.ResourceSchedule.UsageStatistics.App
     * @systemapi Hide this for inner system use.
     */
    interface BundleActiveFormInfo {
        /**
         * the form name.
         */
        formName: string;
        /**
         * the form dimension.
         */
        formDimension: number;
        /**
         * the form id.
         */
        formId: number;
        /**
         * the last time when the form was accessed, in milliseconds..
         */
        formLastUsedTime: number;
        /**
         * the click count of module.
         */
        count: number;
    }

    /**
     * @since 9
     * @syscap SystemCapability.ResourceSchedule.UsageStatistics.App
     * @systemapi Hide this for inner system use.
     */
    interface BundleActiveModuleInfo {
        /**
         * the device id of module.
         */
        deviceId?: string;
        /**
         * the bundle name.
         */
        bundleName: string;
        /**
         * the module name.
         */
        moduleName: string;
        /**
         * the main ability name of module.
         */
        abilityName?: string;
        /**
         * the label id of application.
         */
        appLabelId?: number;
        /**
         * the label id of module.
         */
        labelId?: number;
        /**
         * the description id of application.
         */
        descriptionId?: number;
        /**
         * the ability id of main ability.
         */
        abilityLableId?: number;
        /**
         * the description id of main ability.
         */
        abilityDescriptionId?: number;
        /**
         * the icon id of main ability.
         */
        abilityIconId?: number;
        /**
         * the launch count of module.
         */
        launchedCount: number;
        /**
         * the last time when the module was accessed, in milliseconds.
         */
        lastModuleUsedTime: number;
        /**
         * the form usage record list of current module.
         */
        formRecords: Array<BundleActiveFormInfo>;
    }

    /**
     * @since 9
     * @syscap SystemCapability.ResourceSchedule.UsageStatistics.App
     * @systemapi Hide this for inner system use.
     */
    interface BundleActiveEventState {
        /**
         * the bundle name or system event name.
         */
        name: string;

        /**
         * the event id.
         */
        eventId: number;

        /**
         * the the event occurrence number.
         */
        count: number;
    }

    /**
     * @since 7
     * @syscap SystemCapability.ResourceSchedule.UsageStatistics.App
     */
     interface BundleActiveState {
        /**
         * the usage priority group of the application.
         */
        appUsagePriorityGroup?: number;
        /**
         * the bundle name.
         */
        bundleName?: string;
        /**
         * the shortcut ID.
         */
        indexOfLink?: string;
        /**
         * the class name.
         */
        nameOfClass?: string;
        /**
         * the time when this state occurred, in milliseconds.
         */
        stateOccurredTime?: number;
        /**
         * the state type.
         */
        stateType?: number;
    }
    /**
     * @since 9
     * @syscap SystemCapability.ResourceSchedule.UsageStatistics.AppGroup
     * @systemapi Hide this for inner system use.
     */
    interface BundleActiveGroupCallbackInfo {
        /*
         * the usage old group of the application
         */
        appUsageOldGroup: number;
        /*
         * the usage new group of the application
         */
        appUsageNewGroup: number;
        /*
         * the use id
         */
        userId: number;
        /*
         * the change reason
         */
        changeReason: number;
        /*
         * the bundle name
         */
        bundleName: string;
    }

    /**
     * Checks whether the application with a specified bundle name is in the idle state.
     *
     * @since 7
     * @syscap SystemCapability.ResourceSchedule.UsageStatistics.AppGroup
     * @param bundleName Indicates the bundle name of the application to query.
     * @return Returns {@code true} if the application is idle in a particular period;
     * returns {@code false} otherwise. The time range of the particular period is defined by the system,
     * which may be hours or days.
     */
    function isIdleState(bundleName: string, callback: AsyncCallback<boolean>): void;
    function isIdleState(bundleName: string): Promise<boolean>;

    /**
     * Queries the usage priority group of the calling application.
     *
     * <p>The priority defined in a priority group restricts the resource usage of an application,
     * for example, restricting the running of background tasks. </p>
     *
     * @since 7
     * @syscap SystemCapability.ResourceSchedule.UsageStatistics.AppGroup
     * @return Returns the usage priority group of the calling application.
     */
    function queryAppUsagePriorityGroup(callback: AsyncCallback<number>): void;
    function queryAppUsagePriorityGroup(): Promise<number>;

    /**
     * @since 7
     * @syscap SystemCapability.ResourceSchedule.UsageStatistics.App
     */
     interface BundleActiveInfoResponse {
        [key: string]: BundleStateInfo;
    }

    /**
     * Queries usage information about each bundle within a specified period.
     *
     * <p>This method queries usage information at the {@link #BY_OPTIMIZED} interval by default.</p>
     *
     * @since 7
     * @syscap SystemCapability.ResourceSchedule.UsageStatistics.App
     * @permission ohos.permission.BUNDLE_ACTIVE_INFO
     * @systemapi Hide this for inner system use.
     * @param begin Indicates the start time of the query period, in milliseconds.
     * @param end Indicates the end time of the query period, in milliseconds.
     * @return Returns the {@link BundleActiveInfoResponse} objects containing the usage information about each bundle.
     */
    function queryBundleStateInfos(begin: number, end: number, callback: AsyncCallback<BundleActiveInfoResponse>): void;
    function queryBundleStateInfos(begin: number, end: number): Promise<BundleActiveInfoResponse>;

    /**
     * Declares interval type.
     *
     * @since 7
     * @syscap SystemCapability.ResourceSchedule.UsageStatistics.App
     */
    export enum IntervalType {
        /**
         * Indicates the interval type that will determine the optimal interval based on the start and end time.
         */
        BY_OPTIMIZED = 0,

        /**
         * Indicates the daily interval.
         */
        BY_DAILY = 1,

        /**
         * Indicates the weekly interval.
         */
        BY_WEEKLY = 2,

        /**
         * Indicates the monthly interval.
         */
        BY_MONTHLY = 3,

        /**
         * Indicates the annually interval.
         */
        BY_ANNUALLY = 4
    }

    /**
     * Queries usage information about each bundle within a specified period at a specified interval.
     *
     * @since 7
     * @syscap SystemCapability.ResourceSchedule.UsageStatistics.App
     * @permission ohos.permission.BUNDLE_ACTIVE_INFO
     * @systemapi Hide this for inner system use.
     * @param byInterval Indicates the interval at which the usage statistics are queried.
     * The value can be {@link #BY_OPTIMIZED}, {@link #BY_DAILY},
     * {@link #BY_WEEKLY}, {@link #BY_MONTHLY}, or {@link #BY_ANNUALLY}.
     * @param begin Indicates the start time of the query period, in milliseconds.
     * @param end Indicates the end time of the query period, in milliseconds.
     * @return Returns the list of {@link BundleStateInfo} objects containing the usage information about each bundle.
     */
    function queryBundleStateInfoByInterval(byInterval: IntervalType, begin: number, end: number, callback: AsyncCallback<Array<BundleStateInfo>>): void;
    function queryBundleStateInfoByInterval(byInterval: IntervalType, begin: number, end: number): Promise<Array<BundleStateInfo>>;

    /**
     * Queries state data of all bundles within a specified period identified by the start and end time.
     *
     * @since 7
     * @syscap SystemCapability.ResourceSchedule.UsageStatistics.App
     * @permission ohos.permission.BUNDLE_ACTIVE_INFO
     * @systemapi Hide this for inner system use.
     * @param begin Indicates the start time of the query period, in milliseconds.
     * @param end Indicates the end time of the query period, in milliseconds.
     * @return Returns the list of {@link BundleActiveState} objects containing the state data of all bundles.
     */
    function queryBundleActiveStates(begin: number, end: number, callback: AsyncCallback<Array<BundleActiveState>>): void;
    function queryBundleActiveStates(begin: number, end: number): Promise<Array<BundleActiveState>>;

    /**
     * Queries state data of the current bundle within a specified period.
     *
     * @since 7
     * @syscap SystemCapability.ResourceSchedule.UsageStatistics.App
     * @param begin Indicates the start time of the query period, in milliseconds.
     * @param end Indicates the end time of the query period, in milliseconds.
     * @return Returns the {@link BundleActiveState} object Array containing the state data of the current bundle.
     */
    function queryCurrentBundleActiveStates(begin: number, end: number, callback: AsyncCallback<Array<BundleActiveState>>): void;
    function queryCurrentBundleActiveStates(begin: number, end: number): Promise<Array<BundleActiveState>>;

    /**
     * Queries recently module usage records.
     *
     * @since 9
     * @syscap SystemCapability.ResourceSchedule.UsageStatistics.App
     * @permission ohos.permission.BUNDLE_ACTIVE_INFO
     * @systemapi Hide this for inner system use.
     * @param maxNum Indicates max record number in result, max value is 1000, default value is 1000.
     * @return Returns the {@link BundleActiveModuleInfo} object Array containing the usage data of the modules.
     */
    function getRecentlyUsedModules(maxNum?: number, callback: AsyncCallback<Array<BundleActiveModuleInfo>>): void;
    function getRecentlyUsedModules(maxNum?: number): Promise<Array<BundleActiveModuleInfo>>;

    /**
     * Queries the usage priority group of the calling application.
     *
     * <p>The priority defined in a priority group restricts the resource usage of an application,
     * for example, restricting the running of background tasks. </p>
     *
     * @since 9
     * @syscap SystemCapability.ResourceSchedule.UsageStatistics.AppGroup
     * @permission ohos.permission.BUNDLE_ACTIVE_INFO
     * @systemapi Hide this for inner system use.
     * @param bundleName, name of the application.
     * @return Returns the usage priority group of the calling application.
     */
     function queryAppUsagePriorityGroup(bundleName? : string, callback: AsyncCallback<number>): void;
     function queryAppUsagePriorityGroup(bundleName? : string): Promise<number>;

     /**
     * Declares group type.
     *
     * @since 9
     * @syscap SystemCapability.ResourceSchedule.UsageStatistics.AppGroup
     * @systemapi Hide this for inner system use.
     */
      export enum GroupType {
        /**
         * Indicates the alive group.
         */
        ACTIVE_GROUP_ALIVE = 10,

        /**
         * Indicates the daily group.
         */
        ACTIVE_GROUP_DAILY = 20,

        /**
         * Indicates the fixed group.
         */
        ACTIVE_GROUP_FIXED = 30,

        /**
         * Indicates the rare group.
         */
        ACTIVE_GROUP_RARE = 40,

        /**
         * Indicates the limit group.
         */
         ACTIVE_GROUP_LIMIT = 50,

         /**
         * Indicates the never group.
         */
         ACTIVE_GROUP_NEVER = 60
    }

    /**
     * set bundle group by bundleName and number.
     *
     * @since 9
     * @syscap SystemCapability.ResourceSchedule.UsageStatistics.AppGroup
     * @permission ohos.permission.BUNDLE_ACTIVE_INFO
     * @systemapi Hide this for inner system use.
     * @param bundleName, name of the application.
     * @param newGroup,the group of the application whose name is bundleName.
     * @return Returns the result of setBundleGroup, true of false.
     */
    function setBundleGroup(bundleName: string, newGroup: GroupType, callback: AsyncCallback<void>): void;
    function setBundleGroup(bundleName: string, newGroup: GroupType): Promise<void>;

    /**
     * register callback to service.
     *
     * @since 9
     * @syscap SystemCapability.ResourceSchedule.UsageStatistics.AppGroup
     * @permission ohos.permission.BUNDLE_ACTIVE_INFO
     * @systemapi Hide this for inner system use.
     * @param Callback<BundleActiveGroupCallbackInfo>, callback when application group change,return the BundleActiveGroupCallbackInfo.
     * @return Returns BundleActiveGroupCallbackInfo when the group of bundle changed. the result of AsyncCallback is true or false.
     */
    function registerGroupCallBack(groupCallback: Callback<BundleActiveGroupCallbackInfo>, callback: AsyncCallback<void>): void;
    function registerGroupCallBack(groupCallback: Callback<BundleActiveGroupCallbackInfo>): Promise<void>;

    /**
     * unRegister callback from service.
     *
     * @since 9
     * @syscap SystemCapability.ResourceSchedule.UsageStatistics.AppGroup
     * @permission ohos.permission.BUNDLE_ACTIVE_INFO
     * @systemapi Hide this for inner system use.
     * @return Returns the result of unRegisterGroupCallBack, true of false.
     */
    function unRegisterGroupCallBack(callback: AsyncCallback<void>): void;
    function unRegisterGroupCallBack(): Promise<void>;

    /*
     * Queries system event states data within a specified period identified by the start and end time.
     *
     * @since 9
     * @syscap SystemCapability.ResourceSchedule.UsageStatistics.App
     * @permission ohos.permission.BUNDLE_ACTIVE_INFO
     * @systemapi Hide this for inner system use.
     * @param begin Indicates the start time of the query period, in milliseconds.
     * @param end Indicates the end time of the query period, in milliseconds.
     * @return Returns the {@link BundleActiveEventState} object Array containing the event states data.
     */
    function queryBundleActiveEventStates(begin: number, end: number, callback: AsyncCallback<Array<BundleActiveEventState>>): void;
    function queryBundleActiveEventStates(begin: number, end: number): Promise<Array<BundleActiveEventState>>;

    /**
     * Queries app notification number within a specified period identified by the start and end time.
     *
     * @since 9
     * @syscap SystemCapability.ResourceSchedule.UsageStatistics.App
     * @permission ohos.permission.BUNDLE_ACTIVE_INFO
     * @systemapi Hide this for inner system use.
     * @param begin Indicates the start time of the query period, in milliseconds.
     * @param end Indicates the end time of the query period, in milliseconds.
     * @return Returns the {@link BundleActiveEventState} object Array containing the event states data.
     */
    function queryAppNotificationNumber(begin: number, end: number, callback: AsyncCallback<Array<BundleActiveEventState>>): void;
    function queryAppNotificationNumber(begin: number, end: number): Promise<Array<BundleActiveEventState>>;
}

export default bundleState;

