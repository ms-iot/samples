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

/**
 * @file
 * This file contains the APIs for routing table manager.
 */
#ifndef ROUTING_TABLE_MANAGER_H_
#define ROUTING_TABLE_MANAGER_H_

#ifndef SINGLE_THREAD
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#endif

#if defined(__ANDROID__)
#include <linux/time.h>
#endif
#include "ulinklist.h"
#include "uarraylist.h"
#include "octypes.h"


#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Maximum hop/destination address length.
 */
#define MAX_DEST_ADDR_LEN 40

/**
 * Maximum number of observers for the gateway resource.
 */
#define MAX_NUM_OBSERVERS 10

/**
 * Maximum time after which gateway should send a notification for its existence.
 * Every 30s gateway will send its existence notification.
 */
#define GATEWAY_ALIVE_TIMEOUT 30

/**
 * The routing table entries are validated for every 40 seconds for its existence.
 */
#define ROUTINGTABLE_REFRESH_TIMEOUT 40

/**
 * The routing table entries are removed if entries are invalid every 45 seconds.
 */
#define ROUTINGTABLE_VALIDATION_TIMEOUT 45

/**
 * Destination Interface Address entries.
 */
typedef struct
{
    uint32_t observerId;                    /**< Observer Id. */
    CAEndpoint_t destIntfAddr;              /**< Destination Interface Address. */
    uint32_t timeElapsed;                   /**< Time elapsed. */
    bool isValid;                           /**< Valid check for Gateway. */
} RTMDestIntfInfo_t;

/**
 * Endpoint Address entries.
 */
typedef struct
{
    uint16_t endpointId;                    /**< Endpoint Id. */
    CAEndpoint_t destIntfAddr;              /**< Destination Interface Address. */
} RTMEndpointEntry_t;

/**
 * Gateway Address entries.
 */
typedef struct gatewayAddress
{
    uint32_t gatewayId;                     /**< Gateway Id. */
    u_arraylist_t *destIntfAddr;            /**< Destination Interface Addresses. */
} RTMGatewayId_t;

/**
 * Routing table entries at Gateway.
 */
typedef struct
{
    RTMGatewayId_t *destination;            /**< destination Address. */
    RTMGatewayId_t *nextHop;                /**< Next Hop Information. */
    uint32_t routeCost;                     /**< routeCost. */
    uint16_t mcastMessageSeqNum;            /**< sequence number for last mcast packet. */
    uint32_t seqNum;                        /**< sequence number for notification. */
} RTMGatewayEntry_t;

/**
 * Initialize the Routing Table Manager.
 * @param[in/out] gatewayTable      Gateway Routing Table.
 * @param[in/out] endpointTable     Endpoint Routing Table.
 * @return  ::OC_STACK_OK or Appropriate error code.
 */
OCStackResult RTMInitialize(u_linklist_t **gatewayTable, u_linklist_t **endpointTable);

/**
 * Terminates the Routing Table Manager.
 * @param[in/out] gatewayTable      Gateway Routing Table.
 * @param[in/out] endpointTable     Endpoint Routing Table.
 * @return  ::OC_STACK_OK or Appropriate error code.
 */
OCStackResult RTMTerminate(u_linklist_t **gatewayTable, u_linklist_t **endpointTable);

/**
 * Frees the gateway table memory with nodes containing structure RTMGatewayEntry_t.
 * @param[in/out] gatewayTable      Gateway Routing Table.
 * @return  ::OC_STACK_OK or Appropriate error code.
 */
OCStackResult RTMFreeGatewayRouteTable(u_linklist_t **gatewayTable);

/**
 * Frees the gateway ID list memory with nodes containing structute RTMGatewayId_t.
 * @param[in/out] gatewayIdTable      Gateway ID list.
 * @return  ::OC_STACK_OK or Appropriate error code.
 */
OCStackResult RTMFreeGatewayIdList(u_linklist_t **gatewayIdTable);

/**
 * Frees the endpoint table memory with nodes containing structute RTMEndpointEntry_t.
 * @param[in/out] endpointTable     Endpoint Routing Table.
 * @return  ::OC_STACK_OK or Appropriate error code.
 */
OCStackResult RTMFreeEndpointRouteTable(u_linklist_t **endpointTable);

