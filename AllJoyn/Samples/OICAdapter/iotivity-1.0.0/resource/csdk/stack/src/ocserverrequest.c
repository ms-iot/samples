//******************************************************************
//
// Copyright 2014 Intel Mobile Communications GmbH All Rights Reserved.
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
#include "ocserverrequest.h"
#include "ocresourcehandler.h"
#include "oic_malloc.h"
#include "oic_string.h"
#include "ocpayload.h"
#include "ocpayloadcbor.h"
#include "logger.h"

#if defined (ROUTING_GATEWAY) || defined (ROUTING_EP)
#include "routingutility.h"
#endif

#include "cacommon.h"
#include "cainterface.h"

#include "utlist.h"
#include "pdu.h"

// Module Name
#define VERIFY_NON_NULL(arg) { if (!arg) {OC_LOG(FATAL, TAG, #arg " is NULL"); goto exit;} }

#define TAG  "ocserverrequest"

static struct OCServerRequest * serverRequestList = NULL;
static struct OCServerResponse * serverResponseList = NULL;

//-------------------------------------------------------------------------------------------------
// Local functions
//-------------------------------------------------------------------------------------------------

/**
 * Add a server response to the server response list
 *
 * @param response initialized server response that is created by this function
 * @param requestHandle - handle of the response
 *
 * @return
 *     OCStackResult
 */
static OCStackResult AddServerResponse (OCServerResponse ** response, OCRequestHandle requestHandle)
{
    if (!response)
    {
        return OC_STACK_INVALID_PARAM;
    }

    OCServerResponse * serverResponse = NULL;

    serverResponse = (OCServerResponse *) OICCalloc(1, sizeof(OCServerResponse));
    VERIFY_NON_NULL(serverResponse);

    serverResponse->payload = NULL;

    serverResponse->requestHandle = requestHandle;

    *response = serverResponse;
    OC_LOG(INFO, TAG, "Server Response Added!!");
    LL_APPEND (serverResponseList, serverResponse);
    return OC_STACK_OK;

exit:
    if (serverResponse)
    {
        OICFree(serverResponse);
        serverResponse = NULL;
    }
    *response = NULL;
    return OC_STACK_NO_MEMORY;
}

/**
 * Delete a server request from the server request list
 *
 * @param serverRequest - server request to delete
 */
static void DeleteServerRequest(OCServerRequest * serverRequest)
{
    if(serverRequest)
    {
        LL_DELETE(serverRequestList, serverRequest);
        OICFree(serverRequest->requestToken);
        OICFree(serverRequest);
        serverRequest = NULL;
        OC_LOG(INFO, TAG, "Server Request Removed!!");
    }
}

/**
 * Delete a server response from the server response list
 *
 * @param serverResponse - server response to delete
 */
static void DeleteServerResponse(OCServerResponse * serverResponse)
{
    if(serverResponse)
    {
        LL_DELETE(serverResponseList, serverResponse);
        OICFree(serverResponse->payload);
        OICFree(serverResponse);
        OC_LOG(INFO, TAG, "Server Response Removed!!");
    }
}

/**
 * Find a server response and delete it from the server response list
 *
 * @param serverResponse - server response to find and delete
 */
static void FindAndDeleteServerResponse(OCServerResponse * serverResponse)
{
    OCServerResponse* tmp;
    if(serverResponse)
    {
        LL_FOREACH(serverResponseList, tmp)
        {
            if (serverResponse == tmp)
            {
                DeleteServerResponse(tmp);
                return;
            }
        }
    }
}

