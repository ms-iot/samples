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
#include "routingtablemanager.h"
#include "routingutility.h"
#include "oic_malloc.h"
#include "oic_string.h"
#include "include/logger.h"

/**
 * Logging tag for module name.
 */
#define TAG "RTM"

/**
 * Tag for printing the Routing table.
 */
#define RM_TAG "RAP"


/**
 * Initial Length of observer list.
 */
#define MAX_OBSERVER_LIST_LENGTH 10

static const uint64_t USECS_PER_SEC = 1000000;

OCStackResult RTMInitialize(u_linklist_t **gatewayTable, u_linklist_t **endpointTable)
{
    OC_LOG(DEBUG, TAG, "RTMInitialize IN");
    RM_NULL_CHECK_WITH_RET(gatewayTable, TAG, "gatewayTable");
    RM_NULL_CHECK_WITH_RET(endpointTable, TAG, "endpointTable");
    if (NULL == *gatewayTable)
    {
        *gatewayTable = u_linklist_create();
        if (NULL == *gatewayTable)
        {
            OC_LOG(ERROR, TAG, "Creating Routing Table failed");
            RTMTerminate(gatewayTable, endpointTable);
            return OC_STACK_ERROR;
        }
    }

    if (NULL == *endpointTable)
    {
        *endpointTable = u_linklist_create();
        if (NULL == *endpointTable)
        {
           OC_LOG(ERROR, TAG, "Creating Routing Table failed");
            RTMTerminate(gatewayTable, endpointTable);
           return OC_STACK_ERROR;
        }
    }
    OC_LOG(DEBUG, TAG, "RTMInitialize OUT");
    return OC_STACK_OK;
}

/*
 * Freeing every char pointer and array list of gateway entry here and frees the table.
 */
OCStackResult RTMFreeGatewayRouteTable(u_linklist_t **gatewayTable)
{
    OC_LOG(DEBUG, TAG, "RTMFreeGatewayRouteTable IN");
    if (NULL == gatewayTable || NULL == *gatewayTable)
    {
        return OC_STACK_OK;
    }

    u_linklist_iterator_t *iterTable = NULL;
    u_linklist_init_iterator(*gatewayTable, &iterTable);
    while (NULL != iterTable)
    {
        RTMGatewayEntry_t *hop = u_linklist_get_data(iterTable);
        if (NULL != hop && NULL != hop->destination)
        {
            while (u_arraylist_length(hop->destination->destIntfAddr) > 0)
            {
                if (NULL != hop->destination)
                {
                    RTMDestIntfInfo_t *data = u_arraylist_remove(hop->destination->destIntfAddr, 0);
                    OICFree(data);
                }
            }
            u_arraylist_free(&(hop->destination->destIntfAddr));
            OICFree(hop->destination);
            // No need to free next hop as it is already freed during it's gateway free
            OICFree(hop);
        }

        OCStackResult ret = u_linklist_remove(*gatewayTable, &iterTable);
        if (OC_STACK_OK != ret)
        {
           OC_LOG(ERROR, TAG, "Deleting Entry from Routing Table failed");
           return OC_STACK_ERROR;
        }
    }
    u_linklist_free(gatewayTable);
    OC_LOG(DEBUG, TAG, "RTMFreeGatewayRouteTable OUT");
    return OC_STACK_OK;
}

/*
 * Freeing every char pointer of endpoint entry here frees the table.
 */
OCStackResult RTMFreeEndpointRouteTable(u_linklist_t **endpointTable)
{
    OC_LOG(DEBUG, TAG, "IN");
    if (NULL == endpointTable || NULL == *endpointTable)
    {
        return OC_STACK_OK;
    }

    u_linklist_iterator_t *iterTable = NULL;
    u_linklist_init_iterator(*endpointTable, &iterTable);
    while (NULL != iterTable)
    {
        RTMEndpointEntry_t *hop = u_linklist_get_data(iterTable);
        if (NULL != hop)
        {
            OICFree(hop);
        }

        OCStackResult ret = u_linklist_remove(*endpointTable, &iterTable);
        if (OC_STACK_OK != ret)
        {
            OC_LOG(ERROR, TAG, "Deleting Entry from Routing Table failed");
            return OC_STACK_ERROR;
        }
    }
    u_linklist_free(endpointTable);
    OC_LOG(DEBUG, TAG, "OUT");
    return OC_STACK_OK;
}

OCStackResult RTMFreeGatewayIdList(u_linklist_t **gatewayIdTable)
{
    OC_LOG(DEBUG, TAG, "IN");
    if (NULL == gatewayIdTable || NULL == *gatewayIdTable)
    {
        return OC_STACK_OK;
    }

    u_linklist_iterator_t *iterTable = NULL;
    u_linklist_init_iterator(*gatewayIdTable, &iterTable);
    while (iterTable != NULL)
    {
        RTMGatewayId_t *hop = u_linklist_get_data(iterTable);
        if (NULL != hop)
        {
            while (u_arraylist_length(hop->destIntfAddr) > 0)
            {
               RTMDestIntfInfo_t *data = u_arraylist_remove(hop->destIntfAddr, 0);
               OICFree(data);
            }
            u_arraylist_free(&(hop->destIntfAddr));
            OICFree(hop);

            OCStackResult ret = u_linklist_remove(*gatewayIdTable, &iterTable);
            if (OC_STACK_OK != ret)
            {
               OC_LOG(ERROR, TAG, "Deleting Entry from Routing Table failed");
               return OC_STACK_ERROR;
            }
        }
        else
        {
            OCStackResult res = u_linklist_remove(*gatewayIdTable, &iterTable);
            if (OC_STACK_OK != res)
            {
                OC_LOG(ERROR, TAG, "Deleting Entry from Routing Table failed");
                return OC_STACK_ERROR;
            }
        }
    }
    OC_LOG(DEBUG, TAG, "OUT");
    return OC_STACK_OK;
}

/*
 * Freeing memory first and then Freeing linked list for gateway and endpoint.
 */
