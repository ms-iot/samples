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


#include "occlientcb.h"
#include "utlist.h"
#include "logger.h"
#include "oic_malloc.h"
#include <string.h>

#ifdef WITH_ARDUINO
#include "Time.h"
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif
#include "coap_time.h"

#include "cacommon.h"
#include "cainterface.h"

/// Module Name
#define TAG "occlientcb"

struct ClientCB *cbList = NULL;
static OCMulticastNode * mcPresenceNodes = NULL;

OCStackResult
AddClientCB (ClientCB** clientCB, OCCallbackData* cbData,
             CAToken_t token, uint8_t tokenLength,
             OCDoHandle *handle, OCMethod method,
             OCDevAddr *devAddr, char * requestUri,
             char * resourceTypeName, uint32_t ttl)
{
    if(!clientCB || !cbData || !handle || !requestUri || tokenLength > CA_MAX_TOKEN_LEN)
    {
        return OC_STACK_INVALID_PARAM;
    }

    ClientCB *cbNode = NULL;

#ifdef WITH_PRESENCE
    if(method == OC_REST_PRESENCE)
    {   // Retrieve the presence callback structure for this specific requestUri.
        cbNode = GetClientCB(NULL, 0, NULL, requestUri);
    }
#endif // WITH_PRESENCE

    if(!cbNode)// If it does not already exist, create new node.
    {
        cbNode = (ClientCB*) OICMalloc(sizeof(ClientCB));
        if(!cbNode)
        {
            *clientCB = NULL;
            goto exit;
        }
        else
        {
            OC_LOG(INFO, TAG, "Adding client callback with token");
            OC_LOG_BUFFER(INFO, TAG, (const uint8_t *)token, tokenLength);
            cbNode->callBack = cbData->cb;
            cbNode->context = cbData->context;
            cbNode->deleteCallback = cbData->cd;
            //Note: token memory is allocated in the caller OCDoResource
            //but freed in DeleteClientCB
            cbNode->token = token;
            cbNode->tokenLength = tokenLength;
            cbNode->handle = *handle;
            cbNode->method = method;
            cbNode->sequenceNumber = 0;
            #ifdef WITH_PRESENCE
            cbNode->presence = NULL;
            cbNode->filterResourceType = NULL;
            #endif // WITH_PRESENCE

            if (method == OC_REST_PRESENCE ||
                method == OC_REST_OBSERVE  ||
                method == OC_REST_OBSERVE_ALL)
            {
                cbNode->TTL = 0;
            }
            else
            {
                cbNode->TTL = ttl;
            }
            cbNode->requestUri = requestUri;    // I own it now
            cbNode->devAddr = devAddr;          // I own it now
            OC_LOG_V(INFO, TAG, "Added Callback for uri : %s", requestUri);
            LL_APPEND(cbList, cbNode);
            *clientCB = cbNode;
        }
    }
    else
    {
        // Ensure that the handle the SDK hands back up to the application layer for the
        // OCDoResource call matches the found ClientCB Node.
        *clientCB = cbNode;

        if (cbData->cd)
        {
            cbData->cd(cbData->context);
        }

        OICFree(token);
        OICFree(*handle);
        OICFree(requestUri);
        OICFree(devAddr);
        *handle = cbNode->handle;
    }

#ifdef WITH_PRESENCE
    if(method == OC_REST_PRESENCE && resourceTypeName)
    {
        // Amend the found or created node by adding a new resourceType to it.
        return InsertResourceTypeFilter(cbNode,(char *)resourceTypeName);
        // I own resourceTypName now.
    }
    else
    {
        OICFree(resourceTypeName);
    }
#else
    OICFree(resourceTypeName);
#endif

    return OC_STACK_OK;

exit:
     return OC_STACK_NO_MEMORY;
}

void DeleteClientCB(ClientCB * cbNode)
{
    if(cbNode)
    {
        LL_DELETE(cbList, cbNode);
        OC_LOG (INFO, TAG, "Deleting token");
        OC_LOG_BUFFER(INFO, TAG, (const uint8_t *)cbNode->token, cbNode->tokenLength);
        CADestroyToken (cbNode->token);
        OICFree(cbNode->devAddr);
        OICFree(cbNode->handle);
        OC_LOG_V (INFO, TAG, "Deleting callback with uri %s", cbNode->requestUri);
        OICFree(cbNode->requestUri);
        if(cbNode->deleteCallback)
        {
            cbNode->deleteCallback(cbNode->context);
        }

#ifdef WITH_PRESENCE
        if(cbNode->presence)
        {
            OICFree(cbNode->presence->timeOut);
            OICFree(cbNode->presence);
        }
        if(cbNode->method == OC_REST_PRESENCE)
        {
            OCResourceType * pointer = cbNode->filterResourceType;
            OCResourceType * next = NULL;
            while(pointer)
            {
                next = pointer->next;
                OICFree(pointer->resourcetypename);
                OICFree(pointer);
                pointer = next;
            }
        }
#endif // WITH_PRESENCE
        OICFree(cbNode);
        cbNode = NULL;
    }
}

