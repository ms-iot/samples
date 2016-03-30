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

#include <string.h>
#include "ocstack.h"
#include "logger.h"
#include "cainterface.h"
#include "resourcemanager.h"
#include "credresource.h"
#include "policyengine.h"
#include "srmutility.h"
#include "amsmgr.h"
#include "oic_string.h"
#include "oic_malloc.h"
#include "securevirtualresourcetypes.h"
#include "secureresourcemanager.h"
#include "srmresourcestrings.h"

#define TAG  "SRM"

#ifdef __WITH_X509__
#include "crlresource.h"
#endif // __WITH_X509__

//Request Callback handler
static CARequestCallback gRequestHandler = NULL;
//Response Callback handler
static CAResponseCallback gResponseHandler = NULL;
//Error Callback handler
static CAErrorCallback gErrorHandler = NULL;
//Persistent Storage callback handler for open/read/write/close/unlink
static OCPersistentStorage *gPersistentStorageHandler =  NULL;
//Provisioning response callback
static SPResponseCallback gSPResponseHandler = NULL;

/**
 * A single global Policy Engine context will suffice as long
 * as SRM is single-threaded.
 */
PEContext_t g_policyEngineContext;

/**
 * @brief function to register provisoning API's response callback.
 * @param respHandler response handler callback.
 */
void SRMRegisterProvisioningResponseHandler(SPResponseCallback respHandler)
{
    gSPResponseHandler = respHandler;
}


static void SRMSendUnAuthorizedAccessresponse(PEContext_t *context)
{
    CAResponseInfo_t responseInfo = {.result = CA_EMPTY};
    memcpy(&responseInfo.info, &(context->amsMgrContext->requestInfo->info),
            sizeof(responseInfo.info));
    responseInfo.info.payload = NULL;
    responseInfo.result = CA_UNAUTHORIZED_REQ;
    if (CA_STATUS_OK != CASendResponse(context->amsMgrContext->endpoint, &responseInfo))
    {
        OC_LOG(ERROR, TAG, "Failed in sending response to a unauthorized request!");
    }
    else
    {
        OC_LOG(INFO, TAG, "Succeed in sending response to a unauthorized request!");
    }
}


void SRMSendResponse(SRMAccessResponse_t responseVal)
{
    OC_LOG(DEBUG, TAG, "Sending response to remote device");

    if (IsAccessGranted(responseVal) && gRequestHandler)
    {
        OC_LOG_V(INFO, TAG, "%s : Access granted. Passing Request to RI layer", __func__);
        if (!g_policyEngineContext.amsMgrContext->endpoint ||
                !g_policyEngineContext.amsMgrContext->requestInfo)
        {
            OC_LOG_V(ERROR, TAG, "%s : Invalid arguments", __func__);
            SRMSendUnAuthorizedAccessresponse(&g_policyEngineContext);
            goto exit;
        }
        gRequestHandler(g_policyEngineContext.amsMgrContext->endpoint,
                g_policyEngineContext.amsMgrContext->requestInfo);
    }
    else
    {
        OC_LOG_V(INFO, TAG, "%s : ACCESS_DENIED.", __func__);
        SRMSendUnAuthorizedAccessresponse(&g_policyEngineContext);
    }

exit:
    //Resting PE state to AWAITING_REQUEST
    SetPolicyEngineState(&g_policyEngineContext, AWAITING_REQUEST);
}


/**
 * @brief   Handle the request from the SRM.
 * @param   endPoint       [IN] Endpoint object from which the response is received.
 * @param   requestInfo    [IN] Information for the request.
 * @return  NONE
 */
