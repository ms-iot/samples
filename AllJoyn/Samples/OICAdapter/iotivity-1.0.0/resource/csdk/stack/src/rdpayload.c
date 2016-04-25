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
#include "rdpayload.h"

#include "oic_malloc.h"
#include "oic_string.h"
#include "octypes.h"
#include "ocstack.h"
#include "ocpayload.h"
#include "payload_logging.h"

#define TAG "OCRDPayload"

#define CBOR_ROOT_ARRAY_LENGTH 1

static CborError FindStringInMap(CborValue *map, char *tags, char **value);
static CborError FindIntInMap(CborValue *map, char *tags, uint64_t *value);
static CborError FindStringLLInMap(const CborValue *linksMap, char *tag, OCStringLL **links);
static int64_t ConditionalAddTextStringToMap(CborEncoder* map, const char* key, size_t keylen, const char *value);
static int64_t ConditionalAddIntToMap(CborEncoder *map, const char *tags, const size_t size, const uint64_t *value);
static int64_t AddStringLLToMap(CborEncoder *map, char *tag, const size_t size, OCStringLL *value);

int64_t OCRDPayloadToCbor(const OCRDPayload *rdPayload, uint8_t *outPayload, size_t *size)
{
    if (!outPayload || !size)
    {
        OC_LOG(ERROR, TAG, "Invalid parameters.");
        return OC_STACK_INVALID_PARAM;
    }

    CborEncoder encoder;
    int flags = 0;
    cbor_encoder_init(&encoder, outPayload, *size, flags);

    CborEncoder rootArray;
    CborError cborEncoderResult;
    cborEncoderResult = cbor_encoder_create_array(&encoder, &rootArray, CBOR_ROOT_ARRAY_LENGTH);
    if (CborNoError != cborEncoderResult)
    {
        OC_LOG(ERROR, TAG, "Failed creating cbor array.");
        goto cbor_error;
    }

    if (rdPayload->rdDiscovery)
    {
        CborEncoder map;
        cborEncoderResult = cbor_encoder_create_map(&rootArray, &map, CborIndefiniteLength);
        if (CborNoError != cborEncoderResult)
        {
            OC_LOG(ERROR, TAG, "Failed creating discovery map.");
            goto cbor_error;
        }
        if (CborNoError != ConditionalAddTextStringToMap(&map, OC_RSRVD_DEVICE_NAME,
                sizeof(OC_RSRVD_DEVICE_NAME) - 1, (char *)rdPayload->rdDiscovery->n.deviceName))
        {
            OC_LOG(ERROR, TAG, "Failed setting OC_RSRVD_DEVICE_NAME.");
            goto cbor_error;
        }
        if (CborNoError != ConditionalAddTextStringToMap(&map, OC_RSRVD_DEVICE_ID,
                sizeof(OC_RSRVD_DEVICE_ID) - 1, (char *)rdPayload->rdDiscovery->di.id))
        {
            OC_LOG(ERROR, TAG, "Failed setting OC_RSRVD_DEVICE_ID.");
            goto cbor_error;
        }
        uint64_t sel = (uint8_t) rdPayload->rdDiscovery->sel;
        if (CborNoError != ConditionalAddIntToMap(&map, OC_RSRVD_RD_DISCOVERY_SEL,
            sizeof(OC_RSRVD_RD_DISCOVERY_SEL) - 1, &sel))
        {
            OC_LOG(ERROR, TAG, "Failed setting OC_RSRVD_RD_DISCOVERY_SEL.");
            goto cbor_error;
        }
        cborEncoderResult = cbor_encoder_close_container(&rootArray, &map);
        if (CborNoError != cborEncoderResult)
        {
            OC_LOG(ERROR, TAG, "Failed closing discovery map.");
            goto cbor_error;
        }
    }
    else if (rdPayload->rdPublish)
    {
        CborEncoder colArray;
        cborEncoderResult = cbor_encoder_create_array(&rootArray, &colArray, CborIndefiniteLength);
        if (CborNoError != cborEncoderResult)
        {
            OC_LOG(ERROR, TAG, "Failed creating collection array.");
            goto cbor_error;
        }

        OCResourceCollectionPayload *rdPublish = rdPayload->rdPublish;
        while (rdPublish)
        {
            if (OC_STACK_OK != OCTagsPayloadToCbor(rdPublish->tags, &colArray))
            {
                OC_LOG(ERROR, TAG, "Failed creating tags payload.");
                goto cbor_error;
            }
            if (OC_STACK_OK != OCLinksPayloadToCbor(rdPublish->setLinks, &colArray))
            {
                OC_LOG(ERROR, TAG, "Failed creating links payload.");
                goto cbor_error;
            }
            rdPublish = rdPublish->next;
        }
        cborEncoderResult = cbor_encoder_close_container(&rootArray, &colArray);
        if (CborNoError != cborEncoderResult)
        {
            OC_LOG(ERROR, TAG, "Failed closing collection array.");
            goto cbor_error;
        }
    }
    cborEncoderResult = cbor_encoder_close_container(&encoder, &rootArray);
    if (CborNoError != cborEncoderResult)
    {
        OC_LOG(ERROR, TAG, "Failed closing root array container. ");
        goto cbor_error;
    }

    *size = encoder.ptr - outPayload;
    return OC_STACK_OK;

cbor_error:
    OICFree(outPayload);
    return OC_STACK_ERROR;
}

