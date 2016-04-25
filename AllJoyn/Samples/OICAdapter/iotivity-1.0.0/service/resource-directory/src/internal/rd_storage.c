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
#include "rd_storage.h"

#include <pthread.h>
#include <string.h>

#include "payload_logging.h"
#include "oic_malloc.h"

#include "rdpayload.h"

#define TAG  PCF("RDStorage")

pthread_mutex_t storageMutex = PTHREAD_MUTEX_INITIALIZER;
// This variable holds the published resources on the RD.
static OCRDStorePublishResources *g_rdStorage = NULL;

static void printStoragedResources(OCRDStorePublishResources *payload)
{
    OC_LOG(DEBUG, TAG, "Print Storage Resources ... ");
    for (OCRDStorePublishResources *temp = payload; temp; temp = temp->next)
    {
        if (temp->publishedResource)
        {
            OCTagsLog(DEBUG, temp->publishedResource->tags);
            OCLinksLog(DEBUG, temp->publishedResource->setLinks);
        }
    }
}

OCStackResult OCRDStorePublishedResources(const OCResourceCollectionPayload *payload)
{
    OCResourceCollectionPayload *storeResource = (OCResourceCollectionPayload *)OICCalloc(1, sizeof(OCResourceCollectionPayload));
    if (!storeResource)
    {
        OC_LOG(ERROR, TAG, "Failed allocating memory for OCRDStorePublishResources.");
        return OC_STACK_NO_MEMORY;
    }

    OC_LOG(DEBUG, TAG, "Storing Resources ... ");

    OCTagsPayload *tags = payload->tags;
    storeResource->tags = OCCopyTagsResources(tags->n.deviceName, tags->di.id, tags->baseURI,
        tags->bitmap, tags->port, tags->ins, tags->rts, tags->drel, tags->ttl);
    if (!storeResource->tags)
    {
        OC_LOG(ERROR, TAG, "Failed allocating memory for tags.");
        OCFreeCollectionResource(storeResource);
        return OC_STACK_NO_MEMORY;
    }

    for (OCLinksPayload *links = payload->setLinks; links; links = links->next)
    {
        if (!storeResource->setLinks)
        {
            storeResource->setLinks = OCCopyLinksResources(links->href, links->rt, links->itf,
                links->rel, links->obs, links->title, links->uri, links->ins, links->mt);
            if (!storeResource->setLinks)
            {
                OC_LOG(ERROR, TAG, "Failed allocating memory for links.");
                OCFreeCollectionResource(storeResource);
                return OC_STACK_NO_MEMORY;
            }
        }
        else
        {
            OCLinksPayload *temp = storeResource->setLinks;
            while (temp->next)
            {
                temp = temp->next;
            }
            temp->next = OCCopyLinksResources(links->href, links->rt, links->itf, links->rel,
                links->obs, links->title, links->uri, links->ins, links->mt);
            if (!temp->next)
            {
                OC_LOG(ERROR, TAG, "Failed allocating memory for links.");
                OCFreeCollectionResource(storeResource);
                return OC_STACK_NO_MEMORY;
            }
        }

    }
    storeResource->next = NULL;
    OCRDStorePublishResources *resources = (OCRDStorePublishResources *)OICCalloc(1, sizeof(OCRDStorePublishResources));
    if (!resources)
    {
        OCFreeCollectionResource(storeResource);
        return OC_STACK_NO_MEMORY;
    }
    resources->publishedResource = storeResource;

    pthread_mutex_lock(&storageMutex);
    if (g_rdStorage)
    {
        OCRDStorePublishResources *temp = g_rdStorage;
        while (temp->next)
        {
            temp = temp->next;
        }
        temp->next = resources;
    }
    else
    {
        g_rdStorage = resources;
    }
    pthread_mutex_unlock(&storageMutex);

    printStoragedResources(g_rdStorage);
    return OC_STACK_OK;
}

