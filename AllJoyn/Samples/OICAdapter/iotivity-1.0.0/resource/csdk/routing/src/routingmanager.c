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
#include "routingmanager.h"
#include "routingmanagerinterface.h"
#include "routingtablemanager.h"
#include "routingmessageparser.h"
#include "oic_malloc.h"
#include "oic_string.h"
#include "ocrandom.h"
#include "ulinklist.h"
#include "uarraylist.h"
#include "ocstackinternal.h"
#include "include/logger.h"

/**
 * Logging tag for module name.
 */
#define TAG "RM"

/**
 * Tag for printing the logs of forwarding the packet.
 */
#define RM_TAG "RAP"


/**
 * Unique gateway ID generated before hosting a gateway resource.
 */
uint32_t g_GatewayID = 0;

/**
 * Used for assigning unique ID.to endpoint's connected to this gateway
 */
static uint16_t g_EndpointCount = 0;

/**
 * Routing table which holds hop entries of Gateways with routeCost.
 */
static u_linklist_t *g_routingGatewayTable = NULL;

/**
 * List which holds hop entries with Endpoint information.
 */
static u_linklist_t *g_routingEndpointTable = NULL;

/**
 * Current time in microseconds.
 */
static uint64_t g_aliveTime = 0;

/**
 * Time to refresh the table entries.
 */
static uint64_t g_refreshTableTime = 0;

/**
 * Sequence number for the notification.
 */
static uint32_t g_sequenceNumber = 1;

/**
 * To check if the routing table is validated on 25th seconds.
 */
static bool g_isValidated = false;

/**
 * Multi cast Sequence number.
 */
static uint16_t g_mcastsequenceNumber = 1;

/**
 * To check if RM is initialized.
 */
static bool g_isRMInitialized = false;

/**
 * API to handle the GET request received for a Gateway Resource.
 * @param[in]   request     Request Received.
 * @param[in]   resource    Resource handle used for sending the response.
 * @return  ::OC_STACK_OK or Appropriate error code.
 */
OCStackResult RMHandleGETRequest(const OCServerRequest *request, const OCResource *resource);

/**
 * API to handle the OBSERVE request received for a Gateway Resource.
 * @param[in,out]   request    Request Received.
 * @param[in]       resource   Resource handle used for sending the response.
 * @return  ::OC_STACK_OK or Appropriate error code.
 */
OCStackResult RMHandleOBSERVERequest(OCServerRequest *request, const OCResource *resource);

/**
 * API to handle the OBSERVE request received for a Gateway Resource.
 * @param[in,out]   request    Request Received.
 * @param[in]       resource   Resource handle used for sending the response.
 * @return  ::OC_STACK_OK or Appropriate error code.
 */
OCStackResult RMHandleDELETERequest(const OCServerRequest *request, const OCResource *resource);

/**
 * Adds a observer after generating observer ID whenever observe
 * request is received.
 * @param[in,out]   request     Request handle of the Observe request.
 *                              Sets the value of observeResult as OC_STACK_OK
 *                              if observer is added successfully.
 * @param[out]      obsID       Observer ID generated for the observer.
 * @return  ::OC_STACK_OK or Appropriate error code.
 */
OCStackResult RMAddObserver(OCServerRequest *request, OCObservationId *obsID);

/**
 * Send Notification to all the observers.
 * @param[in]   payload                 Payload to be sent in notification.
 * @return  ::OC_STACK_OK or Appropriate error code.
 */
OCStackResult RMSendNotificationToAll(const OCRepPayload *payload);

/**
 * Send Delete request to all the neighbour nodes.
 * @return  NONE.
 */
void RMSendDeleteToNeighbourNodes();

void RMGenerateGatewayID(uint8_t *id, size_t idLen)
{
    OC_LOG(DEBUG, TAG, "RMGenerateGatewayID IN");
    OCFillRandomMem(id, idLen);
    OC_LOG(DEBUG, TAG, "RMGenerateGatewayID OUT");
}
OCStackResult RMInitialize()
{
    OC_LOG(DEBUG, TAG, "RMInitialize IN");
    if (g_isRMInitialized)
    {
        OC_LOG(DEBUG, TAG, "RM already initialized");
        return OC_STACK_OK;
    }

    // Initialize the GatewayResource[/oic/gateway].
    OCStackResult result = RMInitGatewayResource();
    if (OC_STACK_OK != result)
    {
        OC_LOG_V(ERROR, TAG, "RMInitGatewayResource failed[%d]", result);
        return result;
    }

    // Generates a 4 byte Gateway ID.
    RMGenerateGatewayID((uint8_t *)&g_GatewayID, sizeof(g_GatewayID));

    OC_LOG_V(INFO, RM_TAG, "Gateway ID: %u", g_GatewayID);

    // Initialize the Routing table manager.
    result = RTMInitialize(&g_routingGatewayTable, &g_routingEndpointTable);
    if (OC_STACK_OK != result)
    {
        OC_LOG_V(ERROR, TAG, "RTMInitialize failed[%d]", result);
        return result;
    }

    g_isRMInitialized = true;

    // Send a DISCOVER request for the gateway resource.
    result = RMDiscoverGatewayResource();
    if (OC_STACK_OK != result)
    {
        OC_LOG_V(ERROR, TAG, "RMDiscoverGatewayResource failed[%d]", result);
        RTMTerminate(&g_routingGatewayTable, &g_routingEndpointTable);
        return result;
    }

    // Initialize the timer with the current time.
    g_aliveTime = RTMGetCurrentTime();
    g_refreshTableTime = g_aliveTime;

    OC_LOG(DEBUG, TAG, "RMInitialize OUT");
    return result;
}

