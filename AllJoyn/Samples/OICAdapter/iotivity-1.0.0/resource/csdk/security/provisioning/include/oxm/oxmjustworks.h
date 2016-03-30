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

#ifndef OXM_JUST_WORKS_H_
#define OXM_JUST_WORKS_H_

#include "ocstack.h"
#include "securevirtualresourcetypes.h"
#include "ownershiptransfermanager.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

//
//Declaration of default callback functions for just works OxM.
//

/**
 * In case of just works OxM, no need to implement.
 */
OCStackResult LoadSecretJustWorksCallback(OTMContext_t* UNUSED_PARAM);

/**
 * To establish a secure channel with anonymous cipher suite
 *
 * @param[in] selectedDeviceInfo Selected device infomation
 * @return OC_STACK_SUCCESS in case of success and other value otherwise.
 */
OCStackResult CreateSecureSessionJustWorksCallback(OTMContext_t* otmCtx);

/**
 * Generate payload for select OxM request.
 *
 * @param[in] selectedDeviceInfo Selected device infomation
 * @return DOXM JSON payload including the selected OxM.
 *         NOTE : Returned memory should be deallocated by caller.
 */
char* CreateJustWorksSelectOxmPayload(OTMContext_t* otmCtx);

/**
 * Generate payload for owner transfer request.
 *
 * @param[in] selectedDeviceInfo Selected device infomation
 * @return DOXM JSON payload including the owner information.
 *         NOTE : Returned memory should be deallocated by caller.
 */
char* CreateJustWorksOwnerTransferPayload(OTMContext_t* otmCtx);

#ifdef __cplusplus
}
#endif
#endif //OXM_JUST_WORKS_H_