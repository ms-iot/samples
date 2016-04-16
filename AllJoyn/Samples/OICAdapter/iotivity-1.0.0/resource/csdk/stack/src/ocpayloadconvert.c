//******************************************************************
//
// Copyright 2015 Intel Mobile Communications GmbH All Rights Reserved.
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

#include "ocpayloadcbor.h"
#include "platform_features.h"
#include <stdlib.h>
#include "oic_malloc.h"
#include "oic_string.h"
#include "logger.h"
#include "ocpayload.h"
#include "ocrandom.h"
#include "ocresourcehandler.h"
#include "cbor.h"
#include "rdpayload.h"

#define TAG "OCPayloadConvert"
// Arbitrarily chosen size that seems to contain the majority of packages
#define INIT_SIZE (255)

// CBOR Array Length
#define DISCOVERY_CBOR_ARRAY_LEN 1
// CBOR Res Map Length
#define DISCOVERY_CBOR_RES_MAP_LEN 2
// CBOR Links Map Length
#define DISCOVERY_CBOR_LINKS_MAP_LEN 4

// Functions all return either a CborError, or a negative version of the OC_STACK return values
static int64_t OCConvertPayloadHelper(OCPayload* payload, uint8_t* outPayload, size_t* size);
static int64_t OCConvertDiscoveryPayload(OCDiscoveryPayload* payload, uint8_t* outPayload,
        size_t* size);
static int64_t OCConvertDevicePayload(OCDevicePayload* payload, uint8_t* outPayload,
        size_t* size);
static int64_t OCConvertPlatformPayload(OCPlatformPayload* payload, uint8_t* outPayload,
        size_t* size);
static int64_t OCConvertRepPayload(OCRepPayload* payload, uint8_t* outPayload, size_t* size);
static int64_t OCConvertPresencePayload(OCPresencePayload* payload, uint8_t* outPayload,
        size_t* size);
static int64_t OCConvertSecurityPayload(OCSecurityPayload* payload, uint8_t* outPayload,
        size_t* size);
static int64_t OCConvertSingleRepPayload(CborEncoder* parent, const OCRepPayload* payload);
static int64_t OCConvertArray(CborEncoder* parent, const OCRepPayloadValueArray* valArray);

static int64_t AddTextStringToMap(CborEncoder* map, const char* key, size_t keylen,
        const char* value);

static int64_t ConditionalAddTextStringToMap(CborEncoder* map, const char* key, size_t keylen,
        const char* value);

OCStackResult OCConvertPayload(OCPayload* payload, uint8_t** outPayload, size_t* size)
{
    // TinyCbor Version 47a78569c0 or better on master is required for the re-allocation
    // strategy to work.  If you receive the following assertion error, please do a git-pull
    // from the extlibs/tinycbor/tinycbor directory
    #define CborNeedsUpdating  (CborErrorOutOfMemory < CborErrorDataTooLarge)
//    OC_STATIC_ASSERT(!CborNeedsUpdating, "tinycbor needs to be updated to at least 47a78569c0");  //JD
    #undef CborNeedsUpdating
    if (!payload)
    {
        OC_LOG(ERROR, TAG, "Payload parameter NULL");
        return OC_STACK_INVALID_PARAM;
    }

    if (!outPayload || !size)
    {
        OC_LOG(ERROR, TAG, "Out parameter/s parameter NULL");
        return OC_STACK_INVALID_PARAM;
    }

    OC_LOG_V(INFO, TAG, "Converting payload of type %d", payload->type);

    size_t curSize = INIT_SIZE;
    uint8_t* out = (uint8_t*)OICCalloc(1, curSize);
    int64_t err = OCConvertPayloadHelper(payload, out, &curSize);

    if (err == CborErrorOutOfMemory)
    {
        // reallocate "out" and try again!
        uint8_t* out2 = (uint8_t*)OICRealloc(out, curSize);

        if (!out2)
        {
            OICFree(out);
            return OC_STACK_NO_MEMORY;
        }

        out = out2;
        err = OCConvertPayloadHelper(payload, out, &curSize);
    }

    if (err == 0)
    {
        if (curSize < INIT_SIZE)
        {
            uint8_t* out2 = (uint8_t*)OICRealloc(out, curSize);

            if (!out2)
            {
                OICFree(out);
                return OC_STACK_NO_MEMORY;
            }

            out = out2;
        }

        *size = curSize;
        *outPayload = out;
        return OC_STACK_OK;
    }
    else if (err < 0)
    {
        return (OCStackResult)-err;
    }
    else
    {
        return OC_STACK_ERROR;
    }
}

