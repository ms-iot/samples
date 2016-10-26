//******************************************************************
//
// Copyright 2015 Samsung Electronics All Rights Reserved.
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
#include "rd_client.h"

#include <stdarg.h>

#include "oic_string.h"
#include "oic_malloc.h"
#include "payload_logging.h"

#include "rdpayload.h"
#include "ocpayload.h"

#define DEFAULT_CONTEXT_VALUE 0x99
#define OC_RD_PUBLISH_TTL 86400

#define TAG  PCF("RDClient")

static OCStackResult sendRequest(OCMethod method, char *uri, OCDevAddr *addr,
        OCPayload *payload, OCCallbackData cbData)
{
    OCDoHandle handle;
    OCStackResult result;

    result = OCDoResource(&handle,
        method,
        uri,
        addr,
        payload,
        CT_ADAPTER_IP,
        OC_LOW_QOS,
        &cbData,
        NULL,
        0);

    if (result == OC_STACK_OK)
    {
        OC_LOG(DEBUG, TAG, "Resource Directory send successful...");
    }
    else
    {
        OC_LOG(ERROR, TAG, "Resource Directory send failed...");
    }

    return result;
}

static OCStackApplicationResult handlePublishCB(__attribute__((unused))void *ctx,
        __attribute__((unused)) OCDoHandle handle,
        __attribute__((unused)) OCClientResponse *clientResponse)
{
    OCStackApplicationResult ret = OC_STACK_DELETE_TRANSACTION;
    OC_LOG(DEBUG, TAG, "Successfully published resources.");

    if (OC_STACK_OK == OCStopMulticastServer())
    {
        OC_LOG_V(DEBUG, TAG, "Stopped receiving the multicast traffic.");
    }
    else
    {
        OC_LOG_V(DEBUG, TAG, "Failed stopping the multicast traffic.");
    }

    return ret;
}

static void retreiveRDDetails(OCClientResponse *clientResponse, OCRDBiasFactorCB clientCB)
{
    OC_LOG_V(DEBUG, TAG, "\tAddress of the RD: %s:%d", clientResponse->devAddr.addr,
            clientResponse->devAddr.port);

    OCRDPayload *payload = (OCRDPayload *) clientResponse->payload;
    OC_LOG_PAYLOAD(DEBUG, (OCPayload *) payload);

    // TODO: Multiple Resource Directory will have different biasFactor,
    // needs to cache here detail
    // and after certain timeout then decide based on the biasFactor.
    //if (biasFactor > 75)
    if (clientCB)
    {
        clientCB(clientResponse->devAddr.addr, clientResponse->devAddr.port);
    }
}

static OCStackApplicationResult handleDiscoverCB(void *ctx,
        __attribute__((unused)) OCDoHandle handle, OCClientResponse *clientResponse)
{
    OC_LOG(DEBUG, TAG, "Found Resource Directory");
    OCStackApplicationResult ret = OC_STACK_DELETE_TRANSACTION;

    OCRDClientContextCB *cb = (OCRDClientContextCB *)ctx;
    if (!cb)
    {
        OC_LOG(ERROR, TAG, "RD Context Invalid Parameters.");
        return ret;
    }

    if (cb->context != (void *) DEFAULT_CONTEXT_VALUE)
    {
        OC_LOG(ERROR, TAG, "RD Context Invalid Context Value Parameters.");
        return ret;
    }

    OC_LOG_V(DEBUG, TAG, "Callback Context for DISCOVER query received successfully :%d.", clientResponse->result);

    if (clientResponse && clientResponse->result == OC_STACK_OK)
    {
        retreiveRDDetails(clientResponse, cb->cbFunc);
    }
    else
    {
        OC_LOG(ERROR, TAG, "Discovery of RD Failed");
    }

    OICFree(cb);

    return ret;
}

OCStackResult OCRDDiscover(OCRDBiasFactorCB cbBiasFactor)
{
    if (!cbBiasFactor)
    {
        OC_LOG(DEBUG, TAG, "No callback function specified.");
        return OC_STACK_INVALID_CALLBACK;
    }

    /* Start a discovery query*/
    char queryUri[MAX_URI_LENGTH] = { '\0' };
    snprintf(queryUri, MAX_URI_LENGTH, "coap://%s%s", OC_MULTICAST_PREFIX, OC_RSRVD_RD_URI);

    OC_LOG_V(DEBUG, TAG, "Querying RD: %s\n", queryUri);

    OCRDClientContextCB *cbContext = (OCRDClientContextCB *)OICCalloc(1, sizeof(OCRDClientContextCB));
    if (!cbContext)
    {
        OC_LOG(ERROR, TAG, "Failed allocating memory.");
        return OC_STACK_NO_MEMORY;
    }

    cbContext->context = (void *)DEFAULT_CONTEXT_VALUE;
    cbContext->cbFunc = cbBiasFactor;

    OCCallbackData cbData;
    cbData.cb = handleDiscoverCB;
    cbData.context = (void *)(cbContext);
    cbData.cd = NULL;

    return sendRequest(OC_REST_DISCOVER, queryUri, NULL, NULL, cbData);
}

