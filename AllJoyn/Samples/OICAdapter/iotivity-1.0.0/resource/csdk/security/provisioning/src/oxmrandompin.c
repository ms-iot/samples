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

#include <memory.h>

#include "ocstack.h"
#include "securevirtualresourcetypes.h"
#include "doxmresource.h"
#include "credresource.h"
#include "cacommon.h"
#include "cainterface.h"
#include "ocrandom.h"
#include "oic_malloc.h"
#include "logger.h"
#include "pbkdf2.h"
#include "global.h"
#include "base64.h"
#include "oxmrandompin.h"
#include "ownershiptransfermanager.h"
#include "pinoxmcommon.h"

#define TAG "OXM_RandomPIN"

char* CreatePinBasedSelectOxmPayload(OTMContext_t* otmCtx)
{
    if(!otmCtx || !otmCtx->selectedDeviceInfo)
    {
        return NULL;
    }

    otmCtx->selectedDeviceInfo->doxm->oxmSel = OIC_RANDOM_DEVICE_PIN;

    OicUuid_t uuidPT = {.id={0}};
    if (OC_STACK_OK != GetDoxmDeviceID(&uuidPT))
    {
        OC_LOG(ERROR, TAG, "Error while retrieving provisioning tool's device ID");
        return NULL;
    }
    memcpy(otmCtx->selectedDeviceInfo->doxm->owner.id, uuidPT.id, UUID_LENGTH);

    return BinToDoxmJSON(otmCtx->selectedDeviceInfo->doxm);
}

char* CreatePinBasedOwnerTransferPayload(OTMContext_t* otmCtx)
{
    if(!otmCtx || !otmCtx->selectedDeviceInfo)
    {
        return NULL;
    }

    OicUuid_t uuidPT = {.id={0}};

    if (OC_STACK_OK != GetDoxmDeviceID(&uuidPT))
    {
        OC_LOG(ERROR, TAG, "Error while retrieving provisioning tool's device ID");
        return NULL;
    }
    memcpy(otmCtx->selectedDeviceInfo->doxm->owner.id, uuidPT.id , UUID_LENGTH);
    otmCtx->selectedDeviceInfo->doxm->owned = true;

    return BinToDoxmJSON(otmCtx->selectedDeviceInfo->doxm);
}

OCStackResult InputPinCodeCallback(OTMContext_t* otmCtx)
{
    if(!otmCtx || !otmCtx->selectedDeviceInfo)
    {
        return OC_STACK_INVALID_PARAM;
    }

    uint8_t pinData[OXM_RANDOM_PIN_SIZE + 1];

    OCStackResult res = InputPin((char*)pinData, OXM_RANDOM_PIN_SIZE + 1);
    if(OC_STACK_OK != res)
    {
        OC_LOG(ERROR, TAG, "Failed to input PIN");
        return res;
    }

    OicUuid_t deviceUUID = {.id={0}};
    if (OC_STACK_OK != GetDoxmDeviceID(&deviceUUID))
    {
        OC_LOG(ERROR, TAG, "Error while retrieving provisioning tool's device ID");
        return OC_STACK_ERROR;
    }

    res = AddTmpPskWithPIN(&otmCtx->selectedDeviceInfo->doxm->deviceID,
                           SYMMETRIC_PAIR_WISE_KEY,
                           (char*)pinData, OXM_RANDOM_PIN_SIZE,
                           1, &deviceUUID, &otmCtx->subIdForPinOxm);
    if(res != OC_STACK_OK)
    {
        OC_LOG_V(ERROR, TAG, "Failed to save the temporal PSK : %d", res);
    }

    return res;
}

OCStackResult CreateSecureSessionRandomPinCallbak(OTMContext_t* otmCtx)
{
    OC_LOG(INFO, TAG, "IN CreateSecureSessionRandomPinCallbak");

    if(!otmCtx || !otmCtx->selectedDeviceInfo)
    {
        return OC_STACK_INVALID_PARAM;
    }

    CAResult_t caresult = CAEnableAnonECDHCipherSuite(false);
    if (CA_STATUS_OK != caresult)
    {
        OC_LOG_V(ERROR, TAG, "Unable to disable anon cipher suite");
        return OC_STACK_ERROR;
    }
    OC_LOG(INFO, TAG, "Anonymous cipher suite disabled.");

    caresult  = CASelectCipherSuite(TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA_256);
    if (CA_STATUS_OK != caresult)
    {
        OC_LOG_V(ERROR, TAG, "Failed to select TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA_256");
        return OC_STACK_ERROR;
    }
    OC_LOG(INFO, TAG, "TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA_256 cipher suite selected.");


    OCProvisionDev_t* selDevInfo = otmCtx->selectedDeviceInfo;
    CAEndpoint_t *endpoint = (CAEndpoint_t *)OICCalloc(1, sizeof (CAEndpoint_t));
    if(NULL == endpoint)
    {
        return OC_STACK_NO_MEMORY;
    }
    memcpy(endpoint,&selDevInfo->endpoint,sizeof(CAEndpoint_t));
    endpoint->port = selDevInfo->securePort;
    caresult = CAInitiateHandshake(endpoint);
    OICFree(endpoint);
    if (CA_STATUS_OK != caresult)
    {
        OC_LOG_V(ERROR, TAG, "DTLS handshake failure.");
        return OC_STACK_ERROR;
    }

    OC_LOG(INFO, TAG, "OUT CreateSecureSessionRandomPinCallbak");

    return OC_STACK_OK;
}
