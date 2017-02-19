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

#ifndef PM_CREDENTIAL_GENERATOR_H
#define PM_CREDENTIAL_GENERATOR_H

#include "ocstack.h"
#include "securevirtualresourcetypes.h"

/**
 * Function to generate credentials according to the type.
 *
 * @param[in]  type           Type of credential.
 * @param[in]  keysize        size of key.
 * @param[in]  ptDeviceId     Device ID of provisioning tool.
 * @param[in]  firstDeviceId  DeviceID of the first device.
 * @param[in]  secondDeviceId DeviceID of the second device.
 * @param[out] firstCred      Generated credential for first device.
 * @param[out] secondCred     Generated credential for second device.
 * @return  OC_STACK_OK on success
 */
OCStackResult PMGeneratePairWiseCredentials(OicSecCredType_t type, size_t keySize,
                                       const OicUuid_t *ptDeviceId,
                                       const OicUuid_t *firstDeviceId,
                                       const OicUuid_t *secondDeviceId,
                                       OicSecCred_t **firstCred,
                                       OicSecCred_t **secondCred);

/**
 * Function to generate certificate credentials.
 *
 * @param[in]  ptDeviceId     Device ID of provisioning tool.
 * @param[in]  deviceId       DeviceID of the device.
 * @param[out] cred           Generated credential for device.
 * @return  OC_STACK_OK on success
 */
OCStackResult PMGenerateCertificateCredentials(const OicUuid_t *ptDeviceId,
                                          const OicUuid_t *deviceId, OicSecCred_t **cred);

#ifdef __cplusplus
}
#endif
#endif //PM_CREDENTIAL_GENERATOR_H
