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

// Defining _POSIX_C_SOURCE macro with 199309L (or greater) as value
// causes header files to expose definitions
// corresponding to the POSIX.1b, Real-time extensions
// (IEEE Std 1003.1b-1993) specification
//
// For this specific file, see use of clock_gettime,
// Refer to http://pubs.opengroup.org/stage7tc1/functions/clock_gettime.html
// and to http://man7.org/linux/man-pages/man2/clock_gettime.2.html
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdbool.h>
#include <string.h>

#include "logger.h"
#include "oic_malloc.h"
#include "oic_string.h"
#include "cacommon.h"
#include "cainterface.h"
#include "base64.h"
#include "cJSON.h"
#include "global.h"

#include "srmresourcestrings.h"
#include "doxmresource.h"
#include "pstatresource.h"
#include "credresource.h"
#include "aclresource.h"
#include "ownershiptransfermanager.h"
#include "securevirtualresourcetypes.h"
#include "oxmjustworks.h"
#include "pmtypes.h"
#include "pmutility.h"
#include "srmutility.h"
#include "provisioningdatabasemanager.h"
#include "oxmrandompin.h"

#define TAG "OTM"

/**
 * Array to store the callbacks for each owner transfer method.
 */
static OTMCallbackData_t g_OTMDatas[OIC_OXM_COUNT];

/**
 * Variable for storing provisioning tool's provisioning capabilities
 * Must be in decreasing order of preference. More prefered method should
 * have lower array index.
 */
static OicSecDpom_t gProvisioningToolCapability[] = { SINGLE_SERVICE_CLIENT_DRIVEN };

/**
 * Number of supported provisioning methods
 * current version supports only one.
 */
static size_t gNumOfProvisioningMethodsPT = 1;

/**
 * Function to getting string of ownership transfer method
 */
static const char* GetOxmString(OicSecOxm_t oxmType)
{
    switch(oxmType)
    {
        case OIC_JUST_WORKS:
            return OXM_JUST_WORKS;
        case OIC_RANDOM_DEVICE_PIN:
            return OXM_RANDOM_DEVICE_PIN;
        case OIC_MANUFACTURER_CERTIFICATE:
            return OXM_MANUFACTURER_CERTIFICATE;
        default:
            return NULL;
    }
}

/**
 * Function to select appropriate  provisioning method.
 *
 * @param[in] supportedMethods   Array of supported methods
 * @param[in] numberOfMethods   number of supported methods
 * @param[out]  selectedMethod         Selected methods
 * @return  OC_STACK_OK on success
 */
static OCStackResult SelectProvisioningMethod(const OicSecOxm_t *supportedMethods,
                                                            size_t numberOfMethods,
                                                            OicSecOxm_t *selectedMethod)
{
    OC_LOG(DEBUG, TAG, "IN SelectProvisioningMethod");

    if(numberOfMethods == 0 || !supportedMethods)
    {
        OC_LOG(WARNING, TAG, "Could not find a supported OxM.");
        return OC_STACK_ERROR;
    }

    *selectedMethod  = supportedMethods[0];
    for(size_t i = 0; i < numberOfMethods; i++)
    {
        if(*selectedMethod < supportedMethods[i])
        {
            *selectedMethod =  supportedMethods[i];
        }
    }

    return OC_STACK_OK;
}

/**
 * Function to select operation mode.This function will return most secure common operation mode.
 *
 * @param[in] selectedDeviceInfo   selected device information to performing provisioning.
 * @param[out]   selectedMode   selected operation mode
 * @return  OC_STACK_OK on success
 */
static void SelectOperationMode(const OCProvisionDev_t *selectedDeviceInfo,
                                OicSecDpom_t *selectedMode)
{
    OC_LOG(DEBUG, TAG, "IN SelectOperationMode");

    size_t i = 0;
    size_t j = 0;

    while (i < gNumOfProvisioningMethodsPT && j < selectedDeviceInfo->pstat->smLen)
    {
        if (gProvisioningToolCapability[i] < selectedDeviceInfo->pstat->sm[j])
        {
            i++;
        }
        else if (selectedDeviceInfo->pstat->sm[j] < gProvisioningToolCapability[i])
        {
            j++;
        }
        else /* if gProvisioningToolCapability[i] == deviceSupportedMethods[j] */
        {
            *selectedMode = gProvisioningToolCapability[j];
            break;
        }
    }
    OC_LOG(DEBUG, TAG, "OUT SelectOperationMode");
}

/**
 * Function to update owner transfer mode
 *
 * @param[in]  otmCtx  Context value of ownership transfer.
 * @return  OC_STACK_OK on success
 */
static OCStackResult PutOwnerTransferModeToResource(OTMContext_t* otmCtx);

/**
 * Function to send request to resource to get its pstat resource information.
 *
 * @param[in]  otmCtx  Context value of ownership transfer.
 * @return  OC_STACK_OK on success
 */
static OCStackResult GetProvisioningStatusResource(OTMContext_t* otmCtx);


/**
 * Function to send ownerShip info. This function would update Owned as true and
 * owner as UUID for provisioning tool
 *
 * @param[in]  otmCtx  Context value of ownership transfer.
 * @return  OC_STACK_OK on success
 */
static OCStackResult PutOwnershipInformation(OTMContext_t* otmCtx);

/**
 * Function to update the operation mode. As per the spec. Operation mode in client driven
 * single service provisioning it will be updated to 0x3
 *
 * @param[in]  otmCtx  Context value of ownership transfer.
 * @param[in] selectedOperationMode selected operation mode
 * @return  OC_STACK_OK on success
 */
static OCStackResult PutUpdateOperationMode(OTMContext_t* otmCtx,
                                    OicSecDpom_t selectedOperationMode);