OCStackResult OCTagsPayloadToCbor(OCTagsPayload *tags, CborEncoder *setMap)
{
    CborEncoder tagsMap;
    CborError cborEncoderResult = cbor_encoder_create_map(setMap, &tagsMap, CborIndefiniteLength);
    if (CborNoError != cborEncoderResult)
    {
        OC_LOG(ERROR, TAG, "Failed creating TAGS map.");
        return OC_STACK_ERROR;
    }

    if (CborNoError != ConditionalAddTextStringToMap(&tagsMap, OC_RSRVD_DEVICE_NAME,
            sizeof(OC_RSRVD_DEVICE_NAME) - 1, (char *)tags->n.deviceName))
    {
        OC_LOG(ERROR, TAG, "Failed adding OC_RSRVD_DEVICE_NAME in TAGS map.");
        return OC_STACK_ERROR;
    }
    if (CborNoError != ConditionalAddTextStringToMap(&tagsMap, OC_RSRVD_DEVICE_ID,
            sizeof(OC_RSRVD_DEVICE_ID) - 1, (char *)tags->di.id))
    {
        OC_LOG(ERROR, TAG, "Failed adding OC_RSRVD_DEVICE_ID in TAGS map.");
        return OC_STACK_ERROR;
    }
    if (CborNoError != ConditionalAddTextStringToMap(&tagsMap, OC_RSRVD_RTS,
            sizeof(OC_RSRVD_RTS) - 1, (char *)tags->rts))
    {
        OC_LOG(ERROR, TAG, "Failed adding OC_RSRVD_RTS in TAGS map.");
        return OC_STACK_ERROR;
    }
    if (CborNoError != ConditionalAddTextStringToMap(&tagsMap, OC_RSRVD_DREL,
            sizeof(OC_RSRVD_DREL) - 1, (char *)tags->drel))
    {
        OC_LOG(ERROR, TAG, "Failed adding OC_RSRVD_DREL in TAGS map.");
        return OC_STACK_ERROR;
    }
    if (CborNoError != ConditionalAddTextStringToMap(&tagsMap, OC_RSRVD_BASE_URI,
            sizeof(OC_RSRVD_BASE_URI) - 1, (char *)tags->baseURI))
    {
        OC_LOG(ERROR, TAG, "Failed adding OC_RSRVD_BASE_URI in TAGS map.");
        return OC_STACK_ERROR;
    }
    uint64_t temp = (uint64_t)tags->bitmap;
    if (CborNoError != ConditionalAddIntToMap(&tagsMap, OC_RSRVD_BITMAP,
            sizeof(OC_RSRVD_BITMAP) - 1, &temp))
    {
        OC_LOG(ERROR, TAG, "Failed adding OC_RSRVD_BITMAP in TAGS map.");
        return OC_STACK_ERROR;
    }
    temp = (uint64_t)tags->port;
    if (CborNoError != ConditionalAddIntToMap(&tagsMap, OC_RSRVD_HOSTING_PORT,
            sizeof(OC_RSRVD_HOSTING_PORT) - 1, &temp))
    {
        OC_LOG(ERROR, TAG, "Failed adding OC_RSRVD_HOSTING_PORT in TAGS map.");
        return OC_STACK_ERROR;
    }
    temp = (uint64_t)tags->ins;
    if (CborNoError != ConditionalAddIntToMap(&tagsMap, OC_RSRVD_INS,
            sizeof(OC_RSRVD_INS) - 1, &temp))
    {
        OC_LOG(ERROR, TAG, "Failed adding OC_RSRVD_INS in TAGS map.");
        return OC_STACK_ERROR;
    }
    temp = (uint64_t)tags->ttl;
    if (CborNoError != ConditionalAddIntToMap(&tagsMap, OC_RSRVD_TTL,
            sizeof(OC_RSRVD_TTL) - 1, &temp))
    {
        OC_LOG(ERROR, TAG, "Failed adding OC_RSRVD_TTL in TAGS map.");
        return OC_STACK_ERROR;
    }
    cborEncoderResult = cbor_encoder_close_container(setMap, &tagsMap);
    if (CborNoError != cborEncoderResult)
    {
        OC_LOG(ERROR, TAG, "Failed closing TAGS map.");
        return OC_STACK_ERROR;
    }
    return OC_STACK_OK;
}

