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

// Defining _POSIX_C_SOURCE macro with 200112L (or greater) as value
// causes header files to expose definitions
// corresponding to the POSIX.1-2001 base
// specification (excluding the XSI extension).
// For POSIX.1-2001 base specification,
// Refer http://pubs.opengroup.org/onlinepubs/009695399/
// Required for strok_r
#define _POSIX_C_SOURCE 200112L
#include <string.h>
#include "ocpayloadcbor.h"
#include <stdlib.h>
#include "logger.h"
#include "oic_string.h"
#include "oic_malloc.h"
#include "ocstackinternal.h"
#include "ocpayload.h"
#include "cbor.h"
#include "oic_string.h"
#include "payload_logging.h"
#include "rdpayload.h"

#define TAG "OCPayloadParse"

static OCStackResult OCParseDiscoveryPayload(OCPayload** outPayload, CborValue* arrayVal);
static OCStackResult OCParseDevicePayload(OCPayload** outPayload, CborValue* arrayVal);
static OCStackResult OCParsePlatformPayload(OCPayload** outPayload, CborValue* arrayVal);
static bool OCParseSingleRepPayload(OCRepPayload** outPayload, CborValue* repParent);
static OCStackResult OCParseRepPayload(OCPayload** outPayload, CborValue* arrayVal);
static OCStackResult OCParsePresencePayload(OCPayload** outPayload, CborValue* arrayVal);
static OCStackResult OCParseSecurityPayload(OCPayload** outPayload, CborValue* arrayVal);

OCStackResult OCParsePayload(OCPayload** outPayload, OCPayloadType payloadType,
        const uint8_t* payload, size_t payloadSize)
{
    CborParser parser;
    CborValue rootValue;
    bool err = false;

    OC_LOG_V(INFO, TAG, "CBOR Parsing size: %d", payloadSize);
    if((err = cbor_parser_init(payload, payloadSize, 0, &parser, &rootValue)) != false)
    {
        OC_LOG_V(ERROR, TAG, "CBOR Parser init failed: %d", err);
        return OC_STACK_ERROR;
    }

    if(!cbor_value_is_array(&rootValue))
    {
        OC_LOG_V(ERROR, TAG, "CBOR payload root object is not an array :%x", rootValue.type);
        return OC_STACK_MALFORMED_RESPONSE;
    }

    CborValue arrayValue;
    // enter the array
    err = err || cbor_value_enter_container(&rootValue, &arrayValue);

    if(err)
    {
        OC_LOG_V(ERROR, TAG, "CBOR payload parse failed :%d", err);
        return OC_STACK_MALFORMED_RESPONSE;
    }

    OCStackResult result = OC_STACK_ERROR;
    switch(payloadType)
    {
        case PAYLOAD_TYPE_DISCOVERY:
            result = OCParseDiscoveryPayload(outPayload, &arrayValue);
            break;
        case PAYLOAD_TYPE_DEVICE:
            result = OCParseDevicePayload(outPayload, &arrayValue);
            break;
        case PAYLOAD_TYPE_PLATFORM:
            result = OCParsePlatformPayload(outPayload, &arrayValue);
            break;
        case PAYLOAD_TYPE_REPRESENTATION:
            result = OCParseRepPayload(outPayload, &arrayValue);
            break;
        case PAYLOAD_TYPE_PRESENCE:
            result = OCParsePresencePayload(outPayload, &arrayValue);
            break;
        case PAYLOAD_TYPE_SECURITY:
            result = OCParseSecurityPayload(outPayload, &arrayValue);
            break;
        case PAYLOAD_TYPE_RD:
            result = OCRDCborToPayload(&arrayValue, outPayload);
            break;
        default:
            OC_LOG_V(ERROR, TAG, "ParsePayload Type default: %d", payloadType);
            result = OC_STACK_ERROR;
            break;
    }

    if(result == OC_STACK_OK)
    {
        err = err || cbor_value_leave_container(&rootValue, &arrayValue);
        if(err != CborNoError)
        {
            return OC_STACK_MALFORMED_RESPONSE;
        }
    }
    else
    {
        OC_LOG_V(INFO, TAG, "Finished parse payload, result is %d", result);
    }

    return result;
}

void OCFreeOCStringLL(OCStringLL* ll);

static OCStackResult OCParseSecurityPayload(OCPayload** outPayload, CborValue* arrayVal)
{
    if (!outPayload)
    {
        return OC_STACK_INVALID_PARAM;
    }

    bool err = false;
    char * securityData = NULL;

    if(cbor_value_is_map(arrayVal))
    {
        CborValue curVal;
        err = err || cbor_value_map_find_value(arrayVal, OC_RSRVD_REPRESENTATION, &curVal);

        if(cbor_value_is_valid(&curVal))
        {
            size_t len;
            err = err || cbor_value_dup_text_string(&curVal, &securityData, &len, NULL);
        }
    }
    else
    {
        OC_LOG(ERROR, TAG, "Cbor main value not a map");
        return OC_STACK_MALFORMED_RESPONSE;
    }

    err = err || cbor_value_advance(arrayVal);

    if(err)
    {
        OC_LOG(ERROR, TAG, "Cbor in error condition");
        OICFree(securityData);
        return OC_STACK_MALFORMED_RESPONSE;
    }

    *outPayload = (OCPayload*)OCSecurityPayloadCreate(securityData);
    OICFree(securityData);

    return OC_STACK_OK;

}