static int64_t OCConvertPayloadHelper(OCPayload* payload, uint8_t* outPayload, size_t* size)
{
    switch(payload->type)
    {
        case PAYLOAD_TYPE_DISCOVERY:
            return OCConvertDiscoveryPayload((OCDiscoveryPayload*)payload, outPayload, size);
        case PAYLOAD_TYPE_DEVICE:
            return OCConvertDevicePayload((OCDevicePayload*)payload, outPayload, size);
        case PAYLOAD_TYPE_PLATFORM:
            return OCConvertPlatformPayload((OCPlatformPayload*)payload, outPayload, size);
        case PAYLOAD_TYPE_REPRESENTATION:
            return OCConvertRepPayload((OCRepPayload*)payload, outPayload, size);
        case PAYLOAD_TYPE_PRESENCE:
            return OCConvertPresencePayload((OCPresencePayload*)payload, outPayload, size);
        case PAYLOAD_TYPE_SECURITY:
            return OCConvertSecurityPayload((OCSecurityPayload*)payload, outPayload, size);
        case PAYLOAD_TYPE_RD:
            return OCRDPayloadToCbor((OCRDPayload*)payload, outPayload, size);
        default:
            OC_LOG_V(INFO,TAG, "ConvertPayload default %d", payload->type);
            return OC_STACK_NOTIMPL;
    }
}

static int64_t checkError(int64_t err, CborEncoder* encoder, uint8_t* outPayload, size_t* size)
{
    if (err == CborErrorOutOfMemory)
    {
        *size += encoder->ptr - encoder->end;
        return err;
    }
    else if (err != 0)
    {
        OC_LOG_V(ERROR, TAG, "Convert Payload failed", err);
        return err;
    }
    else
    {
        *size = encoder->ptr - outPayload;
        return 0;
    }
}
static int64_t OCConvertSecurityPayload(OCSecurityPayload* payload, uint8_t* outPayload,
        size_t* size)
{
    CborEncoder encoder;
    int64_t err = 0;

    cbor_encoder_init(&encoder, outPayload, *size, 0);

    CborEncoder rootArray;
    err = err | cbor_encoder_create_array(&encoder, &rootArray, 1);
    CborEncoder map;

    err = err | cbor_encoder_create_map(&rootArray, &map, CborIndefiniteLength);

    if(payload->securityData)
    {
        err = err | AddTextStringToMap(&map, OC_RSRVD_REPRESENTATION,
                sizeof(OC_RSRVD_REPRESENTATION) - 1,
                payload->securityData);
    }

    err = err | cbor_encoder_close_container(&rootArray, &map);

    err = err | cbor_encoder_close_container(&encoder, &rootArray);
    return checkError(err, &encoder, outPayload, size);

}

static char* OCStringLLJoin(OCStringLL* val)
{
    OCStringLL* temp = val;
    size_t size = strlen(temp->value);

    while (temp->next)
    {
        ++size;
        temp = temp->next;
        size += strlen(temp->value);
    }

    char* joinedStr = (char*)OICCalloc(sizeof(char), size + 1);

    if (!joinedStr)
    {
        return NULL;
    }

    OICStrcat(joinedStr, size + 1, val->value);
    while (val->next)
    {
        val = val->next;
        OICStrcat(joinedStr, size + 1, " ");
        OICStrcat(joinedStr, size + 1, val->value);
    }
    return joinedStr;
}