OCStackResult RMTerminate()
{
    OC_LOG(DEBUG, TAG, "RMTerminate IN");
    if (!g_isRMInitialized)
    {
        OC_LOG(ERROR, TAG, "RM not initialized");
        return OC_STACK_ERROR;
    }
    // Send DELETE request to neighbour nodes
    RMSendDeleteToNeighbourNodes();

    OCStackResult result = RTMTerminate(&g_routingGatewayTable, &g_routingEndpointTable);
    if (OC_STACK_OK != result)
    {
        OC_LOG_V(ERROR, TAG, "CARegisterRoutingMessageHandler failed[%d]", result);
        return result;
    }
    g_isRMInitialized = false;
    OC_LOG(DEBUG, TAG, "RMTerminate OUT");
    return result;
}

OCStackResult RMHandleGatewayRequest(OCServerRequest *request, const OCResource *resource)
{
    OC_LOG(DEBUG, TAG, "RMHandleGatewayRequest IN");

    if (!g_isRMInitialized)
    {
        OC_LOG(ERROR, TAG, "RM not initialized");
        return OC_STACK_ERROR;
    }

    RM_NULL_CHECK_WITH_RET(request, TAG, "request");
    RM_NULL_CHECK_WITH_RET(resource, TAG, "resource");

    OC_LOG_V(DEBUG, TAG, "Received request of method: %d", request->method);

    if (OC_REST_GET == request->method)
    {
        switch((OCObserveAction)request->observationOption)
        {
            case OC_OBSERVE_REGISTER:
                OC_LOG(DEBUG, TAG, "Received OBSERVE request");
                RMHandleOBSERVERequest(request, resource);
                break;
            case OC_OBSERVE_DEREGISTER:
                //TODO: Handle this case
                OC_LOG(DEBUG, TAG, "Received OBSERVE deregister");
                break;
            case OC_OBSERVE_NO_OPTION:
                OC_LOG(DEBUG, TAG, "Received GET request");
                RMHandleGETRequest(request, resource);
                break;
            default:
                OC_LOG(DEBUG, TAG, "Not Supported by Routing Manager");
        }
    }
    else if (OC_REST_DELETE == request->method)
    {
        OC_LOG(DEBUG, TAG, "Received a Delete request");
        RMHandleDELETERequest(request, resource);
    }
    OC_LOG(DEBUG, TAG, "RMHandleGatewayRequest OUT");
    return OC_STACK_OK;
}

OCStackResult RMHandleRequestPayload(OCDevAddr devAddr, const uint8_t *reqPayload,
                                     size_t payloadSize)
{
    OC_LOG(DEBUG, TAG, "RMHandleRequestPayload IN");
    RM_NULL_CHECK_WITH_RET(reqPayload, TAG, "reqPayload");

    uint32_t gatewayId = 0;

    OCStackResult result = RMPParseRequestPayload(reqPayload, payloadSize, &gatewayId);
    RM_VERIFY_SUCCESS(result, OC_STACK_OK);
    OC_LOG(INFO, TAG, "RMPParseRequestPayload is success");
    // Check if the entry is its own.
    if (gatewayId == g_GatewayID)
    {
        OC_LOG(INFO, TAG, "Own Request Received!!");
        return OC_STACK_CONTINUE;
    }

    CAEndpoint_t endpoint = {.adapter = CA_DEFAULT_ADAPTER};
    CopyDevAddrToEndpoint(&devAddr, &endpoint);

    OC_LOG_V(INFO, TAG, "Add the gateway ID: %u", gatewayId);
    RTMDestIntfInfo_t destInterfaces = {.observerId = 0};
    destInterfaces.destIntfAddr = endpoint;
    result = RTMAddGatewayEntry(gatewayId, 0, 1, &destInterfaces, &g_routingGatewayTable);

    if (OC_STACK_OK != result)
    {
        OC_LOG(DEBUG, TAG, "Gateway was not added to the routing table");
        return result;
    }

    OC_LOG(INFO, TAG, "Gateway was added");
    // Create a list to add the updated entries and notify the observers
    u_linklist_t *updatedTableList = u_linklist_create();
    if(!updatedTableList)
    {
        OC_LOG(DEBUG, TAG, "Failure to notify");
        return OC_STACK_NO_MEMORY;
    }

    RTMGatewayId_t gwId = {.gatewayId = gatewayId};
    RTMGatewayEntry_t newNode;
    newNode.destination = &gwId;
    newNode.routeCost = 1;
    u_linklist_add(updatedTableList, (void *)&newNode);
    RTMPrintTable(g_routingGatewayTable, g_routingEndpointTable);

    OCRepPayload *updatedPayload = NULL;

    g_sequenceNumber++;
    result = RMPConstructObserveResPayload(g_GatewayID, g_sequenceNumber,
                                           updatedTableList, false,
                                           &updatedPayload);
    if (OC_STACK_OK != result)
    {
        OC_LOG_V(ERROR, TAG, "RMPConstructObserveResPayload failed[%d]", result);
        RMPFreePayload(updatedPayload);
        goto exit;
    }

    result = RMSendNotificationToAll(updatedPayload);
    RMPFreePayload(updatedPayload);
    RM_VERIFY_SUCCESS(result, OC_STACK_OK);

exit:
    u_linklist_free(&updatedTableList);
    OC_LOG(DEBUG, TAG, "RMHandleRequestPayload OUT");
    return result;
}

