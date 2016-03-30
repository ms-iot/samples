/* *****************************************************************
 *
 * Copyright 2015 Samsung Electronics All Rights Reserved.
 *
 *
 *
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
 *
 * *****************************************************************/

#ifndef PROVISIONING_DATABASE_MANAGER_H
#define PROVISIONING_DATABASE_MANAGER_H
#include "securevirtualresourcetypes.h"
#include "ocstack.h"
#include "pmtypes.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * This method is used by provisioning manager to open provisioning database.
 *
 * @param[in] dbPath file path of the sqlite3 db
 *
 * @return OC_STACK_OK in case of success and other value otherwise.
 */
OCStackResult PDMInit(const char* dbPath);

/**
 * This method is used by provisioning manager to check duplication of device's Device ID with
 * provisioning database.
 *
 * @param[in] uuidOfDevice information about the target device's uuid.
 * @param[out] result true in case device UUID already exist otherwise false.
 *
 * @return OC_STACK_OK in case of success and other value otherwise.
 */
OCStackResult PDMIsDuplicateDevice(const OicUuid_t* uuidOfDevice, bool* result);

/**
 * This method is used by provisioning manager to add owned device's Device ID.
 *
 * @param[in] uuidOfDevice information about the owned device's uuid.
 *
 * @return OC_STACK_OK in case of success and other value otherwise.
 */
OCStackResult PDMAddDevice(const OicUuid_t* uuidOfDevice);

/**
 * This method is used by provisioning manager to update linked status of owned devices.
 *
 * @param[in] uuidOfDevice1 DeviceID which is going to be linked with uuid of device 2.
 * @param[in] uuidOfDevice2 DeviceID which is going to be linked with uuid of device 1.
 *
 * @return OC_STACK_OK in case of success and other value otherwise.
 */
OCStackResult PDMLinkDevices(const OicUuid_t *uuidOfDevice1, const OicUuid_t *uuidOfDevice2);

/**
 * This method is used by provisioning manager to unlink pairwise devices.
 *
 * @param[in] uuidOfDevice1 DeviceID which is going to be unlinked with uuid of device 2.
 * @param[in] uuidOfDevice2 DeviceID which is going to be unlinked with uuid of device 1.
 *
 * @return OC_STACK_OK in case of success and other value otherwise.
 */
OCStackResult PDMUnlinkDevices(const OicUuid_t *uuidOfDevice1, const OicUuid_t *uuidOfDevice2);

/**
 * This method is used by provisioning manager to delete owned device's Device ID.
 *
 * @param[in] uuidOfDevice information about the owned device's uuid to be deleted.
 *
 * @return OC_STACK_OK in case of success and other value otherwise.
 */
OCStackResult PDMDeleteDevice(const OicUuid_t *uuidOfDevice);

/**
 * This method is used by provisioning manager to get owned devices' Device IDs.
 *
 * @param[out] uuidList information about the list of owned devices' uuids.
 * @param[out] numOfDevices total number of owned devices.
 *
 * @return OC_STACK_OK in case of success and other value otherwise.
 */
OCStackResult PDMGetOwnedDevices(OCUuidList_t** uuidList, size_t* numOfDevices);

/**
 * This method is used by provisioning manager to get linked devices' IDs.
 *
 * @param[in] uuidOfDevice a target device's uuid.
 * @param[out] uuidList information about the list of linked devices' uuids.
 * @param[out] numOfDevices total number of linked devices.
 *
 * @return OC_STACK_OK in case of success and other value otherwise.
 */
OCStackResult PDMGetLinkedDevices(const OicUuid_t* uuidOfDevice, OCUuidList_t** uuidList,
                                    size_t* numOfDevices);

/**
 * This method is used by provisioning manager to update linked status as stale.
 *
 * @param[in] uuidOfDevice1 first id of stale link.
 * @param[in] uuidOfDevice2 other id for stale link.
 *
 * @return OC_STACK_OK in case of success and other value otherwise.
 */
OCStackResult PDMSetLinkStale(const OicUuid_t* uuidOfDevice1, const OicUuid_t* uuidOfDevice2);

/**
 * This method is used by provisioning manager to get stale devices.
 *
 * @note in case of sqllite, the caller should set NULL for parameters.
 *
 * @param[out] staleDevices information about the list of "To be Removed" devices' uuid.
 * @param[out] numOfDevices total number of devices to be removed.
 *
 * @return OC_STACK_OK in case of success and other value otherwise.
 */
OCStackResult PDMGetToBeUnlinkedDevices(OCPairList_t** staleDevList, size_t* numOfDevices);

/**
 * This method is used by provisioning manager to close provisioning database.
 *
 * @return  OC_STACK_OK in case of success and other value otherwise.
 */
OCStackResult PDMClose();

/**
 * This method is used by provisioning manager free memory allocated to OCUuidList_t lists.
 *
 * @param[in] ptr start pointer of link list.
 */
void PDMDestoryOicUuidLinkList(OCUuidList_t* ptr);

/**
 * This method is used by provisioning manager free memory allocated to Stalelist.
 *
 * @param[in] ptr start pointer of link list.
 *
 */
void PDMDestoryStaleLinkList(OCPairList_t* ptr);

/**
 * This method is used by provisioning manager to check does the link exists between
 * two devices or not.
 *
 * @param[in] uuidOfDevice1 UUID of device1.
 * @param[in] uuidOfDevice2 UUID of device2.
 * @param[out] result true when link exists otherwise false.
 *
 * @return OC_STACK_OK in case of success and other value otherwise.
 */
OCStackResult PDMIsLinkExists(const OicUuid_t* uuidOfDevice1, const OicUuid_t* uuidOfDevice2,
                                bool *result );

#ifdef __cplusplus
}
#endif
#endif //PROVISIONING_DATABASE_MANAGER_H