static int64_t OCConvertDiscoveryPayload(OCDiscoveryPayload* payload, uint8_t* outPayload,
        size_t* size)
{
    CborEncoder encoder = {0};
    CborEncoder rootArray = {0};
    int64_t err = 0;

    cbor_encoder_init(&encoder, outPayload, *size, 0);

    if (payload->collectionResources)
    {
        CborError cborEncoderResult;
        cborEncoderResult = cbor_encoder_create_array(&encoder, &rootArray, DISCOVERY_CBOR_ARRAY_LEN);
        if (CborNoError != cborEncoderResult)
        {
            OC_LOG(ERROR, TAG, "Failed creating root array.");
            goto cbor_error;
        }

        CborEncoder colArray;
        cborEncoderResult = cbor_encoder_create_array(&rootArray, &colArray, CborIndefiniteLength);
        if (CborNoError != cborEncoderResult)
        {
            OC_LOG(ERROR, TAG, "Failed creating collection array.");
            goto cbor_error;
        }

        OCResourceCollectionPayload *colResources = payload->collectionResources;
        while (colResources)
        {
            if (OC_STACK_OK != OCTagsPayloadToCbor(colResources->tags, &colArray))
            {
                goto cbor_error;
            }
            if (OC_STACK_OK != OCLinksPayloadToCbor(colResources->setLinks, &colArray))
            {
                goto cbor_error;
            }
            colResources = colResources->next;
        }
        cborEncoderResult = cbor_encoder_close_container(&rootArray, &colArray);
        if (CborNoError != cborEncoderResult)
        {
            OC_LOG(ERROR, TAG, "Failed closing collection array.");
            goto cbor_error;
        }
        cborEncoderResult = cbor_encoder_close_container(&encoder, &rootArray);
        if (CborNoError != cborEncoderResult)
        {
            OC_LOG(ERROR, TAG, "Failed closing root array.");
            goto cbor_error;
        }
    }
    else if (payload->resources)
    {
        size_t resourceCount =  OCDiscoveryPayloadGetResourceCount(payload);
        err = err | cbor_encoder_create_array(&encoder, &rootArray, resourceCount);

        for(size_t i = 0; i < resourceCount; ++i)
        {
            CborEncoder map;
            OCResourcePayload* resource = OCDiscoveryPayloadGetResource(payload, i);
            if(!resource)
            {
                OICFree(outPayload);
                return OC_STACK_INVALID_PARAM;
            }

            err = err | cbor_encoder_create_map(&rootArray, &map, DISCOVERY_CBOR_RES_MAP_LEN);

            // Device ID
            err = err | cbor_encode_text_string(&map, OC_RSRVD_DEVICE_ID,
                    sizeof(OC_RSRVD_DEVICE_ID) - 1);
            err = err | cbor_encode_byte_string(&map, resource->sid, UUID_SIZE);

            {
                CborEncoder linkArray;
                err = err | cbor_encode_text_string(&map, OC_RSRVD_LINKS, sizeof(OC_RSRVD_LINKS) -1);
                err = err | cbor_encoder_create_array(&map, &linkArray, CborIndefiniteLength);

                // Link Map
                {
                    CborEncoder linkMap;
                    err = err | cbor_encoder_create_map(&linkArray, &linkMap, DISCOVERY_CBOR_LINKS_MAP_LEN);

                    // Uri
                    err = err | AddTextStringToMap(&linkMap, OC_RSRVD_HREF,
                            sizeof(OC_RSRVD_HREF) - 1,
                            resource->uri);
                    // Resource Type
                    if (resource->types)
                    {
                        char* joinedTypes = OCStringLLJoin(resource->types);
                        if (joinedTypes)
                        {
                            err = err | cbor_encode_text_string(&linkMap, OC_RSRVD_RESOURCE_TYPE,
                                    sizeof(OC_RSRVD_RESOURCE_TYPE) - 1);
                            err = err | cbor_encode_text_string(&linkMap, joinedTypes,
                                    strlen(joinedTypes));
                            OICFree(joinedTypes);
                        }
                        else
                        {
                            return OC_STACK_NO_MEMORY;
                        }
                    }
                    // Interface Types
                    if (resource->interfaces)
                    {
                        char* joinedInterfaces = OCStringLLJoin(resource->interfaces);
                        if (joinedInterfaces)
                        {
                            err = err | cbor_encode_text_string(&linkMap, OC_RSRVD_INTERFACE,
                                    sizeof(OC_RSRVD_INTERFACE) - 1);
                            err = err | cbor_encode_text_string(&linkMap, joinedInterfaces,
                                    strlen(joinedInterfaces));
                            OICFree(joinedInterfaces);
                        }
                        else
                        {
                            return OC_STACK_NO_MEMORY;
                        }
                    }
                    // Policy
                    {
                        CborEncoder policyMap;
                        err = err | cbor_encode_text_string(&linkMap, OC_RSRVD_POLICY,
                                sizeof(OC_RSRVD_POLICY) - 1);
                        err = err | cbor_encoder_create_map(&linkMap, &policyMap, CborIndefiniteLength);

                        // Bitmap
                        err = err | cbor_encode_text_string(&policyMap, OC_RSRVD_BITMAP,
                                sizeof(OC_RSRVD_BITMAP) - 1);
                        err = err | cbor_encode_uint(&policyMap, resource->bitmap);

                        if(resource->secure)
                        {
                            err = err | cbor_encode_text_string(&policyMap, OC_RSRVD_SECURE,
                                    sizeof(OC_RSRVD_SECURE) - 1);
                            err = err | cbor_encode_boolean(&policyMap, OC_RESOURCE_SECURE);

                            if(resource->port != 0)
                            {
                                err = err | cbor_encode_text_string(&policyMap, OC_RSRVD_HOSTING_PORT,
                                        sizeof(OC_RSRVD_HOSTING_PORT) - 1);
                                err = err | cbor_encode_uint(&policyMap, resource->port);
                            }
                        }

                        err = err | cbor_encoder_close_container(&linkMap, &policyMap);
                    }
                    // Close
                    err = err | cbor_encoder_close_container(&linkArray, &linkMap);
                }
                err = err | cbor_encoder_close_container(&map, &linkArray);
            }
            err = err | cbor_encoder_close_container(&rootArray, &map);
        }
        // Close main array
        err = err | cbor_encoder_close_container(&encoder, &rootArray);
    }

    return checkError(err, &encoder, outPayload, size);
cbor_error:
    OICFree(outPayload);
    return OC_STACK_ERROR;
}