OCStackResult RMHandleResponsePayload(const OCDevAddr *devAddr, const OCRepPayload *respPayload)
{
    OC_LOG(DEBUG, TAG, "RMHandleResponsePayload IN");
    RM_NULL_CHECK_WITH_RET(respPayload, TAG, "respPayload");

    // Parse the Payload to get the Gateway ID of neighbouring node.
    uint32_t gatewayId = 0;
    uint32_t seqNum = 0;
    u_linklist_t *gatewayTableList = NULL;
    bool isUpdateSeqNum = false;

    OCStackResult result = RMPParseResponsePayload(respPayload, &gatewayId, &seqNum,
                                                   &gatewayTableList,  &isUpdateSeqNum);
    RM_VERIFY_SUCCESS(result, OC_STACK_OK);
    // Check if the entry is its own.
    if (gatewayId == g_GatewayID)
    {
        OC_LOG(INFO, TAG, "-------------->Own entry, continue!!");
        RTMFreeGatewayRouteTable(&gatewayTableList);
        return OC_STACK_ERROR;
    }
    // Convert OCDevAddr to endpoint address
    CAEndpoint_t endpoint = {.adapter = CA_DEFAULT_FLAGS};
    CopyDevAddrToEndpoint(devAddr, &endpoint);
    RTMDestIntfInfo_t destInterfaces = {.observerId = 0};
    destInterfaces.destIntfAddr = endpoint;
    if (0 < seqNum)
    {
        OC_LOG_V(DEBUG, TAG, "Sequence Number of Resp payload is %d, Forceupdate: %d",
                 seqNum, isUpdateSeqNum);
        result = RTMUpdateEntryParameters(gatewayId, seqNum, &destInterfaces,
                                          &g_routingGatewayTable, isUpdateSeqNum);
        if (OC_STACK_COMM_ERROR == result)
        {
            OC_LOG(ERROR, TAG, "Few packet drops are found, sequence number is not matching");
            // Send a observe request to the gateway.
            RMSendObserveRequest(devAddr, NULL);
            RTMFreeGatewayRouteTable(&gatewayTableList);
            return result;
        }
        else if (OC_STACK_DUPLICATE_REQUEST == result)
        {
            OC_LOG(ERROR, TAG, "Same sequence number is received");
            RTMFreeGatewayRouteTable(&gatewayTableList);
            return result;
        }
    }

    // Check if the payload is for Removal
    bool doRemoveEntry = false;

    if (NULL != gatewayTableList && NULL != gatewayTableList->list)
    {
        RTMGatewayEntry_t *headPtr = u_linklist_get_data(gatewayTableList->list);
        if (headPtr && 0 == headPtr->routeCost)
        {
            OC_LOG(INFO, TAG, "Remove entry is called");
            doRemoveEntry = true;
        }
    }

    // Create a list to add the updated entries and notify the observers
    u_linklist_t *updatedTableList = u_linklist_create();
    if(!updatedTableList)
    {
        OC_LOG(DEBUG, TAG, "Failed to allocate memory");
        return OC_STACK_NO_MEMORY;
    }

    u_linklist_t *alternativeRouteList = u_linklist_create();
    if(!alternativeRouteList)
    {
        OC_LOG(DEBUG, TAG, "Failed to allocate memory");
        return OC_STACK_NO_MEMORY;
    }

    OCRepPayload *updatedPayload = NULL;
    if (false == doRemoveEntry)
    {
        OC_LOG_V(INFO, TAG, "Add the gateway ID: %u", gatewayId);
        result = RTMAddGatewayEntry(gatewayId, 0, 1, &destInterfaces, &g_routingGatewayTable);
        if (OC_STACK_OK == result)
        {
            OC_LOG(INFO, TAG, "Node was added");
            RTMGatewayId_t gwId = {.gatewayId = gatewayId};
            RTMGatewayEntry_t newNode;
            newNode.destination = &gwId;
            newNode.routeCost = 1;
            u_linklist_add(updatedTableList, (void *)&newNode);
            RTMPrintTable(g_routingGatewayTable, g_routingEndpointTable);

            if (NULL == gatewayTableList)
            {
                OC_LOG(INFO, TAG, "Received a Discover Payload");
                g_sequenceNumber++;
                result = RMPConstructObserveResPayload(g_GatewayID, g_sequenceNumber,
                                                       updatedTableList, false,
                                                       &updatedPayload);
                RM_VERIFY_SUCCESS(result, OC_STACK_OK);
                goto sendNotification;
            }
        }
    }

    // Iterate the Table and get each entry
    u_linklist_iterator_t *iterTable = NULL;
    u_linklist_init_iterator(gatewayTableList, &iterTable);

    while (NULL != iterTable)
    {
        RTMGatewayEntry_t *entry = u_linklist_get_data(iterTable);
        // Check if the entry is its own.
        if (!entry || entry->destination->gatewayId == g_GatewayID)
        {
            OC_LOG(INFO, TAG, "Ignore entry, continue!!");
            u_linklist_get_next(&iterTable);
            continue;
        }

        OC_LOG_V(INFO, TAG, "Gateway ID: %u", entry->destination->gatewayId);
        if (true == doRemoveEntry)
        {
            // Remove the entry from RTM.
            RTMGatewayEntry_t *existEntry = NULL;
            result = RTMRemoveGatewayDestEntry(entry->destination->gatewayId, gatewayId,
                                               &destInterfaces, &existEntry,
                                               &g_routingGatewayTable);
            if (OC_STACK_OK != result && NULL != existEntry)
            {
                u_linklist_add(alternativeRouteList, (void *)existEntry);
            }
        }
        else
        {
            // Add the entry to RTM.
            entry->routeCost = entry->routeCost + 1;
            result = RTMAddGatewayEntry(entry->destination->gatewayId, gatewayId,
                                        entry->routeCost, NULL, &g_routingGatewayTable);
        }

        if (OC_STACK_OK == result)
        {
            OC_LOG(INFO, TAG, "Gateway was added/removed");
            u_linklist_add(updatedTableList, (void *)entry);
            RTMPrintTable(g_routingGatewayTable, g_routingEndpointTable);
        }
        u_linklist_get_next(&iterTable);
    }

    if ( 0 < u_linklist_length(alternativeRouteList))
    {
        OC_LOG(DEBUG, TAG, "Alternative routing found");
        // Send the notification.
        OCRepPayload *removeTablePayload = NULL;
        g_sequenceNumber++;
        result = RMPConstructObserveResPayload(g_GatewayID, g_sequenceNumber,
                                               alternativeRouteList, false,
                                               &removeTablePayload);
        if (OC_STACK_OK != result)
        {
            OC_LOG_V(ERROR, TAG, "RMPConstructObserveResPayload failed[%d]", result);
            RMPFreePayload(removeTablePayload);
            goto exit;
        }
        result = RMSendNotificationToAll(removeTablePayload);
        RMPFreePayload(removeTablePayload);
        RM_VERIFY_SUCCESS(result, OC_STACK_OK);
    }

    if ( 0 >= u_linklist_length(updatedTableList))
    {
        OC_LOG_V(DEBUG, TAG, "No updation is needed, Length is %d",
                 u_linklist_length(updatedTableList));
        goto exit;
    }

    g_sequenceNumber++;
    if (true == doRemoveEntry)
    {
        result = RMPConstructRemovalPayload(g_GatewayID, g_sequenceNumber, updatedTableList,
                                            false, &updatedPayload);
    }
    else
    {
        result = RMPConstructObserveResPayload(g_GatewayID, g_sequenceNumber,
                                               updatedTableList, false,
                                               &updatedPayload);
    }
    RM_VERIFY_SUCCESS(result, OC_STACK_OK);

sendNotification:
    result = RMSendNotificationToAll(updatedPayload);
    RM_VERIFY_SUCCESS(result, OC_STACK_OK);

exit:
    RMPFreePayload(updatedPayload);
    RTMFreeGatewayRouteTable(&gatewayTableList);
    u_linklist_free(&updatedTableList);
    u_linklist_free(&alternativeRouteList);
    OC_LOG(DEBUG, TAG, "RMHandleResponsePayload OUT");
    return OC_STACK_OK;
}