OCStackResult RTMTerminate(u_linklist_t **gatewayTable, u_linklist_t **endpointTable)
{
    OC_LOG(DEBUG, TAG, "IN");

    OCStackResult ret = RTMFreeGatewayRouteTable(gatewayTable);
    if (OC_STACK_OK != ret)
    {
        OC_LOG(ERROR, TAG, "Deleting Gateway Routing Table failed");
    }
    if (NULL != *gatewayTable)
    {
        *gatewayTable = NULL;
    }

    ret = RTMFreeEndpointRouteTable(endpointTable);
    if (OC_STACK_OK != ret)
    {
        OC_LOG(ERROR, TAG, "Deleting Endpoint Routing Table failed");
    }
    if (NULL != *endpointTable)
    {
        *endpointTable = NULL;
    }
    OC_LOG(DEBUG, TAG, "OUT");
    return OC_STACK_OK;
}

/*
 * Checks if destination gateway to be added is already present and update if present or
 * adds new entry if not present.
 * Adds Entry to head if route cost is 1.
 * Adds Entry to Tail if route cost is > 1.
 * Checks for Gateway id Memory and assigns to next hop address to achieve better memory usage.
 */
OCStackResult RTMAddGatewayEntry(uint32_t gatewayId, uint32_t nextHop, uint32_t routeCost,
                                 const RTMDestIntfInfo_t *destInterfaces, u_linklist_t **gatewayTable)
{
    OC_LOG(DEBUG, TAG, "IN");
    RM_NULL_CHECK_WITH_RET(gatewayTable, TAG, "gatewayTable");
    if (NULL == *gatewayTable)
    {
        *gatewayTable = u_linklist_create();
        if (NULL == *gatewayTable)
        {
            OC_LOG(ERROR, TAG, "u_linklist_create failed");
            return OC_STACK_NO_MEMORY;
        }
    }

    if (1 == routeCost && 0 != nextHop)
    {
        OC_LOG(ERROR, TAG, "Adding Gateway Failed as Next Hop should be 0 for route cost 1");
        return OC_STACK_ERROR;
    }

    if (0 == routeCost)
    {
        OC_LOG(ERROR, TAG, "Adding Gateway Failed as Route cost shouldnot be less than 1");
        return OC_STACK_ERROR;
    }

    u_linklist_iterator_t *destNode = NULL;
    RTMGatewayId_t *gatewayNodeMap = NULL;   // Gateway id ponter can be mapped to NextHop of entry.

    u_linklist_iterator_t *iterTable = NULL;
    u_linklist_init_iterator(*gatewayTable, &iterTable);
    // Iterate over gateway list to find if already entry with this gatewayid is present.
    while (NULL != iterTable)
    {
        RTMGatewayEntry_t *entry = u_linklist_get_data(iterTable);
        if (NULL == entry)
        {
            break;
        }

        // To save node with same gateway id (To update entry instead of add new entry).
        if (NULL == destNode && NULL != entry->destination &&
            (gatewayId == entry->destination->gatewayId))
        {
            destNode = iterTable;
        }

        // To find pointer of gateway id for a node provided next hop equals to existing gateway id.
        if (0 != nextHop && NULL != entry->destination &&
            nextHop == entry->destination->gatewayId)
        {
            gatewayNodeMap = entry->destination;
        }

        if (NULL != destNode && NULL != gatewayNodeMap)
        {
            break;
        }

        u_linklist_get_next(&iterTable);
    }

    if (1 < routeCost && NULL == gatewayNodeMap)
    {
        OC_LOG(ERROR, TAG, "Adding Gateway Failed as Next Hop is invalid");
        return OC_STACK_ERROR;
    }

    //Logic to update entry if it is already destination present or to add new entry.
    if (NULL != destNode)
    {
        RTMGatewayEntry_t *entry = u_linklist_get_data(destNode);

        if (NULL != entry  && 1 == entry->routeCost && 0 == nextHop)
        {
            if (NULL == destInterfaces)
            {
                OC_LOG(ERROR, TAG, "Not Adding Gateway destInterfaces is NULL");
                return OC_STACK_ERROR;
            }
            OCStackResult update = RTMUpdateDestinationIntfAdr(gatewayId, *destInterfaces, true,
                                                               gatewayTable);
            if (OC_STACK_OK != update)
            {
                OC_LOG(ERROR, TAG, "RTMUpdateDestinationIntfAdr failed");
            }
            return update;
        }
        else if (NULL != entry  && entry->routeCost >= routeCost)
        {
            if (entry->routeCost == routeCost && NULL != entry->nextHop &&
                (nextHop == entry->nextHop->gatewayId))
            {
                OC_LOG(ERROR, TAG, "Not Adding Gateway As it is Duplicate request");
                return OC_STACK_DUPLICATE_REQUEST;
            }

            //Mapped nextHop gateway to another entries having gateway as destination.
            if (NULL != gatewayNodeMap)
            {
                entry->destination->gatewayId = gatewayId;
                entry->nextHop = gatewayNodeMap;
                entry->destination->destIntfAddr = NULL;
                entry->routeCost = routeCost;
            }
            else if (0 == nextHop)
            {
                entry->routeCost = 1;
                // Entry can't be updated if Next hop is not same as existing Destinations of Table.
                OC_LOG(DEBUG, TAG, "Updating the gateway");
                entry->nextHop = NULL;
                entry->destination->destIntfAddr = u_arraylist_create();
                if (NULL == entry->destination->destIntfAddr)
                {
                    OC_LOG(ERROR, TAG, "Failed to create array list");
                    return OC_STACK_ERROR;
                }

                RTMDestIntfInfo_t *destAdr =
                    (RTMDestIntfInfo_t *) OICCalloc(1, sizeof(RTMDestIntfInfo_t));
                if (NULL == destAdr)
                {
                    OC_LOG(ERROR, TAG, "Failed to Calloc destAdr");
                    return OC_STACK_ERROR;
                }

                *destAdr = *destInterfaces;
                destAdr->timeElapsed = RTMGetCurrentTime();
                destAdr->isValid = true;
                bool result =
                    u_arraylist_add(entry->destination->destIntfAddr, (void *)destAdr);
                if (!result)
                {
                    OC_LOG(ERROR, TAG, "Adding node to head failed");
                    OICFree(destAdr);
                    return OC_STACK_ERROR;
                }
            }
            else
            {
                OC_LOG(ERROR, TAG, "Adding Gateway Failed as Next hop is invalid");
                return OC_STACK_ERROR;
            }

        }
        else if (NULL != entry  && entry->routeCost < routeCost)
        {
            OC_LOG(ERROR, TAG, "Adding Gateway Failed as Route cost is more than old");
            return OC_STACK_ERROR;
        }

        // Logic to add updated node to Head of list as route cost is 1.
        if (1 == routeCost && NULL != entry)
        {
            OCStackResult res = u_linklist_remove(*gatewayTable, &destNode);
            if (OC_STACK_OK != res)
            {
                OC_LOG(ERROR, TAG, "Removing node failed");
            }
            else
            {
                res = u_linklist_add_head(*gatewayTable, (void *)entry);
                if (OC_STACK_OK != res)
                {
                    OC_LOG(ERROR, TAG, "Adding node to head failed");
                }
            }
        }
    }
    else
    {
        // Filling new Entry
        RTMGatewayEntry_t *hopEntry = (RTMGatewayEntry_t *)OICCalloc(1, sizeof(RTMGatewayEntry_t));
        if (NULL == hopEntry)
        {
            OC_LOG(ERROR, TAG, "Calloc failed for hop entry");
            return OC_STACK_ERROR;
        }

        hopEntry->destination = (RTMGatewayId_t*)OICCalloc(1, sizeof(RTMGatewayId_t));
        if (NULL == hopEntry->destination)
        {
            OC_LOG(ERROR, TAG, "Calloc failed for hop entry destination");
            OICFree(hopEntry);
            return OC_STACK_ERROR;
        }

        hopEntry->destination->gatewayId = gatewayId;
        if (NULL != destInterfaces && strlen((*destInterfaces).destIntfAddr.addr) > 0)
        {
            hopEntry->destination->destIntfAddr = u_arraylist_create();
            RTMDestIntfInfo_t *destAdr =
                (RTMDestIntfInfo_t *) OICCalloc(1, sizeof(RTMDestIntfInfo_t));
            if (NULL == destAdr)
            {
                OC_LOG(ERROR, TAG, "Calloc failed for destAdr");
                u_arraylist_free(&(hopEntry->destination->destIntfAddr));
                OICFree(hopEntry->destination);
                OICFree(hopEntry);
                return OC_STACK_ERROR;
            }

            *destAdr = *destInterfaces;
            destAdr->timeElapsed = RTMGetCurrentTime();
            destAdr->isValid = true;
            u_arraylist_add(hopEntry->destination->destIntfAddr, (void *)destAdr);
        }
        else
        {
            hopEntry->destination->destIntfAddr = NULL;
        }

        hopEntry->routeCost = routeCost;
        // Mapped nextHop gateway to another entries having gateway as destination.
        if (NULL != gatewayNodeMap)
        {
            hopEntry->nextHop = gatewayNodeMap;
        }
        else if (1 == routeCost)
        {
            hopEntry->nextHop = NULL;
        }
        else
        {
            OC_LOG(ERROR, TAG, "Adding Gateway Failed as Next Hop is invalid");
            while (u_arraylist_length(hopEntry->destination->destIntfAddr) > 0)
            {
                RTMDestIntfInfo_t *data =
                    u_arraylist_remove(hopEntry->destination->destIntfAddr, 0);
                OICFree(data);
            }
            u_arraylist_free(&(hopEntry->destination->destIntfAddr));
            OICFree(hopEntry->destination);
            OICFree(hopEntry);

            return OC_STACK_ERROR;
        }

        OCStackResult ret = OC_STACK_OK;
        if (hopEntry->routeCost == 1)
        {
            ret = u_linklist_add_head(*gatewayTable, (void *)hopEntry);
        }
        else
        {
            ret = u_linklist_add(*gatewayTable, (void *)hopEntry);
        }

        if (OC_STACK_OK != ret)
        {
            OC_LOG(ERROR, TAG, "Adding Gateway Entry to Routing Table failed");
            while (u_arraylist_length(hopEntry->destination->destIntfAddr) > 0)
            {
                RTMDestIntfInfo_t *data = u_arraylist_remove(hopEntry->destination->destIntfAddr, 0);
                OICFree(data);
            }
            u_arraylist_free(&(hopEntry->destination->destIntfAddr));
            OICFree(hopEntry->destination);
            OICFree(hopEntry);
            return OC_STACK_ERROR;
        }
    }
    OC_LOG(DEBUG, TAG, "OUT");
    return OC_STACK_OK;
}

