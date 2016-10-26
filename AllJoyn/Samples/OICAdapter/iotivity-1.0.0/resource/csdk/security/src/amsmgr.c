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

#include "oic_malloc.h"
#include "amsmgr.h"
#include "resourcemanager.h"
#include "securevirtualresourcetypes.h"
#include "srmresourcestrings.h"
#include "logger.h"
#include "ocrandom.h"
#include "aclresource.h"
#include "amaclresource.h"
#include "srmutility.h"
#include "base64.h"
#include "secureresourcemanager.h"
#include "doxmresource.h"
#include "policyengine.h"
#include "oic_string.h"
#include "caremotehandler.h"
#include <string.h>

#define TAG "SRM-AMSMGR"


 //Callback for AMS service multicast discovery request.
static OCStackApplicationResult AmsMgrDiscoveryCallback(void *ctx, OCDoHandle handle,
                         OCClientResponse * clientResponse);

//Callback for unicast secured port discovery request.
static OCStackApplicationResult SecurePortDiscoveryCallback(void *ctx, OCDoHandle handle,
                         OCClientResponse * clientResponse);

//Callback for unicast ACL request
static OCStackApplicationResult AmsMgrAclReqCallback(void *ctx, OCDoHandle handle,
    OCClientResponse * clientResponse);


OCStackResult DiscoverAmsService(PEContext_t *context)
{
    OC_LOG(INFO, TAG, "IN DiscoverAmsService");

    OCStackResult ret = OC_STACK_ERROR;
    const char DOXM_DEVICEID_QUERY_FMT[] = "%s?%s=%s";
    char uri[MAX_URI_LENGTH + MAX_QUERY_LENGTH] = {0};
    OCCallbackData cbData = {.context=NULL};
    char base64Buff[B64ENCODE_OUT_SAFESIZE(sizeof(((OicUuid_t*)0)->id)) + 1] = {0};
    uint32_t outLen = 0;
    B64Result b64Ret;

    VERIFY_NON_NULL(TAG, context, ERROR);
    b64Ret = b64Encode(context->amsMgrContext->amsDeviceId.id,
          sizeof(context->amsMgrContext->amsDeviceId.id), base64Buff, sizeof(base64Buff), &outLen);
    VERIFY_SUCCESS(TAG, B64_OK == b64Ret, ERROR);
    snprintf(uri, sizeof(uri), DOXM_DEVICEID_QUERY_FMT, OIC_RSRC_DOXM_URI,
                                       OIC_JSON_DEVICE_ID_NAME, base64Buff);

    cbData.cb = &AmsMgrDiscoveryCallback;
    cbData.context = (void*)context;

    /* TODO
     * If no good response was received for this discovery request,
     * PE would be blocked forever waiting for AMS service to respond with the ACE.
     * Need logic to reset the PE state and send ACCESS_DENIED response,
     * when discovery response from AMS service is not received within certain time.
     */
    OC_LOG_V(INFO, TAG,"AMS Manager Sending Multicast Discovery with URI = %s", uri);
    ret = OCDoResource(NULL, OC_REST_DISCOVER, uri, NULL, NULL,
                       CT_DEFAULT, OC_LOW_QOS, &cbData, NULL, 0);

exit:
    OC_LOG(INFO, TAG, "Leaving DiscoverAmsService");
    return ret;
}