/**
 * Function to start ownership transfer.
 * This function will send the first request for provisioning,
 * The next request message is sent from the response handler for this request.
 *
 * @param[in] ctx   context value passed to callback from calling function.
 * @param[in] selectedDevice   selected device information to performing provisioning.
 * @return  OC_STACK_OK on success
 */
static OCStackResult StartOwnershipTransfer(void* ctx, OCProvisionDev_t* selectedDevice);

/*
 * Function to finalize provisioning.
 * This function will send default ACL and commit hash.
 *
 * @param[in] otmCtx   Context value of ownership transfer.
 * @return  OC_STACK_OK on success
 */
static OCStackResult FinalizeProvisioning(OTMContext_t* otmCtx);


static bool IsComplete(OTMContext_t* otmCtx)
{
    for(size_t i = 0; i < otmCtx->ctxResultArraySize; i++)
    {
        if(OC_STACK_CONTINUE == otmCtx->ctxResultArray[i].res)
        {
            return false;
        }
    }

    return true;
}

/**
 * Function to save the result of provisioning.
 *
 * @param[in,out] otmCtx   Context value of ownership transfer.
 * @param[in] res   result of provisioning
 */
static void SetResult(OTMContext_t* otmCtx, const OCStackResult res)
{
    OC_LOG(DEBUG, TAG, "IN SetResult");

    if(!otmCtx)
    {
        OC_LOG(WARNING, TAG, "OTMContext is NULL");
        return;
    }

    if(otmCtx->selectedDeviceInfo)
    {
        for(size_t i = 0; i < otmCtx->ctxResultArraySize; i++)
        {
            if(memcmp(otmCtx->selectedDeviceInfo->doxm->deviceID.id,
                      otmCtx->ctxResultArray[i].deviceId.id, UUID_LENGTH) == 0)
            {
                otmCtx->ctxResultArray[i].res = res;
                if(OC_STACK_OK != res)
                {
                    otmCtx->ctxHasError = true;
                }
            }
        }

        //If all request is completed, invoke the user callback.
        if(IsComplete(otmCtx))
        {
            otmCtx->ctxResultCallback(otmCtx->userCtx, otmCtx->ctxResultArraySize,
                                       otmCtx->ctxResultArray, otmCtx->ctxHasError);
            OICFree(otmCtx->ctxResultArray);
            OICFree(otmCtx);
        }
        else
        {
            if(OC_STACK_OK != StartOwnershipTransfer(otmCtx,
                                                     otmCtx->selectedDeviceInfo->next))
            {
                OC_LOG(ERROR, TAG, "Failed to StartOwnershipTransfer");
            }
        }
    }

    OC_LOG(DEBUG, TAG, "OUT SetResult");
}


/**
 * Function to save ownerPSK at provisioning tool end.
 *
 * @param[in] selectedDeviceInfo   selected device information to performing provisioning.
 * @return  OC_STACK_OK on success
 */
static OCStackResult SaveOwnerPSK(OCProvisionDev_t *selectedDeviceInfo)
{
    OC_LOG(DEBUG, TAG, "IN SaveOwnerPSK");

    OCStackResult res = OC_STACK_ERROR;

    CAEndpoint_t endpoint;
    memset(&endpoint, 0x00, sizeof(CAEndpoint_t));
    OICStrcpy(endpoint.addr, MAX_ADDR_STR_SIZE_CA, selectedDeviceInfo->endpoint.addr);
    endpoint.addr[MAX_ADDR_STR_SIZE_CA - 1] = '\0';
    endpoint.port = selectedDeviceInfo->securePort;

    OicUuid_t ptDeviceID = {.id={0}};
    if (OC_STACK_OK != GetDoxmDeviceID(&ptDeviceID))
    {
        OC_LOG(ERROR, TAG, "Error while retrieving provisioning tool's device ID");
        return res;
    }

    uint8_t ownerPSK[OWNER_PSK_LENGTH_128] = {0};

    //Generating OwnerPSK
    CAResult_t pskRet = CAGenerateOwnerPSK(&endpoint,
            (uint8_t *)GetOxmString(selectedDeviceInfo->doxm->oxmSel),
            strlen(GetOxmString(selectedDeviceInfo->doxm->oxmSel)), ptDeviceID.id,
            sizeof(ptDeviceID.id), selectedDeviceInfo->doxm->deviceID.id,
            sizeof(selectedDeviceInfo->doxm->deviceID.id), ownerPSK,
            OWNER_PSK_LENGTH_128);

    if (CA_STATUS_OK == pskRet)
    {
        OC_LOG(INFO, TAG,"ownerPSK dump:\n");
        OC_LOG_BUFFER(INFO, TAG,ownerPSK, OWNER_PSK_LENGTH_128);
        //Generating new credential for provisioning tool
        size_t ownLen = 1;
        uint32_t outLen = 0;

        char base64Buff[B64ENCODE_OUT_SAFESIZE(sizeof(ownerPSK)) + 1] = {};
        B64Result b64Ret = b64Encode(ownerPSK, sizeof(ownerPSK), base64Buff, sizeof(base64Buff),
                &outLen);
        VERIFY_SUCCESS(TAG, B64_OK == b64Ret, ERROR);

        OicSecCred_t *cred = GenerateCredential(&selectedDeviceInfo->doxm->deviceID,
                SYMMETRIC_PAIR_WISE_KEY, NULL,
                base64Buff, ownLen, &ptDeviceID);
        VERIFY_NON_NULL(TAG, cred, ERROR);

        res = AddCredential(cred);
        if(res != OC_STACK_OK)
        {
            DeleteCredList(cred);
            return res;
        }
    }
    else
    {
        OC_LOG(ERROR, TAG, "CAGenerateOwnerPSK failed");
    }

    OC_LOG(DEBUG, TAG, "OUT SaveOwnerPSK");
exit:
    return res;
}

