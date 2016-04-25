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

#ifndef OCPROVISIONINGMANAGER_H_
#define OCPROVISIONINGMANAGER_H_

#include "octypes.h"
#include "pmtypes.h"
#include "ownershiptransfermanager.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * The function is responsible for initializaton of the provisioning manager. It will load
 * provisioning database which have owned device's list and their linked status.
 * TODO: In addition, if there is a device(s) which has not up-to-date credentials, this function will
 * automatically try to update the deivce(s).
 *
 * @param[in] dbPath file path of the sqlite3 db
 *
 * @return OC_STACK_OK in case of success and other value otherwise.
 */
OCStackResult OCInitPM(const char* dbPath);

/**
 * The function is responsible for discovery of device is current subnet. It will list
 * all the device in subnet which are not yet owned. Please call OCInit with OC_CLIENT_SERVER as
 * OCMode.
 *
 * @param[in] timeout Timeout in seconds, value till which function will listen to responses from
 *                    client before returning the list of devices.
 * @param[out] ppList List of candidate devices to be provisioned
 * @return OTM_SUCCESS in case of success and other value otherwise.
 */
OCStackResult OCDiscoverUnownedDevices(unsigned short waittime, OCProvisionDev_t **ppList);

/**
 * Do ownership transfer for un-owned device.
 *
 * @param[in] ctx Application context would be returned in result callback
 * @param[in] targetDevices List of devices to perform ownership transfer.
 * @param[in] resultCallback Result callback function to be invoked when ownership transfer finished.
 * @return OC_STACK_OK in case of success and other value otherwise.
 */
OCStackResult OCDoOwnershipTransfer(void* ctx,
                                    OCProvisionDev_t *targetDevices,
                                    OCProvisionResultCB resultCallback);

/**
 * API to register for particular OxM.
 *
 * @param[in] Ownership transfer method.
 * @param[in] Implementation of callback functions for owership transfer.
 * @return OC_STACK_OK in case of success and other value otherwise.
 */
OCStackResult OCSetOwnerTransferCallbackData(OicSecOxm_t oxm, OTMCallbackData_t* callbackData);

/**
 * The function is responsible for discovery of owned device is current subnet. It will list
 * all the device in subnet which are owned by calling provisioning client.
 *
 * @param[in] timeout Timeout in seconds, value till which function will listen to responses from
 *                    client before returning the list of devices.
 * @param[out] ppList List of device owned by provisioning tool.
 * @return OTM_SUCCESS in case of success and other value otherwise.
 */
OCStackResult OCDiscoverOwnedDevices(unsigned short timeout, OCProvisionDev_t **ppList);

/**
 * API to provision credentials between two devices and ACLs for the devices who act as a server.
 *
 * @param[in] ctx Application context would be returned in result callback.
 * @param[in] type Type of credentials to be provisioned to the device.
 * @param[in] pDev1 Pointer to OCProvisionDev_t instance,respresenting device to be provisioned.
 * @param[in] acl ACL for device 1. If this is not required set NULL.
 * @param[in] pDev2 Pointer to OCProvisionDev_t instance,respresenting device to be provisioned.
 * @param[in] acl ACL for device 2. If this is not required set NULL.
 * @param[in] resultCallback callback provided by API user, callback will be called when
 *            provisioning request recieves a response from first resource server.
 * @return OC_STACK_OK in case of success and other value otherwise.
 */
OCStackResult OCProvisionPairwiseDevices(void* ctx, OicSecCredType_t type, size_t keySize,
                                         const OCProvisionDev_t *pDev1, OicSecAcl_t *pDev1Acl,
                                         const OCProvisionDev_t *pDev2, OicSecAcl_t *pDev2Acl,
                                         OCProvisionResultCB resultCallback);

/**
 * API to send ACL information to device.
 *
 * @param[in] ctx Application context would be returned in result callback.
 * @param[in] selectedDeviceInfo Selected target device.
 * @param[in] acl ACL to provision.
 * @param[in] resultCallback callback provided by API user, callback will be called when provisioning
              request recieves a response from resource server.
 * @return OC_STACK_OK in case of success and other value otherwise.
 */
OCStackResult OCProvisionACL(void *ctx, const OCProvisionDev_t *selectedDeviceInfo, OicSecAcl_t *acl,
                             OCProvisionResultCB resultCallback);

/**
 * API to provision credential to devices.
 *
 * @param[in] ctx Application context would be returned in result callback.
 * @param[in] type Type of credentials to be provisioned to the device.
 * @param[in] pDev1 Pointer to OCProvisionDev_t instance,respresenting resource to be provsioned.
   @param[in] pDev2 Pointer to OCProvisionDev_t instance,respresenting resource to be provsioned.
 * @param[in] resultCallback callback provided by API user, callback will be called when
 *            provisioning request recieves a response from first resource server.
 * @return OC_STACK_OK in case of success and other value otherwise.
 */