static char* InPlaceStringTrim(char* str)
{
    while (str[0] == ' ')
    {
        ++str;
    }

    size_t lastchar = strlen(str);

    while (str[lastchar] == ' ')
    {
        str[lastchar] = '\0';
        --lastchar;
    }

    return str;
}

static OCStackResult OCParseDiscoveryPayload(OCPayload** outPayload, CborValue* arrayVal)
{
    if (!outPayload)
    {
        return OC_STACK_INVALID_PARAM;
    }

    bool err = false;
    OCResourcePayload* resource = NULL;

    OCDiscoveryPayload* out = OCDiscoveryPayloadCreate();
    if(!out)
    {
        return OC_STACK_NO_MEMORY;
    }

    if (cbor_value_is_array(arrayVal))
    {
        OCLinksPayload *linksPayload = NULL;
        OCTagsPayload *tagsPayload = NULL;
        while (cbor_value_is_container(arrayVal))
        {
            linksPayload = NULL;
            tagsPayload = NULL;
            CborValue colResources;
            CborError cborFindResult = cbor_value_enter_container(arrayVal, &colResources);
            if (CborNoError != cborFindResult)
            {
                goto cbor_error;
            }

            if (OC_STACK_OK != OCTagsCborToPayload(&colResources, &tagsPayload))
            {
                OC_LOG(ERROR, TAG, "Tags cbor parsing failed.");
                OCFreeTagsResource(tagsPayload);
                goto cbor_error;
            }

            if (OC_STACK_OK != OCLinksCborToPayload(&colResources, &linksPayload))
            {
                OC_LOG(ERROR, TAG, "Links cbor parsing failed.");
                OCFreeTagsResource(tagsPayload);
                OCFreeLinksResource(linksPayload);
                goto cbor_error;
            }

            if (OC_STACK_OK != OCDiscoveryCollectionPayloadAddResource(out, tagsPayload, linksPayload))
            {
                OC_LOG(ERROR, TAG, "Memory allocation failed");
                OCFreeLinksResource(linksPayload);
                OCFreeTagsResource(tagsPayload);
                OCDiscoveryPayloadDestroy(out);
                return OC_STACK_NO_MEMORY;
            }
            if (CborNoError != cbor_value_advance(arrayVal))
            {
                OC_LOG(ERROR, TAG, "Cbor value advanced failed.");
                goto cbor_error;
            }
        }
    }
    if (cbor_value_is_map(arrayVal))
    {
        size_t resourceCount = 0;
        while (cbor_value_is_map(arrayVal))
        {
            resource = (OCResourcePayload*)OICCalloc(1, sizeof(OCResourcePayload));
            if(!resource)
            {
                OC_LOG(ERROR, TAG, "Memory allocation failed");
                OCDiscoveryPayloadDestroy(out);
                return OC_STACK_NO_MEMORY;
            }
            CborValue curVal;
            // DI
            err = cbor_value_map_find_value(arrayVal, OC_RSRVD_DEVICE_ID, &curVal);
            if (CborNoError != err)
            {
                OC_LOG(ERROR, TAG, "Cbor find value failed.");
                goto malformed_cbor;
            }
            size_t len;
            err = cbor_value_dup_byte_string(&curVal, &(resource->sid), &len, NULL);
            if (CborNoError != err)
            {
                OC_LOG(ERROR, TAG, "Cbor di finding failed.");
                goto malformed_cbor;
            }
            // Links TAG
            {
                CborValue linkArray;
                err = cbor_value_map_find_value(arrayVal, OC_RSRVD_LINKS, &linkArray);
                if (CborNoError != err)
                {
                    OC_LOG(ERROR, TAG, "Cbor links finding failed.");
                    goto malformed_cbor;
                }
                CborValue linkMap;
                err = cbor_value_enter_container(&linkArray, &linkMap);
                if (CborNoError != err)
                {
                    OC_LOG(ERROR, TAG, "Cbor entering map failed.");
                    goto malformed_cbor;
                }
                // Uri
                err = cbor_value_map_find_value(&linkMap, OC_RSRVD_HREF, &curVal);
                if (CborNoError != err)
                {
                    OC_LOG(ERROR, TAG, "Cbor finding href type failed.");
                    goto malformed_cbor;
                }
                err = cbor_value_dup_text_string(&curVal, &(resource->uri), &len, NULL);
                if (CborNoError != err)
                {
                    OC_LOG(ERROR, TAG, "Cbor finding href value failed.");
                    goto malformed_cbor;
                }
                // ResourceTypes
                CborValue rtVal;
                err = cbor_value_map_find_value(&linkMap, OC_RSRVD_RESOURCE_TYPE, &rtVal);
                if (CborNoError != err)
                {
                    OC_LOG(ERROR, TAG, "Cbor finding rt type failed.");
                    goto malformed_cbor;
                }
                if (cbor_value_is_text_string(&rtVal))
                {
                    char* input = NULL;
                    char* savePtr;
                    err = cbor_value_dup_text_string(&rtVal, &input, &len, NULL);
                    if (CborNoError != err)
                    {
                        OC_LOG(ERROR, TAG, "Cbor finding rt value failed.");
                        goto malformed_cbor;
                    }
                    if (input)
                    {
                        char* curPtr = strtok_r(input, " ", &savePtr);

                        while (curPtr)
                        {
                            char* trimmed = InPlaceStringTrim(curPtr);
                            if (trimmed[0] !='\0')
                            {
                                if (!OCResourcePayloadAddResourceType(resource, trimmed))
                                {
                                    OICFree(resource->uri);
                                    OICFree(resource->sid);
                                    OCFreeOCStringLL(resource->types);
                                    OICFree(resource);
                                    OCDiscoveryPayloadDestroy(out);
                                    return OC_STACK_NO_MEMORY;
                                }
                            }
                            curPtr = strtok_r(NULL, " ", &savePtr);
                        }
                        OICFree(input);
                    }
                }

                // Interface Types
                CborValue ifVal;
                err = cbor_value_map_find_value(&linkMap, OC_RSRVD_INTERFACE, &ifVal);
                if (CborNoError != err)
                {
                    OC_LOG(ERROR, TAG, "Cbor finding if type failed.");
                    goto malformed_cbor;
                }
                if (!err && cbor_value_is_text_string(&ifVal))
                {
                    char* input = NULL;
                    char* savePtr;
                    err = cbor_value_dup_text_string(&ifVal, &input, &len, NULL);
                    if (CborNoError != err)
                    {
                        OC_LOG(ERROR, TAG, "Cbor finding if value failed.");
                        goto malformed_cbor;
                    }
                    if (input)
                    {
                        char* curPtr = strtok_r(input, " ", &savePtr);

                        while (curPtr)
                        {
                            char* trimmed = InPlaceStringTrim(curPtr);
                            if (trimmed[0] !='\0')
                            {
                                if (!OCResourcePayloadAddInterface(resource, trimmed))
                                {
                                    OICFree(resource->uri);
                                    OICFree(resource->sid);
                                    OCFreeOCStringLL(resource->types);
                                    OICFree(resource);
                                    OCDiscoveryPayloadDestroy(out);
                                    return OC_STACK_NO_MEMORY;
                                }
                            }
                            curPtr = strtok_r(NULL, " ", &savePtr);
                        }
                        OICFree(input);
                    }
                }
                // Policy
                {
                    CborValue policyMap;
                    err = cbor_value_map_find_value(&linkMap, OC_RSRVD_POLICY, &policyMap);
                    if (CborNoError != err)
                    {
                        OC_LOG(ERROR, TAG, "Cbor finding policy type failed.");
                        goto malformed_cbor;
                    }
                    // Bitmap
                    CborValue val;
                    err = cbor_value_map_find_value(&policyMap, OC_RSRVD_BITMAP, &val);
                    if (CborNoError != err)
                    {
                        OC_LOG(ERROR, TAG, "Cbor finding bitmap type failed.");
                        goto malformed_cbor;
                    }
                    uint64_t temp = 0;
                    err = cbor_value_get_uint64(&val, &temp);
                    if (CborNoError != err)
                    {
                        OC_LOG(ERROR, TAG, "Cbor finding bitmap value failed.");
                        goto malformed_cbor;
                    }
                    resource->bitmap = (uint8_t)temp;
                    // Secure Flag
                    err = cbor_value_map_find_value(&policyMap, OC_RSRVD_SECURE, &val);
                    if (CborNoError != err)
                    {
                        OC_LOG(ERROR, TAG, "Cbor finding secure type failed.");
                        goto malformed_cbor;
                    }
                    if(cbor_value_is_valid(&val))
                    {
                        err = cbor_value_get_boolean(&val, &(resource->secure));
                        if (CborNoError != err)
                        {
                            OC_LOG(ERROR, TAG, "Cbor finding secure value failed.");
                            goto malformed_cbor;
                        }
                        // Port
                        CborValue port;
                        err = cbor_value_map_find_value(&policyMap, OC_RSRVD_HOSTING_PORT,
                                        &port);
                        if (CborNoError != err)
                        {
                            OC_LOG(ERROR, TAG, "Cbor finding port type failed.");
                            goto malformed_cbor;
                        }
                        if(cbor_value_is_valid(&port))
                        {
                            err = cbor_value_get_uint64(&port, &temp);
                            if (CborNoError != err)
                            {
                                OC_LOG(ERROR, TAG, "Cbor finding port value failed.");
                                goto malformed_cbor;
                            }
                            resource->port = (uint16_t)temp;
                        }
                    }
                }
            }
            err = cbor_value_advance(arrayVal);
            if (CborNoError != err)
            {
                OC_LOG(ERROR, TAG, "Cbor advance value failed.");
                goto malformed_cbor;
            }
            ++resourceCount;
            OCDiscoveryPayloadAddNewResource(out, resource);
        }
    }

    *outPayload = (OCPayload*)out;
    return OC_STACK_OK;

malformed_cbor:
    OICFree(resource->uri);
    OICFree(resource->sid);
    OCFreeOCStringLL(resource->types);
    OCFreeOCStringLL(resource->interfaces);
    OICFree(resource);
    OCDiscoveryPayloadDestroy(out);
    return OC_STACK_MALFORMED_RESPONSE;

cbor_error:
    OCDiscoveryCollectionPayloadDestroy(out);
    return OC_STACK_MALFORMED_RESPONSE;
}