static int64_t OCConvertDevicePayload(OCDevicePayload* payload, uint8_t* outPayload,
        size_t* size)
{
    CborEncoder encoder = {0};
    int64_t err = 0;

    cbor_encoder_init(&encoder, outPayload, *size, 0);
    CborEncoder rootArray;
    err = err | cbor_encoder_create_array(&encoder, &rootArray, 1);

    {
        CborEncoder map;
        err = err | cbor_encoder_create_map(&rootArray, &map, CborIndefiniteLength);

        // uri
        err = err | ConditionalAddTextStringToMap(&map, OC_RSRVD_HREF, sizeof(OC_RSRVD_HREF) - 1,
                payload->uri);

        // Rep Map
        {
            CborEncoder repMap;
            err = err | cbor_encode_text_string(&map, OC_RSRVD_REPRESENTATION,
                    sizeof(OC_RSRVD_REPRESENTATION) - 1);
            err = err | cbor_encoder_create_map(&map, &repMap, CborIndefiniteLength);

            // Device ID
            err = err | cbor_encode_text_string(&repMap, OC_RSRVD_DEVICE_ID,
                    sizeof(OC_RSRVD_DEVICE_ID) - 1);
            err = err | cbor_encode_byte_string(&repMap, payload->sid, UUID_SIZE);

            // Device Name
            err = err | ConditionalAddTextStringToMap(&repMap, OC_RSRVD_DEVICE_NAME,
                    sizeof(OC_RSRVD_DEVICE_NAME) - 1,
                    payload->deviceName);

            // Device Spec Version
            err = err | ConditionalAddTextStringToMap(&repMap, OC_RSRVD_SPEC_VERSION,
                    sizeof(OC_RSRVD_SPEC_VERSION) - 1,
                    payload->specVersion);

            // Device data Model Version
            err = err | ConditionalAddTextStringToMap(&repMap, OC_RSRVD_DATA_MODEL_VERSION,
                    sizeof(OC_RSRVD_DATA_MODEL_VERSION) - 1,
                    payload->dataModelVersion);

            err = err | cbor_encoder_close_container(&map, &repMap);
        }

        // Close Map
        err = err | cbor_encoder_close_container(&rootArray, &map);
    }

    // Close main array
    err = err | cbor_encoder_close_container(&encoder, &rootArray);

    return checkError(err, &encoder, outPayload, size);
}