static OCStackApplicationResult AmsMgrDiscoveryCallback(void *ctx, OCDoHandle handle,
                         OCClientResponse * clientResponse)
{
    OC_LOG_V(INFO, TAG, "%s Begin", __func__ );

    if (!ctx ||
        !clientResponse ||
        !clientResponse->payload||
        (PAYLOAD_TYPE_SECURITY != clientResponse->payload->type)||
        (OC_STACK_OK != clientResponse->result))
    {
        OC_LOG_V(ERROR, TAG, "%s Invalid Response ", __func__);
        return OC_STACK_KEEP_TRANSACTION;
    }

    (void)handle;
    PEContext_t *context = (PEContext_t *) ctx;
    if (context->state != AWAITING_AMS_RESPONSE)
    {
        OC_LOG_V(ERROR, TAG, "%s Invalid PE State ", __func__);
        return OC_STACK_DELETE_TRANSACTION;
    }

    OicSecDoxm_t *doxm = NULL;
    OC_LOG_V(INFO, TAG, "Doxm DeviceId Discovery response = %s\n",
          ((OCSecurityPayload*)clientResponse->payload)->securityData);
    doxm = JSONToDoxmBin(((OCSecurityPayload*)clientResponse->payload)->securityData);

    //As doxm is NULL amsmgr can't test if response from trusted AMS service
    //so keep the transaction.
    if(NULL == doxm)
    {
        OC_LOG_V(ERROR, TAG, "%s : Unable to convert JSON to Binary",__func__);
        return OC_STACK_KEEP_TRANSACTION;
    }

    OicUuid_t deviceId = {.id={0}};
    memcpy(&deviceId, &doxm->deviceID, sizeof(deviceId));
    OICFree(doxm);

    /* TODO : By assuming that the first response received is the actual
     * AMS service, a 'bad device' can cause DoS attack.
     */
    if (memcmp(&context->amsMgrContext->amsDeviceId, &deviceId,
            sizeof(context->amsMgrContext->amsDeviceId)) == 0)
    {
        OC_LOG(INFO, TAG, "AMS Manager Sending unicast discovery to get secured port info");
        //Sending Unicast discovery to get secure port information
        if(OC_STACK_OK == SendUnicastSecurePortDiscovery(context, &clientResponse->devAddr,
                clientResponse->connType))
        {
            context->retVal = ACCESS_WAITING_FOR_AMS;
            return OC_STACK_DELETE_TRANSACTION;
        }
    }
    context->retVal = ACCESS_DENIED_AMS_SERVICE_ERROR;
    SRMSendResponse(context->retVal);
    return OC_STACK_DELETE_TRANSACTION;
}


OCStackResult SendUnicastSecurePortDiscovery(PEContext_t *context,OCDevAddr *devAddr,
                                      OCConnectivityType connType)
{
    OC_LOG(INFO, TAG, "IN SendUnicastSecurePortDiscovery");

    const char RES_DOXM_QUERY_FMT[] = "%s?%s=%s";
    OCCallbackData cbData = {.context=NULL};
    char uri[MAX_URI_LENGTH + MAX_QUERY_LENGTH] = {0};
    snprintf(uri, sizeof(uri), RES_DOXM_QUERY_FMT, OC_RSRVD_WELL_KNOWN_URI,
            OC_RSRVD_RESOURCE_TYPE, OIC_RSRC_TYPE_SEC_DOXM);

    cbData.cb = &SecurePortDiscoveryCallback;
    cbData.context = context;

    OC_LOG_V(INFO, TAG, "AMS Manager Sending Unicast Discovery with URI = %s", uri);

    return  OCDoResource(NULL, OC_REST_DISCOVER, uri, devAddr, NULL,
                         connType, OC_LOW_QOS, &cbData, NULL, 0);
}

static OCStackApplicationResult SecurePortDiscoveryCallback(void *ctx, OCDoHandle handle,
                         OCClientResponse * clientResponse)
{
    OC_LOG(INFO, TAG, "In SecurePortDiscoveryCallback");

    if (!ctx ||
        !clientResponse ||
        !clientResponse->payload||
        (PAYLOAD_TYPE_DISCOVERY != clientResponse->payload->type)||
        (OC_STACK_OK != clientResponse->result))
        {
            OC_LOG_V(ERROR, TAG, "%s Invalid Response ", __func__);
            SRMSendResponse(ACCESS_DENIED_AMS_SERVICE_ERROR);
            return OC_STACK_DELETE_TRANSACTION;
        }

    PEContext_t *context = (PEContext_t *) ctx;
    (void)handle;
    if (context->state != AWAITING_AMS_RESPONSE)
    {
        OC_LOG_V(ERROR, TAG, "%s Invalid PE State ", __func__);
        context->retVal = ACCESS_DENIED_AMS_SERVICE_ERROR;
        SRMSendResponse(context->retVal);
        return OC_STACK_DELETE_TRANSACTION;
    }
    OCResourcePayload* resPayload = ((OCDiscoveryPayload*)clientResponse->payload)->resources;

    //Verifying if the ID of the sender is an AMS service that this device trusts.
    if(resPayload &&
       memcmp(context->amsMgrContext->amsDeviceId.id, resPayload->sid,
                    sizeof(context->amsMgrContext->amsDeviceId.id)) != 0)
    {
        context->retVal = ACCESS_DENIED_AMS_SERVICE_ERROR;
        SRMSendResponse(context->retVal);
        return OC_STACK_DELETE_TRANSACTION;
    }

    if (resPayload && resPayload->secure)
    {
        if(OC_STACK_OK == SendAclReq(context, &clientResponse->devAddr, clientResponse->connType,
                resPayload->port))
        {
            return OC_STACK_DELETE_TRANSACTION;
        }
    }
    OC_LOG(INFO, TAG, "Can not find secure port information");
    context->retVal = ACCESS_DENIED_AMS_SERVICE_ERROR;
    SRMSendResponse(context->retVal);
    return OC_STACK_DELETE_TRANSACTION;
}


