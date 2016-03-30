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

#ifndef SRP_SECURERESOURCEPROVIDER_H
#define SRP_SECURERESOURCEPROVIDER_H

#include "ocstack.h"
#include "securevirtualresourcetypes.h"
#include "pmtypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * API to send ACL information to resource.
 *
 * @param[in] selectedDeviceInfo Selected target device.
 * @param[in] acl ACL to provision.
 * @param[in] resultCallback callback provided by API user, callback will be called when
 *            provisioning request recieves a response from resource server.
 * @return OC_STACK_OK in case of success and other value otherwise.
 */
OCStackResult SRPProvisionACL(void *ctx, const OCProvisionDev_t *selectedDeviceInfo,
                                        OicSecAcl_t *acl, OCProvisionResultCB resultCallback);

/**
 * API to provision credential to devices.
 *
 * @param[in] type Type of credentials to be provisioned to the device.
 * @param[in] pDev1 Pointer to PMOwnedDeviceInfo_t instance,respresenting resource to be provsioned.
   @param[in] pDev2 Pointer to PMOwnedDeviceInfo_t instance,respresenting resource to be provsioned.
 * @param[in] resultCallback callback provided by API user, callback will be called when
 *            provisioning request recieves a response from first resource server.
 * @return OC_STACK_OK in case of success and other value otherwise.
 */
OCStackResult SRPProvisionCredentials(void *ctx,OicSecCredType_t type, size_t keySize,
                                      const OCProvisionDev_t *pDev1,
                                      const OCProvisionDev_t *pDev2,
                                      OCProvisionResultCB resultCallback);

/**
 * Function to unlink devices.
 * This function will remove the credential & relationship between the two devices.
 *
 * @param[in] ctx Application context would be returned in result callback
 * @param[in] pTargetDev1 first device information to be unlinked.
 * @param[in] pTargetDev2 second device information to be unlinked.
 * @param[in] resultCallback callback provided by API user, callback will be called when
 *            device unlink is finished.
 *            when there is an error, this user callback is called immediately.
 * @return OC_STACK_OK in case of success and other value otherwise.
 */
OCStackResult SRPUnlinkDevices(void* ctx,
                              const OCProvisionDev_t* pTargetDev1,
                              const OCProvisionDev_t* pTargetDev2,
                              OCProvisionResultCB resultCallback);

/*
 * Function to device revocation.
 * This function will remove credential of target device from all devices in subnet.
 *
 * @param[in] ctx Application context would be returned in result callback
 * @param[in] waitTimeForOwnedDeviceDiscovery Maximum wait time for owned device discovery.(seconds)
 * @param[in] pTargetDev Device information to be revoked.
 * @param[in] resultCallback callback provided by API user, callback will be called when
 *            credential revocation is finished.
 *            when there is an error, this user callback is called immediately.
 * @return OC_STACK_OK in case of success and other value otherwise.
 *         If OC_STACK_OK is returned, the caller of this API should wait for callback.
 *         OC_STACK_CONTINUE means operation is success but no request is need to be initiated.
 */
OCStackResult SRPRemoveDevice(void* ctx,
                              unsigned short waitTimeForOwnedDeviceDiscovery,
                              const OCProvisionDev_t* pTargetDev,
                              OCProvisionResultCB resultCallback);

#ifdef __cplusplus
}
#endif
#endif //SRP_SECURERESOURCEPROVIDER_H
