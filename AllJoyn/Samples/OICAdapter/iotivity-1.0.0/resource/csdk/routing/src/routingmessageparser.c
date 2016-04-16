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
#include "routingmessageparser.h"
#include "routingtablemanager.h"
#include "routingutility.h"
#include "ocpayload.h"
#include "ocpayloadcbor.h"
#include "oic_malloc.h"
#include "oic_string.h"
#include "include/logger.h"

/**
 * Logging tag for module name.
 */
#define TAG "RPM"

/**
 * Table key to parser Payload Table.
 */
static const char TABLE[] = "table";

/**
 * Length key to get length Payload Array.
 */
static const char LENGTH_PROP[] = "len";

/**
 * Gateway key to parser Payload Table.
 */
static const char GATEWAY[] = "gateway";

/**
 * Route Cost key to parser Payload Table.
 */
static const char ROUTE_COST[] = "routecost";

/**
 * Sequence Number key to parser Payload Table.
 */
static const char SEQ_NUM[] = "seqnum";

/**
 * Response Type key to parser Payload Table.
 */
static const char UPDATE_SEQ_NUM[] = "updateseqnum";

OCStackResult RMPConstructGatewayPayload(uint32_t gatewayId, OCRepPayload **payload)
{
    OC_LOG(DEBUG, TAG, "RMPConstructGatewayPayload IN");
    RM_NULL_CHECK_WITH_RET(payload, TAG, "payload");

    *payload = OCRepPayloadCreate();
    if(!*payload)
    {
        OC_LOG(ERROR, TAG, "Failed to allocate Payload");
        return OC_STACK_ERROR;
    }

    (*payload)->base.type = PAYLOAD_TYPE_REPRESENTATION;
    OCRepPayloadSetPropInt(*payload, GATEWAY, gatewayId);
    OCRepPayloadSetPropInt(*payload, LENGTH_PROP, 0);

    OC_LOG(DEBUG, TAG, "RMPConstructGatewayPayload OUT");

    return OC_STACK_OK;
}

OCStackResult RMPConstructObserveResPayload(uint32_t gatewayId, uint32_t seqNum,
                                            const u_linklist_t *routingtable, bool isUpdateSeqNeeded,
                                            OCRepPayload **payload)
{
    OC_LOG(DEBUG, TAG, "RMPConstructObserveResPayload IN");
    RM_NULL_CHECK_WITH_RET(payload, TAG, "payload");

    *payload =  (OCRepPayload *)OCRepPayloadCreate();
    if(!*payload)
    {
        OC_LOG(ERROR, TAG, "Failed to allocate Payload");
        return OC_STACK_ERROR;
    }

    (*payload)->base.type = PAYLOAD_TYPE_REPRESENTATION;
    OCRepPayloadSetPropInt(*payload, GATEWAY, gatewayId);
    OCRepPayloadSetPropInt(*payload, SEQ_NUM, seqNum);
    OCRepPayloadSetPropBool(*payload, UPDATE_SEQ_NUM, isUpdateSeqNeeded);
    if (NULL == routingtable)
    {
        OCRepPayloadSetPropInt(*payload, LENGTH_PROP, 0);
        OC_LOG(DEBUG, TAG, "Routing Table NULL for ObserveRes Payload");
        return OC_STACK_OK;
    }

    u_linklist_iterator_t *iterTable = NULL;
    u_linklist_init_iterator(routingtable, &iterTable);

    uint32_t len = u_linklist_length(routingtable);
    const OCRepPayload *arrayPayload[len];

    size_t dimensions[MAX_REP_ARRAY_DEPTH] = {len, 0, 0};

    int i = 0;
    while (NULL != iterTable)
    {
        RTMGatewayEntry_t *entry = u_linklist_get_data(iterTable);

        OCRepPayload *add = OCRepPayloadCreate();
        if(!add)
        {
            OC_LOG(ERROR, TAG, "Failed to allocate Payload");
            return OC_STACK_ERROR;
        }

        add->base.type = PAYLOAD_TYPE_REPRESENTATION;
        OCRepPayloadSetPropInt(add, GATEWAY, entry->destination->gatewayId);
        OCRepPayloadSetPropInt(add, ROUTE_COST, entry->routeCost);
        arrayPayload[i] = add;

        i++;
        u_linklist_get_next(&iterTable);
    }
    OCRepPayloadSetPropInt(*payload, LENGTH_PROP, i);
    if (i > 0)
    {
        bool res = OCRepPayloadSetPropObjectArray(*payload, TABLE, arrayPayload, dimensions);
        if (!res)
        {
            OC_LOG(ERROR, TAG, "Failed to Construct Observer response Payload");
            return OC_STACK_ERROR;
        }
    }

    OC_LOG(DEBUG, TAG, "RMPConstructObserveResPayload OUT");
    return OC_STACK_OK;
}