static int64_t OCConvertPlatformPayload(OCPlatformPayload* payload, uint8_t* outPayload,
        size_t* size)
{
    CborEncoder encoder = {0};
    int64_t err = 0;

    cbor_encoder_init(&encoder, outPayload, *size, 0);
    CborEncoder rootArray;
    err = err | cbor_encoder_create_array(&encoder, &rootArray, 1);
    {
        CborEncoder map;
        err = err | cbor_encoder_create_map(&rootArray, &map, CborIndefiniteLength);

        // uri
        err = err | ConditionalAddTextStringToMap(&map, OC_RSRVD_HREF, sizeof(OC_RSRVD_HREF) - 1,
                payload->uri);

        // Rep Map
        {
            CborEncoder repMap;
            err = err | cbor_encode_text_string(&map, OC_RSRVD_REPRESENTATION,
                    sizeof(OC_RSRVD_REPRESENTATION) - 1);
            err = err | cbor_encoder_create_map(&map, &repMap, CborIndefiniteLength);

            // Platform ID
            err = err | ConditionalAddTextStringToMap(&repMap, OC_RSRVD_PLATFORM_ID,
                    sizeof(OC_RSRVD_PLATFORM_ID) - 1,
                    payload->info.platformID);

            // MFG Name
            err = err | ConditionalAddTextStringToMap(&repMap, OC_RSRVD_MFG_NAME,
                    sizeof(OC_RSRVD_MFG_NAME) - 1,
                    payload->info.manufacturerName);

            // MFG Url
            err = err | ConditionalAddTextStringToMap(&repMap, OC_RSRVD_MFG_URL,
                    sizeof(OC_RSRVD_MFG_URL) - 1,
                    payload->info.manufacturerUrl);

            // Model Num
            err = err | ConditionalAddTextStringToMap(&repMap, OC_RSRVD_MODEL_NUM,
                    sizeof(OC_RSRVD_MODEL_NUM) - 1,
                    payload->info.modelNumber);

            // Date of Mfg
            err = err | ConditionalAddTextStringToMap(&repMap, OC_RSRVD_MFG_DATE,
                    sizeof(OC_RSRVD_MFG_DATE) - 1,
                    payload->info.dateOfManufacture);

            // Platform Version
            err = err | ConditionalAddTextStringToMap(&repMap, OC_RSRVD_PLATFORM_VERSION,
                    sizeof(OC_RSRVD_PLATFORM_VERSION) - 1,
                    payload->info.platformVersion);

            // OS Version
            err = err | ConditionalAddTextStringToMap(&repMap, OC_RSRVD_OS_VERSION,
                    sizeof(OC_RSRVD_OS_VERSION) - 1,
                    payload->info.operatingSystemVersion);

            // Hardware Version
            err = err | ConditionalAddTextStringToMap(&repMap, OC_RSRVD_HARDWARE_VERSION,
                    sizeof(OC_RSRVD_HARDWARE_VERSION) - 1,
                    payload->info.hardwareVersion);

            // Firmware Version
            err = err | ConditionalAddTextStringToMap(&repMap, OC_RSRVD_FIRMWARE_VERSION,
                    sizeof(OC_RSRVD_FIRMWARE_VERSION) - 1,
                    payload->info.firmwareVersion);

            // Support URL
            err = err | ConditionalAddTextStringToMap(&repMap, OC_RSRVD_SUPPORT_URL,
                    sizeof(OC_RSRVD_SUPPORT_URL) - 1,
                    payload->info.supportUrl);

            // System Time
            err = err | ConditionalAddTextStringToMap(&repMap, OC_RSRVD_SYSTEM_TIME,
                    sizeof(OC_RSRVD_SYSTEM_TIME) - 1,
                    payload->info.systemTime);
            err = err | cbor_encoder_close_container(&map, &repMap);
        }

        // Close Map
        err = err | cbor_encoder_close_container(&rootArray, &map);
    }

    // Close main array
    err = err | cbor_encoder_close_container(&encoder, &rootArray);

    return checkError(err, &encoder, outPayload, size);
}