OCStackResult OCLinksPayloadToCbor(OCLinksPayload *rtPtr, CborEncoder *setMap)
{
    CborEncoder linksArray;
    CborError cborEncoderResult;

    cborEncoderResult = cbor_encoder_create_array(setMap, &linksArray, CborIndefiniteLength);
    if (CborNoError != cborEncoderResult)
    {
        OC_LOG(ERROR, TAG, "Failed creating LINKS array.");
        return OC_STACK_ERROR;
    }
    while (rtPtr)
    {
        CborEncoder linksMap;
        cborEncoderResult = cbor_encoder_create_map(&linksArray, &linksMap,
                CborIndefiniteLength);
        if (CborNoError != cborEncoderResult)
        {
            OC_LOG(ERROR, TAG, "Failed creating LINKS map.");
            return OC_STACK_ERROR;
        }
        if (CborNoError != ConditionalAddTextStringToMap(&linksMap, OC_RSRVD_HREF,
                sizeof(OC_RSRVD_HREF) - 1, rtPtr->href))
        {
            OC_LOG(ERROR, TAG, "Failed adding OC_RSRVD_HREF in LINKS map.");
            return OC_STACK_ERROR;
        }
        if (CborNoError != ConditionalAddTextStringToMap(&linksMap, OC_RSRVD_REL,
                sizeof(OC_RSRVD_REL) - 1,  rtPtr->rel))
        {
            OC_LOG(ERROR, TAG, "Failed adding OC_RSRVD_REL in LINKS map.");
            return OC_STACK_ERROR;
        }
        if (CborNoError != ConditionalAddTextStringToMap(&linksMap, OC_RSRVD_TITLE,
                sizeof(OC_RSRVD_TITLE) - 1, rtPtr->title))
        {
            OC_LOG(ERROR, TAG, "Failed adding OC_RSRVD_TITLE in LINKS map.");
            return OC_STACK_ERROR;
        }
        if (CborNoError != ConditionalAddTextStringToMap(&linksMap, OC_RSRVD_URI,
                sizeof(OC_RSRVD_URI) - 1, rtPtr->uri))
        {
            OC_LOG(ERROR, TAG, "Failed adding OC_RSRVD_URI in LINKS map.");
            return OC_STACK_ERROR;
        }
        if (CborNoError != AddStringLLToMap(&linksMap, OC_RSRVD_RESOURCE_TYPE,
                sizeof(OC_RSRVD_RESOURCE_TYPE) - 1, rtPtr->rt))
        {
            OC_LOG(ERROR, TAG, "Failed adding OC_RSRVD_RESOURCE_TYPE in LINKS map.");
            return OC_STACK_ERROR;
        }
        if (CborNoError != AddStringLLToMap(&linksMap, OC_RSRVD_INTERFACE,
                sizeof(OC_RSRVD_INTERFACE) - 1, rtPtr->itf))
        {
            OC_LOG(ERROR, TAG, "Failed adding OC_RSRVD_INTERFACE in LINKS map.");
            return OC_STACK_ERROR;
        }
        if (CborNoError != AddStringLLToMap(&linksMap, OC_RSRVD_MEDIA_TYPE,
                sizeof(OC_RSRVD_MEDIA_TYPE) - 1, rtPtr->mt))
        {
            OC_LOG(ERROR, TAG, "Failed adding OC_RSRVD_MEDIA_TYPE in LINKS map.");
            return OC_STACK_ERROR;
        }
        uint64_t temp = (uint64_t)rtPtr->ins;
        if (CborNoError != ConditionalAddIntToMap(&linksMap, OC_RSRVD_INS,
            sizeof(OC_RSRVD_INS) - 1, &temp))
        {
            OC_LOG(ERROR, TAG, "Failed adding OC_RSRVD_INS in LINKS map.");
            return OC_STACK_ERROR;
        }
        cborEncoderResult = cbor_encoder_close_container(&linksArray, &linksMap);
        if (CborNoError != cborEncoderResult)
        {
            OC_LOG(ERROR, TAG, "Failed closing LINKS map.");
            return OC_STACK_ERROR;
        }
        rtPtr = rtPtr->next;
    }
    cborEncoderResult = cbor_encoder_close_container(setMap, &linksArray);
    if (CborNoError != cborEncoderResult)
    {
        OC_LOG(ERROR, TAG, "Failed closing LINKS array.");
        return OC_STACK_ERROR;;
    }
    return OC_STACK_OK;
}

