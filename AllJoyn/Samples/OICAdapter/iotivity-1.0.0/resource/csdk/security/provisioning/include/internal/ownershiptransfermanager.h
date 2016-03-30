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

#ifndef OTM_OWNERSHIPTRANSFERMANAGER_H_
#define OTM_OWNERSHIPTRANSFERMANAGER_H_

#include "pmtypes.h"
#include "ocstack.h"
#include "octypes.h"
#include "securevirtualresourcetypes.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define OXM_STRING_MAX_LENGTH 32


/**
 * Context for ownership transfer(OT)
 */
typedef struct OTMContext{
    void* userCtx;                         /**< Context for user.*/
    OCProvisionDev_t* selectedDeviceInfo;  /**< Selected device info for OT. */
    OicUuid_t subIdForPinOxm;              /**< Subject Id which uses PIN based OTM. */
    OCProvisionResultCB ctxResultCallback; /**< Function pointer to store result callback. */
    OCProvisionResult_t* ctxResultArray;   /**< Result array having result of all device. */
    size_t ctxResultArraySize;             /**< No of elements in result array. */
    bool ctxHasError;                      /**< Does OT process have any error. */
}OTMContext_t;

/**
 * Do ownership transfer for the unowned devices.
 *
 * @param[in] ctx Application context would be returned in result callback
 * @param[in] selectedDeviceList linked list of ownership transfer candidate devices.
 * @param[in] resultCB Result callback function to be invoked when ownership transfer finished.
 * @return OC_STACK_OK in case of success and other value otherwise.
 */
OCStackResult OTMDoOwnershipTransfer(void* ctx,
                                     OCProvisionDev_t* selectedDeviceList, OCProvisionResultCB resultCB);

/*
 *Callback for load secret for temporal secure session
 *
 * e.g) in case of PIN based, input the pin through this callback
 *       in case of X.509 based, input the certificate through this callback
 */
typedef OCStackResult (*OTMLoadSecret)(OTMContext_t* otmCtx);


/*
 * Callback for create secure channel using secret inputed from OTMLoadSecret callback
 */
typedef OCStackResult (*OTMCreateSecureSession)(OTMContext_t* otmCtx);

/*
 * Callback for creating CoAP payload.
 */
typedef char* (*OTMCreatePayloadCallback)(OTMContext_t* otmCtx);

/**
 * Required callback for performing ownership transfer
 */
typedef struct OTMCallbackData{
    OTMLoadSecret loadSecretCB;
    OTMCreateSecureSession createSecureSessionCB;
    OTMCreatePayloadCallback createSelectOxmPayloadCB;
    OTMCreatePayloadCallback createOwnerTransferPayloadCB;
}OTMCallbackData_t;

/**
 * Set the callbacks for ownership transfer
 *
 * @param[in] oxm Ownership transfer method
 * @param[in] callbackData the implementation of the ownership transfer function for each step.
 * @return OC_STACK_OK in case of success and other value otherwise.
 */
OCStackResult OTMSetOwnershipTransferCallbackData(OicSecOxm_t oxm, OTMCallbackData_t* callbackData);


#ifdef __cplusplus
}
#endif
#endif //OTM_OWNERSHIPTRANSFERMANAGER_H_