OCStackResult SendAclReq(PEContext_t *context, OCDevAddr *devAddr, OCConnectivityType connType,
        uint16_t securedPort)
{
    OCStackResult ret = OC_STACK_ERROR;
    const char GET_ACE_QUERY_FMT[] = "%s?%s=%s;%s=%s";
    char base64Buff[B64ENCODE_OUT_SAFESIZE(sizeof(((OicUuid_t*)0)->id)) + 1] = {0};
    uint32_t outLen = 0;
    char uri[MAX_URI_LENGTH + MAX_QUERY_LENGTH] = {0};
    OCCallbackData cbData = {.context=NULL};
    OCDevAddr destAddr = {.adapter = OC_ADAPTER_IP};
    B64Result b64Ret;

    VERIFY_NON_NULL(TAG, context, ERROR);
    VERIFY_NON_NULL(TAG, devAddr, ERROR);

    b64Ret = b64Encode(context->subject.id, sizeof(context->subject.id),
                       base64Buff, sizeof(base64Buff), &outLen);
    VERIFY_SUCCESS(TAG, B64_OK == b64Ret, ERROR);

    snprintf(uri, sizeof(uri), GET_ACE_QUERY_FMT, OIC_RSRC_ACL_URI,
                                    OIC_JSON_SUBJECT_NAME, base64Buff,
                                    OIC_JSON_RESOURCES_NAME, context->resource);

    cbData.cb = &AmsMgrAclReqCallback;
    cbData.context = context;

    destAddr = *devAddr;
    //update port info
    destAddr.flags = (OCTransportFlags)(destAddr.flags | OC_FLAG_SECURE);
    destAddr.port = securedPort;

    OC_LOG_V(INFO, TAG, "AMS Manager Sending Unicast ACL request with URI = %s", uri);
    ret = OCDoResource(NULL, OC_REST_GET, uri, &destAddr, NULL,
            connType, OC_LOW_QOS, &cbData, NULL, 0);

exit:
    OC_LOG_V(INFO, TAG, "%s returns %d ", __func__, ret);
    return ret;
}


static OCStackApplicationResult AmsMgrAclReqCallback(void *ctx, OCDoHandle handle,
    OCClientResponse * clientResponse)
{
    OC_LOG_V(INFO, TAG, "%s Begin", __func__ );

    (void)handle;
    PEContext_t *context = (PEContext_t *) ctx;
    SRMAccessResponse_t rsps;

    if (!ctx ||
        !clientResponse ||
        !clientResponse->payload||
        (PAYLOAD_TYPE_SECURITY != clientResponse->payload->type) ||
        (clientResponse->result != OC_STACK_OK))
    {
        SRMSendResponse(ACCESS_DENIED_AMS_SERVICE_ERROR);
        goto exit;
    }

    if (context->state != AWAITING_AMS_RESPONSE)
    {
        OC_LOG_V(ERROR, TAG, "%s Invalid State ", __func__);
        context->retVal = ACCESS_DENIED_AMS_SERVICE_ERROR;
        SRMSendResponse(context->retVal);
        return OC_STACK_DELETE_TRANSACTION;
    }

    // Verify before installing ACL if the ID of the sender of this ACL is an AMS
    //service that this device trusts.
    rsps = ACCESS_DENIED;
    if((UUID_LENGTH == clientResponse->identity.id_length) &&
        memcmp(context->amsMgrContext->amsDeviceId.id, clientResponse->identity.id,
                       sizeof(context->amsMgrContext->amsDeviceId.id)) == 0)
    {
        OCStackResult ret =
                InstallNewACL(((OCSecurityPayload*)clientResponse->payload)->securityData);
        VERIFY_SUCCESS(TAG, OC_STACK_OK == ret, ERROR);

        OC_LOG_V(INFO, TAG, "%s : Calling checkPermission", __func__);
        rsps = CheckPermission(context, &context->subject, context->resource, context->permission);
        VERIFY_SUCCESS(TAG, (true == IsAccessGranted(rsps)), ERROR);

        OC_LOG_V(INFO, TAG, "%sAccess granted, Calling SRMCallCARequestHandler", __func__);
        context->retVal = ACCESS_GRANTED;
        SRMSendResponse(context->retVal);
        return OC_STACK_DELETE_TRANSACTION;
    }

exit:
    context->retVal = ACCESS_DENIED_AMS_SERVICE_ERROR;
    SRMSendResponse(context->retVal);
    FreeCARequestInfo(context->amsMgrContext->requestInfo);
    OICFree(context->amsMgrContext->endpoint);
    return OC_STACK_DELETE_TRANSACTION;
}


