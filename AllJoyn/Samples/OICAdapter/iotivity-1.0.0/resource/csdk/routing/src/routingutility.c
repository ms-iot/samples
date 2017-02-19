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
#include "routingutility.h"
#include "routingmanager.h"
#include "oic_malloc.h"
#include "include/logger.h"

/**
 * Logging tag for module name.
 */
#define TAG "RM_UTIL"

/**
 * Tag for printing the logs of forwarding the packet.
 */
#define RM_TAG "RAP"

/**
 * Minimum routing option data length is
 * length of src address(1byte) + length of destination address(1byte) + hop count(2bytes)
 */
#define MIN_ROUTE_OPTION_LEN 4

// destination and source are <GatewayId><ClientId> here, where ClientId is optional.
OCStackResult RMAddInfo(const char *destination, CAHeaderOption_t **options,
                        uint8_t *numOptions)
{
    OC_LOG(DEBUG, TAG, "IN");
    RM_NULL_CHECK_WITH_RET(options, TAG, "options");
    RM_NULL_CHECK_WITH_RET(numOptions, TAG, "numOptions");

    CAHeaderOption_t *optionPtr = NULL;
    int8_t index = -1;

    RMGetRouteOptionIndex(*options, *numOptions, &index);

    if (-1 < index)
    {
        OC_LOG(INFO, TAG, "Route option is present");
        optionPtr = *options;
    }
    else
    {
        OC_LOG(INFO, TAG, "Route option is not present");
        index = *numOptions;
        optionPtr = OICCalloc((*numOptions + 1), sizeof(CAHeaderOption_t));
        if (!optionPtr)
        {
            OC_LOG(ERROR, TAG, "OICCalloc failed");
            return OC_STACK_NO_MEMORY;
        }

        memcpy(optionPtr, *options, sizeof(CAHeaderOption_t) * (*numOptions));
    }

    OCStackResult res = OC_STACK_OK;
    RMRouteOption_t routeOption = {.destGw = 0};
    if (*numOptions != index)
    {
        OC_LOG(INFO, TAG, "Route option is already present");
        res = RMParseRouteOption(&optionPtr[index], &routeOption);
        if (OC_STACK_OK != res)
        {
            OC_LOG(ERROR, TAG, "RMParseRouteOption failed");
            return OC_STACK_ERROR;
        }
    }

    if(destination)
    {
        memcpy(&(routeOption.destGw), destination, sizeof(routeOption.destGw));
        memcpy(&(routeOption.destEp), destination + sizeof(routeOption.destGw),
               sizeof(routeOption.destEp));
    }

#ifdef ROUTING_GATEWAY
    // A gateway is supposed to add its ID as source.
    uint32_t gatewayId = RMGetGatewayId();
    if (gatewayId)
    {
        memcpy(&(routeOption.srcGw), &gatewayId, sizeof(routeOption.srcGw));
    }

    if(!routeOption.destGw)
    {
        routeOption.mSeqNum = RMGetMcastSeqNumber();
    }
#endif

    res = RMCreateRouteOption(&routeOption, optionPtr + index);
    if (OC_STACK_OK != res)
    {
        OC_LOG(ERROR, TAG, "Creation of routing option failed");
        OICFree(optionPtr);
        return res;
    }

    if ((*numOptions) == index )
    {
        (*numOptions) = (*numOptions) + 1;
        OICFree(*options);
        *options = optionPtr;
    }

    OC_LOG(DEBUG, TAG, "OUT");
    return OC_STACK_OK;
}