/**
 * Callback handler for OwnerShipTransferModeHandler API.
 *
 * @param[in] ctx             ctx value passed to callback from calling function.
 * @param[in] UNUSED          handle to an invocation
 * @param[in] clientResponse  Response from queries to remote servers.
 * @return  OC_STACK_DELETE_TRANSACTION to delete the transaction
 *          and  OC_STACK_KEEP_TRANSACTION to keep it.
 */
static OCStackApplicationResult OwnerTransferModeHandler(void *ctx, OCDoHandle UNUSED,
                                                         OCClientResponse *clientResponse)
{
    OC_LOG(DEBUG, TAG, "IN OwnerTransferModeHandler");

    VERIFY_NON_NULL(TAG, clientResponse, WARNING);
    VERIFY_NON_NULL(TAG, ctx, WARNING);

    OTMContext_t* otmCtx = (OTMContext_t*)ctx;
    (void)UNUSED;
    if(clientResponse->result == OC_STACK_OK)
    {
        OC_LOG(INFO, TAG, "OwnerTransferModeHandler : response result = OC_STACK_OK");
        //Send request : GET /oic/sec/pstat
        OCStackResult res = GetProvisioningStatusResource(otmCtx);
        if(OC_STACK_OK != res)
        {
            OC_LOG(WARNING, TAG, "Failed to get pstat information");
            SetResult(otmCtx, res);
        }
    }
    else
    {
        OC_LOG_V(WARNING, TAG, "OwnerTransferModeHandler : Client response is incorrect : %d",
        clientResponse->result);
        SetResult(otmCtx, clientResponse->result);
    }

    OC_LOG(DEBUG, TAG, "OUT OwnerTransferModeHandler");

exit:
    return  OC_STACK_DELETE_TRANSACTION;
}

/**
 * Callback handler for ProvisioningStatusResouceHandler API.
 *
 * @param[in] ctx             ctx value passed to callback from calling function.
 * @param[in] UNUSED          handle to an invocation
 * @param[in] clientResponse  Response from queries to remote servers.
 * @return  OC_STACK_DELETE_TRANSACTION to delete the transaction
 *          and  OC_STACK_KEEP_TRANSACTION to keep it.
 */
static OCStackApplicationResult ListMethodsHandler(void *ctx, OCDoHandle UNUSED,
                                                    OCClientResponse *clientResponse)
{
    OC_LOG(DEBUG, TAG, "IN ListMethodsHandler");

    VERIFY_NON_NULL(TAG, clientResponse, WARNING);
    VERIFY_NON_NULL(TAG, ctx, WARNING);

    OTMContext_t* otmCtx = (OTMContext_t*)ctx;
    (void)UNUSED;
    if  (OC_STACK_OK == clientResponse->result)
    {
        if  (NULL == clientResponse->payload)
        {
            OC_LOG(INFO, TAG, "Skiping Null payload");
            SetResult(otmCtx, OC_STACK_ERROR);
            return OC_STACK_DELETE_TRANSACTION;
        }

        if (PAYLOAD_TYPE_SECURITY != clientResponse->payload->type)
        {
            OC_LOG(INFO, TAG, "Unknown payload type");
            SetResult(otmCtx, OC_STACK_ERROR);
            return OC_STACK_DELETE_TRANSACTION;
        }

        OicSecPstat_t* pstat = JSONToPstatBin(
                ((OCSecurityPayload*)clientResponse->payload)->securityData);
        if(NULL == pstat)
        {
            OC_LOG(ERROR, TAG, "Error while converting json to pstat bin");
            SetResult(otmCtx, OC_STACK_ERROR);
            return OC_STACK_DELETE_TRANSACTION;
        }
        otmCtx->selectedDeviceInfo->pstat = pstat;

        //Select operation mode (Currently supported SINGLE_SERVICE_CLIENT_DRIVEN only)
        OicSecDpom_t selectedOperationMode;
        SelectOperationMode(otmCtx->selectedDeviceInfo, &selectedOperationMode);

        //Send request : PUT /oic/sec/pstat [{"OM":"0x11", .. }]
        OCStackResult res = PutUpdateOperationMode(otmCtx, selectedOperationMode);
        if (OC_STACK_OK != res)
        {
            OC_LOG(ERROR, TAG, "Error while updating operation mode.");
            SetResult(otmCtx, res);
        }
    }
    else
    {
        OC_LOG_V(WARNING, TAG, "ListMethodsHandler : Client response is incorrect : %d",
            clientResponse->result);
        SetResult(otmCtx, clientResponse->result);
    }

    OC_LOG(DEBUG, TAG, "OUT ListMethodsHandler");
exit:
    return  OC_STACK_DELETE_TRANSACTION;
}

/**
 * Callback handler for OwnershipInformationHandler API.
 *
 * @param[in] ctx             ctx value passed to callback from calling function.
 * @param[in] UNUSED          handle to an invocation
 * @param[in] clientResponse  Response from queries to remote servers.
 * @return  OC_STACK_DELETE_TRANSACTION to delete the transaction
 *          and  OC_STACK_KEEP_TRANSACTION to keep it.
 */