static int64_t OCConvertArrayItem(CborEncoder* array, const OCRepPayloadValueArray* valArray,
        size_t index)
{
    int64_t err = 0;
    switch (valArray->type)
    {
        case OCREP_PROP_NULL:
            OC_LOG(ERROR, TAG, "ConvertArray Invalid NULL");
            err = CborUnknownError;
            break;
        case OCREP_PROP_INT:
            err = err | cbor_encode_int(array, valArray->iArray[index]);
            break;
        case OCREP_PROP_DOUBLE:
            err = err | cbor_encode_double(array, valArray->dArray[index]);
            break;
        case OCREP_PROP_BOOL:
            err = err | cbor_encode_boolean(array, valArray->bArray[index]);
            break;
        case OCREP_PROP_STRING:
            if (!valArray->strArray[index])
            {
                err = err | cbor_encode_null(array);
            }
            else
            {
                err = err | cbor_encode_text_string(array, valArray->strArray[index],
                        strlen(valArray->strArray[index]));
            }
            break;
        case OCREP_PROP_OBJECT:
            if (!valArray->objArray[index])
            {
                err = err | cbor_encode_null(array);
            }
            else
            {
                err = OCConvertSingleRepPayload(array, valArray->objArray[index]);
            }
            break;
        case OCREP_PROP_ARRAY:
            OC_LOG(ERROR, TAG, "ConvertArray Invalid child array");
            err = CborUnknownError;
            break;
    }

    return err;
}
static int64_t OCConvertArray(CborEncoder* parent, const OCRepPayloadValueArray* valArray)
{
    CborEncoder array;
    int64_t err = 0;

    err = err | cbor_encoder_create_array(parent, &array, valArray->dimensions[0]);

    for (size_t i = 0; i < valArray->dimensions[0];++i)
    {
        if (valArray->dimensions[1] != 0)
        {
            CborEncoder array2;
            err = err | cbor_encoder_create_array(&array, &array2, valArray->dimensions[1]);

            for (size_t j = 0; j < valArray->dimensions[1]; ++j)
            {
                if (valArray->dimensions[2] != 0)
                {
                    CborEncoder array3;
                    err = err | cbor_encoder_create_array(&array2, &array3,
                            valArray->dimensions[2]);

                    for(size_t k = 0; k < valArray->dimensions[2]; ++k)
                    {
                        OCConvertArrayItem(&array3, valArray,
                            j * valArray->dimensions[2] +
                            i * valArray->dimensions[2] * valArray->dimensions[1] +
                            k);
                    }
                    err = err | cbor_encoder_close_container(&array2, &array3);
                }
                else
                {
                    OCConvertArrayItem(&array2, valArray,
                            i * valArray->dimensions[1] + j);
                }
            }
            err = err | cbor_encoder_close_container(&array, &array2);
        }
        else
        {
            OCConvertArrayItem(&array, valArray, i);
        }
    }
    err = err | cbor_encoder_close_container(parent, &array);
    return err;
}

static int64_t OCConvertSingleRepPayload(CborEncoder* parent, const OCRepPayload* payload)
{
    int64_t err = 0;
    CborEncoder map;
    err = err | cbor_encoder_create_map(parent, &map, CborIndefiniteLength);

    // Uri
    err = err | ConditionalAddTextStringToMap(&map, OC_RSRVD_HREF,
            sizeof(OC_RSRVD_HREF) - 1,
            payload->uri);

    // Prop Map
    // resource types, interfaces
    if(payload->types || payload->interfaces)
    {
        OC_LOG(INFO, TAG, "Payload has types or interfaces");
        err = err | cbor_encode_text_string(&map,
                OC_RSRVD_PROPERTY,
                sizeof(OC_RSRVD_PROPERTY) - 1);
        CborEncoder propMap;
        err = err | cbor_encoder_create_map(&map, &propMap, 2);

        if (payload->types)
        {
            char* joinedTypes = OCStringLLJoin(payload->types);
            if (joinedTypes)
            {
                err = err | cbor_encode_text_string(&propMap, OC_RSRVD_RESOURCE_TYPE,
                        sizeof(OC_RSRVD_RESOURCE_TYPE) - 1);
                err = err | cbor_encode_text_string(&propMap, joinedTypes,
                        strlen(joinedTypes));
                OICFree(joinedTypes);
            }
            else
            {
                return OC_STACK_NO_MEMORY;
            }
        }
        if (payload->interfaces)
        {
            char* joinedInterfaces = OCStringLLJoin(payload->interfaces);
            if (joinedInterfaces)
            {
                err = err | cbor_encode_text_string(&propMap, OC_RSRVD_INTERFACE,
                        sizeof(OC_RSRVD_INTERFACE) - 1);
                err = err | cbor_encode_text_string(&propMap, joinedInterfaces,
                        strlen(joinedInterfaces));
                OICFree(joinedInterfaces);
            }
            else
            {
                return OC_STACK_NO_MEMORY;
            }
        }
        err = err | cbor_encoder_close_container(&map, &propMap);
    }

    // Rep Map
    {
        CborEncoder repMap;
        err = err | cbor_encode_text_string(&map,
                OC_RSRVD_REPRESENTATION,
                sizeof(OC_RSRVD_REPRESENTATION) - 1);
        err = err | cbor_encoder_create_map(&map, &repMap, CborIndefiniteLength);
        OCRepPayloadValue* value = payload->values;
        while(value)
        {
            err = err | cbor_encode_text_string(&repMap,
                    value->name,
                    strlen(value->name));
            switch(value->type)
            {
                case OCREP_PROP_NULL:
                    err = err | cbor_encode_null(&repMap);
                    break;
                case OCREP_PROP_INT:
                    err = err | cbor_encode_int(&repMap,
                            value->i);
                    break;
                case OCREP_PROP_DOUBLE:
                    err = err | cbor_encode_double(&repMap,
                            value->d);
                    break;
                case OCREP_PROP_BOOL:
                    err = err | cbor_encode_boolean(&repMap,
                            value->b);
                    break;
                case OCREP_PROP_STRING:
                    err = err | cbor_encode_text_string(&repMap,
                            value->str, strlen(value->str));
                    break;
                case OCREP_PROP_OBJECT:
                    err = err | OCConvertSingleRepPayload(&repMap, value->obj);
                    break;
                case OCREP_PROP_ARRAY:
                    err = err | OCConvertArray(&repMap, &value->arr);
                    break;
                default:
                    OC_LOG_V(ERROR, TAG, "Invalid Prop type: %d",
                            value->type);
                    break;
            }
            value = value->next;
        }

        err = err | cbor_encoder_close_container(&map, &repMap);
    }

    // Close Map
    err = err | cbor_encoder_close_container(parent, &map);

    return err;
}