/*
 * This function checks if the node is past its time to live and
 * deletes it if timed-out. Calling this function with a  presence or observe
 * callback with ttl set to 0 will not delete anything as presence nodes have
 * their own mechanisms for timeouts. A null argument will cause the function to
 * silently return.
 */
static void CheckAndDeleteTimedOutCB(ClientCB* cbNode)
{
    if (!cbNode)
    {
        return;
    }
    if (cbNode->TTL == 0)
    {
        return;
    }
    coap_tick_t now;
    coap_ticks(&now);

    if (cbNode->TTL < now)
    {
        OC_LOG(INFO, TAG, "Deleting timed-out callback");
        DeleteClientCB(cbNode);
    }
}

bool ClientTokenExist(const CAToken_t token, uint8_t tokenLength)
{
    bool bRet = false;
    ClientCB* out = NULL;

    if (token && *token && tokenLength <= CA_MAX_TOKEN_LEN && tokenLength > 0)
    {
        OC_LOG(INFO, TAG, "Looking for token");
        OC_LOG_BUFFER(INFO, TAG, (const uint8_t *)token, tokenLength);
        OC_LOG(INFO, TAG, "\tFound in callback list");
        LL_FOREACH(cbList, out)
        {
            OC_LOG_BUFFER(INFO, TAG, (const uint8_t *)out->token, tokenLength);

            if (memcmp(out->token, token, tokenLength) == 0)
            {
                bRet = true;
                break;
            }
        }
    }

    return bRet;
}

ClientCB* GetClientCB(const CAToken_t token, uint8_t tokenLength,
        OCDoHandle handle, const char * requestUri)
{

    ClientCB* out = NULL;

    if(token && *token && tokenLength <= CA_MAX_TOKEN_LEN && tokenLength > 0)
    {
        OC_LOG (INFO, TAG,  "Looking for token");
        OC_LOG_BUFFER(INFO, TAG, (const uint8_t *)token, tokenLength);
        OC_LOG(INFO, TAG, "\tFound in callback list");
        LL_FOREACH(cbList, out)
        {
            OC_LOG_BUFFER(INFO, TAG, (const uint8_t *)out->token, tokenLength);

            if(memcmp(out->token, token, tokenLength) == 0)
            {
                return out;
            }
            CheckAndDeleteTimedOutCB(out);
        }
    }
    else if(handle)
    {
        LL_FOREACH(cbList, out)
        {
            if(out->handle == handle)
            {
                return out;
            }
            CheckAndDeleteTimedOutCB(out);
        }
    }
    else if(requestUri)
    {
        OC_LOG_V(INFO, TAG, "Looking for uri %s", requestUri);
        LL_FOREACH(cbList, out)
        {
            OC_LOG_V(INFO, TAG, "\tFound %s", out->requestUri);
            if(out->requestUri && strcmp(out->requestUri, requestUri ) == 0)
            {
                return out;
            }
            CheckAndDeleteTimedOutCB(out);
        }
    }
    OC_LOG(INFO, TAG, "Callback Not found !!");
    return NULL;
}

#ifdef WITH_PRESENCE
OCStackResult InsertResourceTypeFilter(ClientCB * cbNode, char * resourceTypeName)
{
    OCResourceType * newResourceType = NULL;
    if(cbNode && resourceTypeName)
    {
        // Form a new resourceType member.
        newResourceType = (OCResourceType *) OICMalloc(sizeof(OCResourceType));
        if(!newResourceType)
        {
            return OC_STACK_NO_MEMORY;
        }

        newResourceType->next = NULL;
        newResourceType->resourcetypename = resourceTypeName;

        LL_APPEND(cbNode->filterResourceType, newResourceType);
        return OC_STACK_OK;
    }
    return OC_STACK_ERROR;
}
#endif // WITH_PRESENCE

void DeleteClientCBList()
{
    ClientCB* out;
    ClientCB* tmp;
    LL_FOREACH_SAFE(cbList, out, tmp)
    {
        DeleteClientCB(out);
    }
    cbList = NULL;
}

void FindAndDeleteClientCB(ClientCB * cbNode)
{
    ClientCB* tmp;
    if(cbNode)
    {
        LL_FOREACH(cbList, tmp)
        {
            if (cbNode == tmp)
            {
                DeleteClientCB(tmp);
                break;
            }
        }
    }
}

OCStackResult AddMCPresenceNode(OCMulticastNode** outnode, char* uri, uint32_t nonce)
{
    if(!outnode)
    {
        return OC_STACK_INVALID_PARAM;
    }

    OCMulticastNode *node;

    node = (OCMulticastNode*) OICMalloc(sizeof(OCMulticastNode));

    if (node)
    {
        node->nonce = nonce;
        node->uri = uri;
        LL_APPEND(mcPresenceNodes, node);
        *outnode = node;
        return OC_STACK_OK;
    }
    *outnode = NULL;
    return OC_STACK_NO_MEMORY;
}

OCMulticastNode* GetMCPresenceNode(const char * uri)
{
    OCMulticastNode* out = NULL;

    if(uri)
    {
        LL_FOREACH(mcPresenceNodes, out)
        {
            if(out->uri && strcmp(out->uri, uri) == 0)
            {
                return out;
            }
        }
    }
    OC_LOG(INFO, TAG, "MulticastNode Not found !!");
    return NULL;
}