OCStackResult RTMAddEndpointEntry(uint16_t *endpointId, const CAEndpoint_t *destAddr,
                                  u_linklist_t **endpointTable)
{
    OC_LOG(DEBUG, TAG, "IN");
    RM_NULL_CHECK_WITH_RET(endpointId, TAG, "endpointId");
    RM_NULL_CHECK_WITH_RET(destAddr, TAG, "destAddr");
    RM_NULL_CHECK_WITH_RET(endpointTable, TAG, "endpointTable");
    if (NULL == *endpointTable)
    {
        *endpointTable = u_linklist_create();
        if (NULL == *endpointTable)
        {
            OC_LOG(ERROR, TAG, "u_linklist_create failed");
            return OC_STACK_NO_MEMORY;
        }
    }

    u_linklist_iterator_t *iterTable = NULL;
    u_linklist_init_iterator(*endpointTable, &iterTable);
    // Iterate over gateway list to find if already entry with this gatewayid is present.
    while (NULL != iterTable)
    {
        RTMEndpointEntry_t *entry =
            (RTMEndpointEntry_t *) u_linklist_get_data(iterTable);

        if (NULL != entry && (0 == memcmp(destAddr->addr, entry->destIntfAddr.addr,
                              strlen(entry->destIntfAddr.addr)))
            && destAddr->port == entry->destIntfAddr.port)
        {
            *endpointId = entry->endpointId;
            OC_LOG(ERROR, TAG, "Adding failed as Enpoint Entry Already present in Table");
            return OC_STACK_DUPLICATE_REQUEST;
        }
        u_linklist_get_next(&iterTable);
    }

    // Filling Entry.
    RTMEndpointEntry_t *hopEntry = (RTMEndpointEntry_t *)OICCalloc(1, sizeof(RTMEndpointEntry_t));

    if (NULL == hopEntry)
    {
       OC_LOG(ERROR, TAG, "Malloc failed for hop entry");
       return OC_STACK_ERROR;
    }

    hopEntry->endpointId = *endpointId;
    hopEntry->destIntfAddr = *destAddr;

    OCStackResult ret = u_linklist_add(*endpointTable, (void *)hopEntry);
    if (OC_STACK_OK != ret)
    {
       OC_LOG(ERROR, TAG, "Adding Enpoint Entry to Routing Table failed");
       OICFree(hopEntry);
       return OC_STACK_ERROR;
    }
    OC_LOG(DEBUG, TAG, "OUT");
    return OC_STACK_OK;
}