OCStackResult OCProvisionCredentials(void *ctx, OicSecCredType_t type, size_t keySize,
                                      const OCProvisionDev_t *pDev1,
                                      const OCProvisionDev_t *pDev2,
                                      OCProvisionResultCB resultCallback);

/**
 * Function to unlink devices.
 * This function will remove the credential & relasionship between the two devices.
 *
 * @param[in] ctx Application context would be returned in result callback
 * @param[in] pTargetDev1 fitst device information to be unlinked.
 * @param[in] pTargetDev2 second device information to be unlinked.
 * @param[in] resultCallback callback provided by API user, callback will be called when
 *            device unlink is finished.
 * @return OC_STACK_OK in case of success and other value otherwise.
 */
OCStackResult OCUnlinkDevices(void* ctx,
                              const OCProvisionDev_t* pTargetDev1,
                              const OCProvisionDev_t* pTargetDev2,
                              OCProvisionResultCB resultCallback);

/**
 * Function for device revocation
 * This function will remove credential of target device from all devices in subnet.
 *
 * @param[in] ctx Application context would be returned in result callback
 * @param[in] waitTimeForOwnedDeviceDiscovery Maximum wait time for owned device discovery.(seconds)
 * @param[in] pTargetDev Device information to be revoked.
 * @param[in] resultCallback callback provided by API user, callback will be called when
 *            credential revocation is finished.
 * @return OC_STACK_OK in case of success and other value otherwise.
 *         if OC_STACK_OK is returned, the caller of this API should wait for callback.
 *         OC_STACK_CONTINUE means operation is success but no need to wait for callback.
 */
OCStackResult OCRemoveDevice(void* ctx,
                             unsigned short waitTimeForOwnedDeviceDiscovery,
                             const OCProvisionDev_t* pTargetDev,
                             OCProvisionResultCB resultCallback);
/**
 * API to get status of all the devices in current subnet. The status include endpoint information
 * and doxm information which can be extracted duing owned and unowned discovery. Along with this
 * information. The API will provide information about devices' status
 * Device can have following states
 *  - ON/OFF: Device is switched on or off.
 *
 * NOTE: Caller need to call OCDeleteDiscoveredDevices to delete memory allocated by this API for out
 * variables pOwnedDevList and pUnownedDevList.
 *
 * @param[in] waitime Wait time for the API. The wait time will be divided by 2, and half of wait time
 * will be used for unowned discovery and remaining half for owned discovery.
 * @param[out] pOwnedDevList  list of owned devices.
 * @param[out] pUnownedDevList  list of unowned devices.
 * @return OC_STACK_OK in case of success and other value otherwise.
 */
OCStackResult OCGetDevInfoFromNetwork(unsigned short waittime,
                                       OCProvisionDev_t** pOwnedDevList,
                                       OCProvisionDev_t** pUnownedDevList);
/**
 * This method is used to get linked devices' IDs.
 *
 * @param[in] uuidOfDevice a target device's uuid.
 * @param[out] uuidList information about the list of linked devices' uuids.
 * @param[out] numOfDevices total number of linked devices.
 * @return OC_STACK_OK in case of success and other value otherwise.
 */
OCStackResult OCGetLinkedStatus(const OicUuid_t* uuidOfDevice,
                                  OCUuidList_t** uuidList,
                                  size_t* numOfDevices);

/**
 * API to delete memory allocated to linked list created by OCDiscover_XXX_Devices API.
 *
 * @param[in] pList Pointer to OCProvisionDev_t which should be deleted.
 */
void OCDeleteDiscoveredDevices(OCProvisionDev_t *pList);

/**
 * API to delete memory allocated to OicUuid_t list.
 *
 * @param[in] pList Pointer to OicUuid_t list which should be deleted.
 */
void OCDeleteUuidList(OCUuidList_t* pList);

/**
 * This function deletes ACL data.
 *
 * @param pAcl Pointer to OicSecAcl_t structure.
 */
void OCDeleteACLList(OicSecAcl_t* pAcl);

#ifdef __WITH_X509__
/**
 * this function sends CRL information to resource.
 *
 * @param[in] ctx Application context would be returned in result callback.
 * @param[in] selectedDeviceInfo Selected target device.
 * @param[in] crl CRL to provision.
 * @param[in] resultCallback callback provided by API user, callback will be called when provisioning
              request recieves a response from resource server.
 * @return  OC_STACK_OK in case of success and other value otherwise.
 */
OCStackResult OCProvisionCRL(void* ctx, const OCProvisionDev_t *selectedDeviceInfo, OicSecCrl_t *crl,
                             OCProvisionResultCB resultCallback);
#endif // __WITH_X509__


#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* OCPROVISIONINGMANAGER_H_ */