/**
 * Adds the entry to the routing table if the entry for Gateway id is
 * not preset in Routing table, Updates the Old entry if Entry for
 * Gateway Id is already present in Routing table i.e routeCost and NextHop
 * will be updated for efficient hop result.
 *
 * @param[in]       gatewayId           Gateway Id.
 * @param[in]       nextHop             Next Hop address.
 * @param[in]       routeCost           Shortest Path to Destination - Hopcount.
 * @param[in]       destInterfaces      Destination Interface Information.
 * @param[in/out]   gatewayTable        Gateway Routing Table.
 * @return  ::OC_STACK_OK or Appropriate error code.
 */
OCStackResult RTMAddGatewayEntry(uint32_t gatewayId, uint32_t nextHop, uint32_t routeCost,
                                 const RTMDestIntfInfo_t *destInterfaces, u_linklist_t **gatewayTable);

/**
 * Adds the endpoint entry to the routing table.
 * @param[in/out]   endpointId          Endpoint Id.
 * @param[in]       destAddr            Destination Address.
 * @param[in/out]   endpointTable       Endpoint Routing Table.
 * @return  ::OC_STACK_OK or Appropriate error code.
 */
OCStackResult RTMAddEndpointEntry(uint16_t *endpointId, const CAEndpoint_t *destAddr,
                                  u_linklist_t **endpointTable);

/**
 * Removes the gateway entry from the routing table and also removes
 * corresponding entries having nexthop as removed gateway.
 * @param[in]        gatewayId              Gateway id of node need to be removed.
 * @param[in/out]    removedGatewayNodes    Linklist containing removed gateway nodes
 *                                          list need to be freed by caller.
 * @param[in/out]    gatewayTable           Gateway Routing Table.
 * @return  ::OC_STACK_OK or Appropriate error code.
 */
OCStackResult RTMRemoveGatewayEntry(uint32_t gatewayId, u_linklist_t **removedGatewayNodes,
                                    u_linklist_t **gatewayTable);

/**
 * Removes the endpoint entry from the routing table.
 * @param[in]       endpointId        Endpoint id of node need to be removed.
 * @param[in/out]   endpointTable     Endpoint Routing Table.
 * @return  ::OC_STACK_OK or Appropriate error code.
 */
OCStackResult RTMRemoveEndpointEntry(uint16_t endpointId, u_linklist_t **endpointTable);

/**
 * Removes the gateway entry from the routing table which has gateway id and nexthop as given.
 * @param[in]        gatewayId              Gateway Id.
 * @param[in]        nextHop                Next Hop address.
 * @param[in]        destInfAdr             Destination Address of Next Hop to update time.
 * @param[in/out]    existEntry             Entry which has different Next Hop.
 * @param[in/out]    gatewayTable           Gateway Routing Table.
 * @return  ::OC_STACK_OK or Appropriate error code.
 */
OCStackResult RTMRemoveGatewayDestEntry(uint32_t gatewayId, uint32_t nextHop,
                                        const RTMDestIntfInfo_t *destInfAdr,
                                        RTMGatewayEntry_t **existEntry, u_linklist_t **gatewayTable);
/**
 * Removes the gateway nodes.
 * @param[in/out]    gatewayTable           Gateway Routing Table.
 * @return  ::OC_STACK_OK or Appropriate error code.
 */
OCStackResult RTMRemoveGateways(u_linklist_t **gatewayTable);

/**
 * Removes the endpoint nodes.
 * @param[in/out]   endpointTable           Endpoint Routing Table.
 * @return  ::OC_STACK_OK or Appropriate error code.
 */
OCStackResult RTMRemoveEndpoints(u_linklist_t **endpointTable);

/**
 * Gets the neighbor nodes i.e nodes with routecost 1.
 * @param[in/out]   neighbourNodes        link list containing neighbor nodes.
                                          this list will be pointer to GatewayIds
                                          and must be freed by caller.
 * @param[in]       gatewayTable           Gateway Routing Table.
 */
void RTMGetNeighbours(u_linklist_t **neighbourNodes, const u_linklist_t *gatewayTable);

/**
 * Gets next hop from the routing table.
 * @param[in]        gatewayId              Gateway Id.
 * @param[in]        gatewayTable           Gateway Routing Table.
 * @return  Next Hop address - returns NULL if it is End Device.
 */
RTMGatewayId_t *RTMGetNextHop(uint32_t gatewayId, const u_linklist_t *gatewayTable);

/**
 * Gets endpoint entry
 * @param[in]       endpointId        Endpoint id of node need to be removed.
 * @param[in]       endpointTable     Endpoint Routing Table.
 * @return  Endpoint Destination inteface address.
 */
CAEndpoint_t *RTMGetEndpointEntry(uint16_t endpointId, const u_linklist_t *endpointTable);