OCStackResult RTMAddObserver(uint32_t obsID, CAEndpoint_t devAddr, u_linklist_t **gatewayTable)
{
    OC_LOG(DEBUG, TAG, "IN");
    RM_NULL_CHECK_WITH_RET(gatewayTable, TAG, "gatewayTable");
    RM_NULL_CHECK_WITH_RET(*gatewayTable, TAG, "*gatewayTable");

    u_linklist_iterator_t *iterTable = NULL;
    u_linklist_init_iterator(*gatewayTable, &iterTable);
    while (NULL != iterTable)
    {
        RTMGatewayEntry_t *entry = u_linklist_get_data(iterTable);

        for (uint32_t i = 0; i < u_arraylist_length(entry->destination->destIntfAddr); i++)
        {
            RTMDestIntfInfo_t *destCheck = u_arraylist_get(entry->destination->destIntfAddr, i);
            if (NULL != destCheck &&
                (0 == memcmp(destCheck->destIntfAddr.addr, devAddr.addr, strlen(devAddr.addr)))
                && devAddr.port == destCheck->destIntfAddr.port)
            {
                destCheck->observerId = obsID;
                OC_LOG(DEBUG, TAG, "OUT");
                return OC_STACK_OK;
            }
        }
        u_linklist_get_next(&iterTable);
    }
    OC_LOG(DEBUG, TAG, "OUT");
    return OC_STACK_ERROR;
}

bool RTMIsObserverPresent(CAEndpoint_t devAddr, OCObservationId *obsID,
                          const u_linklist_t *gatewayTable)
{
    OC_LOG(DEBUG, TAG, "IN");
    if (NULL == obsID)
    {
        OC_LOG(ERROR, TAG, "obsID is null");
        return false;
    }

    if (NULL == gatewayTable)
    {
        OC_LOG(ERROR, TAG, "gatewayTable is null");
        return false;
    }

    u_linklist_iterator_t *iterTable = NULL;
    u_linklist_init_iterator(gatewayTable, &iterTable);
    while (NULL != iterTable)
    {
        RTMGatewayEntry_t *entry = u_linklist_get_data(iterTable);
        if (NULL == entry && NULL == entry->destination)
        {
            OC_LOG(ERROR, TAG, "entry is NULL");
            return false;
        }
        for (uint32_t i = 0; i < u_arraylist_length(entry->destination->destIntfAddr); i++)
        {
            RTMDestIntfInfo_t *destCheck =
                u_arraylist_get(entry->destination->destIntfAddr, i);
            if (NULL != destCheck && (0 == memcmp(destCheck->destIntfAddr.addr, devAddr.addr,
                                      strlen(devAddr.addr)))
                && devAddr.port == destCheck->destIntfAddr.port && 0 != destCheck->observerId)
            {
                *obsID = destCheck->observerId;
                OC_LOG(DEBUG, TAG, "OUT");
                return true;
            }
        }
        u_linklist_get_next(&iterTable);
    }
    OC_LOG(DEBUG, TAG, "OUT");
    return false;
}

OCStackResult RTMRemoveGatewayEntry(uint32_t gatewayId, u_linklist_t **removedGatewayNodes,
                                    u_linklist_t **gatewayTable)
{
    OC_LOG(DEBUG, TAG, "IN");
    RM_NULL_CHECK_WITH_RET(gatewayTable, TAG, "gatewayTable");
    RM_NULL_CHECK_WITH_RET(*gatewayTable, TAG, "*gatewayTable");

    u_linklist_iterator_t *iterTable = NULL;

    // if link list is not null we can directly add removed nodes to it instead of creating everytime.
    if (NULL == *removedGatewayNodes)
    {
        *removedGatewayNodes = u_linklist_create();
        if (NULL == *removedGatewayNodes)
        {
            OC_LOG(ERROR, TAG, "u_linklist_create failed");
            return OC_STACK_NO_MEMORY;
        }
    }
    OCStackResult ret = OC_STACK_OK;
    u_linklist_init_iterator(*gatewayTable, &iterTable);
    while (NULL != iterTable)
    {
        RTMGatewayEntry_t *entry = u_linklist_get_data(iterTable);
        if (NULL == entry || NULL == entry->destination)
        {
            u_linklist_get_next(&iterTable);
            continue;
        }

        if (gatewayId == entry->destination->gatewayId || (NULL != entry->nextHop &&
            (gatewayId == entry->nextHop->gatewayId)))
        {
            OC_LOG_V(DEBUG, TAG, "Removing the gateway entry: %u", entry->destination->gatewayId);
            ret = u_linklist_remove(*gatewayTable, &iterTable);
            if (OC_STACK_OK != ret)
            {
               OC_LOG(ERROR, TAG, "Deleting Entry from Routing Table failed");
               return OC_STACK_ERROR;
            }
            else
            {
                u_linklist_add(*removedGatewayNodes, (void *)entry);
            }
        }
        else
        {
            u_linklist_get_next(&iterTable);
        }
    }
    OC_LOG(DEBUG, TAG, "RTMRemoveGatewayEntry OUT");
    return OC_STACK_OK;
}