static OCStackApplicationResult OwnershipInformationHandler(void *ctx, OCDoHandle UNUSED,
                                OCClientResponse *clientResponse)
{
    VERIFY_NON_NULL(TAG, clientResponse, WARNING);
    VERIFY_NON_NULL(TAG, ctx, WARNING);

    OC_LOG(DEBUG, TAG, "IN OwnershipInformationHandler");
    (void)UNUSED;
    OCStackResult res = OC_STACK_OK;
    OTMContext_t* otmCtx = (OTMContext_t*)ctx;
    if  (OC_STACK_OK == clientResponse->result)
    {
        if(OIC_RANDOM_DEVICE_PIN == otmCtx->selectedDeviceInfo->doxm->oxmSel)
        {
            res = RemoveCredential(&otmCtx->subIdForPinOxm);
            if(OC_STACK_RESOURCE_DELETED != res)
            {
                OC_LOG_V(ERROR, TAG, "Failed to remove temporal PSK : %d", res);
                return OC_STACK_DELETE_TRANSACTION;
            }
        }

        res = SaveOwnerPSK(otmCtx->selectedDeviceInfo);
        if(OC_STACK_OK != res)
        {
            OC_LOG(ERROR, TAG, "OperationModeUpdate : Failed to owner PSK generation");
            SetResult(otmCtx, res);
            return OC_STACK_DELETE_TRANSACTION;
        }

        CAEndpoint_t* endpoint = (CAEndpoint_t *)&otmCtx->selectedDeviceInfo->endpoint;
        endpoint->port = otmCtx->selectedDeviceInfo->securePort;
        CAResult_t caResult = CACloseDtlsSession(endpoint);
        if(CA_STATUS_OK != caResult)
        {
            OC_LOG(ERROR, TAG, "Failed to close DTLS session");
            SetResult(otmCtx, caResult);
            return OC_STACK_DELETE_TRANSACTION;
        }

        /**
         * If we select NULL cipher,
         * client will select appropriate cipher suite according to server's cipher-suite list.
         */
        caResult = CASelectCipherSuite(TLS_NULL_WITH_NULL_NULL);
        if(CA_STATUS_OK != caResult)
        {
            OC_LOG(ERROR, TAG, "Failed to select TLS_NULL_WITH_NULL_NULL");
            SetResult(otmCtx, caResult);
            return OC_STACK_DELETE_TRANSACTION;
        }

        OC_LOG(INFO, TAG, "Ownership transfer was successfully completed.");
        OC_LOG(INFO, TAG, "Start defualt ACL & commit-hash provisioning.");

        res = FinalizeProvisioning(otmCtx);
        if(OC_STACK_OK != res)
        {
            SetResult(otmCtx, res);
        }
    }
    else
    {
        res = clientResponse->result;
    }

    OC_LOG(DEBUG, TAG, "OUT OwnershipInformationHandler");

exit:
    return  OC_STACK_DELETE_TRANSACTION;
}

/**
 * Response handler for update operation mode.
 *
 * @param[in] ctx             ctx value passed to callback from calling function.
 * @param[in] UNUSED          handle to an invocation
 * @param[in] clientResponse  Response from queries to remote servers.
 * @return  OC_STACK_DELETE_TRANSACTION to delete the transaction
 *          and  OC_STACK_KEEP_TRANSACTION to keep it.
 */
static OCStackApplicationResult OperationModeUpdateHandler(void *ctx, OCDoHandle UNUSED,
                                OCClientResponse *clientResponse)
{
    OC_LOG(DEBUG, TAG, "IN OperationModeUpdateHandler");

    VERIFY_NON_NULL(TAG, clientResponse, WARNING);
    VERIFY_NON_NULL(TAG, ctx, WARNING);

    OTMContext_t* otmCtx = (OTMContext_t*)ctx;
    (void) UNUSED;
    if  (OC_STACK_OK == clientResponse->result)
    {
        OCStackResult res = OC_STACK_ERROR;
        OicSecOxm_t selOxm = otmCtx->selectedDeviceInfo->doxm->oxmSel;
        //DTLS Handshake
        //Load secret for temporal secure session.
        if(g_OTMDatas[selOxm].loadSecretCB)
        {
            res = g_OTMDatas[selOxm].loadSecretCB(otmCtx);
            if(OC_STACK_OK != res)
            {
                OC_LOG(ERROR, TAG, "OperationModeUpdate : Failed to load secret");
                SetResult(otmCtx, res);
                return  OC_STACK_DELETE_TRANSACTION;
            }
        }

        //Try DTLS handshake to generate secure session
        if(g_OTMDatas[selOxm].createSecureSessionCB)
        {
            res = g_OTMDatas[selOxm].createSecureSessionCB(otmCtx);
            if(OC_STACK_OK != res)
            {
                OC_LOG(ERROR, TAG, "OperationModeUpdate : Failed to create DTLS session");
                SetResult(otmCtx, res);
                return OC_STACK_DELETE_TRANSACTION;
            }
        }

        //Send request : PUT /oic/sec/doxm [{"Owned":"True", .. , "Owner":"PT's UUID"}]
        res = PutOwnershipInformation(otmCtx);
        if(OC_STACK_OK != res)
        {
            OC_LOG(ERROR, TAG, "OperationModeUpdate : Failed to send owner information");
            SetResult(otmCtx, res);
        }
    }
    else
    {
        OC_LOG(ERROR, TAG, "Error while update operation mode");
        SetResult(otmCtx, clientResponse->result);
    }

    OC_LOG(DEBUG, TAG, "OUT OperationModeUpdateHandler");

exit:
    return  OC_STACK_DELETE_TRANSACTION;
}