static OCStackResult OCParseDevicePayload(OCPayload** outPayload, CborValue* arrayVal)
{
    if (!outPayload)
    {
        return OC_STACK_INVALID_PARAM;
    }

    bool err = false;

    if(cbor_value_is_map(arrayVal))
    {
        char* uri = NULL;
        uint8_t* sid = NULL;
        char* dname = NULL;
        char* specVer = NULL;
        char* dmVer = NULL;
        CborValue curVal;
         err = err || cbor_value_map_find_value(arrayVal, OC_RSRVD_HREF, &curVal);
        size_t len;
         err = err || cbor_value_dup_text_string(&curVal, &uri, &len, NULL);

        // Representation
        {
             err = err || cbor_value_map_find_value(arrayVal, OC_RSRVD_REPRESENTATION, &curVal);

            CborValue repVal;
            // Device ID
            err = err || cbor_value_map_find_value(&curVal, OC_RSRVD_DEVICE_ID, &repVal);
            if(cbor_value_is_valid(&repVal))
            {
                err = err || cbor_value_dup_byte_string(&repVal, &sid, &len, NULL);
            }
            // Device Name
            err = err || cbor_value_map_find_value(&curVal, OC_RSRVD_DEVICE_NAME, &repVal);
            if(cbor_value_is_valid(&repVal))
            {
                err = err || cbor_value_dup_text_string(&repVal, &dname, &len, NULL);
            }
            // Device Spec Version
            err = err || cbor_value_map_find_value(&curVal, OC_RSRVD_SPEC_VERSION, &repVal);
            if(cbor_value_is_valid(&repVal))
            {
                err = err || cbor_value_dup_text_string(&repVal, &specVer, &len, NULL);
            }

            // Data Model Version
            err = err || cbor_value_map_find_value(&curVal, OC_RSRVD_DATA_MODEL_VERSION, &repVal);
            if (cbor_value_is_valid(&repVal))
            {
                err = err || cbor_value_dup_text_string(&repVal, &dmVer, &len, NULL);
            }
        }

         err = err || cbor_value_advance(arrayVal);

        if(err)
        {
            OICFree(uri);
            OICFree(sid);
            OICFree(dname);
            OICFree(specVer);
            OICFree(dmVer);
            OC_LOG_V(ERROR, TAG, "CBOR in error condition %d", err);
            return OC_STACK_MALFORMED_RESPONSE;
        }

        *outPayload = (OCPayload*)OCDevicePayloadCreate(uri, sid, dname, specVer, dmVer);

        OICFree(uri);
        OICFree(sid);
        OICFree(dname);
        OICFree(specVer);
        OICFree(dmVer);
        if(!*outPayload)
        {
            return OC_STACK_NO_MEMORY;
        }

        return OC_STACK_OK;
    }
    else
    {
        OC_LOG(ERROR, TAG, "Root device node was not a map");
        return OC_STACK_MALFORMED_RESPONSE;
    }

}