/**
 * Updates destination interface address of an entry with provided gateway id
 * as destination.
 * @param[in]        gatewayId               Gateway Id of Hop need to be updated.
 * @param[in]        destInterfaces          Destination Interface Information.
 * @param[in]        addAdr                  Add/Remove destination address.
 * @param[in/out]    gatewayTable            Gateway Routing Table.
 * @return  ::OC_STACK_OK or Appropriate error code.
 */
OCStackResult RTMUpdateDestinationIntfAdr(uint32_t gatewayId, RTMDestIntfInfo_t destInterfaces,
                                          bool addAdr, u_linklist_t **gatewayTable);

/**
 * Updates Multicast sequence number for gatewayID
 * @param[in]       gatewayId           Gateway Id of Hop need to be updated.
 * @param[in]       seqNum              Sequence number for last cast packet from gateway.
 * @param[in/out]   gatewayTable        Gateway Routing Table.
 * @return  ::OC_STACK_OK or Appropriate error code.
 */
OCStackResult RTMUpdateMcastSeqNumber(uint32_t gatewayId, uint16_t seqNum,
                                      u_linklist_t **gatewayTable);

/**
 * Prints the routing table
 * @param[in]    gatewayTable           Gateway Routing Table.
 * @param[in]    endpointTable          Endpoint Routing Table.
 */
void RTMPrintTable(const u_linklist_t *gatewayTable, const u_linklist_t *endpointTable);

/**
 * Frees the GatewayId
 * @param[in]        gateway                Gateway Structure pointer.
 * @param[in/out]    gatewayTable           Gateway Routing Table.
 */
void RTMFreeGateway(RTMGatewayId_t *gateway, u_linklist_t **gatewayTable);

/**
 * Gets the list of observer IDs.
 * @param[in/out]    obsList                List of Observation IDs.
 * @param[in/out]    obsListLen             Length if Observation ID list.
 * @param[in]        gatewayTable           Gateway Routing Table.
 */
void RTMGetObserverList(OCObservationId **obsList, uint8_t *obsListLen,
                        const u_linklist_t *gatewayTable);

/**
 * Adds a observer address and obsID to the list.
 * @param[in]        obsID                  Observation ID.
 * @param[in]        devAddr                Address of Gateway.
 * @param[in/out]    gatewayTable           Gateway Routing Table.
 * @return  ::OC_STACK_OK or Appropriate error code.
 */
OCStackResult RTMAddObserver(uint32_t obsID, CAEndpoint_t devAddr, u_linklist_t **gatewayTable);

/**
 * Check if a particular observer address is already registerd and returns
 * its obsID if present.
 * @param[in]        devAddr                Address of Gateway.
 * @param[in/out]    obsID                  Observation ID.
 * @param[in]        gatewayTable           Gateway Routing Table.
 * @return  true or false.
 */
bool RTMIsObserverPresent(CAEndpoint_t devAddr, OCObservationId *obsID,
                          const u_linklist_t *gatewayTable);

/**
 * Gets Current Time in Micro Seconds.
 * @return  Current Time.
 */
uint64_t RTMGetCurrentTime();

/**
 * Update Gateway Address Validity.
 * @param[in/out]    invalidTable      Removed entries Table.
 * @param[in/out]    gatewayTable      Gateway Routing Table.
 * @return  ::OC_STACK_OK or Appropriate error code.
 */
OCStackResult RTMUpdateDestAddrValidity(u_linklist_t **invalidTable, u_linklist_t **gatewayTable);

/**
 * Removes invalid gateways.
 * @param[in/out]    invalidTable      Removed entries Table.
 * @param[in/out]    gatewayTable      Gateway Routing Table.
 * @return  ::OC_STACK_OK or Appropriate error code.
 */
OCStackResult RTMRemoveInvalidGateways(u_linklist_t **invalidTable, u_linklist_t **gatewayTable);

/**
 * Update Sequence number of Gateway Entry.
 * @param[in]       gatewayId           Gateway Id.
 * @param[in]       seqNum              Sequence Number of Entry.
 * @param[in]       destInterfaces      Destination Interface Information.
 * @param[out]      gatewayTable        Gateway Routing Table.
 * @param[in]       forceUpdate         To Update parameters forcefully.
 * @return  ::OC_STACK_OK or Appropriate error code.
 */
OCStackResult RTMUpdateEntryParameters(uint32_t gatewayId, uint32_t seqNum,
                                       const RTMDestIntfInfo_t *destAdr, u_linklist_t **gatewayTable,
                                       bool forceUpdate);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ROUTING_TABLE_MANAGER_H_ */