/**
 * Ensure no accept header option is included when sending responses and add routing info to
 * outgoing response.
 *
 * @param object CA remote endpoint.
 * @param requestInfo CA request info.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
static OCStackResult OCSendResponse(const CAEndpoint_t *object, CAResponseInfo_t *responseInfo)
{
#if defined (ROUTING_GATEWAY) || defined (ROUTING_EP)
    // Add route info in RM option.
    OCStackResult rmResult = RMAddInfo(object->routeData, &(responseInfo->info.options),
                       &(responseInfo->info.numOptions));
    if(OC_STACK_OK != rmResult)
    {
        OC_LOG(ERROR, TAG, "Add option failed");
        return rmResult;
    }
#endif

    // Do not include the accept header option
    responseInfo->info.acceptFormat = CA_FORMAT_UNDEFINED;
    CAResult_t result = CASendResponse(object, responseInfo);
    if(CA_STATUS_OK != result)
    {
        OC_LOG_V(ERROR, TAG, "CASendResponse failed with CA error %u", result);
        return CAResultToOCResult(result);
    }
    return OC_STACK_OK;
}

//-------------------------------------------------------------------------------------------------
// Internal APIs
//-------------------------------------------------------------------------------------------------

/**
 * Get a server request from the server request list using the specified token.
 *
 * @param token - token of server request
 * @param tokenLength - length of token
 *
 * @return
 *     OCServerRequest*
 */
OCServerRequest * GetServerRequestUsingToken (const CAToken_t token, uint8_t tokenLength)
{
    if(!token)
    {
        OC_LOG(ERROR, TAG, "Invalid Parameter Token");
        return NULL;
    }

    OCServerRequest * out = NULL;
    OC_LOG(INFO, TAG,"Get server request with token");
    OC_LOG_BUFFER(INFO, TAG, (const uint8_t *)token, tokenLength);

    OC_LOG(INFO, TAG,"Found token");
    LL_FOREACH (serverRequestList, out)
    {
        OC_LOG_BUFFER(INFO, TAG, (const uint8_t *)out->requestToken, tokenLength);
        if(memcmp(out->requestToken, token, tokenLength) == 0)
        {
            return out;
        }
    }
    OC_LOG(ERROR, TAG, "Server Request not found!!");
    return NULL;
}

/**
 * Get a server request from the server request list using the specified handle
 *
 * @param handle - handle of server request
 * @return
 *     OCServerRequest*
 */
OCServerRequest * GetServerRequestUsingHandle (const OCServerRequest * handle)
{
    OCServerRequest * out = NULL;
    LL_FOREACH (serverRequestList, out)
    {
        if(out == handle)
        {
            return out;
        }
    }
    OC_LOG(ERROR, TAG, "Server Request not found!!");
    return NULL;
}

/**
 * Get a server response from the server response list using the specified handle
 *
 * @param handle - handle of server response
 *
 * @return
 *     OCServerResponse*
 */
OCServerResponse * GetServerResponseUsingHandle (const OCServerRequest * handle)
{
    OCServerResponse * out = NULL;
    LL_FOREACH (serverResponseList, out)
    {
        if(out->requestHandle == handle)
        {
            return out;
        }
    }
    OC_LOG(ERROR, TAG, "Server Response not found!!");
    return NULL;
}