static OCStackResult PutOwnerTransferModeToResource(OTMContext_t* otmCtx)
{
    OC_LOG(DEBUG, TAG, "IN PutOwnerTransferModeToResource");

    if(!otmCtx || !otmCtx->selectedDeviceInfo)
    {
        OC_LOG(ERROR, TAG, "Invalid parameters");
        return OC_STACK_INVALID_PARAM;
    }

    OCProvisionDev_t* deviceInfo = otmCtx->selectedDeviceInfo;
    OicSecOxm_t selectedOxm = deviceInfo->doxm->oxmSel;
    char query[MAX_URI_LENGTH + MAX_QUERY_LENGTH] = {0};

    if(!PMGenerateQuery(false,
                        deviceInfo->endpoint.addr, deviceInfo->endpoint.port,
                        deviceInfo->connType,
                        query, sizeof(query), OIC_RSRC_DOXM_URI))
    {
        OC_LOG(ERROR, TAG, "PutOwnerTransferModeToResource : Failed to generate query");
        return OC_STACK_ERROR;
    }
    OC_LOG_V(DEBUG, TAG, "Query=%s", query);
    OCSecurityPayload* secPayload = (OCSecurityPayload*)OICCalloc(1, sizeof(OCSecurityPayload));
    if(!secPayload)
    {
        OC_LOG(ERROR, TAG, "Failed to memory allocation");
        return OC_STACK_NO_MEMORY;
    }
    secPayload->base.type = PAYLOAD_TYPE_SECURITY;
    secPayload->securityData = g_OTMDatas[selectedOxm].createSelectOxmPayloadCB(otmCtx);
    if (NULL == secPayload->securityData)
    {
        OICFree(secPayload);
        OC_LOG(ERROR, TAG, "Error while converting bin to json");
        return OC_STACK_ERROR;
    }
    OC_LOG_V(DEBUG, TAG, "Payload : %s", secPayload->securityData);

    OCCallbackData cbData;
    cbData.cb = &OwnerTransferModeHandler;
    cbData.context = (void *)otmCtx;
    cbData.cd = NULL;
    OCStackResult res = OCDoResource(NULL, OC_REST_PUT, query,
                                     &deviceInfo->endpoint, (OCPayload*)secPayload,
                                     deviceInfo->connType, OC_LOW_QOS, &cbData, NULL, 0);
    if (res != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack resource error");
    }

    OC_LOG(DEBUG, TAG, "OUT PutOwnerTransferModeToResource");

    return res;
}

static OCStackResult GetProvisioningStatusResource(OTMContext_t* otmCtx)
{
    OC_LOG(DEBUG, TAG, "IN GetProvisioningStatusResource");

    if(!otmCtx || !otmCtx->selectedDeviceInfo)
    {
        OC_LOG(ERROR, TAG, "Invailed parameters");
        return OC_STACK_INVALID_PARAM;
    }

    OCProvisionDev_t* deviceInfo = otmCtx->selectedDeviceInfo;
    char query[MAX_URI_LENGTH + MAX_QUERY_LENGTH] = {0};
    if(!PMGenerateQuery(false,
                        deviceInfo->endpoint.addr, deviceInfo->endpoint.port,
                        deviceInfo->connType,
                        query, sizeof(query), OIC_RSRC_PSTAT_URI))
    {
        OC_LOG(ERROR, TAG, "GetProvisioningStatusResource : Failed to generate query");
        return OC_STACK_ERROR;
    }
    OC_LOG_V(DEBUG, TAG, "Query=%s", query);

    OCCallbackData cbData;
    cbData.cb = &ListMethodsHandler;
    cbData.context = (void *)otmCtx;
    cbData.cd = NULL;
    OCStackResult res = OCDoResource(NULL, OC_REST_GET, query, NULL, NULL,
                                     deviceInfo->connType, OC_LOW_QOS, &cbData, NULL, 0);
    if (res != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack resource error");
    }

    OC_LOG(DEBUG, TAG, "OUT GetProvisioningStatusResource");

    return res;
}


static OCStackResult PutOwnershipInformation(OTMContext_t* otmCtx)
{
    OC_LOG(DEBUG, TAG, "IN PutOwnershipInformation");

    if(!otmCtx || !otmCtx->selectedDeviceInfo)
    {
        OC_LOG(ERROR, TAG, "Invailed parameters");
        return OC_STACK_INVALID_PARAM;
    }

    OCProvisionDev_t* deviceInfo = otmCtx->selectedDeviceInfo;
    char query[MAX_URI_LENGTH + MAX_QUERY_LENGTH] = {0};
    if(!PMGenerateQuery(true,
                        deviceInfo->endpoint.addr, deviceInfo->securePort,
                        deviceInfo->connType,
                        query, sizeof(query), OIC_RSRC_DOXM_URI))
    {
        OC_LOG(ERROR, TAG, "PutOwnershipInformation : Failed to generate query");
        return OC_STACK_ERROR;
    }
    OC_LOG_V(DEBUG, TAG, "Query=%s", query);

    //OwnershipInformationHandler
    OicSecOxm_t selOxm = deviceInfo->doxm->oxmSel;
    OCSecurityPayload* secPayload = (OCSecurityPayload*)OICCalloc(1, sizeof(OCSecurityPayload));
    if(!secPayload)
    {
        OC_LOG(ERROR, TAG, "Failed to memory allocation");
        return OC_STACK_NO_MEMORY;
    }
    secPayload->base.type = PAYLOAD_TYPE_SECURITY;
    secPayload->securityData = g_OTMDatas[selOxm].createOwnerTransferPayloadCB(otmCtx);
    if (NULL == secPayload->securityData)
    {
        OICFree(secPayload);
        OC_LOG(ERROR, TAG, "Error while converting doxm bin to json");
        return OC_STACK_INVALID_PARAM;
    }

    OCCallbackData cbData;
    cbData.cb = &OwnershipInformationHandler;
    cbData.context = (void *)otmCtx;
    cbData.cd = NULL;
    OCStackResult res = OCDoResource(NULL, OC_REST_PUT, query, 0, (OCPayload*)secPayload,
                                     deviceInfo->connType, OC_LOW_QOS, &cbData, NULL, 0);
    if (res != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack resource error");
    }

    OC_LOG(DEBUG, TAG, "OUT PutOwnershipInformation");

    return res;
}