OCStackResult RMPConstructRemovalPayload(uint32_t gatewayId, uint32_t seqNum,
                                         const u_linklist_t *removedGateways, bool isUpdateSeqNeeded,
                                         OCRepPayload **removedPayload)
{
    OC_LOG(DEBUG, TAG, "RMPConstructRemovalPayload IN");
    RM_NULL_CHECK_WITH_RET(removedGateways, TAG, "removedGateways");
    RM_NULL_CHECK_WITH_RET(removedPayload, TAG, "removedPayload");

    u_linklist_iterator_t *iterTable = NULL;
    u_linklist_init_iterator(removedGateways, &iterTable);

    *removedPayload =  OCRepPayloadCreate();
    if(!*removedPayload)
    {
        OC_LOG(ERROR, TAG, "Failed to allocate Payload");
        return OC_STACK_ERROR;
    }

    uint32_t len = u_linklist_length(removedGateways);
    const OCRepPayload *arrayPayload[len];

    size_t dimensions[MAX_REP_ARRAY_DEPTH] = {len, 0, 0};

    (*removedPayload)->base.type = PAYLOAD_TYPE_REPRESENTATION;
    OCRepPayloadSetPropInt(*removedPayload, GATEWAY, gatewayId);
    OCRepPayloadSetPropInt(*removedPayload, SEQ_NUM, seqNum);
    OCRepPayloadSetPropBool(*removedPayload, UPDATE_SEQ_NUM, isUpdateSeqNeeded);

    int i = 0;
    while (NULL != iterTable)
    {
        RTMGatewayEntry_t *entry = (RTMGatewayEntry_t *) u_linklist_get_data(iterTable);
        if (!entry)
        {
            u_linklist_get_next(&iterTable);
            continue;
        }
        OCRepPayload *add = OCRepPayloadCreate();
        if(!add)
        {
            OC_LOG(ERROR, TAG, "Failed to allocate Payload");
            return OC_STACK_ERROR;
        }

        add->base.type = PAYLOAD_TYPE_REPRESENTATION;
        OC_LOG_V(DEBUG, TAG, "Removing the gateway entry: %u", entry->destination->gatewayId);
        OCRepPayloadSetPropInt(add, GATEWAY, entry->destination->gatewayId);
        OCRepPayloadSetPropInt(add, ROUTE_COST, 0);
        arrayPayload[i] = add;

        i++;
        u_linklist_get_next(&iterTable);
    }
    OCRepPayloadSetPropInt(*removedPayload, LENGTH_PROP, i);
    bool res = OCRepPayloadSetPropObjectArray(*removedPayload, TABLE, arrayPayload, dimensions);
    if (!res)
    {
        OC_LOG(ERROR, TAG, "Failed to Construct Removal Payload");
        return OC_STACK_ERROR;
    }

    OC_LOG(DEBUG, TAG, "RMPConstructRemovalPayload OUT");
    return OC_STACK_OK;
}

OCStackResult RMPParseRequestPayload(const uint8_t* payload, size_t payloadSize,
                                     uint32_t *gatewayId)
{
    OCPayload *ocPayload = NULL;
    OCParsePayload(&ocPayload, PAYLOAD_TYPE_REPRESENTATION, payload, payloadSize);
    OCRepPayload *repPayload = (OCRepPayload *)ocPayload;
    OCStackResult res = RMPParseResponsePayload(repPayload, gatewayId, NULL, NULL, NULL);
    if (OC_STACK_OK != res)
    {
        OC_LOG(DEBUG, TAG, "ParseResponsePayload failed");
    }

    return res;
}