OCStackResult AddServerRequest (OCServerRequest ** request, uint16_t coapID,
        uint8_t delayedResNeeded, uint8_t notificationFlag, OCMethod method,
        uint8_t numRcvdVendorSpecificHeaderOptions, uint32_t observationOption,
        OCQualityOfService qos, char * query,
        OCHeaderOption * rcvdVendorSpecificHeaderOptions,
        uint8_t * payload, CAToken_t requestToken, uint8_t tokenLength,
        char * resourceUrl, size_t reqTotalSize, OCPayloadFormat acceptFormat,
        const OCDevAddr *devAddr)
{
    if (!request)
    {
        return OC_STACK_INVALID_PARAM;
    }

    OCServerRequest * serverRequest = NULL;

    OC_LOG_V(INFO, TAG, "addserverrequest entry!! [%s:%u]", devAddr->addr, devAddr->port);

    serverRequest = (OCServerRequest *) OICCalloc(1, sizeof(OCServerRequest) +
        (reqTotalSize ? reqTotalSize : 1) - 1);
    VERIFY_NON_NULL(devAddr);
    VERIFY_NON_NULL(serverRequest);

    serverRequest->coapID = coapID;
    serverRequest->delayedResNeeded = delayedResNeeded;
    serverRequest->notificationFlag = notificationFlag;

    serverRequest->method = method;
    serverRequest->numRcvdVendorSpecificHeaderOptions = numRcvdVendorSpecificHeaderOptions;
    serverRequest->observationOption = observationOption;
    serverRequest->observeResult = OC_STACK_ERROR;
    serverRequest->qos = qos;
    serverRequest->acceptFormat = acceptFormat;
    serverRequest->ehResponseHandler = HandleSingleResponse;
    serverRequest->numResponses = 1;

    if(query)
    {
        OICStrcpy(serverRequest->query, sizeof(serverRequest->query), query);
    }

    if(rcvdVendorSpecificHeaderOptions)
    {
        memcpy(serverRequest->rcvdVendorSpecificHeaderOptions, rcvdVendorSpecificHeaderOptions,
            MAX_HEADER_OPTIONS * sizeof(OCHeaderOption));
    }
    if(payload && reqTotalSize)
    {
       // destination is at least 1 greater than the source, so a NULL always exists in the
        // last character
        memcpy(serverRequest->payload, payload, reqTotalSize);
        serverRequest->payloadSize = reqTotalSize;
    }

    serverRequest->requestComplete = 0;
    if(requestToken)
    {
        // If tokenLength is zero, the return value depends on the
        // particular library implementation (it may or may not be a null pointer).
        if (tokenLength)
        {
            serverRequest->requestToken = (CAToken_t) OICMalloc(tokenLength);
            VERIFY_NON_NULL(serverRequest->requestToken);
            memcpy(serverRequest->requestToken, requestToken, tokenLength);
        }

    }
    serverRequest->tokenLength = tokenLength;

    if(resourceUrl)
    {
        OICStrcpy(serverRequest->resourceUrl, sizeof(serverRequest->resourceUrl),
            resourceUrl);
    }

    serverRequest->devAddr = *devAddr;

    *request = serverRequest;
    OC_LOG(INFO, TAG, "Server Request Added!!");
    LL_APPEND (serverRequestList, serverRequest);
    return OC_STACK_OK;

exit:
    if (serverRequest)
    {
        OICFree(serverRequest);
        serverRequest = NULL;
    }
    *request = NULL;
    return OC_STACK_NO_MEMORY;
}

OCStackResult FormOCEntityHandlerRequest(
        OCEntityHandlerRequest * entityHandlerRequest,
        OCRequestHandle request,
        OCMethod method,
        OCDevAddr *endpoint,
        OCResourceHandle resource,
        char * queryBuf,
        OCPayloadType payloadType,
        uint8_t * payload,
        size_t payloadSize,
        uint8_t numVendorOptions,
        OCHeaderOption * vendorOptions,
        OCObserveAction observeAction,
        OCObservationId observeID)
{
    if (entityHandlerRequest)
    {
        entityHandlerRequest->resource = (OCResourceHandle) resource;
        entityHandlerRequest->requestHandle = request;
        entityHandlerRequest->method = method;
        entityHandlerRequest->devAddr = *endpoint;
        entityHandlerRequest->query = queryBuf;
        entityHandlerRequest->obsInfo.action = observeAction;
        entityHandlerRequest->obsInfo.obsId = observeID;

        if(payload && payloadSize)
        {
            if(OCParsePayload(&entityHandlerRequest->payload, payloadType,
                        payload, payloadSize) != OC_STACK_OK)
            {
                return OC_STACK_ERROR;
            }
        }
        else
        {
            entityHandlerRequest->payload = NULL;
        }

        entityHandlerRequest->numRcvdVendorSpecificHeaderOptions = numVendorOptions;
        entityHandlerRequest->rcvdVendorSpecificHeaderOptions = vendorOptions;

        return OC_STACK_OK;
    }

    return OC_STACK_INVALID_PARAM;
}

/**
 * Find a server request in the server request list and delete
 *
 * @param serverRequest - server request to find and delete
 */
void FindAndDeleteServerRequest(OCServerRequest * serverRequest)
{
    OCServerRequest* tmp;
    if(serverRequest)
    {
        LL_FOREACH(serverRequestList, tmp)
        {
            if (serverRequest == tmp)
            {
                DeleteServerRequest(tmp);
                return;
            }
        }
    }
}