static OCStackResult PutUpdateOperationMode(OTMContext_t* otmCtx,
                                    OicSecDpom_t selectedOperationMode)
{
    OC_LOG(DEBUG, TAG, "IN PutUpdateOperationMode");

    if(!otmCtx || !otmCtx->selectedDeviceInfo)
    {
        return OC_STACK_INVALID_PARAM;
    }

    OCProvisionDev_t* deviceInfo = otmCtx->selectedDeviceInfo;
    char query[MAX_URI_LENGTH + MAX_QUERY_LENGTH] = {0};
    if(!PMGenerateQuery(false,
                        deviceInfo->endpoint.addr, deviceInfo->endpoint.port,
                        deviceInfo->connType,
                        query, sizeof(query), OIC_RSRC_PSTAT_URI))
    {
        OC_LOG(ERROR, TAG, "PutUpdateOperationMode : Failed to generate query");
        return OC_STACK_ERROR;
    }
    OC_LOG_V(DEBUG, TAG, "Query=%s", query);

    deviceInfo->pstat->om = selectedOperationMode;

    OCSecurityPayload* secPayload = (OCSecurityPayload*)OICCalloc(1, sizeof(OCSecurityPayload));
    if(!secPayload)
    {
        OC_LOG(ERROR, TAG, "Failed to memory allocation");
        return OC_STACK_NO_MEMORY;
    }
    secPayload->base.type = PAYLOAD_TYPE_SECURITY;
    secPayload->securityData = BinToPstatJSON(deviceInfo->pstat);
    if (NULL == secPayload->securityData)
    {
        OICFree(secPayload);
        OC_LOG(ERROR, TAG, "Error while converting pstat bin to json");
        return OC_STACK_INVALID_PARAM;
    }

    OCCallbackData cbData;
    cbData.cb = &OperationModeUpdateHandler;
    cbData.context = (void *)otmCtx;
    cbData.cd = NULL;
    OCStackResult res = OCDoResource(NULL, OC_REST_PUT, query, 0, (OCPayload*)secPayload,
                                     deviceInfo->connType, OC_LOW_QOS, &cbData, NULL, 0);
    if (res != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack resource error");
    }

    OC_LOG(DEBUG, TAG, "OUT PutUpdateOperationMode");

    return res;
}

static OCStackResult StartOwnershipTransfer(void* ctx, OCProvisionDev_t* selectedDevice)
{
    OC_LOG(INFO, TAG, "IN StartOwnershipTransfer");
    OTMContext_t* otmCtx = (OTMContext_t*)ctx;
    otmCtx->selectedDeviceInfo = selectedDevice;

    //Set to the lowest level OxM, and then find more higher level OxM.
    OCStackResult res = SelectProvisioningMethod(selectedDevice->doxm->oxm,
                                                 selectedDevice->doxm->oxmLen,
                                                 &selectedDevice->doxm->oxmSel);
    if(OC_STACK_OK != res)
    {
        OC_LOG(ERROR, TAG, "Failed to select the provisioning method");
        SetResult(otmCtx, res);
        return res;
    }
    OC_LOG_V(DEBUG, TAG, "Selected provisoning method = %d", selectedDevice->doxm->oxmSel);

    //Send Req: PUT /oic/sec/doxm [{..."OxmSel" :g_OTMDatas[Index of Selected OxM].OXMString,...}]
    res = PutOwnerTransferModeToResource(otmCtx);
    if(OC_STACK_OK != res)
    {
        OC_LOG(WARNING, TAG, "Failed to select the provisioning method");
        SetResult(otmCtx, res);
        return res;
    }

    OC_LOG(INFO, TAG, "OUT StartOwnershipTransfer");

    return res;

}

OCStackResult OTMSetOwnershipTransferCallbackData(OicSecOxm_t oxmType, OTMCallbackData_t* data)
{
    OC_LOG(DEBUG, TAG, "IN OTMSetOwnerTransferCallbackData");

    if(!data)
    {
        OC_LOG(ERROR, TAG, "OTMSetOwnershipTransferCallbackData : Invalid parameters");
        return OC_STACK_INVALID_PARAM;
    }
    if(oxmType >= OIC_OXM_COUNT)
    {
        OC_LOG(INFO, TAG, "Unknow ownership transfer method");
        return OC_STACK_INVALID_PARAM;
    }

    g_OTMDatas[oxmType].loadSecretCB= data->loadSecretCB;
    g_OTMDatas[oxmType].createSecureSessionCB = data->createSecureSessionCB;
    g_OTMDatas[oxmType].createSelectOxmPayloadCB = data->createSelectOxmPayloadCB;
    g_OTMDatas[oxmType].createOwnerTransferPayloadCB = data->createOwnerTransferPayloadCB;

    OC_LOG(DEBUG, TAG, "OUT OTMSetOwnerTransferCallbackData");

    return OC_STACK_OK;
}

/**
 * NOTE : Unowned discovery should be done before performing OTMDoOwnershipTransfer
 */