OCStackResult OCRDCborToPayload(const CborValue *cborPayload, OCPayload **outPayload)
{
    CborValue *rdCBORPayload = (CborValue *)cborPayload;
    CborError cborFindResult;

    OCRDPayload *rdPayload = OCRDPayloadCreate();
    if (!rdPayload)
    {
        goto no_memory;
    }

    if (cbor_value_is_array(rdCBORPayload))
    {
        OCLinksPayload *linksPayload = NULL;
        OCTagsPayload *tagsPayload = NULL;

        while (cbor_value_is_container(rdCBORPayload))
        {
            // enter tags map
            CborValue tags;
            cborFindResult = cbor_value_enter_container(rdCBORPayload, &tags);
            if (cborFindResult != CborNoError)
            {
                goto cbor_error;
            }
            if (OC_STACK_OK != OCTagsCborToPayload(&tags, &tagsPayload))
            {
                OCFreeTagsResource(tagsPayload);
                goto cbor_error;
            }
            OCTagsLog(DEBUG, tagsPayload);
            if (OC_STACK_OK != OCLinksCborToPayload(&tags, &linksPayload))
            {
                OCFreeLinksResource(linksPayload);
                OCFreeTagsResource(tagsPayload);
                goto cbor_error;
            }
            OCLinksLog(DEBUG, linksPayload);
            // Move from tags payload to links array.
            if (CborNoError != cbor_value_advance(rdCBORPayload))
            {
                OC_LOG(DEBUG, TAG, "Failed advancing from tags payload to links.");
                OCFreeLinksResource(linksPayload);
                OCFreeTagsResource(tagsPayload);
                goto cbor_error;
            }
        }
        rdPayload->rdPublish = OCCopyCollectionResource(tagsPayload, linksPayload);
        if (!rdPayload->rdPublish)
        {
            goto cbor_error;
        }
    }
    else if (cbor_value_is_map(rdCBORPayload))
    {
        char *name = NULL;
        if (CborNoError != FindStringInMap(rdCBORPayload, OC_RSRVD_DEVICE_NAME, &name))
        {
            goto cbor_error;
        }
        char *id = NULL;
        if (CborNoError != FindStringInMap(rdCBORPayload, OC_RSRVD_DEVICE_ID, &id))
        {
            goto cbor_error;
        }
        uint64_t biasFactor = 0;
        if (CborNoError != FindIntInMap(rdCBORPayload, OC_RSRVD_RD_DISCOVERY_SEL, &biasFactor))
        {
            goto cbor_error;
        }
        rdPayload->rdDiscovery = OCRDDiscoveryPayloadCreate(name, id, (uint8_t)biasFactor);
        if (!rdPayload->rdDiscovery)
        {
            goto no_memory;
        }
        OICFree(id);
        OICFree(name);
        cborFindResult =  cbor_value_advance(rdCBORPayload);
        if (CborNoError != cborFindResult)
        {
            goto cbor_error;
        }
    }
    OC_LOG_PAYLOAD(DEBUG, (OCPayload *) rdPayload);
    *outPayload = (OCPayload *)rdPayload;
    return OC_STACK_OK;
no_memory:
    OC_LOG(ERROR, TAG, "Failed allocating memory.");
    OCRDPayloadDestroy(rdPayload);
    return OC_STACK_NO_MEMORY;

cbor_error:
    OCRDPayloadDestroy(rdPayload);
    return OC_STACK_ERROR;
}

static CborError FindStringInMap(CborValue *map, char *tags, char **value)
{
    CborValue curVal;
    size_t len;
    CborError cborFindResult = cbor_value_map_find_value(map, tags, &curVal);
    if (CborNoError == cborFindResult && cbor_value_is_text_string(&curVal))
    {
        cborFindResult = cbor_value_dup_text_string(&curVal, value, &len, NULL);
        if (CborNoError != cborFindResult)
        {
            OC_LOG_V(ERROR, TAG, "Failed finding value for tag %s .", tags);
            return cborFindResult;
        }
    }
    return CborNoError;
}

static CborError FindIntInMap(CborValue *map, char *tags, uint64_t *value)
{
    CborValue curVal;
    CborError cborFindResult = cbor_value_map_find_value(map, tags, &curVal);
    if (CborNoError == cborFindResult && cbor_value_is_unsigned_integer(&curVal))
    {
        cborFindResult = cbor_value_get_uint64(&curVal, value);
        if (CborNoError != cborFindResult)
        {
            OC_LOG_V(ERROR, TAG, "Failed finding value for tag %s .", tags);
            return cborFindResult;
        }
    }
    return CborNoError;
}

static CborError FindStringLLInMap(const CborValue *linksMap, char *tag, OCStringLL **links)
{
    size_t len;
    CborError cborFindResult;
    CborValue rtArray;
    cborFindResult = cbor_value_map_find_value(linksMap, tag, &rtArray);
    if (CborNoError != cborFindResult)
    {
        return CborUnknownError;
    }
    CborValue rtVal;
    cborFindResult = cbor_value_enter_container(&rtArray, &rtVal);
    if (CborNoError != cborFindResult)
    {
        return CborUnknownError;
    }
    OCStringLL* llPtr = *links;
    while (cbor_value_is_text_string(&rtVal))
    {
        if (llPtr == NULL)
        {
            llPtr = (OCStringLL *)OICCalloc(1, sizeof(OCStringLL));
            if (!llPtr)
            {
                return CborUnknownError;
            }
            *links = llPtr;
        }
        else if(llPtr)
        {
            while (llPtr->next)
            {
                llPtr = llPtr->next;
            }
            llPtr->next = (OCStringLL *)OICCalloc(1, sizeof(OCStringLL));
            if (!llPtr->next)
            {
                return CborUnknownError;
            }
        }
        cborFindResult = cbor_value_dup_text_string(&rtVal, &(llPtr->value), &len, NULL);
        if (CborNoError != cborFindResult)
        {
            return CborUnknownError;
        }
        cborFindResult = cbor_value_advance(&rtVal);
        if (CborNoError != cborFindResult)
        {
            return CborUnknownError;
        }
    }

    cborFindResult = cbor_value_leave_container(&rtArray, &rtVal);
    return cborFindResult;
}