static OCStackResult createStringLL(uint8_t numElements, OCResourceHandle handle,
    const char* (*getValue)(OCResourceHandle handle, uint8_t i), OCStringLL **stringLL)
{
    for (uint8_t i = 0; i < numElements; ++i)
    {
        const char *value = getValue(handle, i);
        if (!*stringLL)
        {
            *stringLL = (OCStringLL *)OICCalloc(1, sizeof(OCStringLL));
            if (!*stringLL)
            {
                OC_LOG(ERROR, TAG, "Failed allocating memory.");
                return OC_STACK_NO_MEMORY;
            }
            (*stringLL)->value = OICStrdup(value);
            if (!(*stringLL)->value)
            {
                OC_LOG(ERROR, TAG, "Failed copying to OCStringLL.");
                return OC_STACK_NO_MEMORY;
            }
        }
        else
        {
            OCStringLL *cur = *stringLL;
            while (cur->next)
            {
                cur = cur->next;
            }
            cur->next = (OCStringLL *)OICCalloc(1, sizeof(OCStringLL));
            if (!cur->next)
            {
                OC_LOG(ERROR, TAG, "Failed allocating memory.");
                return OC_STACK_NO_MEMORY;
            }
            cur->next->value = OICStrdup(value);
            if (!cur->next->value)
            {
                OC_LOG(ERROR, TAG, "Failed copying to OCStringLL.");
                return OC_STACK_NO_MEMORY;
            }
        }
    }
    return OC_STACK_OK;
}

OCStackResult OCRDPublish(char *addr, uint16_t port, int numArg, ... )
{
    if (!addr)
    {
        OC_LOG(ERROR, TAG, "RD address not specified.");
        return OC_STACK_INVALID_PARAM;
    }

    char targetUri[MAX_URI_LENGTH];
    snprintf(targetUri, MAX_URI_LENGTH, "coap://%s:%d%s?rt=%s", addr, port,
            OC_RSRVD_RD_URI, OC_RSRVD_RESOURCE_TYPE_RDPUBLISH);
    OC_LOG_V(DEBUG, TAG, "Target URI : %s", targetUri);

    // Gather all resources locally and do publish
    OCCallbackData cbData = { 0 };
    cbData.cb = &handlePublishCB;
    cbData.cd = NULL;
    cbData.context = (void*)DEFAULT_CONTEXT_VALUE;

    OCTagsPayload *tagsPayload = NULL;
    OCLinksPayload *linksPayload = NULL;
    OCStringLL *rt = NULL;
    OCStringLL *itf = NULL;
    OCStringLL *mt = NULL;

    OCRDPayload *rdPayload = OCRDPayloadCreate();
    if (!rdPayload)
    {
        goto no_memory;
    }

    const unsigned char *id = (unsigned char*) OCGetServerInstanceIDString();
    tagsPayload = OCCopyTagsResources(NULL, id,
            NULL, OC_DISCOVERABLE, 0, 0, NULL, NULL, OC_RD_PUBLISH_TTL);
    if (!tagsPayload)
    {
        goto no_memory;
    }

    va_list arguments;
    va_start (arguments, numArg);

    for (int j = 0 ; j < numArg; j++)
    {
        OCResourceHandle handle = va_arg(arguments, OCResourceHandle);
        if (handle)
        {
            rt = itf = mt = NULL;
            const char *uri = OCGetResourceUri(handle);
            uint8_t numElement;
            if (OC_STACK_OK == OCGetNumberOfResourceTypes(handle, &numElement))
            {
                OCStackResult res = createStringLL(numElement, handle, OCGetResourceTypeName, &rt);
                if (res != OC_STACK_OK || !rt)
                {
                    va_end(arguments);
                    goto no_memory;
                }
            }

            if (OC_STACK_OK == OCGetNumberOfResourceTypes(handle, &numElement))
            {
                OCStackResult res = createStringLL(numElement, handle, OCGetResourceInterfaceName, &itf);
                if (res != OC_STACK_OK || !itf)
                {
                    va_end(arguments);
                    goto no_memory;
                }
            }

            mt = (OCStringLL *)OICCalloc(1, sizeof(OCStringLL));
            if (!mt)
            {
                va_end(arguments);
                goto no_memory;
            }
            mt->value = OICStrdup("application/json");
            if (!mt->value)
            {
                va_end(arguments);
                goto no_memory;
            }

            if (!linksPayload)
            {
                linksPayload = OCCopyLinksResources(uri, rt, itf, NULL, 0, NULL,
                        NULL, j, mt);;
                if (!linksPayload)
                {
                    goto no_memory;
                }
            }
            else
            {
                OCLinksPayload *temp = linksPayload;
                while (temp->next)
                {
                    temp = temp->next;
                }
                temp->next = OCCopyLinksResources(uri, rt, itf, NULL, 0, NULL,
                        NULL, j, mt);
                if (!temp->next)
                {
                    goto no_memory;
                }
            }
            OCFreeOCStringLL(rt);
            OCFreeOCStringLL(itf);
            OCFreeOCStringLL(mt);
        }
    }
    va_end(arguments);

    rdPayload->rdPublish = OCCopyCollectionResource(tagsPayload, linksPayload);
    if (!rdPayload->rdPublish)
    {
        goto no_memory;
    }

    OCDevAddr rdAddr = { 0 };
    OICStrcpy(rdAddr.addr, MAX_ADDR_STR_SIZE, addr);
    rdAddr.port = port;

    OC_LOG_PAYLOAD(DEBUG, (OCPayload *) rdPayload);

    return sendRequest(OC_REST_POST, targetUri, &rdAddr, (OCPayload *)rdPayload, cbData);

no_memory:
    OC_LOG(ERROR, TAG, "Failed allocating memory.");
    OCFreeOCStringLL(rt);
    OCFreeOCStringLL(itf);
    OCFreeOCStringLL(mt);
    OCFreeTagsResource(tagsPayload);
    OCFreeLinksResource(linksPayload);
    OCRDPayloadDestroy(rdPayload);
    return OC_STACK_NO_MEMORY;
}