void SRMRequestHandler(const CAEndpoint_t *endPoint, const CARequestInfo_t *requestInfo)
{
    OC_LOG(DEBUG, TAG, "Received request from remote device");

    if (!endPoint || !requestInfo)
    {
        OC_LOG(ERROR, TAG, "Invalid arguments");
        return;
    }

    // Copy the subjectID
    OicUuid_t subjectId = {.id = {0}};
    memcpy(subjectId.id, requestInfo->info.identity.id, sizeof(subjectId.id));

    //Check the URI has the query and skip it before checking the permission
    char *uri = strstr(requestInfo->info.resourceUri, "?");
    int position = 0;
    if (uri)
    {
        //Skip query and pass the resource uri
        position = uri - requestInfo->info.resourceUri;
    }
    else
    {
        position = strlen(requestInfo->info.resourceUri);
    }
    if (MAX_URI_LENGTH < position  || 0 > position)
    {
        OC_LOG(ERROR, TAG, "Incorrect URI length");
        return;
    }
    SRMAccessResponse_t response = ACCESS_DENIED;
    char newUri[MAX_URI_LENGTH + 1];
    OICStrcpyPartial(newUri, MAX_URI_LENGTH + 1, requestInfo->info.resourceUri, position);

    //New request are only processed if the policy engine state is AWAITING_REQUEST.
    if(AWAITING_REQUEST == g_policyEngineContext.state)
    {
        OC_LOG_V(DEBUG, TAG, "Processing request with uri, %s for method, %d",
                requestInfo->info.resourceUri, requestInfo->method);
        response = CheckPermission(&g_policyEngineContext, &subjectId, newUri,
                GetPermissionFromCAMethod_t(requestInfo->method));
    }
    else
    {
        OC_LOG_V(INFO, TAG, "PE state %d. Ignoring request with uri, %s for method, %d",
                g_policyEngineContext.state, requestInfo->info.resourceUri, requestInfo->method);
    }

    if (IsAccessGranted(response) && gRequestHandler)
    {
        return (gRequestHandler(endPoint, requestInfo));
    }

    // Form a 'Error', 'slow response' or 'access deny' response and send to peer
    CAResponseInfo_t responseInfo = {.result = CA_EMPTY};
    memcpy(&responseInfo.info, &(requestInfo->info), sizeof(responseInfo.info));
    responseInfo.info.payload = NULL;

    VERIFY_NON_NULL(TAG, gRequestHandler, ERROR);

    if(ACCESS_WAITING_FOR_AMS == response)
    {
        OC_LOG(INFO, TAG, "Sending slow response");

        UpdateAmsMgrContext(&g_policyEngineContext, endPoint, requestInfo);
        responseInfo.result = CA_EMPTY;
        responseInfo.info.type = CA_MSG_ACKNOWLEDGE;
    }
    else
    {
        /*
         * TODO Enhance this logic more to decide between
         * CA_UNAUTHORIZED_REQ or CA_FORBIDDEN_REQ depending
         * upon SRMAccessResponseReasonCode_t
         */
        OC_LOG(INFO, TAG, "Sending for regular response");
        responseInfo.result = CA_UNAUTHORIZED_REQ;
    }

    if (CA_STATUS_OK != CASendResponse(endPoint, &responseInfo))
    {
        OC_LOG(ERROR, TAG, "Failed in sending response to a unauthorized request!");
    }
    return;
exit:
    responseInfo.result = CA_INTERNAL_SERVER_ERROR;
    if (CA_STATUS_OK != CASendResponse(endPoint, &responseInfo))
    {
        OC_LOG(ERROR, TAG, "Failed in sending response to a unauthorized request!");
    }
}

/**
 * @brief   Handle the response from the SRM.
 * @param   endPoint     [IN] The remote endpoint.
 * @param   responseInfo [IN] Response information from the endpoint.
 * @return  NONE
 */
void SRMResponseHandler(const CAEndpoint_t *endPoint, const CAResponseInfo_t *responseInfo)
{
    OC_LOG(DEBUG, TAG, "Received response from remote device");

    // isProvResponse flag is to check whether response is catered by provisioning APIs or not.
    // When token sent by CA response matches with token generated by provisioning request,
    // gSPResponseHandler returns true and response is not sent to RI layer. In case
    // gSPResponseHandler is null and isProvResponse is false response then the response is for
    // RI layer.
    bool isProvResponse = false;

    if (gSPResponseHandler)
    {
        isProvResponse = gSPResponseHandler(endPoint, responseInfo);
    }
    if (!isProvResponse && gResponseHandler)
    {
        gResponseHandler(endPoint, responseInfo);
    }
}


/**
 * @brief   Handle the error from the SRM.
 * @param   endPoint  [IN] The remote endpoint.
 * @param   errorInfo [IN] Error information from the endpoint.
 * @return  NONE
 */
void SRMErrorHandler(const CAEndpoint_t *endPoint, const CAErrorInfo_t *errorInfo)
{
    OC_LOG_V(INFO, TAG, "Received error from remote device with result, %d for request uri, %s",
            errorInfo->result, errorInfo->info.resourceUri);
    if (gErrorHandler)
    {
        gErrorHandler(endPoint, errorInfo);
    }
}


/**
 * @brief   Register request and response callbacks.
 *          Requests and responses are delivered in these callbacks.
 * @param   reqHandler   [IN] Request handler callback ( for GET,PUT ..etc)
 * @param   respHandler  [IN] Response handler callback.
 * @return
 *     OC_STACK_OK    - No errors; Success
 *     OC_STACK_INVALID_PARAM - invalid parameter
 */