OCStackResult OTMDoOwnershipTransfer(void* ctx,
                                     OCProvisionDev_t *selectedDevicelist,
                                     OCProvisionResultCB resultCallback)
{
    OC_LOG(DEBUG, TAG, "IN OTMDoOwnershipTransfer");

    if (NULL == selectedDevicelist || NULL == resultCallback )
    {
        return OC_STACK_INVALID_PARAM;
    }

    OTMContext_t* otmCtx = (OTMContext_t*)OICCalloc(1,sizeof(OTMContext_t));
    if(!otmCtx)
    {
        OC_LOG(ERROR, TAG, "Failed to create OTM Context");
        return OC_STACK_NO_MEMORY;
    }
    otmCtx->ctxResultCallback = resultCallback;
    otmCtx->ctxHasError = false;
    otmCtx->userCtx = ctx;
    OCProvisionDev_t* pCurDev = selectedDevicelist;

    //Counting number of selected devices.
    otmCtx->ctxResultArraySize = 0;
    while(NULL != pCurDev)
    {
        otmCtx->ctxResultArraySize++;
        pCurDev = pCurDev->next;
    }

    otmCtx->ctxResultArray =
        (OCProvisionResult_t*)OICCalloc(otmCtx->ctxResultArraySize, sizeof(OCProvisionResult_t));
    if(NULL == otmCtx->ctxResultArray)
    {
        OC_LOG(ERROR, TAG, "OTMDoOwnershipTransfer : Failed to memory allocation");
        OICFree(otmCtx);
        return OC_STACK_NO_MEMORY;
    }
    pCurDev = selectedDevicelist;

    //Fill the device UUID for result array.
    for(size_t devIdx = 0; devIdx < otmCtx->ctxResultArraySize; devIdx++)
    {
        //Checking duplication of Device ID.
        bool isDuplicate = true;
        OCStackResult res = PDMIsDuplicateDevice(&pCurDev->doxm->deviceID, &isDuplicate);
        if (OC_STACK_OK != res)
        {
            OICFree(otmCtx->ctxResultArray);
            OICFree(otmCtx);
            return res;
        }
        if (isDuplicate)
        {
            OC_LOG(ERROR, TAG, "OTMDoOwnershipTransfer : Device ID is duplicated");
            OICFree(otmCtx->ctxResultArray);
            OICFree(otmCtx);
            return OC_STACK_INVALID_PARAM;
        }
        memcpy(otmCtx->ctxResultArray[devIdx].deviceId.id,
               pCurDev->doxm->deviceID.id,
               UUID_LENGTH);
        otmCtx->ctxResultArray[devIdx].res = OC_STACK_CONTINUE;
        pCurDev = pCurDev->next;
    }
    StartOwnershipTransfer(otmCtx, selectedDevicelist);

    OC_LOG(DEBUG, TAG, "OUT OTMDoOwnershipTransfer");
    return OC_STACK_OK;
}

/**
 * Callback handler of SRPFinalizeProvisioning.
 *
 * @param[in] ctx             ctx value passed to callback from calling function.
 * @param[in] UNUSED          handle to an invocation
 * @param[in] clientResponse  Response from queries to remote servers.
 * @return  OC_STACK_DELETE_TRANSACTION to delete the transaction
 *          and OC_STACK_KEEP_TRANSACTION to keep it.
 */
static OCStackApplicationResult FinalizeProvisioningCB(void *ctx, OCDoHandle UNUSED,
                                                       OCClientResponse *clientResponse)
{
    OC_LOG_V(INFO, TAG, "IN FinalizeProvisioningCB.");

    VERIFY_NON_NULL(TAG, clientResponse, ERROR);
    VERIFY_NON_NULL(TAG, ctx, ERROR);

    OTMContext_t* otmCtx = (OTMContext_t*)ctx;
    (void)UNUSED;
    if(OC_STACK_OK == clientResponse->result)
    {
        OCStackResult res = PDMAddDevice(&otmCtx->selectedDeviceInfo->doxm->deviceID);

         if (OC_STACK_OK == res)
         {
                OC_LOG_V(INFO, TAG, "Add device's UUID in PDM_DB");
                SetResult(otmCtx, OC_STACK_OK);
                return OC_STACK_DELETE_TRANSACTION;
         }
         else
         {
              OC_LOG(ERROR, TAG, "Ownership transfer is complete but adding information to DB is failed.");
         }
    }
exit:
    return OC_STACK_DELETE_TRANSACTION;
}

/**
 * Callback handler of default ACL provisioning.
 *
 * @param[in] ctx             ctx value passed to callback from calling function.
 * @param[in] UNUSED          handle to an invocation
 * @param[in] clientResponse  Response from queries to remote servers.
 * @return  OC_STACK_DELETE_TRANSACTION to delete the transaction
 *          and OC_STACK_KEEP_TRANSACTION to keep it.
 */
static OCStackApplicationResult ProvisionDefaultACLCB(void *ctx, OCDoHandle UNUSED,
                                                       OCClientResponse *clientResponse)
{
    OC_LOG_V(INFO, TAG, "IN ProvisionDefaultACLCB.");

    VERIFY_NON_NULL(TAG, clientResponse, ERROR);
    VERIFY_NON_NULL(TAG, ctx, ERROR);

    OTMContext_t* otmCtx = (OTMContext_t*) ctx;
    (void)UNUSED;

    if (OC_STACK_RESOURCE_CREATED == clientResponse->result)
    {
        OC_LOG_V(INFO, TAG, "Staring commit hash task.");
        // TODO hash currently have fixed value 0.
        uint16_t aclHash = 0;
        otmCtx->selectedDeviceInfo->pstat->commitHash = aclHash;
        otmCtx->selectedDeviceInfo->pstat->tm = NORMAL;
        OCSecurityPayload* secPayload = (OCSecurityPayload*)OICCalloc(1, sizeof(OCSecurityPayload));
        if(!secPayload)
        {
            OC_LOG(ERROR, TAG, "Failed to memory allocation");
            return OC_STACK_NO_MEMORY;
        }
        secPayload->base.type = PAYLOAD_TYPE_SECURITY;
        secPayload->securityData = BinToPstatJSON(otmCtx->selectedDeviceInfo->pstat);
        if (NULL == secPayload->securityData)
        {
            OICFree(secPayload);
            SetResult(otmCtx, OC_STACK_INVALID_JSON);
            return OC_STACK_DELETE_TRANSACTION;
        }
        OC_LOG_V(INFO, TAG, "Created payload for commit hash: %s",secPayload->securityData);

        char query[MAX_URI_LENGTH + MAX_QUERY_LENGTH] = {0};
        if(!PMGenerateQuery(true,
                            otmCtx->selectedDeviceInfo->endpoint.addr,
                            otmCtx->selectedDeviceInfo->securePort,
                            otmCtx->selectedDeviceInfo->connType,
                            query, sizeof(query), OIC_RSRC_PSTAT_URI))
        {
            OC_LOG(ERROR, TAG, "ProvisionDefaultACLCB : Failed to generate query");
            return OC_STACK_ERROR;
        }
        OC_LOG_V(DEBUG, TAG, "Query=%s", query);

        OCCallbackData cbData = {.context=NULL, .cb=NULL, .cd=NULL};
        cbData.cb = &FinalizeProvisioningCB;
        cbData.context = (void*)otmCtx;
        cbData.cd = NULL;
        OCStackResult ret = OCDoResource(NULL, OC_REST_PUT, query, 0, (OCPayload*)secPayload,
                otmCtx->selectedDeviceInfo->connType, OC_HIGH_QOS, &cbData, NULL, 0);
        OC_LOG_V(INFO, TAG, "OCDoResource returned: %d",ret);
        if (ret != OC_STACK_OK)
        {
            OC_LOG(ERROR, TAG, "OCStack resource error");
            SetResult(otmCtx, ret);
        }
    }
    else
    {
        OC_LOG_V(INFO, TAG, "Error occured in provisionDefaultACLCB :: %d\n",
                            clientResponse->result);
        SetResult(otmCtx, clientResponse->result);
    }
exit:
    return OC_STACK_DELETE_TRANSACTION;
}