OCStackResult UpdateAmsMgrContext(PEContext_t *context, const CAEndpoint_t *endpoint,
                        const CARequestInfo_t *requestInfo)
{
    OCStackResult ret = OC_STACK_ERROR;

    //The AmsMgr context endpoint and requestInfo will be free from ,
    //AmsMgrAclReqCallback function
    if(context->amsMgrContext->endpoint)
    {
        OICFree(context->amsMgrContext->endpoint);
        context->amsMgrContext->endpoint = NULL;
    }
    context->amsMgrContext->endpoint = (CAEndpoint_t *)OICCalloc(1, sizeof(CAEndpoint_t ));
    VERIFY_NON_NULL(TAG, context->amsMgrContext->endpoint, ERROR);
    *context->amsMgrContext->endpoint = *endpoint;

    if(context->amsMgrContext->requestInfo)
    {
        FreeCARequestInfo(context->amsMgrContext->requestInfo);
        context->amsMgrContext->requestInfo = NULL;
    }
    context->amsMgrContext->requestInfo = CACloneRequestInfo(requestInfo);
    VERIFY_NON_NULL(TAG, context->amsMgrContext->requestInfo, ERROR);
    ret = OC_STACK_OK;
exit:
    return ret;
}

void FreeCARequestInfo(CARequestInfo_t *requestInfo)
{
    OICFree(requestInfo->info.token);
    OICFree(requestInfo->info.options);
    OICFree(requestInfo->info.payload);
    OICFree(requestInfo->info.resourceUri);
    OICFree(requestInfo);
}


//This method checks for Amacl resource. If Amacl is found then it fills up
//context->amsMgrContext->amsDeviceId with amsID of the Amacl else leaves it empty.
bool FoundAmaclForRequest(PEContext_t *context)
{
    OC_LOG_V(INFO, TAG, "%s:no ACL found. Searching for AMACL",__func__);

    bool ret = false;
    VERIFY_NON_NULL(TAG, context, ERROR);
    memset(&context->amsMgrContext->amsDeviceId, 0, sizeof(context->amsMgrContext->amsDeviceId));

    //Call amacl resource function to get the AMS service deviceID for the resource
    if(OC_STACK_OK == AmaclGetAmsDeviceId(context->resource, &context->amsMgrContext->amsDeviceId))
    {
        OC_LOG_V(INFO, TAG, "%s:AMACL found for the requested resource %s",
                __func__, context->resource);
        ret = true;
    }
    else
    {
        OC_LOG_V(INFO, TAG, "%s:AMACL found for the requested resource %s",
                __func__, context->resource);
        ret = false;
    }

 exit:
     return ret;
}


void ProcessAMSRequest(PEContext_t *context)
{
    OicUuid_t  emptyUuid = {.id={0}};
    OC_LOG_V(INFO, TAG, "Entering %s", __func__);
    if(NULL != context)
    {
        if((false == context->matchingAclFound) && (false == context->amsProcessing))
        {
            context->amsProcessing = true;

            //Checking if context AMS deviceId is empty
            if(memcmp(&context->amsMgrContext->amsDeviceId, &emptyUuid, sizeof(OicUuid_t)) != 0 )
            {
                if(OC_STACK_OK == DiscoverAmsService(context))
                {
                    context->retVal = ACCESS_WAITING_FOR_AMS;
                }
                else
                {
                    context->retVal = ACCESS_DENIED_AMS_SERVICE_ERROR;
                }
            }
        }
    }
    else
    {
        OC_LOG_V(INFO, TAG, "Leaving %s(context is NULL)", __func__);
    }

    if(ACCESS_WAITING_FOR_AMS == context->retVal )
    {
        OC_LOG_V(INFO, TAG, "Leaving %s(WAITING_FOR_AMS)", __func__);
    }
}
