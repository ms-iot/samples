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

#ifndef PAYLOAD_LOGGING_H_
#define PAYLOAD_LOGGING_H_

#include "logger.h"
#ifdef __TIZEN__
#include <dlog.h>
#endif

#include "rdpayload.h"

#ifdef __cplusplus
extern "C"
{
#endif

// PL_TAG is made as generic predefined tag because of build problems in arduino for using logging
#define PL_TAG "PayloadLog"

#ifdef TB_LOG
    #define OC_LOG_PAYLOAD(level, payload) OCPayloadLog((level),(payload))
    #define UUID_SIZE (16)
    #define UUID_LENGTH (37)
const char *convertTriggerEnumToString(OCPresenceTrigger trigger);
OCPresenceTrigger convertTriggerStringToEnum(const char * triggerStr);

static inline void OCPayloadLogRep(LogLevel level, OCRepPayload* payload)
{
    OC_LOG(level, (PL_TAG), "Payload Type: Representation");
    OCRepPayload* rep = payload;
    int i = 1;
    while(rep)
    {
        OC_LOG_V(level, PL_TAG, "\tResource #%d", i);
        OC_LOG_V(level, PL_TAG, "\tURI:%s", rep->uri);
        OC_LOG(level, PL_TAG, "\tResource Types:");
        OCStringLL* strll =  rep->types;
        while(strll)
        {
            OC_LOG_V(level, PL_TAG, "\t\t%s", strll->value);
            strll = strll->next;
        }
        OC_LOG(level, PL_TAG, "\tInterfaces:");
        strll =  rep->interfaces;
        while(strll)
        {
            OC_LOG_V(level, PL_TAG, "\t\t%s", strll->value);
            strll = strll->next;
        }

        // TODO Finish Logging: Values
        OCRepPayloadValue* val = rep->values;

        OC_LOG(level, PL_TAG, "\tValues:");

        while(val)
        {
            switch(val->type)
            {
                case OCREP_PROP_NULL:
                    OC_LOG_V(level, PL_TAG, "\t\t%s: NULL", val->name);
                    break;
                case OCREP_PROP_INT:
                    OC_LOG_V(level, PL_TAG, "\t\t%s(int):%lld", val->name, val->i);
                    break;
                case OCREP_PROP_DOUBLE:
                    OC_LOG_V(level, PL_TAG, "\t\t%s(double):%f", val->name, val->d);
                    break;
                case OCREP_PROP_BOOL:
                    OC_LOG_V(level, PL_TAG, "\t\t%s(bool):%s", val->name, val->b ? "true" : "false");
                    break;
                case OCREP_PROP_STRING:
                    OC_LOG_V(level, PL_TAG, "\t\t%s(string):%s", val->name, val->str);
                    break;
                case OCREP_PROP_OBJECT:
                    // Note: Only prints the URI (if available), to print further, you'll
                    // need to dig into the object better!
                    OC_LOG_V(level, PL_TAG, "\t\t%s(OCRep):%s", val->name, val->obj->uri);
                    break;
                case OCREP_PROP_ARRAY:
                    switch(val->arr.type)
                    {
                        case OCREP_PROP_INT:
                            OC_LOG_V(level, PL_TAG, "\t\t%s(int array):%lld x %lld x %lld",
                                    val->name,
                                    val->arr.dimensions[0], val->arr.dimensions[1],
                                    val->arr.dimensions[2]);
                            break;
                        case OCREP_PROP_DOUBLE:
                            OC_LOG_V(level, PL_TAG, "\t\t%s(double array):%lld x %lld x %lld",
                                    val->name,
                                    val->arr.dimensions[0], val->arr.dimensions[1],
                                    val->arr.dimensions[2]);
                            break;
                        case OCREP_PROP_BOOL:
                            OC_LOG_V(level, PL_TAG, "\t\t%s(bool array):%lld x %lld x %lld",
                                    val->name,
                                    val->arr.dimensions[0], val->arr.dimensions[1],
                                    val->arr.dimensions[2]);
                            break;
                        case OCREP_PROP_STRING:
                            OC_LOG_V(level, PL_TAG, "\t\t%s(string array):%lld x %lld x %lld",
                                    val->name,
                                    val->arr.dimensions[0], val->arr.dimensions[1],
                                    val->arr.dimensions[2]);
                            break;
                        case OCREP_PROP_OBJECT:
                            OC_LOG_V(level, PL_TAG, "\t\t%s(OCRep array):%lld x %lld x %lld",
                                    val->name,
                                    val->arr.dimensions[0], val->arr.dimensions[1],
                                    val->arr.dimensions[2]);
                            break;
                        default:
                            OC_LOG_V(ERROR, PL_TAG, "\t\t%s <-- Unknown/unsupported array type!",
                                    val->name);
                            break;
                    }
                    break;
                default:
                    OC_LOG_V(ERROR, PL_TAG, "\t\t%s <-- Unknown type!", val->name);
                    break;
            }
            val = val -> next;
        }

        ++i;
        rep = rep->next;
    }

}

static inline void OCPayloadLogDiscovery(LogLevel level, OCDiscoveryPayload* payload)
{
    OC_LOG(level, PL_TAG, "Payload Type: Discovery");
    int i = 1;

    if(!payload->resources)
    {
        OC_LOG(level, PL_TAG, "\tNO Resources");
        return;
    }

    OCResourcePayload* res = payload->resources;

    while(res)
    {
        OC_LOG_V(level, PL_TAG, "\tResource #%d", i);
        OC_LOG_V(level, PL_TAG, "\tURI:%s", res->uri);
        OC_LOG(level, PL_TAG, "\tSID:");
        OC_LOG_BUFFER(level, PL_TAG, res->sid, UUID_SIZE);
        OC_LOG(level, PL_TAG, "\tResource Types:");
        OCStringLL* strll =  res->types;
        while(strll)
        {
            OC_LOG_V(level, PL_TAG, "\t\t%s", strll->value);
            strll = strll->next;
        }
        OC_LOG(level, PL_TAG, "\tInterfaces:");
        strll =  res->interfaces;
        while(strll)
        {
            OC_LOG_V(level, PL_TAG, "\t\t%s", strll->value);
            strll = strll->next;
        }

        OC_LOG_V(level, PL_TAG, "\tBitmap: %u", res->bitmap);
        OC_LOG_V(level, PL_TAG, "\tSecure?: %s", res->secure ? "true" : "false");
        OC_LOG_V(level, PL_TAG, "\tPort: %u", res->port);
        OC_LOG(level, PL_TAG, "");
        res = res->next;
        ++i;
    }
}

static inline void OCPayloadLogDevice(LogLevel level, OCDevicePayload* payload)
{
    OC_LOG(level, PL_TAG, "Payload Type: Device");
    OC_LOG_V(level, PL_TAG, "\tURI:%s", payload->uri);
    OC_LOG(level, PL_TAG, "\tSID:");
    OC_LOG_BUFFER(level, PL_TAG, payload->sid, UUID_SIZE);
    OC_LOG_V(level, PL_TAG, "\tDevice Name:%s", payload->deviceName);
    OC_LOG_V(level, PL_TAG, "\tSpec Version%s", payload->specVersion);
    OC_LOG_V(level, PL_TAG, "\tData Model Version:%s", payload->dataModelVersion);
}

static inline void OCPayloadLogPlatform(LogLevel level, OCPlatformPayload* payload)
{
    OC_LOG(level, PL_TAG, "Payload Type: Platform");
    OC_LOG_V(level, PL_TAG, "\tURI:%s", payload->uri);
    OC_LOG_V(level, PL_TAG, "\tPlatform ID:%s", payload->info.platformID);
    OC_LOG_V(level, PL_TAG, "\tMfg Name:%s", payload->info.manufacturerName);
    OC_LOG_V(level, PL_TAG, "\tMfg URL:%s", payload->info.manufacturerUrl);
    OC_LOG_V(level, PL_TAG, "\tModel Number:%s", payload->info.modelNumber);
    OC_LOG_V(level, PL_TAG, "\tDate of Mfg:%s", payload->info.dateOfManufacture);
    OC_LOG_V(level, PL_TAG, "\tPlatform Version:%s", payload->info.platformVersion);
    OC_LOG_V(level, PL_TAG, "\tOS Version:%s", payload->info.operatingSystemVersion);
    OC_LOG_V(level, PL_TAG, "\tHardware Version:%s", payload->info.hardwareVersion);
    OC_LOG_V(level, PL_TAG, "\tFirmware Version:%s", payload->info.firmwareVersion);
    OC_LOG_V(level, PL_TAG, "\tSupport URL:%s", payload->info.supportUrl);
    OC_LOG_V(level, PL_TAG, "\tSystem Time:%s", payload->info.systemTime);
}

static inline void OCPayloadLogPresence(LogLevel level, OCPresencePayload* payload)
{
    OC_LOG(level, PL_TAG, "Payload Type: Presence");
    OC_LOG_V(level, PL_TAG, "\tSequence Number:%u", payload->sequenceNumber);
    OC_LOG_V(level, PL_TAG, "\tMax Age:%d", payload->maxAge);
    OC_LOG_V(level, PL_TAG, "\tTrigger:%s", convertTriggerEnumToString(payload->trigger));
    OC_LOG_V(level, PL_TAG, "\tResource Type:%s", payload->resourceType);
}

static inline void OCPayloadLogSecurity(LogLevel level, OCSecurityPayload* payload)
{
    OC_LOG(level, PL_TAG, "Payload Type: Security");
    OC_LOG_V(level, PL_TAG, "\tSecurity Data: %s", payload->securityData);
}

static inline void OCRDPayloadLog(const LogLevel level, const OCRDPayload *payload)
{
    if (!payload)
    {
        return;
    }

    if (payload->rdDiscovery)
    {
        OC_LOG(level, PL_TAG, "RD Discovery");
        OC_LOG_V(level, PL_TAG, "  Device Name : %s", payload->rdDiscovery->n.deviceName);
        OC_LOG_V(level, PL_TAG, "  Device Identity : %s", payload->rdDiscovery->di.id);
        OC_LOG_V(level, PL_TAG, "  Bias: %d", payload->rdDiscovery->sel);
    }
    if (payload->rdPublish)
    {
        OC_LOG(level, PL_TAG, "RD Publish");
        OCResourceCollectionPayload *rdPublish = payload->rdPublish;
        OCTagsLog(level, rdPublish->tags);
        OCLinksLog(level, rdPublish->setLinks);
    }
}

static inline void OCPayloadLog(LogLevel level, OCPayload* payload)
{
    if(!payload)
    {
        OC_LOG(level, PL_TAG, "NULL Payload");
        return;
    }
    switch(payload->type)
    {
        case PAYLOAD_TYPE_REPRESENTATION:
            OCPayloadLogRep(level, (OCRepPayload*)payload);
            break;
        case PAYLOAD_TYPE_DISCOVERY:
            OCPayloadLogDiscovery(level, (OCDiscoveryPayload*)payload);
            break;
        case PAYLOAD_TYPE_DEVICE:
            OCPayloadLogDevice(level, (OCDevicePayload*)payload);
            break;
        case PAYLOAD_TYPE_PLATFORM:
            OCPayloadLogPlatform(level, (OCPlatformPayload*)payload);
            break;
        case PAYLOAD_TYPE_PRESENCE:
            OCPayloadLogPresence(level, (OCPresencePayload*)payload);
            break;
        case PAYLOAD_TYPE_SECURITY:
            OCPayloadLogSecurity(level, (OCSecurityPayload*)payload);
            break;
        case PAYLOAD_TYPE_RD:
            OCRDPayloadLog(level, (OCRDPayload*)payload);
            break;
        default:
            OC_LOG_V(level, PL_TAG, "Unknown Payload Type: %d", payload->type);
            break;
    }
}
#else
    #define OC_LOG_PAYLOAD(level, payload)
#endif

#ifdef __cplusplus
}
#endif

#endif