CAResponseResult_t ConvertEHResultToCAResult (OCEntityHandlerResult result, OCMethod method)
{
    CAResponseResult_t caResult;

    switch (result)
    {
        case OC_EH_OK:
           switch (method)
           {
               case OC_REST_PUT:
               case OC_REST_POST:
                   // This Response Code is like HTTP 204 "No Content" but only used in
                   // response to POST and PUT requests.
                   caResult = CA_CHANGED;
                   break;
               case OC_REST_GET:
                   // This Response Code is like HTTP 200 "OK" but only used in response to
                   // GET requests.
                   caResult = CA_CONTENT;
                   break;
               default:
                   // This should not happen but,
                   // give it a value just in case but output an error
                   caResult = CA_CONTENT;
                   OC_LOG_V(ERROR, TAG, "Unexpected OC_EH_OK return code for method [d].", method);
           }
            break;
        case OC_EH_ERROR:
            caResult = CA_BAD_REQ;
            break;
        case OC_EH_RESOURCE_CREATED:
            caResult = CA_CREATED;
            break;
        case OC_EH_RESOURCE_DELETED:
            caResult = CA_DELETED;
            break;
        case OC_EH_SLOW:
            caResult = CA_CONTENT;
            break;
        case OC_EH_FORBIDDEN:
            caResult = CA_UNAUTHORIZED_REQ;
            break;
        case OC_EH_RESOURCE_NOT_FOUND:
            caResult = CA_NOT_FOUND;
            break;
        default:
            caResult = CA_BAD_REQ;
            break;
    }
    return caResult;
}


/**
 * Handler function for sending a response from a single resource
 *
 * @param ehResponse - pointer to the response from the resource
 *
 * @return
 *     OCStackResult
 */