OCStackResult RTMRemoveGatewayDestEntry(uint32_t gatewayId, uint32_t nextHop,
                                        const RTMDestIntfInfo_t *destInfAdr,
                                        RTMGatewayEntry_t **existEntry, u_linklist_t **gatewayTable)
{
    OC_LOG(DEBUG, TAG, "IN");
    RM_NULL_CHECK_WITH_RET(gatewayTable, TAG, "gatewayTable");
    RM_NULL_CHECK_WITH_RET(*gatewayTable, TAG, "*gatewayTable");
    RM_NULL_CHECK_WITH_RET(destInfAdr, TAG, "destInfAdr");

    u_linklist_iterator_t *iterTable = NULL;

    OCStackResult ret = -1;
    u_linklist_init_iterator(*gatewayTable, &iterTable);
    while (NULL != iterTable)
    {
        RTMGatewayEntry_t *entry = u_linklist_get_data(iterTable);
        if (NULL == entry)
        {
            u_linklist_get_next(&iterTable);
            continue;
        }

        // Update the time for NextHop entry.
        if (NULL != entry->destination && nextHop == entry->destination->gatewayId)
        {
            for (uint32_t i = 0; i < u_arraylist_length(entry->destination->destIntfAddr); i++)
            {
                RTMDestIntfInfo_t *destCheck = u_arraylist_get(entry->destination->destIntfAddr, i);
                if(!destCheck)
                {
                    continue;
                }
                if (0 == memcmp(destCheck->destIntfAddr.addr, destInfAdr->destIntfAddr.addr,
                    strlen(destInfAdr->destIntfAddr.addr))
                    && destInfAdr->destIntfAddr.port == destCheck->destIntfAddr.port)
                {
                    destCheck->timeElapsed =  RTMGetCurrentTime();
                    break;
                }
            }
        }

        // Remove node with given gatewayid and nextHop if not found update exist entry.
        if (NULL != entry->destination && (gatewayId == entry->destination->gatewayId))
        {
            OC_LOG_V(INFO, TAG, "Remove the gateway ID: %u", entry->destination->gatewayId);
            if (NULL != entry->nextHop && nextHop == entry->nextHop->gatewayId)
            {
                ret = u_linklist_remove(*gatewayTable, &iterTable);
                if (OC_STACK_OK != ret)
                {
                   OC_LOG(ERROR, TAG, "Deleting Entry from Routing Table failed");
                   return OC_STACK_ERROR;
                }
                OICFree(entry);
                return OC_STACK_OK;
            }

            *existEntry = entry;
            OC_LOG(DEBUG, TAG, "OUT");
            return OC_STACK_ERROR;
        }

        u_linklist_get_next(&iterTable);
    }
    OC_LOG(DEBUG, TAG, "OUT");
    return OC_STACK_ERROR;
}

OCStackResult RTMRemoveEndpointEntry(uint16_t endpointId, u_linklist_t **endpointTable)
{
    OC_LOG(DEBUG, TAG, "IN");
    RM_NULL_CHECK_WITH_RET(endpointTable, TAG, "endpointTable");
    RM_NULL_CHECK_WITH_RET(*endpointTable, TAG, "*endpointTable");

    u_linklist_iterator_t *iterTable = NULL;
    u_linklist_init_iterator(*endpointTable, &iterTable);
    while (NULL != iterTable)
    {
        RTMEndpointEntry_t *entry = u_linklist_get_data(iterTable);
        if (NULL !=  entry && endpointId == entry->endpointId)
        {
            OCStackResult ret = u_linklist_remove(*endpointTable, &iterTable);
            if (OC_STACK_OK != ret)
            {
               OC_LOG(ERROR, TAG, "Deleting Entry from Routing Table failed");
               return OC_STACK_ERROR;
            }
            OICFree(entry);
        }
        else
        {
            u_linklist_get_next(&iterTable);
        }
    }
    OC_LOG(DEBUG, TAG, "OUT");
    return OC_STACK_OK;
}

OCStackResult RTMRemoveGateways(u_linklist_t **gatewayTable)
{
    OC_LOG(DEBUG, TAG, "IN");

    if (NULL == gatewayTable || NULL == *gatewayTable)
    {
        OC_LOG(DEBUG, TAG, "OUT");
        return OC_STACK_OK;
    }

    OCStackResult ret = RTMFreeGatewayRouteTable(gatewayTable);
    if (OC_STACK_OK != ret)
    {
        OC_LOG(ERROR, TAG, "Removing Gateways failed");
        return ret;
    }
    OC_LOG(DEBUG, TAG, "OUT");
    return OC_STACK_OK;
}

OCStackResult RTMRemoveEndpoints(u_linklist_t **endpointTable)
{
    OC_LOG(DEBUG, TAG, "IN");
    if (NULL == endpointTable || NULL == *endpointTable)
    {
        OC_LOG(DEBUG, TAG, "OUT");
        return OC_STACK_OK;
    }

    OCStackResult ret = RTMFreeEndpointRouteTable(endpointTable);
    if (OC_STACK_OK != ret)
    {
        OC_LOG(ERROR, TAG, "Freeing Endpoints failed");
        return OC_STACK_ERROR;
    }
    OC_LOG(DEBUG, TAG, "OUT");
    return OC_STACK_OK;
}

void RTMFreeGateway(RTMGatewayId_t *gateway, u_linklist_t **gatewayTable)
{
    OC_LOG(DEBUG, TAG, "IN");
    RM_NULL_CHECK_VOID(gateway, TAG, "gateway");
    RM_NULL_CHECK_VOID(gatewayTable, TAG, "gatewayTable");
    RM_NULL_CHECK_VOID(*gatewayTable, TAG, "*gatewayTable");
    while (0 < u_arraylist_length(gateway->destIntfAddr))
    {
        void *data = u_arraylist_remove(gateway->destIntfAddr, 0);
        OICFree(data);
    }
    u_arraylist_free(&(gateway->destIntfAddr));
    OICFree(gateway);
    OC_LOG(DEBUG, TAG, "OUT");
}

void RTMGetNeighbours(u_linklist_t **neighbourNodes, const u_linklist_t *gatewayTable)
{
    OC_LOG(DEBUG, TAG, "IN");
    RM_NULL_CHECK_VOID(neighbourNodes, TAG, "neighbourNodes");
    RM_NULL_CHECK_VOID(gatewayTable, TAG, "gatewayTable");

    *neighbourNodes = u_linklist_create();
    if (NULL == *neighbourNodes)
    {
        OC_LOG(ERROR, TAG, "u_linklist_create failed");
        return;
    }
    u_linklist_iterator_t *iterTable = NULL;
    u_linklist_init_iterator(gatewayTable, &iterTable);
    while (NULL != iterTable)
    {
        RTMGatewayEntry_t *entry = u_linklist_get_data(iterTable);
        if (NULL != entry && 1 == entry->routeCost)
        {
            u_linklist_add(*neighbourNodes, (void *)entry);
        }
        else if (NULL != entry && 1 < entry->routeCost)
        {
            OC_LOG(DEBUG, TAG, "OUT");
            return;
        }

        u_linklist_get_next(&iterTable);
    }
    OC_LOG(DEBUG, TAG, "OUT");
}