static OCStackResult OCParsePlatformPayload(OCPayload** outPayload, CborValue* arrayVal)
{
    if (!outPayload)
    {
        return OC_STACK_INVALID_PARAM;
    }

    bool err = false;

    if(cbor_value_is_map(arrayVal))
    {
        char* uri = NULL;
        OCPlatformInfo info = {0};
        CborValue curVal;
         err = err || cbor_value_map_find_value(arrayVal, OC_RSRVD_HREF, &curVal);
        size_t len;
         err = err || cbor_value_dup_text_string(&curVal, &uri, &len, NULL);

        // Representation
        {
             err = err || cbor_value_map_find_value(arrayVal, OC_RSRVD_REPRESENTATION, &curVal);

            CborValue repVal;
            // Platform ID
             err = err || cbor_value_map_find_value(&curVal, OC_RSRVD_PLATFORM_ID, &repVal);
             if(cbor_value_is_valid(&repVal))
             {
                 err = err || cbor_value_dup_text_string(&repVal, &(info.platformID), &len, NULL);
             }

            // MFG Name
             err = err || cbor_value_map_find_value(&curVal, OC_RSRVD_MFG_NAME, &repVal);
             if(cbor_value_is_valid(&repVal))
             {
                 err = err || cbor_value_dup_text_string(&repVal, &(info.manufacturerName), &len, NULL);
             }

            // MFG URL
             err = err || cbor_value_map_find_value(&curVal, OC_RSRVD_MFG_URL, &repVal);
            if(cbor_value_is_valid(&repVal))
            {
                 err = err || cbor_value_dup_text_string(&repVal, &(info.manufacturerUrl), &len, NULL);
            }

            // Model Num
             err = err || cbor_value_map_find_value(&curVal, OC_RSRVD_MODEL_NUM, &repVal);
            if(cbor_value_is_valid(&repVal))
            {
                 err = err || cbor_value_dup_text_string(&repVal, &(info.modelNumber), &len, NULL);
            }

            // Date of Mfg
             err = err || cbor_value_map_find_value(&curVal, OC_RSRVD_MFG_DATE, &repVal);
            if(cbor_value_is_valid(&repVal))
            {
                 err = err || cbor_value_dup_text_string(&repVal, &(info.dateOfManufacture), &len,
                        NULL);
            }

            // Platform Version
             err = err || cbor_value_map_find_value(&curVal, OC_RSRVD_PLATFORM_VERSION, &repVal);
            if(cbor_value_is_valid(&repVal))
            {
                 err = err || cbor_value_dup_text_string(&repVal, &(info.platformVersion), &len,
                        NULL);
            }

            // OS Version
             err = err || cbor_value_map_find_value(&curVal, OC_RSRVD_OS_VERSION, &repVal);
            if(cbor_value_is_valid(&repVal))
            {
                 err = err || cbor_value_dup_text_string(&repVal, &(info.operatingSystemVersion),
                        &len, NULL);
            }

            // Hardware Version
             err = err || cbor_value_map_find_value(&curVal, OC_RSRVD_HARDWARE_VERSION, &repVal);
            if(cbor_value_is_valid(&repVal))
            {
                 err = err || cbor_value_dup_text_string(&repVal, &(info.hardwareVersion), &len,
                        NULL);
            }

            // Firmware Version
             err = err || cbor_value_map_find_value(&curVal, OC_RSRVD_FIRMWARE_VERSION, &repVal);
            if(cbor_value_is_valid(&repVal))
            {
                 err = err || cbor_value_dup_text_string(&repVal, &(info.firmwareVersion), &len,
                        NULL);
            }

            // Support URL
             err = err || cbor_value_map_find_value(&curVal, OC_RSRVD_SUPPORT_URL, &repVal);
            if(cbor_value_is_valid(&repVal))
            {
                 err = err || cbor_value_dup_text_string(&repVal, &(info.supportUrl), &len, NULL);
            }

            // System Time
             err = err || cbor_value_map_find_value(&curVal, OC_RSRVD_SYSTEM_TIME, &repVal);
            if(cbor_value_is_valid(&repVal))
            {
                 err = err || cbor_value_dup_text_string(&repVal, &(info.systemTime), &len, NULL);
            }
        }

         err = err || cbor_value_advance(arrayVal);

        if(err)
        {
            OICFree(info.dateOfManufacture);
            OICFree(info.firmwareVersion);
            OICFree(info.hardwareVersion);
            OICFree(info.manufacturerName);
            OICFree(info.manufacturerUrl);
            OICFree(info.modelNumber);
            OICFree(info.operatingSystemVersion);
            OICFree(info.platformID);
            OICFree(info.platformVersion);
            OICFree(info.supportUrl);
            OICFree(info.systemTime);
            OC_LOG(ERROR, TAG, "CBOR error In ParsePlatformPayload");
            return OC_STACK_MALFORMED_RESPONSE;
        }

        *outPayload = (OCPayload*)OCPlatformPayloadCreateAsOwner(uri, &info);

        if(!*outPayload)
        {
            return OC_STACK_NO_MEMORY;
        }

        return OC_STACK_OK;
    }
    else
    {
        OC_LOG(ERROR, TAG, "Root device node was not a map");
        return OC_STACK_MALFORMED_RESPONSE;
    }
}