OCStackResult SRMRegisterHandler(CARequestCallback reqHandler,
                                 CAResponseCallback respHandler,
                                 CAErrorCallback errHandler)
{
    OC_LOG(DEBUG, TAG, "SRMRegisterHandler !!");
    if( !reqHandler || !respHandler || !errHandler)
    {
        OC_LOG(ERROR, TAG, "Callback handlers are invalid");
        return OC_STACK_INVALID_PARAM;
    }
    gRequestHandler = reqHandler;
    gResponseHandler = respHandler;
    gErrorHandler = errHandler;


#if defined(__WITH_DTLS__)
    CARegisterHandler(SRMRequestHandler, SRMResponseHandler, SRMErrorHandler);
#else
    CARegisterHandler(reqHandler, respHandler, errHandler);
#endif /* __WITH_DTLS__ */
    return OC_STACK_OK;
}

/**
 * @brief   Register Persistent storage callback.
 * @param   persistentStorageHandler [IN] Pointers to open, read, write, close & unlink handlers.
 * @return
 *     OC_STACK_OK    - No errors; Success
 *     OC_STACK_INVALID_PARAM - Invalid parameter
 */
OCStackResult SRMRegisterPersistentStorageHandler(OCPersistentStorage* persistentStorageHandler)
{
    OC_LOG(DEBUG, TAG, "SRMRegisterPersistentStorageHandler !!");
    if(!persistentStorageHandler)
    {
        OC_LOG(ERROR, TAG, "The persistent storage handler is invalid");
        return OC_STACK_INVALID_PARAM;
    }
    gPersistentStorageHandler = persistentStorageHandler;
    return OC_STACK_OK;
}

/**
 * @brief   Get Persistent storage handler pointer.
 * @return
 *     The pointer to Persistent Storage callback handler
 */

OCPersistentStorage* SRMGetPersistentStorageHandler()
{
    return gPersistentStorageHandler;
}


/**
 * @brief   Initialize all secure resources ( /oic/sec/cred, /oic/sec/acl, /oic/sec/pstat etc).
 * @retval  OC_STACK_OK for Success, otherwise some error value
 */
OCStackResult SRMInitSecureResources()
{
    // TODO: temporarily returning OC_STACK_OK every time until default
    // behavior (for when SVR DB is missing) is settled.
    InitSecureResources();

#if defined(__WITH_DTLS__)
    CARegisterDTLSCredentialsHandler(GetDtlsPskCredentials);
#endif // (__WITH_DTLS__)
#if defined(__WITH_X509__)
    CARegisterDTLSX509CredentialsHandler(GetDtlsX509Credentials);
    CARegisterDTLSCrlHandler(GetDerCrl);
#endif // (__WITH_X509__)

    return OC_STACK_OK;
}

/**
 * @brief   Perform cleanup for secure resources ( /oic/sec/cred, /oic/sec/acl, /oic/sec/pstat etc).
 * @retval  none
 */
void SRMDeInitSecureResources()
{
    DestroySecureResources();
}

/**
 * @brief   Initialize Policy Engine.
 * @return  OC_STACK_OK for Success, otherwise some error value.
 */
OCStackResult SRMInitPolicyEngine()
{
    return InitPolicyEngine(&g_policyEngineContext);
}

/**
 * @brief   Cleanup Policy Engine.
 * @return  none
 */
void SRMDeInitPolicyEngine()
{
    return DeInitPolicyEngine(&g_policyEngineContext);
}

/**
 * @brief   Check the security resource URI.
 * @param   uri [IN] Pointers to security resource URI.
 * @return  true if the URI is one of security resources, otherwise false.
 */
bool SRMIsSecurityResourceURI(const char* uri)
{
    if (!uri)
    {
        return false;
    }

    const char *rsrcs[] = {
        OIC_RSRC_SVC_URI,
        OIC_RSRC_AMACL_URI,
        OIC_RSRC_CRL_URI,
        OIC_RSRC_CRED_URI,
        OIC_RSRC_ACL_URI,
        OIC_RSRC_DOXM_URI,
        OIC_RSRC_PSTAT_URI,
    };

    // Remove query from Uri for resource string comparison
    size_t uriLen = strlen(uri);
    char *query = strchr (uri, '?');
    if (query)
    {
        uriLen = query - uri;
    }

    for (size_t i = 0; i < sizeof(rsrcs)/sizeof(rsrcs[0]); i++)
    {
        size_t svrLen = strlen(rsrcs[i]);

        if ((uriLen == svrLen) &&
            (strncmp(uri, rsrcs[i], svrLen) == 0))
        {
            return true;
        }
    }

    return false;
}