OCStackResult OCTagsCborToPayload(CborValue *tagsMap, OCTagsPayload **tagsPayload)
{
    OCTagsPayload *tags = (OCTagsPayload *)OICCalloc(1, sizeof(OCTagsPayload));
    if (!tags)
    {
        return OC_STACK_NO_MEMORY;
    }
    if (cbor_value_is_map(tagsMap))
    {
        if (CborNoError != FindStringInMap(tagsMap, OC_RSRVD_DEVICE_NAME, &tags->n.deviceName))
        {
            OCFreeTagsResource(tags);
            return OC_STACK_ERROR;
        }
        if (CborNoError != FindStringInMap(tagsMap, OC_RSRVD_DREL, &tags->drel))
        {
            OCFreeTagsResource(tags);
            return OC_STACK_ERROR;
        }
        if (CborNoError != FindStringInMap(tagsMap, OC_RSRVD_RTS, &tags->rts))
        {
            OCFreeTagsResource(tags);
            return OC_STACK_ERROR;
        }
        if (CborNoError != FindStringInMap(tagsMap, OC_RSRVD_BASE_URI, &tags->baseURI))
        {
            OCFreeTagsResource(tags);
            return OC_STACK_ERROR;
        }
        char *id = NULL;
        if (CborNoError != FindStringInMap(tagsMap, OC_RSRVD_DEVICE_ID, &id))
        {
            OCFreeTagsResource(tags);
            return OC_STACK_ERROR;
        }
        if (id)
        {
            OICStrcpy((char*)tags->di.id, MAX_IDENTITY_SIZE, id);
            tags->di.id_length = MAX_IDENTITY_SIZE;
            OICFree(id);
        }
        uint64_t temp;
        if (CborNoError != FindIntInMap(tagsMap, OC_RSRVD_HOSTING_PORT, &temp))
        {
            OCFreeTagsResource(tags);
            return OC_STACK_ERROR;
        }
        tags->port = (uint16_t) temp;
        if (CborNoError != FindIntInMap(tagsMap, OC_RSRVD_BITMAP, &temp))
        {
            OCFreeTagsResource(tags);
            return OC_STACK_ERROR;
        }
        tags->bitmap = (uint8_t) temp;
        if (CborNoError != FindIntInMap(tagsMap, OC_RSRVD_INS, &temp))
        {
            OCFreeTagsResource(tags);
            return OC_STACK_ERROR;
        }
        tags->ins = (uint8_t) temp;
        if (CborNoError != FindIntInMap(tagsMap, OC_RSRVD_TTL, &temp))
        {
            OCFreeTagsResource(tags);
            return OC_STACK_ERROR;
        }
        tags->ttl = (uint32_t) temp;

        if (CborNoError != cbor_value_advance(tagsMap))
        {
            OCFreeTagsResource(tags);
            return OC_STACK_ERROR;
        }
    }
    *tagsPayload = tags;
    return OC_STACK_OK;
}