static OCRepPayloadPropType DecodeCborType(CborType type)
{
    switch (type)
    {
            case CborNullType:
                return OCREP_PROP_NULL;
            case CborIntegerType:
                return OCREP_PROP_INT;
            case CborDoubleType:
                return OCREP_PROP_DOUBLE;
            case CborBooleanType:
                return OCREP_PROP_BOOL;
            case CborTextStringType:
                return OCREP_PROP_STRING;
            case CborMapType:
                return OCREP_PROP_OBJECT;
            case CborArrayType:
                return OCREP_PROP_ARRAY;
            default:
                return OCREP_PROP_NULL;
    }
}
static bool OCParseArrayFindDimensionsAndType(const CborValue* parent, size_t dimensions[MAX_REP_ARRAY_DEPTH],
        OCRepPayloadPropType* type)
{
    bool err = false;
    CborValue insideArray;
    *type = OCREP_PROP_NULL;
    dimensions[0] = dimensions[1] = dimensions[2] = 0;

    err = err || cbor_value_enter_container(parent, &insideArray);

    while (cbor_value_is_valid(&insideArray))
    {
        OCRepPayloadPropType tempType = DecodeCborType(cbor_value_get_type(&insideArray));

        if (tempType == OCREP_PROP_ARRAY)
        {
            size_t subdim[MAX_REP_ARRAY_DEPTH];
            tempType = OCREP_PROP_NULL;
            err = err || OCParseArrayFindDimensionsAndType(&insideArray, subdim, &tempType);

            if (subdim[2] != 0)
            {
                OC_LOG(ERROR, TAG, "Parse array helper, sub-array too deep");
            }

            dimensions[1] = dimensions[1] >= subdim[0] ? dimensions[1] : subdim[0];
            dimensions[2] = dimensions[2] >= subdim[1] ? dimensions[2] : subdim[1];

            if (*type != OCREP_PROP_NULL && tempType != OCREP_PROP_NULL
                    && *type != tempType)
            {
                OC_LOG(ERROR, TAG, "Array parse failed, mixed arrays not allowed (subtype)");
                return true;
            }
            else if (*type == OCREP_PROP_NULL)
            {
                // We don't know the type of this array yet, so the assignment is OK
                *type = tempType;
            }
        }
        else if (*type == OCREP_PROP_NULL)
        {
            // We don't know the type of this array yet, so the assignment is OK
            *type = tempType;
        }
        // tempType is allowed to be NULL, since it might now know the answer yet
        else if (tempType != OCREP_PROP_NULL && *type != tempType)
        {
            // this is an invalid situation!
            OC_LOG(ERROR, TAG, "Array parse failed, mixed arrays not allowed");
            return true;
        }

        ++dimensions[0];
        cbor_value_advance(&insideArray);
    }

    return err;
}