RTMGatewayId_t *RTMGetNextHop(uint32_t gatewayId, const u_linklist_t *gatewayTable)
{
    OC_LOG(DEBUG, TAG, "IN");
    if (0 == gatewayId)
    {
        OC_LOG(ERROR, TAG, "gatewayId is invalid");
        return NULL;
    }

    if (NULL == gatewayTable)
    {
        OC_LOG(ERROR, TAG, "gatewayTable is null");
        return NULL;
    }

    u_linklist_iterator_t *iterTable = NULL;
    u_linklist_init_iterator(gatewayTable, &iterTable);
    while (NULL != iterTable)
    {
        RTMGatewayEntry_t *entry = u_linklist_get_data(iterTable);
        if (NULL != entry && gatewayId == entry->destination->gatewayId)
        {
            if (1 == entry->routeCost)
            {
                OC_LOG(DEBUG, TAG, "OUT");
                return entry->destination;
            }
            OC_LOG(DEBUG, TAG, "OUT");
            return entry->nextHop;
        }
        u_linklist_get_next(&iterTable);
    }
    OC_LOG(DEBUG, TAG, "OUT");
    return NULL;
}

CAEndpoint_t *RTMGetEndpointEntry(uint16_t endpointId, const u_linklist_t *endpointTable)
{
    OC_LOG(DEBUG, TAG, "IN");
    if (NULL == endpointTable)
    {
        OC_LOG(ERROR, TAG, "endpointTable is null");
        return NULL;
    }

    u_linklist_iterator_t *iterTable = NULL;
    u_linklist_init_iterator(endpointTable, &iterTable);

    while (NULL != iterTable)
    {
        RTMEndpointEntry_t *entry = u_linklist_get_data(iterTable);
        if (NULL != entry && (endpointId == entry->endpointId))
        {
            OC_LOG(DEBUG, TAG, "OUT");
            return &(entry->destIntfAddr);
        }
        u_linklist_get_next(&iterTable);
    }
    OC_LOG(DEBUG, TAG, "OUT");
    return NULL;
}

void RTMGetObserverList(OCObservationId **obsList, uint8_t *obsListLen,
                        const u_linklist_t *gatewayTable)
{
    OC_LOG(DEBUG, TAG, "IN");
    RM_NULL_CHECK_VOID(gatewayTable, TAG, "gatewayTable");
    RM_NULL_CHECK_VOID(obsList, TAG, "obsList");

    *obsList = (OCObservationId *) OICCalloc(MAX_OBSERVER_LIST_LENGTH, sizeof(OCObservationId));
    if (!(*obsList))
    {
        OC_LOG(ERROR, TAG, "out of memory");
        return;
    }

    u_linklist_iterator_t *iterTable = NULL;
    u_linklist_init_iterator(gatewayTable, &iterTable);
    uint8_t len = 0;
    while (NULL != iterTable)
    {
        RTMGatewayEntry_t *entry = u_linklist_get_data(iterTable);
        if (0 < u_arraylist_length(entry->destination->destIntfAddr))
        {
            RTMDestIntfInfo_t *destCheck = u_arraylist_get(entry->destination->destIntfAddr, 0);
            if (NULL == destCheck)
            {
                OC_LOG(ERROR, TAG, "destCheck is null");
                return;
            }
            if (0 != destCheck->observerId)
            {
                OC_LOG_V(DEBUG, TAG, "Observer ID is %d", destCheck->observerId);
                *(*obsList + len) = destCheck->observerId;
                len++;
            }
            if (MAX_OBSERVER_LIST_LENGTH < len)
            {
                *obsList = (OCObservationId *) OICRealloc((void *)*obsList,
                           (sizeof(OCObservationId) * (len + 1)));
            }
        }
        u_linklist_get_next(&iterTable);
    }

    *obsListLen = len;
    OC_LOG(DEBUG, TAG, "OUT");
}

OCStackResult RTMUpdateDestinationIntfAdr(uint32_t gatewayId, RTMDestIntfInfo_t destInterfaces,
                                          bool addAdr, u_linklist_t **gatewayTable)
{
    OC_LOG(DEBUG, TAG, "IN");
    RM_NULL_CHECK_WITH_RET(gatewayTable, TAG, "gatewayTable");
    RM_NULL_CHECK_WITH_RET(*gatewayTable, TAG, "*gatewayTable");

    u_linklist_iterator_t *iterTable = NULL;
    u_linklist_init_iterator(*gatewayTable, &iterTable);
    while (NULL != iterTable)
    {
        RTMGatewayEntry_t *entry = u_linklist_get_data(iterTable);
        if (NULL != entry && NULL != entry->destination &&
            gatewayId == entry->destination->gatewayId)
        {
            if (addAdr)
            {
                for (uint32_t i = 0; i < u_arraylist_length(entry->destination->destIntfAddr); i++)
                {
                    RTMDestIntfInfo_t *destCheck =
                        u_arraylist_get(entry->destination->destIntfAddr, i);
                    if (NULL == destCheck)
                    {
                        OC_LOG(ERROR, TAG, "Destination adr get failed");
                        continue;
                    }

                    if (0 == memcmp(destCheck->destIntfAddr.addr, destInterfaces.destIntfAddr.addr,
                        strlen(destInterfaces.destIntfAddr.addr))
                        && destInterfaces.destIntfAddr.port == destCheck->destIntfAddr.port)
                    {
                        destCheck->timeElapsed = RTMGetCurrentTime();
                        destCheck->isValid = true;
                        OC_LOG(ERROR, TAG, "destInterfaces already present");
                        return OC_STACK_ERROR;
                    }
                }

                RTMDestIntfInfo_t *destAdr =
                        (RTMDestIntfInfo_t *) OICCalloc(1, sizeof(RTMDestIntfInfo_t));
                if (NULL == destAdr)
                {
                    OC_LOG(ERROR, TAG, "Calloc destAdr failed");
                    return OC_STACK_ERROR;
                }
                *destAdr = destInterfaces;
                destAdr->timeElapsed = RTMGetCurrentTime();
                destAdr->isValid = true;
                bool result =
                    u_arraylist_add(entry->destination->destIntfAddr, (void *)destAdr);
                if (!result)
                {
                    OC_LOG(ERROR, TAG, "Updating Destinterface address failed");
                    OICFree(destAdr);
                    return OC_STACK_ERROR;
                }
                OC_LOG(DEBUG, TAG, "OUT");
                return OC_STACK_DUPLICATE_REQUEST;
            }

            for (uint32_t i = 0; i < u_arraylist_length(entry->destination->destIntfAddr); i++)
            {
                RTMDestIntfInfo_t *removeAdr =
                    u_arraylist_get(entry->destination->destIntfAddr, i);
                if (!removeAdr)
                {
                    continue;
                }
                if (0 == memcmp(removeAdr->destIntfAddr.addr, destInterfaces.destIntfAddr.addr,
                    strlen(destInterfaces.destIntfAddr.addr))
                    && destInterfaces.destIntfAddr.port == removeAdr->destIntfAddr.port)
                {
                    RTMDestIntfInfo_t *data =
                        u_arraylist_remove(entry->destination->destIntfAddr, i);
                    OICFree(data);
                    break;
                }
            }
        }
        u_linklist_get_next(&iterTable);
    }
    OC_LOG(DEBUG, TAG, "OUT");
    return OC_STACK_OK;
}