OCStackResult RMHandleGETRequest(const OCServerRequest *request, const OCResource *resource)
{
    OC_LOG(DEBUG, TAG, "RMHandleGETRequest IN");
    RM_NULL_CHECK_WITH_RET(request, TAG, "request");
    RM_NULL_CHECK_WITH_RET(resource, TAG, "resource");

    OCRepPayload *payload = NULL;
    OCStackResult result = RMPConstructGatewayPayload(g_GatewayID, &payload);
    if (OC_STACK_OK != result)
    {
        OC_LOG_V(DEBUG, TAG, "RMPConstructDiscoverPayload failed[%d]", result);
        return result;
    }

    // Send a response for GET request
    result = RMSendResponse(request, resource, payload);
    if (OC_STACK_OK != result)
    {
        OC_LOG_V(DEBUG, TAG, "Send response failed[%d]", result);
        RMPFreePayload(payload);
        return result;
    }

    RMPFreePayload(payload);
    // Send a observe request
    result = RMSendObserveRequest(&(request->devAddr), NULL);
    if (OC_STACK_OK != result)
    {
        OC_LOG_V(DEBUG, TAG, "Send response failed[%d]", result);
    }
    OC_LOG(DEBUG, TAG, "RMHandleGETRequest OUT");
    return result;
}

OCStackResult RMHandleOBSERVERequest(OCServerRequest *request, const OCResource *resource)
{
    OC_LOG(DEBUG, TAG, "RMHandleOBSERVERequest IN");
    RM_NULL_CHECK_WITH_RET(request, TAG, "request");
    RM_NULL_CHECK_WITH_RET(resource, TAG, "resource");

    // Parse payload and add the gateway entry.
    if (0 < request->payloadSize)
    {
        RMHandleRequestPayload(request->devAddr, request->payload, request->payloadSize);
    }

    // Generate and add observer.
    OCObservationId obsID = 0;
    OCStackResult result = RMAddObserver(request, &obsID);
    RM_VERIFY_SUCCESS(result, OC_STACK_OK);
    OC_LOG_V(DEBUG, TAG, "Observer ID is %d", obsID);


    // Get the Routing table from RTM
    OCRepPayload *payload = NULL;
    RTMPrintTable(g_routingGatewayTable, g_routingEndpointTable);
    OC_LOG(DEBUG, TAG, "Construct Routing table payload");
    result = RMPConstructObserveResPayload(g_GatewayID, g_sequenceNumber,
                                           g_routingGatewayTable, true,
                                           &payload);
    if (OC_STACK_OK != result)
    {
        OC_LOG_V(ERROR, TAG, "RMPConstructObserveResPayload failed[%d]", result);
        RMPFreePayload(payload);
        goto exit;
    }

    result = RMSendResponse(request, resource, payload);
    RMPFreePayload(payload);
    RM_VERIFY_SUCCESS(result, OC_STACK_OK);
exit:
    OC_LOG(DEBUG, TAG, "RMHandleOBSERVERequest OUT");
    return result;
}

