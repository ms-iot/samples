/* ****************************************************************
 *
 * Copyright 2015 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "routingmanagerinterface.h"
#include "routingmessageparser.h"
#include "routingutility.h"
#include "ocobserve.h"
#include "include/logger.h"
#include "ocrandom.h"

/**
 * Logging tag for module name.
 */
#define TAG "RM_INTERFACE"

/**
 * Name of resource type.
 */
#define GW_RESOURCE_TYPE_NAME "core.gateway"

/**
 * Name of resource interface.
 */
#define GW_RESOURCE_INTF_NAME "oc.mi.def"

/**
 * URI of the resource.
 */
#define GW_RESOURCE_URI "/oic/gateway"

/**
 * Max Number of times to send data considering wifi packet drops.
 */
#define MAX_SEND_DATA 3

/**
 * Pointer to handle of the newly created gateway resource.
 */
static OCResourceHandle g_gateWayHandle = NULL;

/**
 * Discovery callback registered with RI for a Discover Request.
 */
OCStackApplicationResult RMDiscoverGatewayCallback(void* ctx, OCDoHandle handle,
                                                   OCClientResponse * clientResponse);

/**
 * Observe callback registered with RI for a observe Request.
 */
OCStackApplicationResult RMObserveRequestCallback(void* ctx, OCDoHandle handle,
                                                  OCClientResponse * clientResponse);

OCConnectivityType RMGetConnectivityType(OCTransportAdapter adapter)
{
    switch(adapter)
    {
        case OC_ADAPTER_IP:
            return CT_ADAPTER_IP;
        case OC_ADAPTER_GATT_BTLE:
            return CT_ADAPTER_GATT_BTLE;
        case OC_ADAPTER_RFCOMM_BTEDR:
            return CT_ADAPTER_RFCOMM_BTEDR;
        case OC_DEFAULT_ADAPTER:
            break;
        default:
            OC_LOG(DEBUG, TAG, "Default option will be selected");
    }
    return CT_DEFAULT;
}

OCStackResult RMInitGatewayResource()
{
    OC_LOG(DEBUG, TAG, "RMInitGatewayResource IN");

    // Create a Gateway resource
    OCStackResult result = OCCreateResource(&g_gateWayHandle,
                                            GW_RESOURCE_TYPE_NAME,
                                            GW_RESOURCE_INTF_NAME,
                                            GW_RESOURCE_URI,
                                            NULL,
                                            NULL,
                                            OC_OBSERVABLE);

    if (OC_STACK_OK != result)
    {
        OC_LOG_V(ERROR, TAG, "Create resource for gateway failed[%d]", result);
    }

    OC_LOG(DEBUG, TAG, "RMInitGatewayResource OUT");
    return result;
}

OCStackResult RMDiscoverGatewayResource()
{
    OC_LOG(DEBUG, TAG, "RMDiscoverGatewayResource IN");
    OCCallbackData discoverData = {.cb = RMDiscoverGatewayCallback};
    OCStackResult result = OC_STACK_OK;

    result = OCDoResource(NULL, OC_REST_DISCOVER, GW_RESOURCE_URI, 0, 0,
                          CT_ADAPTER_IP | CT_ADAPTER_RFCOMM_BTEDR,
                          OC_LOW_QOS, &discoverData, NULL, 0);

    // Temp fix for packet drops in WIFI.
    for (uint8_t sendData = 0; sendData < MAX_SEND_DATA; sendData++)
    {
        result = OCDoResource(NULL, OC_REST_DISCOVER, GW_RESOURCE_URI, 0, 0,
                              CT_ADAPTER_IP, OC_LOW_QOS, &discoverData, NULL, 0);
        usleep(100000);
    }
    OC_LOG(DEBUG, TAG, "RMDiscoverGatewayResource OUT");
    return result;
}

OCStackApplicationResult RMDiscoverGatewayCallback(void* ctx, OCDoHandle handle,
                                                   OCClientResponse * clientResponse)
{
    OC_LOG(DEBUG, TAG, "RMDiscoverGatewayCallback IN");
    (void)ctx;
    (void)handle;
    if (NULL == clientResponse)
    {
        OC_LOG(DEBUG, TAG, "clientResponse is NULL");
        return OC_STACK_KEEP_TRANSACTION;
    }

    OCStackResult result = RMHandleResponsePayload(&(clientResponse->devAddr),
                                                   (OCRepPayload *)clientResponse->payload);
    if (OC_STACK_OK != result)
    {
        OC_LOG_V(ERROR, TAG, "RMHandleResponsePayload Failed[%d]", result);
    }

    OCRepPayload *payload = NULL;
    // Created payload is freed in the OCDoResource() api.
    result= RMGetGatewayPayload(&payload);
    if (OC_STACK_OK != result)
    {
        OC_LOG_V(ERROR, TAG, "RMGetGatewayPayload Failed[%d]", result);
    }

    RMSendObserveRequest(&(clientResponse->devAddr), payload);

    OC_LOG(DEBUG, TAG, "RMDiscoverGatewayCallback OUT");
    return OC_STACK_KEEP_TRANSACTION;
}