static size_t getAllocSize(OCRepPayloadPropType type)
{
    switch (type)
    {
        case OCREP_PROP_INT:
            return sizeof (int64_t);
        case OCREP_PROP_DOUBLE:
            return sizeof (double);
        case OCREP_PROP_BOOL:
            return sizeof (bool);
        case OCREP_PROP_STRING:
            return sizeof (char*);
        case OCREP_PROP_OBJECT:
            return sizeof (OCRepPayload*);
        default:
            return 0;
    }
}

static size_t arrayStep(size_t dimensions[MAX_REP_ARRAY_DEPTH], size_t elementNum)
{
    return
        (dimensions[1] == 0 ? 1 : dimensions[1]) *
        (dimensions[2] == 0 ? 1 : dimensions[2]) *
        elementNum;
}

static bool OCParseArrayFillArray(const CborValue* parent, size_t dimensions[MAX_REP_ARRAY_DEPTH],
        OCRepPayloadPropType type, void* targetArray)
{
    bool err = false;
    CborValue insideArray;

    err = err || cbor_value_enter_container(parent, &insideArray);

    size_t i = 0;
    char* tempStr = NULL;
    size_t tempLen = 0;
    OCRepPayload* tempPl = NULL;

    size_t newdim[MAX_REP_ARRAY_DEPTH];
    newdim[0] = dimensions[1];
    newdim[1] = dimensions[2];
    newdim[2] = 0;

    while (!err && i < dimensions[0] && cbor_value_is_valid(&insideArray))
    {
        if (cbor_value_get_type(&insideArray) != CborNullType)
        {
            switch (type)
            {
                case OCREP_PROP_INT:
                    if (dimensions[1] == 0)
                    {
                        err = err || cbor_value_get_int64(&insideArray,
                                &(((int64_t*)targetArray)[i]));
                    }
                    else
                    {
                        err = err || OCParseArrayFillArray(&insideArray, newdim,
                            type,
                            &(((int64_t*)targetArray)[arrayStep(dimensions, i)])
                            );
                    }
                    break;
                case OCREP_PROP_DOUBLE:
                    if (dimensions[1] == 0)
                    {
                        err = err || cbor_value_get_double(&insideArray,
                                &(((double*)targetArray)[i]));
                    }
                    else
                    {
                        err = err || OCParseArrayFillArray(&insideArray, newdim,
                            type,
                            &(((double*)targetArray)[arrayStep(dimensions, i)])
                            );
                    }
                    break;
                case OCREP_PROP_BOOL:
                    if (dimensions[1] == 0)
                    {
                        err = err || cbor_value_get_boolean(&insideArray,
                                &(((bool*)targetArray)[i]));
                    }
                    else
                    {
                        err = err || OCParseArrayFillArray(&insideArray, newdim,
                            type,
                            &(((bool*)targetArray)[arrayStep(dimensions, i)])
                            );
                    }
                    break;
                case OCREP_PROP_STRING:
                    if (dimensions[1] == 0)
                    {
                        err = err || cbor_value_dup_text_string(&insideArray,
                                &tempStr, &tempLen, NULL);
                        ((char**)targetArray)[i] = tempStr;
                        tempStr = NULL;
                    }
                    else
                    {
                        err = err || OCParseArrayFillArray(&insideArray, newdim,
                            type,
                            &(((char**)targetArray)[arrayStep(dimensions, i)])
                            );
                    }
                    break;
                case OCREP_PROP_OBJECT:
                    if (dimensions[1] == 0)
                    {
                        err = err || OCParseSingleRepPayload(&tempPl, &insideArray);
                        ((OCRepPayload**)targetArray)[i] = tempPl;
                        tempPl = NULL;
                    }
                    else
                    {
                        err = err || OCParseArrayFillArray(&insideArray, newdim,
                            type,
                            &(((OCRepPayload**)targetArray)[arrayStep(dimensions, i)])
                            );
                    }
                    break;
                default:
                    OC_LOG(ERROR, TAG, "Invalid Array type in Parse Array");
                    err = true;
                    break;
            }
        }
        ++i;
        err = err || cbor_value_advance(&insideArray);
    }

    return err;
}