OCStackResult HandleSingleResponse(OCEntityHandlerResponse * ehResponse)
{
    OCStackResult result = OC_STACK_ERROR;
    CAEndpoint_t responseEndpoint = {.adapter = CA_DEFAULT_ADAPTER};
    CAResponseInfo_t responseInfo = {.result = CA_EMPTY};
    CAHeaderOption_t* optionsPointer = NULL;

    if(!ehResponse || !ehResponse->requestHandle)
    {
        return OC_STACK_ERROR;
    }

    OCServerRequest *serverRequest = (OCServerRequest *)ehResponse->requestHandle;

    CopyDevAddrToEndpoint(&serverRequest->devAddr, &responseEndpoint);

    responseInfo.info.resourceUri = serverRequest->resourceUrl;
    responseInfo.result = ConvertEHResultToCAResult(ehResponse->ehResult, serverRequest->method);

    if(serverRequest->notificationFlag && serverRequest->qos == OC_HIGH_QOS)
    {
        responseInfo.info.type = CA_MSG_CONFIRM;
    }
    else if(serverRequest->notificationFlag && serverRequest->qos != OC_HIGH_QOS)
    {
        responseInfo.info.type = CA_MSG_NONCONFIRM;
    }
    else if(!serverRequest->notificationFlag && !serverRequest->slowFlag &&
            serverRequest->qos == OC_HIGH_QOS)
    {
        responseInfo.info.type = CA_MSG_ACKNOWLEDGE;
    }
    else if(!serverRequest->notificationFlag && serverRequest->slowFlag &&
            serverRequest->qos == OC_HIGH_QOS)
    {
        responseInfo.info.type = CA_MSG_CONFIRM;
    }
    else if(!serverRequest->notificationFlag)
    {
        responseInfo.info.type = CA_MSG_NONCONFIRM;
    }
    else
    {
        OC_LOG(ERROR, TAG, "default responseInfo type is NON");
        responseInfo.info.type = CA_MSG_NONCONFIRM;
    }

    char rspToken[CA_MAX_TOKEN_LEN + 1] = {0};
    responseInfo.info.messageId = serverRequest->coapID;
    responseInfo.info.token = (CAToken_t)rspToken;

    memcpy(responseInfo.info.token, serverRequest->requestToken, serverRequest->tokenLength);
    responseInfo.info.tokenLength = serverRequest->tokenLength;

    if(serverRequest->observeResult == OC_STACK_OK)
    {
        responseInfo.info.numOptions = ehResponse->numSendVendorSpecificHeaderOptions + 1;
    }
    else
    {
        responseInfo.info.numOptions = ehResponse->numSendVendorSpecificHeaderOptions;
    }

    if(responseInfo.info.numOptions > 0)
    {
        responseInfo.info.options = (CAHeaderOption_t *)
                                      OICCalloc(responseInfo.info.numOptions,
                                              sizeof(CAHeaderOption_t));

        if(!responseInfo.info.options)
        {
            OC_LOG(FATAL, TAG, "Memory alloc for options failed");
            return OC_STACK_NO_MEMORY;
        }

        optionsPointer = responseInfo.info.options;

        // TODO: This exposes CoAP specific details.  At some point, this should be
        // re-factored and handled in the CA layer.
        if(serverRequest->observeResult == OC_STACK_OK)
        {
            responseInfo.info.options[0].protocolID = CA_COAP_ID;
            responseInfo.info.options[0].optionID = COAP_OPTION_OBSERVE;
            responseInfo.info.options[0].optionLength = sizeof(uint32_t);
            uint8_t* observationData = (uint8_t*)responseInfo.info.options[0].optionData;
            uint32_t observationOption= serverRequest->observationOption;

            for (size_t i=sizeof(uint32_t); i; --i)
            {
                observationData[i-1] = observationOption & 0xFF;
                observationOption >>=8;
            }

            // Point to the next header option before copying vender specific header options
            optionsPointer += 1;
        }

        if (ehResponse->numSendVendorSpecificHeaderOptions)
        {
            memcpy(optionsPointer, ehResponse->sendVendorSpecificHeaderOptions,
                            sizeof(OCHeaderOption) *
                            ehResponse->numSendVendorSpecificHeaderOptions);
        }
    }
    else
    {
        responseInfo.info.options = NULL;
    }

    responseInfo.isMulticast = false;
    responseInfo.info.payload = NULL;
    responseInfo.info.payloadSize = 0;
    responseInfo.info.payloadFormat = CA_FORMAT_UNDEFINED;

    // Put the JSON prefix and suffix around the payload
    if(ehResponse->payload)
    {
        if (ehResponse->payload->type == PAYLOAD_TYPE_PRESENCE)
        {
            responseInfo.isMulticast = true;
        }
        else
        {
            responseInfo.isMulticast = false;
        }

        switch(serverRequest->acceptFormat)
        {
            case OC_FORMAT_UNDEFINED:
                // No preference set by the client, so default to CBOR then
            case OC_FORMAT_CBOR:
                if((result = OCConvertPayload(ehResponse->payload, &responseInfo.info.payload,
                                &responseInfo.info.payloadSize))
                        != OC_STACK_OK)
                {
                    OC_LOG(ERROR, TAG, "Error converting payload");
                    OICFree(responseInfo.info.options);
                    return result;
                }
                responseInfo.info.payloadFormat = CA_FORMAT_APPLICATION_CBOR;
                break;
            default:
                responseInfo.result = CA_NOT_ACCEPTABLE;
        }
    }

#ifdef WITH_PRESENCE
    CATransportAdapter_t CAConnTypes[] = {
                            CA_ADAPTER_IP,
                            CA_ADAPTER_GATT_BTLE,
                            CA_ADAPTER_RFCOMM_BTEDR

#ifdef RA_ADAPTER
                            , CA_ADAPTER_REMOTE_ACCESS
#endif

#ifdef TCP_ADAPTER
                            , CA_ADAPTER_TCP
#endif
                        };

    size_t size = sizeof(CAConnTypes)/ sizeof(CATransportAdapter_t);

    CATransportAdapter_t adapter = responseEndpoint.adapter;
    // Default adapter, try to send response out on all adapters.
    if (adapter == CA_DEFAULT_ADAPTER)
    {
        adapter =
            (CATransportAdapter_t)(
                CA_ADAPTER_IP           |
                CA_ADAPTER_GATT_BTLE    |
                CA_ADAPTER_RFCOMM_BTEDR

#ifdef RA_ADAP
                | CA_ADAPTER_REMOTE_ACCESS
#endif

#ifdef TCP_ADAPTER
                | CA_ADAPTER_TCP
#endif
            );
    }

    result = OC_STACK_OK;
    OCStackResult tempResult = OC_STACK_OK;

    for(size_t i = 0; i < size; i++ )
    {
        responseEndpoint.adapter = (CATransportAdapter_t)(adapter & CAConnTypes[i]);
        if(responseEndpoint.adapter)
        {
            //The result is set to OC_STACK_OK only if OCSendResponse succeeds in sending the
            //response on all the n/w interfaces else it is set to OC_STACK_ERROR
            tempResult = OCSendResponse(&responseEndpoint, &responseInfo);
        }
        if(OC_STACK_OK != tempResult)
        {
            result = tempResult;
        }
    }
#else

    OC_LOG(INFO, TAG, "Calling OCSendResponse with:");
    OC_LOG_V(INFO, TAG, "\tEndpoint address: %s", responseEndpoint.addr);
    OC_LOG_V(INFO, TAG, "\tEndpoint adapter: %s", responseEndpoint.adapter);
    OC_LOG_V(INFO, TAG, "\tResponse result : %s", responseInfo.result);
    OC_LOG_V(INFO, TAG, "\tResponse for uri: %s", responseInfo.info.resourceUri);

    result = OCSendResponse(&responseEndpoint, &responseInfo);
#endif

    OICFree(responseInfo.info.payload);
    OICFree(responseInfo.info.options);
    //Delete the request
    FindAndDeleteServerRequest(serverRequest);
    return result;
}