OCStackResult RMSendObserveRequest(const OCDevAddr *devAddr, OCRepPayload *payload)
{
    OC_LOG(DEBUG, TAG, "RMSendObserveRequest IN");
    OC_LOG_V(DEBUG, TAG, "Destination address is %s:%d", devAddr->addr, devAddr->port);
    OCCallbackData observeData = {.cb = RMObserveRequestCallback};
    OC_LOG(DEBUG, TAG, "RMSendObserveRequest OUT");

    return OCDoResource(NULL, OC_REST_OBSERVE, GW_RESOURCE_URI, devAddr, (OCPayload *)payload,
                        RMGetConnectivityType(devAddr->adapter), OC_HIGH_QOS,
                        &observeData, NULL, 0);
}

OCStackResult RMSendDeleteRequest(const OCDevAddr *devAddr, OCRepPayload *payload)
{
    OC_LOG(DEBUG, TAG, "RMSendDeleteRequest IN");
    RM_NULL_CHECK_WITH_RET(payload, TAG, "payload");
    OC_LOG_V(DEBUG, TAG, "Destination address is %s:%d", devAddr->addr, devAddr->port);

    OCCallbackData deleteCb = {.cb = RMDiscoverGatewayCallback};
    OC_LOG(DEBUG, TAG, "RMSendDeleteRequest OUT");
    return OCDoResource(NULL, OC_REST_DELETE, GW_RESOURCE_URI, devAddr, (OCPayload *)payload,
                    RMGetConnectivityType(devAddr->adapter), OC_LOW_QOS,
                    &deleteCb, NULL, 0);
}

OCStackResult RMSendResponse(const OCServerRequest *request, const OCResource *resource,
                             const OCRepPayload *payload)
{
    OC_LOG(DEBUG, TAG, "RMSendResponse IN");
    OCEntityHandlerResponse response = {.ehResult = OC_EH_OK,
                                        .payload = (OCPayload *)payload,
                                        .persistentBufferFlag = 0,
                                        .requestHandle = (OCRequestHandle) request,
                                        .resourceHandle = (OCResourceHandle) resource
                                        };
    OC_LOG(DEBUG, TAG, "RMSendResponse OUT");

    return OCDoResponse(&response);
}

OCStackResult RMSendNotificationForListofObservers(OCObservationId *obsId, uint8_t obsLen,
                                                   const OCRepPayload *payload)
{
    OC_LOG(DEBUG, TAG, "RMSendNotificationForListofObservers IN");
    RM_NULL_CHECK_WITH_RET(obsId, TAG, "obsId");
    RM_NULL_CHECK_WITH_RET(payload, TAG, "payload");
    OCStackResult result = OCNotifyListOfObservers(g_gateWayHandle, obsId, obsLen,
                                                   payload, OC_LOW_QOS);
    OC_LOG_V(DEBUG, TAG, "Result is %d", result);
    OC_LOG(DEBUG, TAG, "RMSendNotificationForListofObservers OUT");
    return result;
}

OCStackApplicationResult RMObserveRequestCallback(void* ctx, OCDoHandle handle,
                                                  OCClientResponse *clientResponse)
{
    OC_LOG(DEBUG, TAG, "RMObserveRequestCallback IN");
    (void)ctx;
    (void)handle;
    if (NULL == clientResponse)
    {
        OC_LOG(DEBUG, TAG, "clientResponse is NULL");
        return OC_STACK_KEEP_TRANSACTION;
    }

    if (OC_STACK_COMM_ERROR == clientResponse->result)
    {
        OC_LOG(DEBUG, TAG, "Received TIMEOUT ERROR");
        return OC_STACK_KEEP_TRANSACTION;
    }

    OCStackResult result = RMHandleResponsePayload(&(clientResponse->devAddr),
                                                   (OCRepPayload *)clientResponse->payload);
    if (OC_STACK_OK != result)
    {
        OC_LOG_V(ERROR, TAG, "RMHandleResponsePayload Failed[%d]", result);
    }

    OC_LOG(DEBUG, TAG, "RMObserveRequestCallback OUT");
    return OC_STACK_KEEP_TRANSACTION;
}

OCStackResult RMAddObserverToStack(const OCServerRequest *request, OCObservationId *obsID)
{
    OC_LOG(DEBUG, TAG, "RMAddObserverToStack IN");
    RM_NULL_CHECK_WITH_RET(request, TAG, "request");
    RM_NULL_CHECK_WITH_RET(obsID, TAG, "obsID");

    OCStackResult result = OC_STACK_OK;
    while (0 == *obsID)
    {
        result = GenerateObserverId(obsID);
    }
    if (OC_STACK_OK != result)
    {
        OC_LOG_V(DEBUG, TAG, "GenerateObserverId failed[%d]", result);
        return result;
    }

    OC_LOG_V(DEBUG, TAG, "Observer ID is %d", *obsID);
    // Add the observer
    result = AddObserver((const char*)(request->resourceUrl),
                (const char *)(request->query),
                *obsID, request->requestToken, request->tokenLength,
                (OCResource *)g_gateWayHandle, request->qos, OC_FORMAT_CBOR,
                &request->devAddr);
    OC_LOG(DEBUG, TAG, "RMAddObserverToStack OUT");
    return result;
}