static bool OCParseArray(OCRepPayload* out, const char* name, CborValue* container)
{
    OCRepPayloadPropType type;
    size_t dimensions[MAX_REP_ARRAY_DEPTH];
    bool err = OCParseArrayFindDimensionsAndType(container, dimensions, &type);

    if (err)
    {
        OC_LOG(ERROR, TAG, "Array details weren't clear");
        return err;
    }

    if (type == OCREP_PROP_NULL)
    {
        err = err || OCRepPayloadSetNull(out, name);
        err = err || cbor_value_advance(container);
        return err;
    }

    size_t dimTotal = calcDimTotal(dimensions);
    size_t allocSize = getAllocSize(type);
    void* arr = OICCalloc(dimTotal, allocSize);

    if (!arr)
    {
        OC_LOG(ERROR, TAG, "Array Parse allocation failed");
        return true;
    }

    err = err || OCParseArrayFillArray(container, dimensions, type, arr);

    switch (type)
    {
        case OCREP_PROP_INT:
            if (err || !OCRepPayloadSetIntArrayAsOwner(out, name, (int64_t*)arr, dimensions))
            {
                OICFree(arr);
                err = true;
            }
            break;
        case OCREP_PROP_DOUBLE:
            if (err || !OCRepPayloadSetDoubleArrayAsOwner(out, name, (double*)arr, dimensions))
            {
                OICFree(arr);
                err = true;
            }
            break;
        case OCREP_PROP_BOOL:
            if (err || !OCRepPayloadSetBoolArrayAsOwner(out, name, (bool*)arr, dimensions))
            {
                OICFree(arr);
                err = true;
            }
            break;
        case OCREP_PROP_STRING:
            if (err || !OCRepPayloadSetStringArrayAsOwner(out, name, (char**)arr, dimensions))
            {
                for(size_t i = 0; i < dimTotal; ++i)
                {
                    OICFree(((char**)arr)[i]);
                }
                OICFree(arr);
                err = true;
            }
            break;
        case OCREP_PROP_OBJECT:
            if (err || !OCRepPayloadSetPropObjectArrayAsOwner(out, name, (OCRepPayload**)arr, dimensions))
            {
                for(size_t i = 0; i < dimTotal; ++i)
                {
                    OCRepPayloadDestroy(((OCRepPayload**)arr)[i]);
                }
                OICFree(arr);
                err = true;
            }
            break;
        default:
            OC_LOG(ERROR, TAG, "Invalid Array type in Parse Array");
            err = true;
            break;
    }

    return err;
}

static bool OCParseSingleRepPayload(OCRepPayload** outPayload, CborValue* repParent)
{
    if (!outPayload)
    {
        return false;
    }

    *outPayload = OCRepPayloadCreate();
    OCRepPayload* curPayload = *outPayload;
    bool err = false;
    if(!*outPayload)
    {
        return CborErrorOutOfMemory;
    }

    size_t len;
    CborValue curVal;
    err = err || cbor_value_map_find_value(repParent, OC_RSRVD_HREF, &curVal);
    if(cbor_value_is_valid(&curVal))
    {
        err = err || cbor_value_dup_text_string(&curVal, &curPayload->uri, &len,
            NULL);
    }

    err = err || cbor_value_map_find_value(repParent, OC_RSRVD_PROPERTY, &curVal);
    if(cbor_value_is_valid(&curVal))
    {
        CborValue insidePropValue = {0};
        err = err || cbor_value_map_find_value(&curVal, OC_RSRVD_RESOURCE_TYPE,
                &insidePropValue);

        if(cbor_value_is_text_string(&insidePropValue))
        {
            char* allRt = NULL;
            err = err || cbor_value_dup_text_string(&insidePropValue, &allRt, &len, NULL);

            char* savePtr;

            if (allRt)
            {
                char* curPtr = strtok_r(allRt, " ", &savePtr);

                while (curPtr)
                {
                    char* trimmed = InPlaceStringTrim(curPtr);
                    if (trimmed[0] != '\0')
                    {
                        OCRepPayloadAddResourceType(curPayload, curPtr);
                    }
                    curPtr = strtok_r(NULL, " ", &savePtr);
                }
            }
            OICFree(allRt);
        }

        err = err || cbor_value_map_find_value(&curVal, OC_RSRVD_INTERFACE, &insidePropValue);

        if(cbor_value_is_text_string(&insidePropValue))
        {
            char* allIf = NULL;
            err = err || cbor_value_dup_text_string(&insidePropValue, &allIf, &len, NULL);

            char* savePtr;

            if (allIf)
            {
                char* curPtr = strtok_r(allIf, " ", &savePtr);

                while (curPtr)
                {
                    char* trimmed = InPlaceStringTrim(curPtr);
                    if (trimmed[0] != '\0')
                    {
                        OCRepPayloadAddInterface(curPayload, curPtr);
                    }
                    curPtr = strtok_r(NULL, " ", &savePtr);
                }
            }
            OICFree(allIf);
        }
    }

    err = err || cbor_value_map_find_value(repParent, OC_RSRVD_REPRESENTATION, &curVal);
    if(cbor_value_is_map(&curVal))
    {
        CborValue repMap;
        err = err || cbor_value_enter_container(&curVal, &repMap);

        while(!err && cbor_value_is_valid(&repMap))
        {
            char* name = NULL;
            err = err || cbor_value_dup_text_string(&repMap, &name, &len, NULL);

            err = err || cbor_value_advance(&repMap);

            int64_t intval = 0;
            bool boolval = false;
            char* strval = NULL;
            double doubleval = 0;
            OCRepPayload* pl = NULL;

            switch(cbor_value_get_type(&repMap))
            {
                case CborNullType:
                    err = !OCRepPayloadSetNull(curPayload, name);
                    break;
                case CborIntegerType:
                    err = err || cbor_value_get_int64(&repMap, &intval);
                    if (!err)
                    {
                        err = !OCRepPayloadSetPropInt(curPayload, name, intval);
                    }
                    break;
                case CborDoubleType:
                    err = err || cbor_value_get_double(&repMap, &doubleval);
                    if (!err)
                    {
                        err = !OCRepPayloadSetPropDouble(curPayload, name, doubleval);
                    }
                    break;
                case CborBooleanType:
                    err = err || cbor_value_get_boolean(&repMap, &boolval);
                    if (!err)
                    {
                        err = !OCRepPayloadSetPropBool(curPayload, name, boolval);
                    }
                    break;
                case CborTextStringType:
                    err = err || cbor_value_dup_text_string(&repMap, &strval, &len, NULL);
                    if (!err)
                    {
                        err = !OCRepPayloadSetPropStringAsOwner(curPayload, name, strval);
                    }
                    break;
                case CborMapType:
                    err = err || OCParseSingleRepPayload(&pl, &repMap);
                    if (!err)
                    {
                        err = !OCRepPayloadSetPropObjectAsOwner(curPayload, name, pl);
                    }
                    break;
                case CborArrayType:
                    err = err || OCParseArray(curPayload, name, &repMap);
                    break;
                default:
                    OC_LOG_V(ERROR, TAG, "Parsing rep property, unknown type %d", repMap.type);
                    err = true;
            }

             err = err || cbor_value_advance(&repMap);
            OICFree(name);
        }
        err = err || cbor_value_leave_container(&curVal, &repMap);
    }

    if(err)
    {
        OCRepPayloadDestroy(*outPayload);
        *outPayload = NULL;
    }

    return err;
}
static OCStackResult OCParseRepPayload(OCPayload** outPayload, CborValue* arrayVal)
{
    if (!outPayload)
    {
        return OC_STACK_INVALID_PARAM;
    }

    bool err = false;

    OCRepPayload* rootPayload = NULL;
    OCRepPayload* curPayload = NULL;
    OCRepPayload* temp = NULL;
    while(!err && cbor_value_is_map(arrayVal))
    {
         err = err || OCParseSingleRepPayload(&temp, arrayVal);

        if(rootPayload == NULL)
        {
            rootPayload = temp;
            curPayload = temp;
        }
        else
        {
            curPayload->next = temp;
            curPayload = curPayload->next;
        }


         err = err || cbor_value_advance(arrayVal);
        if(err)
        {
            OCRepPayloadDestroy(rootPayload);
            OC_LOG(ERROR, TAG, "CBOR error in ParseRepPayload");
            return OC_STACK_MALFORMED_RESPONSE;
        }
    }

    *outPayload = (OCPayload*)rootPayload;

    return OC_STACK_OK;
}