OCStackResult RMHandleDELETERequest(const OCServerRequest *request, const OCResource *resource)
{
    OC_LOG(DEBUG, TAG, "RMHandleDELETERequest IN");
    RM_NULL_CHECK_WITH_RET(request, TAG, "request");
    RM_NULL_CHECK_WITH_RET(resource, TAG, "resource");

    uint32_t gatewayId = 0;
    OCStackResult result = RMPParseRequestPayload(request->payload, request->payloadSize,
                                                  &gatewayId);
    RM_VERIFY_SUCCESS(result, OC_STACK_OK);
    OC_LOG(INFO, TAG, "RMPParseRequestPayload is success");

    OC_LOG_V(INFO, TAG, "Remove the gateway ID: %u", gatewayId);

    u_linklist_t *removedGatewayNodes = NULL;
    result = RTMRemoveGatewayEntry(gatewayId, &removedGatewayNodes, &g_routingGatewayTable);
    RM_VERIFY_SUCCESS(result, OC_STACK_OK);

    if (0 < u_linklist_length(removedGatewayNodes))
    {
        OCRepPayload *resPayloads = NULL;
        g_sequenceNumber++;
        result = RMPConstructRemovalPayload(g_GatewayID, g_sequenceNumber, removedGatewayNodes,
                                            false, &resPayloads);
        if (OC_STACK_OK != result)
        {
            OC_LOG_V(ERROR, TAG, "RMPConstructRemovalPayload failed[%d]", result);
            RMPFreePayload(resPayloads);
            goto exit;
        }
        result = RMSendNotificationToAll(resPayloads);
        RMPFreePayload(resPayloads);
        RM_VERIFY_SUCCESS(result, OC_STACK_OK);
        RTMPrintTable(g_routingGatewayTable, g_routingEndpointTable);
    }

exit:
    RTMFreeGatewayRouteTable(&removedGatewayNodes);
    OC_LOG(DEBUG, TAG, "RMHandleDELETERequest OUT");
    return result;
}

OCStackResult RMAddObserver(OCServerRequest *request, OCObservationId *obsID)
{
    OC_LOG(DEBUG, TAG, "RMAddObserverForGateway OUT");
    RM_NULL_CHECK_WITH_RET(request, TAG, "request");
    RM_NULL_CHECK_WITH_RET(obsID, TAG, "obsID");

    // Generate the ObserverID
    CAEndpoint_t endpoint = {.adapter = 0};
    CopyDevAddrToEndpoint(&(request->devAddr), &endpoint);

    // Check if observer is already added.
    if (true == RTMIsObserverPresent(endpoint, obsID, g_routingGatewayTable))
    {
        OC_LOG(DEBUG, TAG, "Observer is present");
        request->observeResult = OC_STACK_OK;
        return OC_STACK_OK;
    }

    OCStackResult result = RMAddObserverToStack(request, obsID);
    request->observeResult = result;
    if (OC_STACK_OK == result)
    {
        OC_LOG(DEBUG, TAG, "Added observer successfully");

        // Add the observer to the list.
        result = RTMAddObserver(*obsID, endpoint, &g_routingGatewayTable);
        if (OC_STACK_OK != result)
        {
            OC_LOG_V(DEBUG, TAG, "RMAddObserver failed[%d]", result);
        }
    }
    OC_LOG(DEBUG, TAG, "RMAddObserverForGateway OUT");
    return result;
}

OCStackResult RMSendNotificationToAll(const OCRepPayload *payload)
{
    OC_LOG(DEBUG, TAG, "RMSendNotificationToAll IN");
    RM_NULL_CHECK_WITH_RET(payload, TAG, "payload");

    OCObservationId *obsList = NULL;
    uint8_t obsLen = 0;
    // Get the complete observer list.
    RTMGetObserverList(&obsList, &obsLen, g_routingGatewayTable);
    OCStackResult result = OC_STACK_OK;
    OC_LOG_V(DEBUG, TAG, "Number of observers is %d", obsLen);
    if (0 < obsLen)
    {
        // Send notification to the list of observers.
        OC_LOG_V(DEBUG, TAG, "Sending notification with Sequence Number: %d", g_sequenceNumber);
        result = RMSendNotificationForListofObservers(obsList, obsLen, payload);
        RM_VERIFY_SUCCESS(result, OC_STACK_OK);
        g_aliveTime = RTMGetCurrentTime();
    }

exit:
    OICFree(obsList);
    OC_LOG(DEBUG, TAG, "RMSendNotificationToAll OUT");
    return result;
}