static int64_t OCConvertRepPayload(OCRepPayload* payload, uint8_t* outPayload, size_t* size)
{
    CborEncoder encoder = {0};
    int64_t err = 0;

    cbor_encoder_init(&encoder, outPayload, *size, 0);
    CborEncoder rootArray;
    err = err | cbor_encoder_create_array(&encoder, &rootArray, CborIndefiniteLength);

    while(payload != NULL && (err == 0 || err == CborErrorOutOfMemory))
    {
        err = err | OCConvertSingleRepPayload(&rootArray, payload);
        payload = payload->next;
    }

    // Close main array
    err = err | cbor_encoder_close_container(&encoder, &rootArray);

    return checkError(err, &encoder, outPayload, size);
}

static int64_t OCConvertPresencePayload(OCPresencePayload* payload,
        uint8_t* outPayload, size_t* size)
{
    CborEncoder encoder = {0};
    int64_t err = 0;

    cbor_encoder_init(&encoder, outPayload, *size, 0);
    CborEncoder rootArray;

    err = err | cbor_encoder_create_array(&encoder, &rootArray, 1);

    CborEncoder map;
    err = err | cbor_encoder_create_map(&rootArray, &map, CborIndefiniteLength);

    // Sequence Number
    err = err | cbor_encode_text_string(&map,
            OC_RSRVD_NONCE,
            sizeof(OC_RSRVD_NONCE) - 1);
    err = err | cbor_encode_uint(&map, payload->sequenceNumber);

    // Max Age
    err = err | cbor_encode_text_string(&map,
            OC_RSRVD_TTL,
            sizeof(OC_RSRVD_TTL) - 1);
    err = err | cbor_encode_uint(&map, payload->maxAge);

    // Trigger
    const char* triggerStr = convertTriggerEnumToString(payload->trigger);
    err = err | AddTextStringToMap(&map, OC_RSRVD_TRIGGER, sizeof(OC_RSRVD_TRIGGER) - 1,
            triggerStr);

    // Resource type name
    if(payload->trigger != OC_PRESENCE_TRIGGER_DELETE)
    {
        err = err | ConditionalAddTextStringToMap(&map, OC_RSRVD_RESOURCE_TYPE,
                sizeof(OC_RSRVD_RESOURCE_TYPE) - 1, payload->resourceType);
    }

    // Close Map
    err = err | cbor_encoder_close_container(&rootArray, &map);
    err = err | cbor_encoder_close_container(&encoder, &rootArray);

    return checkError(err, &encoder, outPayload, size);
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
