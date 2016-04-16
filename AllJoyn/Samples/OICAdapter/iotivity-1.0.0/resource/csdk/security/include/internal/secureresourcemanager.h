//******************************************************************
//
// Copyright 2015 Intel Mobile Communications GmbH All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#ifndef SECURITYRESOURCEMANAGER_H_
#define SECURITYRESOURCEMANAGER_H_

#include "securevirtualresourcetypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Register Persistent storage callback.
 * @param   persistentStorageHandler [IN] Pointers to open, read, write, close & unlink handlers.
 * @return
 *     OC_STACK_OK    - No errors; Success
 *     OC_STACK_INVALID_PARAM - Invalid parameter
 */
OCStackResult SRMRegisterPersistentStorageHandler(OCPersistentStorage* persistentStorageHandler);

/**
 * @brief   Get Persistent storage handler pointer.
 * @return
 *     The pointer to Persistent Storage callback handler
 */
OCPersistentStorage* SRMGetPersistentStorageHandler();

/**
 * @brief   Register request and response callbacks.
 *          Requests and responses are delivered in these callbacks.
 * @param   reqHandler   [IN] Request handler callback ( for GET,PUT ..etc)
 * @param   respHandler  [IN] Response handler callback.
 * @param   errHandler   [IN] Error handler callback.
 * @return
 *     OC_STACK_OK    - No errors; Success
 *     OC_STACK_INVALID_PARAM - Invalid parameter
 */
OCStackResult SRMRegisterHandler(CARequestCallback reqHandler,
                                 CAResponseCallback respHandler,
                                 CAErrorCallback errHandler);

/**
 * @brief   Initialize all secure resources ( /oic/sec/cred, /oic/sec/acl, /oic/sec/pstat etc).
 * @return  OC_STACK_OK for Success, otherwise some error value
 */
OCStackResult SRMInitSecureResources();

/**
 * @brief   Perform cleanup for secure resources ( /oic/sec/cred, /oic/sec/acl, /oic/sec/pstat etc).
 * @return  none
 */
void SRMDeInitSecureResources();

/**
 * @brief   Initialize Policy Engine context.
 * @return  OC_STACK_OK for Success, otherwise some error value.
 */
OCStackResult SRMInitPolicyEngine();

/**
 * @brief   Cleanup Policy Engine context.
 * @return  none
 */
void SRMDeInitPolicyEngine();

/**
 * @brief   Provisioning API response callback.
 * @param object[IN]       endpoint instance.
 * @param responseInfo[IN] instance of CAResponseInfo_t structure.
 * @return true if received response is for provisioning API false otherwise.
 */
typedef bool (*SPResponseCallback) (const CAEndpoint_t *object,
                                    const CAResponseInfo_t *responseInfo);

/**
 * @brief function to register provisoning API's response callback.
 * @param respHandler response handler callback.
 */
void SRMRegisterProvisioningResponseHandler(SPResponseCallback respHandler);

/**
 * @brief   Check the security resource URI.
 * @param   uri [IN] Pointers to security resource URI.
 * @return  true if the URI is one of security resources, otherwise false.
 */
bool SRMIsSecurityResourceURI(const char* uri);

/**
 * @brief   Sends Response
 * @param   resposeVal       SRMAccessResponse_t value
 * @return  NONE
 */
void SRMSendResponse(SRMAccessResponse_t responseVal);


#ifdef __cplusplus
}
#endif

#endif /* SECURITYRESOURCEMANAGER_H_ */