void RMProcess()
{
    if (!g_isRMInitialized)
    {
        return;
    }

    OCStackResult result = OC_STACK_OK;
    uint64_t currentTime = RTMGetCurrentTime();
    if (GATEWAY_ALIVE_TIMEOUT <= currentTime - g_aliveTime)
    {
        g_aliveTime = currentTime;
        // Construct a payload with only the current sequence number.
        OCRepPayload *payload = NULL;
        result = RMPConstructObserveResPayload(g_GatewayID, g_sequenceNumber, NULL,
                                               false, &payload);
        if (OC_STACK_OK != result)
        {
            OC_LOG_V(ERROR, TAG, "RMPConstructObserveResPayload failed[%d]", result);
            RMPFreePayload(payload);
            goto exit;
        }
        OC_LOG(DEBUG, TAG, "Sending the alive notification to all");
        // Send notification for every 15s to all the neighbours.
        result = RMSendNotificationToAll(payload);
        RMPFreePayload(payload);
        RM_VERIFY_SUCCESS(result, OC_STACK_OK);
    }

    if (ROUTINGTABLE_VALIDATION_TIMEOUT <= currentTime - g_refreshTableTime)
    {
        OC_LOG(DEBUG, TAG, "Validating the routing table");
        u_linklist_t *removedEntries = NULL;
        // Remove the invalid gateway entries.
        RTMRemoveInvalidGateways(&removedEntries, &g_routingGatewayTable);
        if (0 < u_linklist_length(removedEntries))
        {
            OCRepPayload *resPayloads = NULL;
            g_sequenceNumber++;
            result = RMPConstructRemovalPayload(g_GatewayID, g_sequenceNumber, removedEntries,
                                                false, &resPayloads);
            RTMFreeGatewayRouteTable(&removedEntries);
            if (OC_STACK_OK != result)
            {
                OC_LOG_V(ERROR, TAG, "RMPConstructRemovalPayload failed[%d]", result);
                RMPFreePayload(resPayloads);
                goto exit;
            }
            result = RMSendNotificationToAll(resPayloads);
            RMPFreePayload(resPayloads);
            RM_VERIFY_SUCCESS(result, OC_STACK_OK);
            RTMPrintTable(g_routingGatewayTable, g_routingEndpointTable);
        }
        g_refreshTableTime = currentTime;
        g_isValidated = false;
        u_linklist_free(&removedEntries);
        goto exit;
    }

    if (!g_isValidated && ROUTINGTABLE_REFRESH_TIMEOUT <= (currentTime - g_refreshTableTime))
    {
        OC_LOG_V(DEBUG, TAG, "Refreshing the routing table: %u", currentTime);
        u_linklist_t* invalidInterfaces = NULL;
        RTMUpdateDestAddrValidity(&invalidInterfaces, &g_routingGatewayTable);
        if (0 < u_linklist_length(invalidInterfaces))
        {
            u_linklist_iterator_t *iterTable = NULL;
            u_linklist_init_iterator(invalidInterfaces, &iterTable);
            while (NULL != iterTable)
            {
                RTMDestIntfInfo_t *entry = (RTMDestIntfInfo_t *) u_linklist_get_data(iterTable);
                if(!entry)
                {
                    u_linklist_get_next(&iterTable);
                    continue;
                }
                OCDevAddr devAddr = {.adapter = OC_DEFAULT_ADAPTER};
                CopyEndpointToDevAddr(&(entry->destIntfAddr), &devAddr);
                RMSendObserveRequest(&devAddr, NULL);
                u_linklist_get_next(&iterTable);
            }
        }
        g_isValidated = true;
        RTMPrintTable(g_routingGatewayTable, g_routingEndpointTable);
        u_linklist_free(&invalidInterfaces);
    }

exit:
    return;
}

OCStackResult RMGetGatewayPayload(OCRepPayload **payload)
{
    OC_LOG(DEBUG, TAG, "RMGetGatewayPayload IN");
    OCStackResult result = RMPConstructGatewayPayload(g_GatewayID, payload);
    OC_LOG_V(DEBUG, TAG, "RMPConstructDiscoverPayload result is %d", result);
    OC_LOG(DEBUG, TAG, "RMGetGatewayPayload OUT");
    return result;
}

void RMSendDeleteToNeighbourNodes()
{
    OC_LOG(DEBUG, TAG, "RMSendDeleteToNeighbourNodes IN");
    u_linklist_t *neighbourNodes = NULL;
    RTMGetNeighbours(&neighbourNodes, g_routingGatewayTable);

    if (0 >= u_linklist_length(neighbourNodes))
    {
        OC_LOG(DEBUG, TAG, "No neighbour nodes present");
        return;
    }

    u_linklist_iterator_t *iterTable = NULL;
    u_linklist_init_iterator(neighbourNodes, &iterTable);
    while (NULL != iterTable)
    {

        OCRepPayload *payload = NULL;
        // Created payload is freed in the OCDoResource() api.
        OCStackResult result = RMPConstructGatewayPayload(g_GatewayID, &payload);
        if (OC_STACK_OK != result)
        {
            OC_LOG_V(DEBUG, TAG, "RMPConstructGatewayPayload failed[%d]", result);
            RMPFreePayload(payload);
            u_linklist_free(&neighbourNodes);
            return;
        }

        RTMGatewayEntry_t *entry = (RTMGatewayEntry_t *) u_linklist_get_data(iterTable);
        if (entry)
        {
            for (uint32_t i = 0; i < u_arraylist_length(entry->destination->destIntfAddr); i++)
            {
                RTMDestIntfInfo_t *dest = u_arraylist_get(entry->destination->destIntfAddr, i);
                if (!dest)
                {
                    OC_LOG(ERROR, RM_TAG, "Failed to get dest address");
                    continue;
                }
                OCDevAddr devAddr = {.adapter = OC_DEFAULT_ADAPTER};
                CopyEndpointToDevAddr(&(dest->destIntfAddr), &devAddr);
                OC_LOG_V(DEBUG, TAG, "\nDestination interface addresses: %s[%d], OCDevAddr: %s[%d]",
                         dest->destIntfAddr.addr, dest->destIntfAddr.port, devAddr.addr, devAddr.port);
                RMSendDeleteRequest(&devAddr, payload);
            }
        }

        u_linklist_get_next(&iterTable);
    }

    u_linklist_free(&neighbourNodes);
    OC_LOG(DEBUG, TAG, "RMSendDeleteToNeighbourNodes OUT");
}

uint32_t RMGetGatewayId()
{
    if (!g_isRMInitialized)
    {
        OC_LOG(ERROR, TAG, "RM not initialized");
        return 0;
    }
    return g_GatewayID;
}

uint16_t RMGetMcastSeqNumber()
{
    if (!g_isRMInitialized)
    {
        OC_LOG(DEBUG, TAG, "RM not initialized");
        return 0;
    }
    return ++g_mcastsequenceNumber;
}