OCStackResult RMPParseResponsePayload(const OCRepPayload *payload, uint32_t *gatewayId,
                                      uint32_t *seqNum, u_linklist_t **gatewayTable,
                                      bool *isUpdateSeqNeeded)
{
    OC_LOG(DEBUG, TAG, "RMPParsePayload IN");
    RM_NULL_CHECK_WITH_RET(payload, TAG, "payload");
    RM_NULL_CHECK_WITH_RET(gatewayId, TAG, "gatewayId");

    int64_t tempGateway = 0;
    OCRepPayloadGetPropInt(payload, GATEWAY, &tempGateway);
    *gatewayId = tempGateway;

    if (NULL == gatewayId || 0 == *gatewayId)
    {
        return OC_STACK_COMM_ERROR;
    }

    if (NULL != seqNum)
    {
        int64_t tempSeq = 0;
        OCRepPayloadGetPropInt(payload, SEQ_NUM, &tempSeq);
        *seqNum = tempSeq;
    }

    int64_t length = 0;
    OCRepPayloadGetPropInt(payload, LENGTH_PROP, &length);

    if (NULL != isUpdateSeqNeeded)
    {
        OCRepPayloadGetPropBool(payload, UPDATE_SEQ_NUM, isUpdateSeqNeeded);
    }

    int len = length;
    if (0 == len)
    {
        OC_LOG(DEBUG, TAG, "Parsed Gateway Payload");
        return OC_STACK_OK;
    }

    if (NULL == gatewayTable)
    {
        OC_LOG(DEBUG, TAG, "gatewayTable is NULL");
        return OC_STACK_OK;
    }

    OCRepPayload **responsePayload[len];

    size_t dimensions[MAX_REP_ARRAY_DEPTH];
    OCRepPayloadGetPropObjectArray(payload, TABLE, responsePayload, dimensions);

    if (NULL == *responsePayload)
    {
        OC_LOG(DEBUG, TAG, "RMPParsePayload OUT");
        return OC_STACK_OK;
    }

    *gatewayTable = u_linklist_create();
    if (NULL == *gatewayTable)
    {
        OC_LOG(DEBUG, TAG, "Gateway table create failed");
        return OC_STACK_ERROR;
    }

    for (int i = 0; i < len; i++)
    {
        RTMGatewayEntry_t *entry = (RTMGatewayEntry_t *)OICCalloc(1, sizeof(RTMGatewayEntry_t));

        if (NULL == entry)
        {
            OC_LOG(DEBUG, TAG, "RTMGatewayEntry_t Calloc failed");
            return OC_STACK_ERROR;
        }
        // Filling new Entry
        entry->destination = (RTMGatewayId_t*)OICCalloc(1, sizeof(RTMGatewayId_t));
        if (NULL == entry->destination)
        {
            OC_LOG(DEBUG, TAG, "Destination Calloc failed");
            OICFree(entry);
            return OC_STACK_ERROR;
        }
        entry->nextHop = (RTMGatewayId_t*)OICCalloc(1, sizeof(RTMGatewayId_t));
        if (NULL == entry->nextHop)
        {
            OC_LOG(DEBUG, TAG, "nextHop Calloc failed");
            OICFree(entry->destination);
            OICFree(entry);
            return OC_STACK_ERROR;
        }

        entry->nextHop->gatewayId = *gatewayId;

        int64_t gatewayBuf;
        int64_t routeCost;
        OCRepPayloadGetPropInt(*((*responsePayload) + i), GATEWAY, &gatewayBuf);
        OCRepPayloadGetPropInt(*((*responsePayload) + i), ROUTE_COST, &routeCost);

        entry->destination->gatewayId = gatewayBuf;
        entry->routeCost = routeCost;
        u_linklist_add(*gatewayTable, (void *)entry);
    }
    OC_LOG(DEBUG, TAG, "RMPParsePayload OUT");
    return OC_STACK_OK;
}

void RMPFreePayload(OCRepPayload *payload)
{
    OC_LOG(DEBUG, TAG, "RMPFreePayload IN");
    RM_NULL_CHECK_VOID(payload, TAG, "payload");
    OCRepPayloadDestroy(payload);
    OC_LOG(DEBUG, TAG, "RMPFreePayload OUT");
}