OCStackResult FinalizeProvisioning(OTMContext_t* otmCtx)
{
    OC_LOG(INFO, TAG, "IN FinalizeProvisioning");

    if(!otmCtx)
    {
        OC_LOG(ERROR, TAG, "OTMContext is NULL");
        return OC_STACK_INVALID_PARAM;
    }
    if(!otmCtx->selectedDeviceInfo)
    {
        OC_LOG(ERROR, TAG, "Can't find device information in OTMContext");
        OICFree(otmCtx);
        return OC_STACK_INVALID_PARAM;
    }
    // Provision Default ACL to device
    OicSecAcl_t defaultAcl =
    { {.id={0}},
        1,
        NULL,
        0x001F,
        0,
        NULL,
        NULL,
        1,
        NULL,
        NULL,
    };

    OicUuid_t provTooldeviceID = {.id={0}};
    if (OC_STACK_OK != GetDoxmDeviceID(&provTooldeviceID))
    {
        OC_LOG(ERROR, TAG, "Error while retrieving provisioning tool's device ID");
        SetResult(otmCtx, OC_STACK_ERROR);
        return OC_STACK_ERROR;
    }
    OC_LOG(INFO, TAG, "Retieved deviceID");
    memcpy(defaultAcl.subject.id, provTooldeviceID.id, sizeof(defaultAcl.subject.id));
    char *wildCardResource = "*";
    defaultAcl.resources = &wildCardResource;

    defaultAcl.owners = (OicUuid_t *) OICCalloc(1, UUID_LENGTH);
    if(!defaultAcl.owners)
    {
        OC_LOG(ERROR, TAG, "Failed to memory allocation for default ACL");
        SetResult(otmCtx, OC_STACK_NO_MEMORY);
        return OC_STACK_NO_MEMORY;
    }
    memcpy(defaultAcl.owners->id, provTooldeviceID.id, UUID_LENGTH);
    OC_LOG(INFO, TAG, "Provisioning default ACL");

    OCSecurityPayload* secPayload = (OCSecurityPayload*)OICCalloc(1, sizeof(OCSecurityPayload));
    if(!secPayload)
    {
        OC_LOG(ERROR, TAG, "Failed to memory allocation");
        return OC_STACK_NO_MEMORY;
    }
    secPayload->base.type = PAYLOAD_TYPE_SECURITY;
    secPayload->securityData = BinToAclJSON(&defaultAcl);
    OICFree(defaultAcl.owners);
    if(!secPayload->securityData)
    {
        OICFree(secPayload);
        OC_LOG(INFO, TAG, "FinalizeProvisioning : Failed to BinToAclJSON");
        SetResult(otmCtx, OC_STACK_ERROR);
        return OC_STACK_ERROR;
    }
    OC_LOG_V(INFO, TAG, "Provisioning default ACL : %s",secPayload->securityData);

    char query[MAX_URI_LENGTH + MAX_QUERY_LENGTH] = {0};
    if(!PMGenerateQuery(true,
                        otmCtx->selectedDeviceInfo->endpoint.addr,
                        otmCtx->selectedDeviceInfo->securePort,
                        otmCtx->selectedDeviceInfo->connType,
                        query, sizeof(query), OIC_RSRC_ACL_URI))
    {
        OC_LOG(ERROR, TAG, "FinalizeProvisioning : Failed to generate query");
        return OC_STACK_ERROR;
    }
    OC_LOG_V(DEBUG, TAG, "Query=%s", query);

    OC_LOG_V(INFO, TAG, "Request URI for Provisioning default ACL : %s", query);

    OCCallbackData cbData =  {.context=NULL, .cb=NULL, .cd=NULL};
    cbData.cb = &ProvisionDefaultACLCB;
    cbData.context = (void *)otmCtx;
    cbData.cd = NULL;
    OCStackResult ret = OCDoResource(NULL, OC_REST_POST, query,
            &otmCtx->selectedDeviceInfo->endpoint, (OCPayload*)secPayload,
            otmCtx->selectedDeviceInfo->connType, OC_HIGH_QOS, &cbData, NULL, 0);
    if (OC_STACK_OK != ret)
    {
        SetResult(otmCtx, ret);
        return ret;
    }

    OC_LOG(INFO, TAG, "OUT FinalizeProvisioning");

    return ret;

}