/*
 * This function is lifeline of packet forwarding module, hence we are going to do some serious
 * handling here. Following are the expectations from this function:
 * 1) If routing option is not available, forward packet to RI only. else:
 * 2) If source is empty in routing option, packet is from end device, add an end device entry and
 *    add "GatewayId:ClientId" as source.
 * 3) If destination is empty in routing option, its multicast packet, increase hopcount and
 *    multicast on other interfaces. Also remove routing Option and forward to RI. (Before
 *    forwarding, check last mCastSeqNumber for the source gateway otherwise we might be looping
 *     the packet.)
 * 4) If destination is present in routing option, its unicast packet,
 *    a) If self gatewayId is present in destination and no clientId, remove routing option
 *       and forward to RI.
 *    b) If self gatewayId and a clientId is present in destination, forward to end device.
 * 5) Drop a packet if its hop count reaches NUMBER_OF_GATEWAYS.
 */

OCStackResult RMHandlePacket(bool isRequest, void *message, const CAEndpoint_t *sender,
                             bool *selfDestination)
{
    RM_NULL_CHECK_WITH_RET(message, RM_TAG, "message");
    RM_NULL_CHECK_WITH_RET(sender, RM_TAG, "sender");
    RM_NULL_CHECK_WITH_RET(selfDestination, RM_TAG, "selfDestination");

    bool forward = false;
    CAEndpoint_t nextHop = {.adapter = CA_DEFAULT_ADAPTER};
    CAInfo_t *info = NULL;
    if (isRequest)
    {
        CARequestInfo_t *msg = message;
        info = &(msg->info);
        RM_NULL_CHECK_WITH_RET(info, RM_TAG, "info");
    }
    else
    {
        CAResponseInfo_t *msg = message;
        info = &(msg->info);
        RM_NULL_CHECK_WITH_RET(info, RM_TAG, "info");
    }

    // 1.  If routing option is not available, forward packet to RI only.
    int8_t routeIndex = -1;
    RMGetRouteOptionIndex(info->options, info->numOptions, &routeIndex);
    if (-1 >= routeIndex)
    {
        OC_LOG(ERROR, RM_TAG, "No route option present. Let RI Handle");
        // Let RI handle this packet.
        *selfDestination = true;
        return OC_STACK_OK;
    }

    // Get existing values in packet route option.
    RMRouteOption_t routeOption = {.srcGw = 0};
    OCStackResult res = RMParseRouteOption(&info->options[routeIndex], &routeOption);
    if (OC_STACK_OK != res)
    {
        OC_LOG_V(ERROR, RM_TAG, "RMParseRouteOption failed");
        return OC_STACK_ERROR;
    }

    /*
     * 2) If source is empty in routing option, packet is from end device, add an end device entry
     *  and add "GatewayId:ClientId" as source.
     */
    if (g_GatewayID == routeOption.srcGw)
    {
        OC_LOG_V(ERROR, RM_TAG, "Packet is of its own");
        if (0 == routeOption.destGw && g_mcastsequenceNumber < routeOption.mSeqNum)
        {
            g_mcastsequenceNumber = routeOption.mSeqNum;
        }

        return OC_STACK_ERROR;
    }
    else if (0 == routeOption.srcGw)
    {
        OC_LOG(INFO, RM_TAG, "Source missing in option");
        // Packet from end device as Gateway will add source in option.
        uint16_t endpointId = g_EndpointCount + 1;
        OCStackResult res = RTMAddEndpointEntry(&endpointId, sender, &g_routingEndpointTable);
        if (OC_STACK_OK == res)
        {
            g_EndpointCount = endpointId;
            OC_LOG_V(INFO, RM_TAG, "New endpoint added [%d]:[%s]", g_EndpointCount, sender->addr);
        }
        else if (OC_STACK_DUPLICATE_REQUEST == res)
        {
            OC_LOG_V(INFO, RM_TAG, "Endpoint exist [%d]", endpointId);
        }
        else
        {
            OC_LOG(ERROR, RM_TAG, "Add Endpoint failed");
            return OC_STACK_ERROR;
        }

        // add source option.
        routeOption.srcGw = g_GatewayID;
        routeOption.srcEp = endpointId;
        OC_LOG_V(INFO, RM_TAG, "Added source: [%u:%u]", g_GatewayID, endpointId);
    }

    /*
     * 3) If destination is empty in routing option, its a multicast packet, increase hopcount and
     *    multicast on other interfaces. Also remove routing Option and forward to RI (taken care by
     *    caller of this function).
     */
    if (0 == routeOption.destGw)
    {
        OC_LOG(INFO, RM_TAG, "Destination missing in option");
        // This is a multicast packet.
        if (g_GatewayID == routeOption.srcGw)
        {
            routeOption.mSeqNum = ++g_mcastsequenceNumber;
        }
        else
        {
            OCStackResult update = RTMUpdateMcastSeqNumber(routeOption.srcGw, routeOption.mSeqNum,
                                                           &g_routingGatewayTable);
            if (OC_STACK_OK != update)
            {
                // this shouldnt have been forwarded. ignore.
                OC_LOG_V(ERROR, RM_TAG, "Multicast Sequence number not proper: %d",
                         routeOption.mSeqNum);
                return OC_STACK_ERROR;
            }
        }

        // forward
        *selfDestination = true;
        forward = true;

        // Send multicast on every adapter except the one from which packet was received
        // TODO::: support to be added for IP hop.
        if (sender->adapter != CA_ADAPTER_IP)
        {
            nextHop.adapter |= CA_ADAPTER_IP;
            nextHop.flags |= CA_IPV4 | CA_IPV6;
        }

        if(sender->adapter != CA_ADAPTER_GATT_BTLE)
        {
            nextHop.adapter |= CA_ADAPTER_GATT_BTLE;
        }

        if(sender->adapter != CA_ADAPTER_RFCOMM_BTEDR)
        {
            nextHop.adapter |= CA_ADAPTER_RFCOMM_BTEDR;
        }

        // Only requests are sent as multicast.
        if(isRequest)
        {
            CARequestInfo_t *msg = message;
            msg->isMulticast = true;
        }
        goto  rewriteandexit;
    }
    else if (g_GatewayID == routeOption.destGw)
    {
        OC_LOG(INFO, RM_TAG, "GatewayId found in destination");
        /*
         * This unicast packet either belongs to us or any of our connected end devices
         * check if packet belongs to end device.
         */
        if (0 != routeOption.destEp)
        {
            // forward packet to the client.
            OC_LOG_V(INFO, RM_TAG, "Forwarding packet to client id [%u]", routeOption.destEp);
            CAEndpoint_t *clientInfo = RTMGetEndpointEntry(routeOption.destEp,
                                                           g_routingEndpointTable);
            if(!clientInfo)
            {
                OC_LOG(ERROR, RM_TAG, "Failed to get Client info");
                return OC_STACK_ERROR;
            }

            nextHop = *clientInfo;
            forward = true;
            *selfDestination = false;
            goto rewriteandexit;
        }
        else
        {
            // packet is for us.
            OC_LOG(INFO, RM_TAG, "Received packet for self");
            forward = false;
            *selfDestination = true;
            goto rewriteandexit;
        }
    }
    else
    {
        /*
         * This unicast packet belongs to other gateway.
         * we only want to print first 4 bytes of packet as readable GatewayId.
         */
        OC_LOG_V(INFO, RM_TAG, "Forwarding packet to Gateway: %u", routeOption.destGw);
        RTMGatewayId_t *nextHopGw = RTMGetNextHop(routeOption.destGw, g_routingGatewayTable);
        if(!nextHopGw)
        {
            OC_LOG(ERROR, RM_TAG, "Failed to get next hop");
            return OC_STACK_ERROR;
        }

        // TODO:: check preferences among multiple interface addresses, for now sending on first one
        RTMDestIntfInfo_t *address = u_arraylist_get(nextHopGw->destIntfAddr, 0);
        if (!address)
        {
            OC_LOG(ERROR, RM_TAG, "Failed to get address for next hop");
            return OC_STACK_ERROR;
        }

        nextHop = address->destIntfAddr;
        forward = true;
        *selfDestination = false;
        goto rewriteandexit;
    }

rewriteandexit:

    if (forward)
    {
        // Don't forward any packet meant for gateway resource.
        if (info->resourceUri && (0 == strcmp(info->resourceUri, OC_RSRVD_GATEWAY_URI)))
        {
            OC_LOG(ERROR, RM_TAG, "Not forwarding gateway resource packet");
        }
        else if (sender->flags & CA_SECURE)
        {
            OC_LOG(ERROR, RM_TAG, "This is secured request. Not supported by routing manager");
            return OC_STACK_ERROR;
        }
        else
        {
            // rewrite any changes in routing option.
            res = RMCreateRouteOption(&routeOption, &info->options[routeIndex]);
            if (OC_STACK_OK != res)
            {
                OC_LOG_V(ERROR, RM_TAG, "Rewriting RM option failed");
                return res;
            }
            /*
             * When forwarding a packet, do not attempt retransmission as its the responsibility of
             * packet originator node.
             */
            info->skipRetransmission = true;
            if(isRequest)
            {
                CARequestInfo_t *msg = message;
                CAResult_t caRes = CASendRequest(&nextHop, msg);
                if (CA_STATUS_OK != caRes)
                {
                    OC_LOG_V(ERROR, RM_TAG, "Failed to forward request to next hop [%d][%s]", caRes,
                             nextHop.addr);
                    if(0 == routeOption.destGw)
                    {
                        /*
                         * No point going forward as unicast packet could not be forwarded
                         * not returning error for multicast as we may still let RI process
                         * this packet.
                         */
                        return OC_STACK_ERROR;
                    }
                }
            }
            else
            {
                CAResponseInfo_t *msg = message;
                CAResult_t caRes = CASendResponse(&nextHop, msg);
                if (CA_STATUS_OK != caRes)
                {
                    OC_LOG_V(ERROR, RM_TAG, "Failed to forward response to next hop [%d][%s]",
                             caRes, nextHop.addr);
                    // Since a response is always unicast, return error here.
                    return OC_STACK_ERROR;
                }
            }
        }
    }

    OC_LOG_V(INFO, RM_TAG, "Sender: [%u] Destination: [%u]", routeOption.srcGw, routeOption.destGw);
    return OC_STACK_OK;
}

OCStackResult RMHandleRequest(CARequestInfo_t *message, const CAEndpoint_t *sender,
                              bool *selfDestination)
{
    if (!g_isRMInitialized)
    {
        OC_LOG(ERROR, TAG, "RM not initialized");
        *selfDestination = true;
        return OC_STACK_OK;
    }
    OCStackResult res = RMHandlePacket(true, message, sender, selfDestination);
    return res;
}

OCStackResult RMHandleResponse(CAResponseInfo_t *message, const CAEndpoint_t *sender,
                               bool *selfDestination)
{
    if (!g_isRMInitialized)
    {
        OC_LOG(ERROR, TAG, "RM not initialized");
        *selfDestination = true;
        return OC_STACK_OK;
    }
    OCStackResult res = RMHandlePacket(false, message, sender, selfDestination);
    return res;
}