OCStackResult OCRDCheckPublishedResource(const char *interfaceType, const char *resourceType,
        OCResourceCollectionPayload **payload)
{
    // ResourceType and InterfaceType if both are NULL it will return. If either is
    // not null it will continue execution.
    if (!resourceType && !interfaceType)
    {
        OC_LOG(DEBUG, TAG, "Missing resource type and interace type.");
        return OC_STACK_INVALID_PARAM;
    }

    OC_LOG(DEBUG, TAG, "Check Resource in RD");
    if (g_rdStorage && g_rdStorage->publishedResource)
    {
        for (OCRDStorePublishResources *pResource = g_rdStorage;
                pResource; pResource = pResource->next)
        {
            if (pResource->publishedResource->setLinks)
            {
                for (OCLinksPayload *tLinks = pResource->publishedResource->setLinks; tLinks; tLinks = tLinks->next)
                {
                    // If either rt or itf are NULL, it should skip remaining code execution.
                    if (!tLinks->rt || !tLinks->itf)
                    {
                        OC_LOG(DEBUG, TAG, "Either resource type and interface type are missing.");
                        continue;
                    }
                    if (resourceType)
                    {
                        OCStringLL *temp = tLinks->rt;
                        while(temp)
                        {
                            OC_LOG_V(DEBUG, TAG, "Resource Type: %s %s", resourceType, temp->value);
                            if (strcmp(resourceType, temp->value) == 0)
                            {
                                OCTagsPayload *tag = pResource->publishedResource->tags;
                                OCTagsPayload *tags = OCCopyTagsResources(tag->n.deviceName, tag->di.id, tag->baseURI,
                                    tag->bitmap, tag->port, tag->ins, tag->rts, tag->drel, tag->ttl);
                                if (!tags)
                                {
                                    return OC_STACK_NO_MEMORY;
                                }
                                OCLinksPayload *links = OCCopyLinksResources(tLinks->href, tLinks->rt, tLinks->itf,
                                    tLinks->rel, tLinks->obs, tLinks->title, tLinks->uri, tLinks->ins, tLinks->mt);
                                if (!links)
                                {
                                    OCFreeTagsResource(tags);
                                    return OC_STACK_NO_MEMORY;
                                }
                                *payload = OCCopyCollectionResource(tags, links);
                                if (!*payload)
                                {
                                    OCFreeTagsResource(tags);
                                    OCFreeLinksResource(links);
                                    return OC_STACK_NO_MEMORY;
                                }
                                return OC_STACK_OK;
                            }
                            temp = temp->next;
                        }
                    }
                    if (interfaceType)
                    {
                        OCStringLL *temp = tLinks->itf;
                        while (temp)
                        {
                            OC_LOG_V(DEBUG, TAG, "Interface Type: %s %s", interfaceType, temp->value);
                            if (strcmp(interfaceType, temp->value) == 0)
                            {
                                OCTagsPayload *tag = pResource->publishedResource->tags;
                                OCTagsPayload *tags = OCCopyTagsResources(tag->n.deviceName, tag->di.id, tag->baseURI,
                                    tag->bitmap, tag->port, tag->ins, tag->rts, tag->drel, tag->ttl);
                                if (!tags)
                                {
                                    return OC_STACK_NO_MEMORY;
                                }
                                OCLinksPayload *links = OCCopyLinksResources(tLinks->uri, tLinks->rt, tLinks->itf,
                                    tLinks->rel, tLinks->obs, tLinks->title, tLinks->uri, tLinks->ins, tLinks->mt);
                                if (!links)
                                {
                                    OCFreeTagsResource(tags);
                                    return OC_STACK_NO_MEMORY;
                                }
                                *payload = OCCopyCollectionResource(tags, links);
                                if (!*payload)
                                {
                                    OCFreeTagsResource(tags);
                                    OCFreeLinksResource(links);
                                    return OC_STACK_NO_MEMORY;
                                }
                                return OC_STACK_OK;
                            }
                            temp = temp->next;
                        }
                    }
                }
            }
        }
    }
    return OC_STACK_ERROR;
}