/**
 * Handler function for sending a response from multiple resources, such as a collection.
 * Aggregates responses from multiple resource until all responses are received then sends the
 * concatenated response
 *
 * TODO: Need to add a timeout in case a (remote?) resource does not respond
 *
 * @param ehResponse - pointer to the response from the resource
 *
 * @return
 *     OCStackResult
 */
OCStackResult HandleAggregateResponse(OCEntityHandlerResponse * ehResponse)
{
    if(!ehResponse || !ehResponse->payload)
    {
        OC_LOG(ERROR, TAG, "HandleAggregateResponse invalid parameters");
        return OC_STACK_INVALID_PARAM;
    }

    OC_LOG(INFO, TAG, "Inside HandleAggregateResponse");

    OCServerRequest *serverRequest = GetServerRequestUsingHandle((OCServerRequest *)
                                                                 ehResponse->requestHandle);
    OCServerResponse *serverResponse = GetServerResponseUsingHandle((OCServerRequest *)
                                                                    ehResponse->requestHandle);

    OCStackResult stackRet = OC_STACK_ERROR;
    if(serverRequest)
    {
        if(!serverResponse)
        {
            OC_LOG(INFO, TAG, "This is the first response fragment");
            stackRet = AddServerResponse(&serverResponse, ehResponse->requestHandle);
            if (OC_STACK_OK != stackRet)
            {
                OC_LOG(ERROR, TAG, "Error adding server response");
                return stackRet;
            }
            VERIFY_NON_NULL(serverResponse);
        }

        if(ehResponse->payload->type != PAYLOAD_TYPE_REPRESENTATION)
        {
            stackRet = OC_STACK_ERROR;
            OC_LOG(ERROR, TAG, "Error adding payload, as it was the incorrect type");
            goto exit;
        }

        if(!serverResponse->payload)
        {
            serverResponse->payload = ehResponse->payload;
        }
        else
        {
            OCRepPayloadAppend((OCRepPayload*)serverResponse->payload,
                    (OCRepPayload*)ehResponse->payload);
        }


        (serverRequest->numResponses)--;

        if(serverRequest->numResponses == 0)
        {
            OC_LOG(INFO, TAG, "This is the last response fragment");
            ehResponse->payload = serverResponse->payload;
            stackRet = HandleSingleResponse(ehResponse);
            //Delete the request and response
            FindAndDeleteServerRequest(serverRequest);
            FindAndDeleteServerResponse(serverResponse);
        }
        else
        {
            OC_LOG(INFO, TAG, "More response fragments to come");
            stackRet = OC_STACK_OK;
        }
    }
exit:
    return stackRet;
}