OCStackResult OCLinksCborToPayload(CborValue *linksArray, OCLinksPayload **linksPayload)
{
    CborValue linksMap;
    CborError cborFindResult = cbor_value_enter_container(linksArray, &linksMap);
    if (CborNoError != cborFindResult)
    {
        OC_LOG(ERROR, TAG, "Failed enter links map");
        return OC_STACK_ERROR;
    }

    while (cbor_value_is_map(&linksMap))
    {
        OCLinksPayload *setLinks = (OCLinksPayload *)OICCalloc(1, sizeof(OCLinksPayload));
        if (!setLinks)
        {
            OC_LOG(ERROR, TAG, "Failed allocating memory.");
            OCFreeLinksResource(*linksPayload);
            return OC_STACK_NO_MEMORY;
        }
        cborFindResult = FindStringInMap(&linksMap, OC_RSRVD_HREF, &setLinks->href);
        if (CborNoError != cborFindResult)
        {
            OCFreeLinksResource(*linksPayload);
            OCFreeLinksResource(setLinks);
            return OC_STACK_ERROR;
        }
        cborFindResult = FindStringInMap(&linksMap, OC_RSRVD_REL, &setLinks->rel);
        if (CborNoError != cborFindResult)
        {
            OCFreeLinksResource(*linksPayload);
            OCFreeLinksResource(setLinks);
            return OC_STACK_ERROR;
        }

        cborFindResult = FindStringInMap(&linksMap, OC_RSRVD_TITLE, &setLinks->title);
        if (CborNoError != cborFindResult)
        {
            OCFreeLinksResource(*linksPayload);
            OCFreeLinksResource(setLinks);
            return OC_STACK_ERROR;
        }
        cborFindResult = FindStringInMap(&linksMap, OC_RSRVD_URI, &setLinks->uri);
        if (CborNoError != cborFindResult)
        {
            OCFreeLinksResource(*linksPayload);
            OCFreeLinksResource(setLinks);
            return OC_STACK_ERROR;
        }
        cborFindResult = FindStringLLInMap(&linksMap, OC_RSRVD_RESOURCE_TYPE, &setLinks->rt);
        if (CborNoError != cborFindResult)
        {
            OCFreeLinksResource(*linksPayload);
            OCFreeLinksResource(setLinks);
            return OC_STACK_ERROR;
        }
        cborFindResult = FindStringLLInMap(&linksMap, OC_RSRVD_INTERFACE, &setLinks->itf);
        if (CborNoError != cborFindResult)
        {
            OCFreeLinksResource(*linksPayload);
            OCFreeLinksResource(setLinks);
            return OC_STACK_ERROR;
        }
        cborFindResult = FindStringLLInMap(&linksMap, OC_RSRVD_MEDIA_TYPE, &setLinks->mt);
        if (CborNoError != cborFindResult)
        {
            OCFreeLinksResource(*linksPayload);
            OCFreeLinksResource(setLinks);
            return OC_STACK_ERROR;
        }
        uint64_t temp;
        cborFindResult = FindIntInMap(&linksMap, OC_RSRVD_INS, &temp);
        if (CborNoError != cborFindResult)
        {
            OCFreeLinksResource(*linksPayload);
            OCFreeLinksResource(setLinks);
            return OC_STACK_ERROR;
        }
        setLinks->ins = (uint8_t) temp;

        if (!*linksPayload)
        {
            *linksPayload = setLinks;
        }
        else
        {
            OCLinksPayload *temp = *linksPayload;
            while (temp->next)
            {
                temp = temp->next;
            }
            temp->next = setLinks;
        }
        cborFindResult = cbor_value_advance(&linksMap);
        if (CborNoError != cborFindResult)
        {
            OC_LOG(ERROR, TAG, "Failed advancing links map");
            OCFreeLinksResource(*linksPayload);
            OCFreeLinksResource(setLinks);
            return OC_STACK_ERROR;
        }
    }
    return OC_STACK_OK;
}

static int64_t AddTextStringToMap(CborEncoder* map, const char* key, size_t keylen,
        const char* value)
{
    return cbor_encode_text_string(map, key, keylen) |
           cbor_encode_text_string(map, value, strlen(value));
}

static int64_t ConditionalAddTextStringToMap(CborEncoder* map, const char* key, size_t keylen,
        const char* value)
{
    return value ? AddTextStringToMap(map, key, keylen, value) : 0;
}

static int64_t ConditionalAddIntToMap(CborEncoder *map, const char *tags, const size_t size,
    const uint64_t *value)
{
    return (*value) ? (cbor_encode_text_string(map, tags, size) |
                     cbor_encode_uint(map, *value)): 0;
}

static int64_t AddStringLLToMap(CborEncoder *map, char *tag, const size_t size, OCStringLL *value)
{
    CborEncoder array;
    CborError cborEncoderResult;
    cborEncoderResult = cbor_encode_text_string(map, tag, size);
    if (CborNoError != cborEncoderResult)
    {
        return cborEncoderResult;
    }
    cborEncoderResult = cbor_encoder_create_array(map, &array, CborIndefiniteLength);
    if (CborNoError != cborEncoderResult)
    {
        return cborEncoderResult;
    }
    OCStringLL *strType = value;
    while (strType)
    {
        cborEncoderResult = cbor_encode_text_string(&array, strType->value, strlen(strType->value));
        if (CborNoError != cborEncoderResult)
        {
            return cborEncoderResult;
        }
        strType = strType->next;
    }
    cborEncoderResult = cbor_encoder_close_container(map, &array);
    if (CborNoError != cborEncoderResult)
    {
        return cborEncoderResult;
    }
    return cborEncoderResult;
}

OCRDPayload *OCRDPayloadCreate()
{
    OCRDPayload *rdPayload = (OCRDPayload *)OICCalloc(1, sizeof(OCRDPayload));

    if (!rdPayload)
    {
        return NULL;
    }

    rdPayload->base.type = PAYLOAD_TYPE_RD;

    return rdPayload;
}

OCRDDiscoveryPayload *OCRDDiscoveryPayloadCreate(const char *deviceName, const char *id, int biasFactor)
{
    OCRDDiscoveryPayload *discoveryPayload = (OCRDDiscoveryPayload *)OICCalloc(1, sizeof(OCRDDiscoveryPayload));

    if (!discoveryPayload)
    {
        return NULL;
    }

    if (deviceName)
    {
        discoveryPayload->n.deviceName = OICStrdup(deviceName);
        if (!discoveryPayload->n.deviceName)
        {
            OICFree(discoveryPayload);
            return NULL;
        }
    }
    if (id)
    {
        OICStrcpy((char*)discoveryPayload->di.id, MAX_IDENTITY_SIZE, id);
    }

    discoveryPayload->sel = biasFactor;

    return discoveryPayload;
}

