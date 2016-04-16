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

#ifndef _PM_UTILITY_H_
#define _PM_UTILITY_H_

#include <stdbool.h>
#include "ocstack.h"
#include "pmtypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define DEFAULT_SECURE_PORT 5684

#define COAPS_PREFIX "coaps://"
#define COAP_PREFIX "coap://"
#define COAPS_QUERY "coaps://%s:%d%s"
#define COAP_QUERY "coap://%s:%d%s"

/**
 * Discover owned/unowned devices in the same IP subnet. .
 *
 * @param[in] waittime      Timeout in seconds.
 * @param[in] isOwned       bool flag for owned / unowned discovery
 * @param[in] ppList        List of OCProvisionDev_t.
 *
 * @return OC_STACK_OK on success otherwise error.
 */
OCStackResult PMDeviceDiscovery(unsigned short waittime, bool isOwned, OCProvisionDev_t **ppList);

/**
 * This function deletes list of provision target devices
 *
 * @param[in] pList         List of OCProvisionDev_t.
 */
void PMDeleteDeviceList(OCProvisionDev_t *pList);

/**
 * Timeout implementation for secure discovery. When performing secure discovery,
 * we should wait a certain period of time for getting response of each devices.
 *
 * @param[in]  waittime  Timeout in seconds.
 * @param[in]  waitForStackResponse if true timeout function will call OCProcess while waiting.
 * @return OC_STACK_OK on success otherwise error.
 */
OCStackResult PMTimeout(unsigned short waittime, bool waitForStackResponse);

/**
 * Function to clone OCProvisionDev_t
 *
 * @param[in] src pointer of OCProvisionDev_t to be copied.
 *
 * @return copied OCProvisionDev_t on success otherwise NULL.
 */
OCProvisionDev_t* PMCloneOCProvisionDev(const OCProvisionDev_t* src);


/**
 * Function to generate qurey for coap/coaps request.
 *
 * @param[in] isSeucre Choose whether to encrypt the payload.
 * @param[in] address Network address of remote device
 * @param[in] port Port number
 * @param[in] connType Connectivity type of remote device
 * @param[in,out] buffer A buffer to save the query.
 * @param[in] bufferSize Size of buffer
 * @param[in] uri The resource URI
 *
 * @return true on success
 */
bool PMGenerateQuery(bool isSecure,
                     const char* address, uint16_t port,
                     OCConnectivityType connType,
                     char* buffer, size_t bufferSize, const char* uri);

/**
 * Function to print OCProvisionDev_t for debug purpose.
 *
 * @param[in] pDev Pointer to OCProvisionDev_t. It's information will be printed by OC_LOG_XX
 *
 */
void PMPrintOCProvisionDev(const OCProvisionDev_t* pDev);

/** Function to delete matched UUID from the UUIDlist.
 *
 * @param[in] pUuidList a pointer to UUID list.
 * @param[in] targetId  a pointer to UUID to be deleted in the list.
 *
 * @return true when deletion is happened, false when no deletion is occured. In case either of
 * two arguments is null it will return false.
 */
bool PMDeleteFromUUIDList(OCUuidList_t *pUuidList, OicUuid_t *targetId);

#ifdef __cplusplus
}
#endif
#endif //_PM_UTILITY_H_