OCStackResult RTMUpdateMcastSeqNumber(uint32_t gatewayId, uint16_t seqNum,
                                      u_linklist_t **gatewayTable)
{
    OC_LOG(DEBUG, TAG, "IN");
    RM_NULL_CHECK_WITH_RET(gatewayTable, TAG, "gatewayTable");
    RM_NULL_CHECK_WITH_RET(*gatewayTable, TAG, "*gatewayTable");

    u_linklist_iterator_t *iterTable = NULL;
    u_linklist_init_iterator(*gatewayTable, &iterTable);
    while (NULL != iterTable)
    {
        RTMGatewayEntry_t *entry = u_linklist_get_data(iterTable);
        if (NULL != entry && NULL != entry->destination &&
            gatewayId == entry->destination->gatewayId)
        {
            if (0 == entry->mcastMessageSeqNum || entry->mcastMessageSeqNum < seqNum)
            {
                entry->mcastMessageSeqNum = seqNum;
                return OC_STACK_OK;
            }
            else if (entry->mcastMessageSeqNum == seqNum)
            {
                return OC_STACK_DUPLICATE_REQUEST;
            }
            else
            {
                return OC_STACK_COMM_ERROR;
            }
        }
        u_linklist_get_next(&iterTable);
    }
    OC_LOG(DEBUG, TAG, "OUT");
    return OC_STACK_OK;
}

uint64_t RTMGetCurrentTime()
{
    uint64_t currentTime = 0;

#ifdef __ANDROID__
    struct timespec getTs;

    clock_gettime(CLOCK_MONOTONIC, &getTs);

    currentTime = getTs.tv_sec;
#elif defined __ARDUINO__
    currentTime = millis() / 1000;
#else
#if _POSIX_TIMERS > 0
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    currentTime = ts.tv_sec;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    currentTime = tv.tv_sec;
#endif
#endif
    return currentTime;
}

OCStackResult RTMUpdateDestAddrValidity(u_linklist_t **invalidTable, u_linklist_t **gatewayTable)
{
    OC_LOG(DEBUG, TAG, "IN");
    RM_NULL_CHECK_WITH_RET(invalidTable, TAG, "invalidTable");
    RM_NULL_CHECK_WITH_RET(gatewayTable, TAG, "gatewayTable");
    RM_NULL_CHECK_WITH_RET(*gatewayTable, TAG, "*gatewayTable");

    *invalidTable = u_linklist_create();
    if (NULL == *invalidTable)
    {
        OC_LOG(ERROR, TAG, "u_linklist_create failed");
        return OC_STACK_NO_MEMORY;
    }

    u_linklist_iterator_t *iterTable = NULL;
    uint64_t presentTime = RTMGetCurrentTime();

    u_linklist_init_iterator(*gatewayTable, &iterTable);
    while (NULL != iterTable)
    {
        RTMGatewayEntry_t *entry = (RTMGatewayEntry_t *) u_linklist_get_data(iterTable);
        if (NULL == entry)
        {
            u_linklist_get_next(&iterTable);
            continue;
        }
        else if (1 == entry->routeCost)
        {
            for (uint32_t i = 0; i < u_arraylist_length(entry->destination->destIntfAddr); i++)
            {
                RTMDestIntfInfo_t *destCheck = u_arraylist_get(entry->destination->destIntfAddr, i);
                if (!destCheck)
                {
                    continue;
                }
                if (GATEWAY_ALIVE_TIMEOUT < (presentTime - destCheck->timeElapsed))
                {
                    destCheck->isValid = false;
                    u_linklist_add(*invalidTable, (void *)destCheck);
                }
            }
        }
        else if (1 < entry->routeCost)
        {
            break;
        }
        u_linklist_get_next(&iterTable);
    }
    OC_LOG(DEBUG, TAG, "OUT");
    return OC_STACK_OK;
}

OCStackResult RTMRemoveInvalidGateways(u_linklist_t **invalidTable, u_linklist_t **gatewayTable)
{
    OC_LOG(DEBUG, TAG, "IN");
    RM_NULL_CHECK_WITH_RET(invalidTable, TAG, "invalidTable");
    RM_NULL_CHECK_WITH_RET(gatewayTable, TAG, "gatewayTable");
    RM_NULL_CHECK_WITH_RET(*gatewayTable, TAG, "*gatewayTable");

    *invalidTable = u_linklist_create();
    if (NULL == *invalidTable)
    {
        OC_LOG(ERROR, TAG, "u_linklist_create failed");
        return OC_STACK_NO_MEMORY;
    }

    u_linklist_iterator_t *iterTable = NULL;
    u_linklist_init_iterator(*gatewayTable, &iterTable);
    while (iterTable != NULL)
    {
        RTMGatewayEntry_t *entry = u_linklist_get_data(iterTable);

        if (NULL == entry)
        {
            u_linklist_get_next(&iterTable);
            continue;
        }
        else if (NULL != entry->destination && (1 == entry->routeCost))
        {
            for (uint32_t i = 0; i < u_arraylist_length(entry->destination->destIntfAddr); i++)
            {
                RTMDestIntfInfo_t *destCheck = u_arraylist_get(entry->destination->destIntfAddr, i);
                if (!destCheck && !destCheck->isValid)
                {
                    void *data = u_arraylist_remove(entry->destination->destIntfAddr, i);
                    OICFree(data);
                    i--;
                }
            }

            if (0 == u_arraylist_length(entry->destination->destIntfAddr))
            {
                u_arraylist_free(&(entry->destination->destIntfAddr));
                OCStackResult res =
                    RTMRemoveGatewayEntry(entry->destination->gatewayId, invalidTable, gatewayTable);
                if (OC_STACK_OK != res)
                {
                    OC_LOG(ERROR, TAG, "Removing Entries failed");
                    return OC_STACK_ERROR;
                }
                u_linklist_get_next(&iterTable);
            }
            else
            {
                u_linklist_get_next(&iterTable);
            }
        }
        else if (1 < entry->routeCost)
        {
            break;
        }
        else
        {
            u_linklist_get_next(&iterTable);
        }
    }
    OC_LOG(DEBUG, TAG, "OUT");
    return OC_STACK_OK;
}