void OCRDPayloadDestroy(OCRDPayload *payload)
{
    if (!payload)
    {
        return;
    }

    if (payload->rdDiscovery)
    {
        if (payload->rdDiscovery->n.deviceName)
        {
            OICFree(payload->rdDiscovery->n.deviceName);
        }
        OICFree(payload->rdDiscovery);
    }

    if (payload->rdPublish)
    {
        for (OCResourceCollectionPayload *col = payload->rdPublish; col; )
        {
            if (col->setLinks)
            {
                OCFreeLinksResource(col->setLinks);
            }

            if (col->tags)
            {
                OCFreeTagsResource(col->tags);
            }
            OCResourceCollectionPayload *temp = col->next;
            OICFree(col);
            col = temp;
        }
    }

    OICFree(payload);
}

OCTagsPayload* OCCopyTagsResources(const char *deviceName, const unsigned char *id, const char *baseURI,
        uint8_t bitmap, uint16_t port, uint8_t ins, const char *rts,const  char *drel, uint32_t ttl)
{
    OCTagsPayload *tags = (OCTagsPayload *)OICCalloc(1, sizeof(OCTagsPayload));
    if (!tags)
    {
        return NULL;
    }
	if (deviceName)
    {
        tags->n.deviceName = OICStrdup(deviceName);
        if (!tags->n.deviceName)
        {
            goto memory_allocation_failed;
        }
    }
    if (id)
    {
        OICStrcpy((char*)tags->di.id, MAX_IDENTITY_SIZE, (char *)id);
        if (!tags->di.id)
        {
            goto memory_allocation_failed;
        }
    }
    if (baseURI)
    {
        tags->baseURI = OICStrdup(baseURI);
        if (!tags->baseURI)
        {
            goto memory_allocation_failed;
        }
    }
    tags->bitmap = bitmap;
    tags->port = port;
    tags->ins = ins;
    if (rts)
    {
        tags->rts = OICStrdup(rts);
        if (!tags->rts)
        {
            goto memory_allocation_failed;
        }
    }
    if (drel)
    {
        tags->drel = OICStrdup(drel);
        if (!tags->drel)
        {
            goto memory_allocation_failed;
        }
    }
    tags->ttl = ttl;
    return tags;

memory_allocation_failed:
    OC_LOG(ERROR, TAG, "Memory allocation failed.");
    OCFreeTagsResource(tags);
    return NULL;
}

OCLinksPayload* OCCopyLinksResources(const char *href, OCStringLL *rt, OCStringLL *itf,
        const char *rel, bool obs, const char *title, const char *uri, uint8_t ins, OCStringLL *mt)
{
    OCLinksPayload *links = (OCLinksPayload *)OICCalloc(1, sizeof(OCLinksPayload));
    if (!links)
    {
        OC_LOG(ERROR, TAG, "Failed allocating memory.");
        return NULL;
    }
    if (href)
    {
        links->href = OICStrdup(href);
        if (!links->href)
        {
            goto memory_allocation_failed;
        }
    }
    if (rt)
    {
        links->rt = CloneOCStringLL(rt);
        if (!links->rt)
        {
            goto memory_allocation_failed;
        }
    }
    if (itf)
    {
        links->itf = CloneOCStringLL(itf);
        if (!links->itf)
        {
            goto memory_allocation_failed;
        }
    }
    if (rel)
    {
        links->rel = OICStrdup(rel);
        if (!links->rel)
        {
            goto memory_allocation_failed;
        }
    }
    links->obs = obs;
    if (title)
    {
        links->title = OICStrdup(title);
        if (!links->title)
        {
            goto memory_allocation_failed;
        }
    }
    if (uri)
    {
        links->uri = OICStrdup(uri);
        if (!links->uri)
        {
            goto memory_allocation_failed;
        }
    }
    links->ins = ins;
    if (mt)
    {
        links->mt = CloneOCStringLL(mt);
        if (!links->mt)
        {
            goto memory_allocation_failed;
        }
    }
    links->next = NULL;
    return links;

memory_allocation_failed:
    OC_LOG(ERROR, TAG, "Memory allocation failed.");
    OCFreeLinksResource(links);
    return NULL;
}

void OCLinksAddResource(OCDiscoveryPayload *payload, const char *href, OCStringLL *rt,
    OCStringLL *itf, const char *rel, bool obs, const char *title, const char *uri,
    uint8_t ins, OCStringLL *mt)
{
    if(!payload->collectionResources->setLinks)
    {
        payload->collectionResources->setLinks =
            OCCopyLinksResources(href, rt, itf, rel, obs, title, uri, ins, mt);
    }
    else
    {
        OCLinksPayload *p = payload->collectionResources->setLinks;
        while (p->next)
        {
            p = p->next;
        }
        p->next = OCCopyLinksResources(href, rt, itf, rel, obs, title, uri, ins, mt);
    }
}

