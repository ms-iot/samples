/******************************************************************
 *
 * Copyright 2014 Samsung Electronics All Rights Reserved.
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

#include <string.h>

#include "oic_malloc.h"
#include "oic_string.h"
#include "caremotehandler.h"
#include "logger.h"

#define TAG "CA"

CAEndpoint_t *CACloneEndpoint(const CAEndpoint_t *rep)
{
    if (NULL == rep)
    {
        OIC_LOG(ERROR, TAG, "parameter is null");
        return NULL;
    }

    // allocate the remote end point structure.
    CAEndpoint_t *clone = (CAEndpoint_t *)OICMalloc(sizeof (CAEndpoint_t));
    if (NULL == clone)
    {
        OIC_LOG(ERROR, TAG, "CACloneRemoteEndpoint Out of memory");
        return NULL;
    }
    *clone = *rep;

    return clone;
}

CARequestInfo_t *CACloneRequestInfo(const CARequestInfo_t *rep)
{
    if (NULL == rep)
    {
        OIC_LOG(ERROR, TAG, "parameter is null");
        return NULL;
    }

    // allocate the request info structure.
    CARequestInfo_t *clone = (CARequestInfo_t *) OICMalloc(sizeof(CARequestInfo_t));
    if (!clone)
    {
        OIC_LOG(ERROR, TAG, "CACloneRequestInfo Out of memory");
        return NULL;
    }

    CAResult_t result = CACloneInfo(&rep->info, &clone->info);
    if(CA_STATUS_OK != result)
    {
        OIC_LOG(ERROR, TAG, "CACloneRequestInfo error in CACloneInfo");
        CADestroyRequestInfoInternal(clone);
        return NULL;
    }

    clone->method = rep->method;
    clone->isMulticast = rep->isMulticast;

    return clone;
}

CAResponseInfo_t *CACloneResponseInfo(const CAResponseInfo_t *rep)
{
    if (NULL == rep)
    {
        OIC_LOG(ERROR, TAG, "Response pointer is NULL");
        return NULL;
    }

    // check the result value of response info.
    // Keep this check in sync with CAResponseResult_t
    switch (rep->result)
    {
        case CA_EMPTY:
        case CA_CREATED:
        case CA_DELETED:
        case CA_VALID:
        case CA_CONTENT:
        case CA_CHANGED:
        case CA_CONTINUE:
        case CA_BAD_REQ:
        case CA_UNAUTHORIZED_REQ:
        case CA_BAD_OPT:
        case CA_FORBIDDEN_REQ:
        case CA_NOT_FOUND:
        case CA_NOT_ACCEPTABLE:
        case CA_REQUEST_ENTITY_INCOMPLETE:
        case CA_REQUEST_ENTITY_TOO_LARGE:
        case CA_INTERNAL_SERVER_ERROR:
        case CA_RETRANSMIT_TIMEOUT:
            break;
        default:
            OIC_LOG_V(ERROR, TAG, "Response code  %u is invalid", rep->result);
            return NULL;
    }

    // allocate the response info structure.
    CAResponseInfo_t *clone = (CAResponseInfo_t *) OICCalloc(1, sizeof(CAResponseInfo_t));
    if (NULL == clone)
    {
        OIC_LOG(ERROR, TAG, "CACloneResponseInfo Out of memory");
        return NULL;
    }

    CAResult_t result = CACloneInfo(&rep->info, &clone->info);
    if(CA_STATUS_OK != result)
    {
        OIC_LOG(ERROR, TAG, "CACloneResponseInfo error in CACloneInfo");
        CADestroyResponseInfoInternal(clone);
        return NULL;
    }

    clone->isMulticast = rep->isMulticast;
    clone->result = rep->result;
    return clone;
}

CAEndpoint_t *CACreateEndpointObject(CATransportFlags_t flags,
                                     CATransportAdapter_t adapter,
                                     const char *address,
                                     uint16_t port)
{
    CAEndpoint_t *info = (CAEndpoint_t *)OICCalloc(1, sizeof(CAEndpoint_t));
    if (NULL == info)
    {
        OIC_LOG(ERROR, TAG, "Memory allocation failed !");
        return NULL;
    }

    if (address)
    {
        OICStrcpy(info->addr, sizeof(info->addr), address);
        info->addr[MAX_ADDR_STR_SIZE_CA - 1] = '\0';
    }
    info->flags = flags;
    info->adapter = adapter;
    info->port = port;

    return info;
}

void CAFreeEndpoint(CAEndpoint_t *rep)
{
    OICFree(rep);
}

static void CADestroyInfoInternal(CAInfo_t *info)
{
    // free token field
    OICFree(info->token);
    info->token = NULL;
    info->tokenLength = 0;

    // free options field
    OICFree(info->options);
    info->options = NULL;
    info->numOptions = 0;

    // free payload field
    OICFree((char *) info->payload);
    info->payload = NULL;
    info->payloadSize = 0;

    // free uri
    OICFree(info->resourceUri);
    info->resourceUri = NULL;
}

void CADestroyRequestInfoInternal(CARequestInfo_t *rep)
{
    if (NULL == rep)
    {
        OIC_LOG(ERROR, TAG, "parameter is null");
        return;
    }

    CADestroyInfoInternal(&rep->info);
    OICFree(rep);
}

void CADestroyResponseInfoInternal(CAResponseInfo_t *rep)
{
    if (NULL == rep)
    {
        OIC_LOG(ERROR, TAG, "parameter is null");
        return;
    }

    CADestroyInfoInternal(&rep->info);
    OICFree(rep);
}

void CADestroyErrorInfoInternal(CAErrorInfo_t *errorInfo)
{
    if (NULL == errorInfo)
    {
        OIC_LOG(ERROR, TAG, "parameter is null");
        return;
    }

    CADestroyInfoInternal(&errorInfo->info);
    OICFree(errorInfo);
}

CAResult_t CACloneInfo(const CAInfo_t *info, CAInfo_t *clone)
{
    if (!info || !clone)
    {
        OIC_LOG(ERROR, TAG, "input parameter invalid");
        return CA_STATUS_INVALID_PARAM;
    }

    memset(clone, 0 , sizeof(CAInfo_t));

    //Do not free clone. we cannot declare it const, as the content is modified
    if ((info->token) && (0 < info->tokenLength))
    {
        char *temp = NULL;

        // allocate token field
        uint8_t len = info->tokenLength;

        temp = (char *) OICMalloc(len * sizeof(char));
        if (!temp)
        {
            OIC_LOG(ERROR, TAG, "CACloneInfo Out of memory");
            return CA_MEMORY_ALLOC_FAILED;
        }

        memcpy(temp, info->token, len);
        // save the token
        clone->token = temp;
        clone->tokenLength = len;
    }

    if (info->options && (0 < info->numOptions))
    {
        // save the options
        clone->options =
            (CAHeaderOption_t *) OICMalloc(sizeof(CAHeaderOption_t) * info->numOptions);

        if (!clone->options)
        {
            OIC_LOG(ERROR, TAG, "CACloneInfo Out of memory");
            CADestroyInfoInternal(clone);
            return CA_MEMORY_ALLOC_FAILED;
        }
        memcpy(clone->options, info->options, sizeof(CAHeaderOption_t) * info->numOptions);
        clone->numOptions = info->numOptions;
    }

    if ((info->payload) && (0 < info->payloadSize))
    {
        // allocate payload field
        uint8_t *temp = OICMalloc(info->payloadSize);
        if (!temp)
        {
            OIC_LOG(ERROR, TAG, "CACloneInfo Out of memory");
            CADestroyInfoInternal(clone);
            return CA_MEMORY_ALLOC_FAILED;
        }
        memcpy(temp, info->payload, info->payloadSize);

        // save the payload
        clone->payload = temp;
        clone->payloadSize = info->payloadSize;
    }
    clone->payloadFormat = info->payloadFormat;
    clone->acceptFormat = info->acceptFormat;

    if (info->resourceUri)
    {
        // allocate payload field
        char *temp = OICStrdup(info->resourceUri);
        if (!temp)
        {
            OIC_LOG(ERROR, TAG, "CACloneInfo Out of memory");
            CADestroyInfoInternal(clone);
            return CA_MEMORY_ALLOC_FAILED;
        }

        // save the resourceUri
        clone->resourceUri = temp;
    }

#ifdef ROUTING_GATEWAY
    clone->skipRetransmission = info->skipRetransmission;
#endif

    clone->messageId = info->messageId;
    clone->type = info->type;

    return CA_STATUS_OK;

}