OCStackResult RMUpdateInfo(CAHeaderOption_t **options, uint8_t *numOptions,
                           CAEndpoint_t *endpoint)
{
    OC_LOG(DEBUG, TAG, "IN");
    RM_NULL_CHECK_WITH_RET(options, TAG, "options");
    RM_NULL_CHECK_WITH_RET(*options, TAG, "invalid option");
    RM_NULL_CHECK_WITH_RET(numOptions, TAG, "numOptions");
    RM_NULL_CHECK_WITH_RET(endpoint, TAG, "endpoint");

    if (0 >= *numOptions)
    {
        OC_LOG(ERROR, TAG, "Invalid arguement: numOptions");
        return OC_STACK_ERROR;
    }

    int8_t routeIndex = -1;
    RMGetRouteOptionIndex(*options, *numOptions, &routeIndex);

    if (-1 >= routeIndex)
    {
        OC_LOG(DEBUG, TAG, "Nothing to remove.");
        return OC_STACK_OK;
    }

    // Update Endpoint with source address from RM header option.
    if (0 != (*options + routeIndex)->optionLength)
    {
        uint8_t dLen = 0;
        uint16_t count = sizeof(dLen);
        memcpy(&dLen, (*options + routeIndex)->optionData, sizeof(dLen));
        count += dLen;
        uint8_t sLen = 0;
        memcpy(&sLen, (*options + routeIndex)->optionData + count, sizeof(sLen));
        count += sizeof(sLen);
        if (0 < sLen)
        {
            memcpy(endpoint->routeData, (*options + routeIndex)->optionData + count,
                   GATEWAY_ID_LENGTH);
            OC_LOG_V(DEBUG, TAG, "adding srcgid: %u in endpoint [%d]",
                     *((uint32_t *)endpoint->routeData), sLen);

            count += GATEWAY_ID_LENGTH;

            if (GATEWAY_ID_LENGTH < sLen)
            {
                memcpy(endpoint->routeData + GATEWAY_ID_LENGTH,
                       (*options + routeIndex)->optionData + count, ENDPOINT_ID_LENGTH);
                OC_LOG_V(DEBUG, TAG, "adding srceid: %u in endpoint",
                         *((uint16_t *)(endpoint->routeData + GATEWAY_ID_LENGTH)));
            }
        }
    }

    // Remove route option from header.
    for (uint8_t i = routeIndex; i < (*numOptions)-1; i++)
    {
        memcpy((*options) + i, (*options)+i+1, sizeof(**options));
    }
    *numOptions = (*numOptions) - 1;

    if (0 == *numOptions)
    {
        // Remove route option.
        OICFree(*options);
        *options = NULL;
    }
    OC_LOG(DEBUG, TAG, "OUT");
    return OC_STACK_OK;
}

void RMGetRouteOptionIndex(const CAHeaderOption_t *options, uint8_t numOptions, int8_t *index)
{
    OC_LOG(DEBUG, TAG, "IN");
    RM_NULL_CHECK_VOID(options, TAG, "options");
    RM_NULL_CHECK_VOID(index, TAG, "index");
    for (uint32_t i = 0; i < numOptions; i++)
    {
        OC_LOG_V(DEBUG, TAG, "Request- optionID: %d", options[i].optionID);
        if (RM_OPTION_MESSAGE_SWITCHING == options[i].optionID)
        {
            OC_LOG_V(INFO, TAG, "Found Option at %d", i);
            *index = i;
            break;
        }
    }
    OC_LOG(DEBUG, TAG, "OUT");
}