OCStackResult RTMUpdateEntryParameters(uint32_t gatewayId, uint32_t seqNum,
                                       const RTMDestIntfInfo_t *destAdr, u_linklist_t **gatewayTable,
                                       bool forceUpdate)
{
    OC_LOG(DEBUG, TAG, "IN");
    RM_NULL_CHECK_WITH_RET(gatewayTable, TAG, "gatewayTable");
    RM_NULL_CHECK_WITH_RET(*gatewayTable, TAG, "*gatewayTable");
    RM_NULL_CHECK_WITH_RET(destAdr, TAG, "destAdr");

    u_linklist_iterator_t *iterTable = NULL;
    u_linklist_init_iterator(*gatewayTable, &iterTable);
    while (NULL != iterTable)
    {
        RTMGatewayEntry_t *entry = u_linklist_get_data(iterTable);

        if (NULL == entry)
        {
            u_linklist_get_next(&iterTable);
            continue;
        }
        if (NULL != entry->destination && gatewayId == entry->destination->gatewayId)
        {
            for (uint32_t i = 0; i < u_arraylist_length(entry->destination->destIntfAddr); i++)
            {
                RTMDestIntfInfo_t *destCheck =
                    u_arraylist_get(entry->destination->destIntfAddr, i);
                if (NULL != destCheck &&
                    (0 == memcmp(destCheck->destIntfAddr.addr, destAdr->destIntfAddr.addr,
                     strlen(destAdr->destIntfAddr.addr)))
                     && destAdr->destIntfAddr.port == destCheck->destIntfAddr.port)
                {
                    destCheck->timeElapsed = RTMGetCurrentTime();
                    destCheck->isValid = true;
                }
            }

            if (0 != entry->seqNum && seqNum == entry->seqNum)
            {
                return OC_STACK_DUPLICATE_REQUEST;
            }
            else if (0 != entry->seqNum && seqNum != ((entry->seqNum) + 1) && !forceUpdate)
            {
                return OC_STACK_COMM_ERROR;
            }
            else
            {
                entry->seqNum = seqNum;
                OC_LOG(DEBUG, TAG, "OUT");
                return OC_STACK_OK;
            }
        }
        u_linklist_get_next(&iterTable);
    }
    OC_LOG(DEBUG, TAG, "OUT");
    return OC_STACK_OK;
}

void RTMPrintTable(const u_linklist_t *gatewayTable, const u_linklist_t *endpointTable)
{
    RM_NULL_CHECK_VOID(gatewayTable, TAG, "gatewayTable");
    OC_LOG(DEBUG, RM_TAG, "=================Gateway List table============================\n");
    u_linklist_iterator_t *iterTable = NULL;
    u_linklist_init_iterator(gatewayTable, &iterTable);
    while (NULL != iterTable)
    {
        RTMGatewayEntry_t *hop =
            (RTMGatewayEntry_t *) u_linklist_get_data(iterTable);
        if (NULL == hop)
        {
            OC_LOG(ERROR, RM_TAG, "Printing Table Failed");
            return;
        }
        if (NULL == hop->nextHop || 0 == hop->nextHop->gatewayId)
        {
            OC_LOG_V(DEBUG, RM_TAG, "\nDestination : %u\nNextHop : (null)\nHopCount : %d",
                     hop->destination->gatewayId, hop->routeCost);
            OC_LOG_V(DEBUG, RM_TAG, "\nSequence Number :%u", hop->seqNum);
        }
        else
        {
            OC_LOG_V(DEBUG, RM_TAG, "\nDestination : %u\nNextHop : %u\nHopCount : %d",
                     hop->destination->gatewayId, hop->nextHop->gatewayId, hop->routeCost);
            OC_LOG_V(DEBUG, RM_TAG, "\nSequence Number :%u", hop->seqNum);
        }
        if (1 == hop->routeCost && NULL != hop->destination &&
            hop->destination->destIntfAddr != NULL)
        {
            for (uint32_t i = 0; i < u_arraylist_length(hop->destination->destIntfAddr); i++)
            {
                RTMDestIntfInfo_t *dest = u_arraylist_get(hop->destination->destIntfAddr, i);
                if (NULL != dest)
                {
                    OC_LOG_V(DEBUG, RM_TAG, "\nDestination interface addresses: %s Port : %d Obs ID: %d",
                             dest->destIntfAddr.addr, dest->destIntfAddr.port, dest->observerId);
                    OC_LOG_V(DEBUG, RM_TAG, "Validity: %d", dest->isValid);
                }
            }
        }
        OC_LOG(DEBUG, RM_TAG, "********************************************\n");
        u_linklist_get_next(&iterTable);
    }

    OC_LOG(DEBUG, RM_TAG, "=================Endpoint List table============================\n");
    u_linklist_iterator_t *iterEndpointTable = NULL;
    u_linklist_init_iterator(endpointTable, &iterEndpointTable);

    // Iterate over endpoint list to find if already entry for gatewayid is present.
    while (NULL != iterEndpointTable)
    {
        RTMEndpointEntry_t *hop =
            (RTMEndpointEntry_t *) u_linklist_get_data(iterEndpointTable);
        if (NULL == hop)
        {
            OC_LOG(ERROR, RM_TAG, "Printing Table Failed");
            return;
        }
        OC_LOG_V(DEBUG, RM_TAG, "EndpointId : %u\naddr : %s Port : %d",
                  hop->endpointId, hop->destIntfAddr.addr, hop->destIntfAddr.port);

        OC_LOG(DEBUG, RM_TAG, "********************************************\n");
        u_linklist_get_next(&iterEndpointTable);
    }
}
