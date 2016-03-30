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
#include "cacommon.h"
#include "cainterface.h"
#include "oic_malloc.h"
#include "logger.h"
#include "global.h"
#include "pmtypes.h"
#include "ownershiptransfermanager.h"

#define TAG "OXM_JustWorks"

char* CreateJustWorksSelectOxmPayload(OTMContext_t* otmCtx)
{
    if(!otmCtx || !otmCtx->selectedDeviceInfo)
    {
        return NULL;
    }

    otmCtx->selectedDeviceInfo->doxm->oxmSel = OIC_JUST_WORKS;
    return BinToDoxmJSON(otmCtx->selectedDeviceInfo->doxm);
}

char* CreateJustWorksOwnerTransferPayload(OTMContext_t* otmCtx)
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

OCStackResult LoadSecretJustWorksCallback(OTMContext_t* UNUSED_PARAM)
{
    //In case of 'just works', secret data not required
    (void)UNUSED_PARAM;
    return OC_STACK_OK;
}

OCStackResult CreateSecureSessionJustWorksCallback(OTMContext_t* otmCtx)
{
    OC_LOG(INFO, TAG, "IN CreateSecureSessionJustWorksCallback");
    if(!otmCtx || !otmCtx->selectedDeviceInfo)
    {
        return OC_STACK_INVALID_PARAM;
    }

    CAResult_t caresult = CAEnableAnonECDHCipherSuite(true);
    if (CA_STATUS_OK != caresult)
    {
        OC_LOG_V(ERROR, TAG, "Unable to enable anon cipher suite");
        return OC_STACK_ERROR;
    }
    OC_LOG(INFO, TAG, "Anonymous cipher suite Enabled.");

    caresult  = CASelectCipherSuite(TLS_ECDH_anon_WITH_AES_128_CBC_SHA_256);
    if (CA_STATUS_OK != caresult)
    {
        OC_LOG_V(ERROR, TAG, "Failed to select TLS_ECDH_anon_WITH_AES_128_CBC_SHA_256");
        caresult = CAEnableAnonECDHCipherSuite(false);
        if (CA_STATUS_OK != caresult)
        {
            OC_LOG_V(ERROR, TAG, "Unable to enable anon cipher suite");
        }
        else
        {
            OC_LOG(INFO, TAG, "Anonymous cipher suite Disabled.");
        }
        return OC_STACK_ERROR;
    }
    OC_LOG(INFO, TAG, "TLS_ECDH_anon_WITH_AES_128_CBC_SHA_256 cipher suite selected.");

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

    OC_LOG(INFO, TAG, "OUT CreateSecureSessionJustWorksCallback");
    return OC_STACK_OK;
}