OCStackResult RMCreateRouteOption(const RMRouteOption_t *optValue, CAHeaderOption_t *options)
{
    OC_LOG(DEBUG, RM_TAG, "IN");
    RM_NULL_CHECK_WITH_RET(optValue, RM_TAG, "optValue");
    RM_NULL_CHECK_WITH_RET(options, RM_TAG, "options");

    uint8_t dLen = (optValue->destGw ? GATEWAY_ID_LENGTH:0) +
                        (optValue->destEp ? ENDPOINT_ID_LENGTH:0);
    uint8_t sLen = (optValue->srcGw ? GATEWAY_ID_LENGTH:0) +
                        (optValue->srcEp ? ENDPOINT_ID_LENGTH:0);

    OC_LOG_V(DEBUG, RM_TAG, "createoption dlen %u slen [%u]", dLen, sLen);
    unsigned int totalLength = MIN_ROUTE_OPTION_LEN + dLen + sLen;
    void *tempData = OICCalloc(totalLength, sizeof(char));
    if (NULL == tempData)
    {
        OC_LOG(ERROR, RM_TAG, "Calloc failed");
        return OC_STACK_NO_MEMORY;
    }
    memcpy(tempData, &dLen, sizeof(dLen));
    unsigned int count = sizeof(dLen);
    if (0 < dLen)
    {
        if (optValue->destGw)
        {
            memcpy(tempData + count, &(optValue->destGw), GATEWAY_ID_LENGTH);
            count += GATEWAY_ID_LENGTH;
        }

        if (optValue->destEp)
        {
            memcpy(tempData + count, &(optValue->destEp), ENDPOINT_ID_LENGTH);
            count += ENDPOINT_ID_LENGTH;
        }
    }

    memcpy(tempData + count, &sLen, sizeof(sLen));
    count += sizeof(sLen);
    if (0 < sLen)
    {
        if (optValue->srcGw)
        {
            memcpy(tempData + count, &(optValue->srcGw), GATEWAY_ID_LENGTH);
            count += GATEWAY_ID_LENGTH;
        }

        if (optValue->srcEp)
        {
            memcpy(tempData + count, &(optValue->srcEp), ENDPOINT_ID_LENGTH);
            count += ENDPOINT_ID_LENGTH;
        }
    }

    memcpy(tempData + count, &optValue->mSeqNum, sizeof(optValue->mSeqNum));
    memcpy(options->optionData, tempData, totalLength);

    options->optionID = RM_OPTION_MESSAGE_SWITCHING;
    options->optionLength = totalLength;

    OC_LOG_V(INFO, RM_TAG, "Option Length is %d", options->optionLength);

    OICFree(tempData);
    OC_LOG(DEBUG, RM_TAG, "OUT");
    return OC_STACK_OK;
}

OCStackResult RMParseRouteOption(const CAHeaderOption_t *options, RMRouteOption_t *optValue)
{
    OC_LOG(DEBUG, RM_TAG, "IN");
    RM_NULL_CHECK_WITH_RET(options, RM_TAG, "options");
    RM_NULL_CHECK_WITH_RET(optValue, RM_TAG, "optValue");
    if (0 == options->optionLength)
    {
        OC_LOG(ERROR, RM_TAG, "Option data is not present");
        return OC_STACK_ERROR;
    }

    uint8_t dLen = 0 ;
    uint16_t count = sizeof(dLen);
    memcpy(&dLen, options->optionData, sizeof(dLen));
    if (0 < dLen)
    {
        memcpy(&(optValue->destGw), options->optionData + count, GATEWAY_ID_LENGTH);
        count += GATEWAY_ID_LENGTH;

        if (GATEWAY_ID_LENGTH < dLen)
        {
            memcpy(&(optValue->destEp), options->optionData + count, ENDPOINT_ID_LENGTH);
            count += ENDPOINT_ID_LENGTH;
        }
    }

    uint8_t sLen = 0;
    memcpy(&sLen, options->optionData + count, sizeof(sLen));
    count += sizeof(sLen);
    if (0 < sLen)
    {
        memcpy(&(optValue->srcGw), options->optionData + count, GATEWAY_ID_LENGTH);
        count += GATEWAY_ID_LENGTH;

        if (GATEWAY_ID_LENGTH < sLen)
        {
            memcpy(&(optValue->srcEp), options->optionData + count, ENDPOINT_ID_LENGTH);
            count += ENDPOINT_ID_LENGTH;
        }
    }
    memcpy(&optValue->mSeqNum, options->optionData + count, sizeof(optValue->mSeqNum));

    OC_LOG_V(INFO, RM_TAG, "Option hopcount is %d", optValue->mSeqNum);
    OC_LOG_V(INFO, RM_TAG, "Option Sender Addr is [%u][%u]", optValue->srcGw, optValue->srcEp);
    OC_LOG_V(INFO, RM_TAG, "Option Dest Addr is [%u][%u]", optValue->destGw, optValue->destEp);
    OC_LOG(DEBUG, RM_TAG, "OUT");
    return OC_STACK_OK;
}