OCResourceCollectionPayload* OCCopyCollectionResource(OCTagsPayload *tags, OCLinksPayload *links)
{
    if (!tags || !links)
    {
        return NULL;
    }
    OCResourceCollectionPayload *pl = (OCResourceCollectionPayload *)OICCalloc(1, sizeof(OCResourceCollectionPayload));
    if(!pl)
    {
        OC_LOG(ERROR, TAG, "Failed allocating memory for the OCResourceCollectionPayload.");
        return NULL;
    }
    pl->tags = tags;
    pl->setLinks = links;

    return pl;
}

OCStackResult OCDiscoveryCollectionPayloadAddResource(OCDiscoveryPayload *payload, OCTagsPayload *tags, OCLinksPayload *links)
{
    OCResourceCollectionPayload* res = OCCopyCollectionResource(tags, links);
    if (res == NULL)
    {
        return OC_STACK_NO_MEMORY;
    }
    if(!payload->collectionResources)
    {
        payload->collectionResources = res;
    }
    else
    {
        OCResourceCollectionPayload *p = payload->collectionResources;
        while(p->next)
        {
            p = p->next;
        }
        p->next = res;
    }
    return OC_STACK_OK;
}

void OCFreeLinksResource(OCLinksPayload *payload)
{
    if (!payload)
    {
        return;
    }
    OICFree(payload->href);
    OCFreeOCStringLL(payload->rt);
    OCFreeOCStringLL(payload->itf);
    OICFree(payload->rel);
    OICFree(payload->title);
    OICFree(payload->uri);
    OCFreeOCStringLL(payload->mt);
    OCFreeLinksResource(payload->next);
    OICFree(payload);
}

void OCFreeTagsResource(OCTagsPayload *payload)
{
    if (!payload)
    {
        return;
    }
    OICFree(payload->n.deviceName);
    OICFree(payload->baseURI);
    OICFree(payload->rts);
    OICFree(payload->drel);
    OICFree(payload);
}

void OCFreeCollectionResource(OCResourceCollectionPayload *payload)
{
    if (!payload)
    {
        return;
    }
    if (payload->tags)
    {
        OCFreeTagsResource(payload->tags);
    }
    if (payload->setLinks)
    {
        OCFreeLinksResource(payload->setLinks);
    }
    OCFreeCollectionResource(payload->next);
    OICFree(payload);
}

void OCDiscoveryCollectionPayloadDestroy(OCDiscoveryPayload* payload)
{
    if(!payload)
    {
        return;
    }

    OCFreeCollectionResource(payload->collectionResources);
    OICFree(payload);
}


void OCTagsLog(const LogLevel level, const OCTagsPayload *tags)
{
    if (tags)
    {
        if (tags->n.deviceName)
        {
            OC_LOG_V(level, TAG, " Device Name : %s ",tags->n.deviceName);
        }
        if (tags->baseURI)
        {
            OC_LOG_V(level, TAG, " Base URI : %s ",tags->baseURI);
        }
        OC_LOG_V(level, TAG, " Device ID : %s ",tags->di.id);
        OC_LOG_V(level, TAG, " Bitmap : %d ",tags->bitmap);
        OC_LOG_V(level, TAG, " Port : %d ",tags->port);
        OC_LOG_V(level, TAG, " Ins : %d ",tags->ins);
        OC_LOG_V(level, TAG, " Ttl : %d ",tags->ttl);

        if (tags->rts)
        {
            OC_LOG_V(level, TAG, " RTS : %s ",tags->rts);
        }
        if (tags->drel)
        {
            OC_LOG_V(level, TAG, " DREL : %s ",tags->drel);
        }
    }
}

void OCLinksLog(const LogLevel level, const OCLinksPayload *links)
{
    while (links)
    {
        if (links->href)
        {
            OC_LOG_V(level, TAG, " href: %s ",links->href);
        }
        OC_LOG(level, TAG, " RT: ");
        OCStringLL *rt = links->rt;
        while (rt)
        {
            if (rt->value)
            {
                OC_LOG_V(level, TAG, "   %s", rt->value);
            }
            rt = rt->next;
        }
        OC_LOG(level, TAG, " IF: ");
        OCStringLL *itf = links->itf;
        while (itf)
        {
            if (itf->value)
            {
                OC_LOG_V(level, TAG, "   %s", itf->value);
            }
            itf = itf->next;
        }
        OC_LOG(level, TAG, " MT: ");
        OCStringLL *mt = links->mt;
        while (mt)
        {
            if (mt->value)
            {
                OC_LOG_V(level, TAG, "   %s", mt->value);
            }
            mt = mt->next;
        }
        OC_LOG_V(level, TAG, " INS: %d", links->ins);
        OC_LOG_V(level, TAG, " OBS: %d", links->obs);
        if (links->rel)
        {
            OC_LOG_V(level, TAG, " REL: %s", links->rel);
        }
        if (links->title)
        {
            OC_LOG_V(level, TAG, " TITLE: %s", links->title);
        }
        if (links->uri)
        {
            OC_LOG_V(level, TAG, " URI: %s", links->uri);
        }
        links = links->next;
    }
}