static OCStackResult OCParsePresencePayload(OCPayload** outPayload, CborValue* arrayVal)
{
    if (!outPayload)
    {
        return OC_STACK_INVALID_PARAM;
    }

    bool err = false;
    if(cbor_value_is_map(arrayVal))
    {
        uint64_t seqNum = 0;
        uint64_t maxAge = 0;
        OCPresenceTrigger trigger = OC_PRESENCE_TRIGGER_CREATE;
        char* tempStr = NULL;
        size_t len = 0;

        CborValue curVal;
        // Sequence Number
        err = err || cbor_value_map_find_value(arrayVal, OC_RSRVD_NONCE, &curVal);
        err = err || cbor_value_get_uint64(&curVal, &seqNum);

        // Max Age
        err = err || cbor_value_map_find_value(arrayVal, OC_RSRVD_TTL, &curVal);
        err = err || cbor_value_get_uint64(&curVal, &maxAge);

        // Trigger
        err = err || cbor_value_map_find_value(arrayVal, OC_RSRVD_TRIGGER, &curVal);
        err = err || cbor_value_dup_text_string(&curVal, &tempStr, &len, NULL);
        trigger = convertTriggerStringToEnum(tempStr);
        OICFree(tempStr);
        tempStr = NULL;

        // Resource type name
         err = err || cbor_value_map_find_value(arrayVal, OC_RSRVD_RESOURCE_TYPE, &curVal);
        if(cbor_value_is_valid(&curVal))
        {
             err = err || cbor_value_dup_text_string(&curVal, &tempStr, &len, NULL);
        }

        err = err || cbor_value_advance(arrayVal);

        if(!err)
        {
            *outPayload = (OCPayload*)OCPresencePayloadCreate(seqNum, maxAge, trigger, tempStr);
        }
        OICFree(tempStr);

        if(err)
        {
            OCPayloadDestroy(*outPayload);
            OC_LOG(ERROR, TAG, "CBOR error Parse Presence Payload");
            return OC_STACK_MALFORMED_RESPONSE;
        }

        if(!*outPayload)
        {
            return OC_STACK_NO_MEMORY;
        }

        return OC_STACK_OK;
    }
    else
    {
        OC_LOG(ERROR, TAG, "Root presence node was not a map");
        return OC_STACK_MALFORMED_RESPONSE;
    }
}
