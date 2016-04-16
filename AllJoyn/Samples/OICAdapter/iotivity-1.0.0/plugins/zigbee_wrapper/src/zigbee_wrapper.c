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

/**
 * @file
 *
 * This file contains defined interface for the Zigbee Radio.
 */

 #ifdef __cplusplus
#include <cfloat>
#else
#include <float.h>
#endif // __cplusplus

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h> // To convert "int64_t" to string.
#include <math.h>
#include <errno.h>

#include "zigbee_wrapper.h"
#include "telegesis_wrapper.h"
#include "pluginlist.h"

#include "ocpayload.h"
#include "oic_malloc.h"
#include "oic_string.h"
#include "logger.h"

#define HexPrepend "0x"

#define TAG "zigbeeWrapper"

// TODO: These should eventually go into an XML/JSON/Mapping thing
#define MAX_ATTRIBUTES                       10
#define ZB_TEMPERATURE_CLUSTER               "0402"
#define ZB_TEMPERATURE_ATTRIBUTE_ID          "0000"
#define ZB_CURRENT_LEVEL_ATTRIBUTE_READONLY  "0000"
#define ZB_ON_LEVEL_ATTRIBUTE                "0011"
#define ZB_LEVEL_CONTROL_CLUSTER             "0008"
#define ZB_IAS_ZONE_CLUSTER                  "0500"
#define ZB_IAS_ZONE_STATUS_ATTRIBUTE_ID      "0002"
#define ZB_INDICATOR_CLUSTER                 "0003"
#define ZB_INDICATOR_ATTRIBUTE_ID            "0000"
#define ZB_ON_OFF_CLUSTER                    "0006"
#define ZB_ON_OFF_ATTRIBUTE_ID               "0000"
#define ZB_IAS_ZONE_TYPE_ATTRIBUTE_ID        "0001"

#define IAS_ZONE_TYPE_MOTION_SENSOR          "000d"
#define IAS_ZONE_TYPE_CONTACT_SENSOR         "0015"
#define IAS_ZONE_TYPE_WATER_SENSOR           "002a"

#define ZB_DATA_TYPE_NULL                    "00"
#define ZB_DATA_TYPE_1_BYTE                  "08"
#define ZB_DATA_TYPE_2_BYTE                  "09"
#define ZB_DATA_TYPE_3_BYTE                  "0a"
#define ZB_DATA_TYPE_4_BYTE                  "0b"
#define ZB_DATA_TYPE_5_BYTE                  "0c"
#define ZB_DATA_TYPE_6_BYTE                  "0d"
#define ZB_DATA_TYPE_7_BYTE                  "0e"
#define ZB_DATA_TYPE_8_BYTE                  "0f"
#define ZB_DATA_TYPE_BOOL                    "10"
#define ZB_DATA_TYPE_SIGNED_INT_16           "29"
#define ZB_DATA_TYPE_UNSIGNED_INT_8          "20"
#define ZB_DATA_TYPE_UNSIGNED_INT_16         "21"

#define MAX_STRLEN_INT (10)
// DBL_MANT_DIG = Max # of digits after decimal after the leading zeros.
// DBL_MIN_EXP = Max # of leading zeros of the mantissa.
// Magic number '3' represents a '-' (negative sign), '0' (a possible zero), and '.' (a period).
//       "-0." from a number like "-0.999999991245", the "-0." adds 3 unaccounted characters.
#define MAX_STRLEN_DOUBLE (3 + DBL_MANT_DIG - DBL_MIN_EXP)
#define MAX_STRLEN_BOOL (1)

#define DEFAULT_TRANS_TIME "0000"
#define DEFAULT_MOVETOLEVEL_MODE "0"

static const char* OIC_TEMPERATURE_SENSOR = "oic.r.temperature";
static const char* OIC_DIMMABLE_LIGHT = "oic.r.light.dimming";
static const char* OIC_CONTACT_SENSOR = "oic.r.sensor.contact";
static const char* OIC_MOTION_SENSOR = "oic.r.sensor.motion";
static const char* OIC_WATER_SENSOR = "oic.r.sensor.water";
static const char* OIC_BINARY_SWITCH = "oic.r.switch.binary";

static const char* OIC_TEMPERATURE_ATTRIBUTE = "temperature";
static const char* OIC_DIMMING_ATTRIBUTE = "dimmingSetting";
static const char* OIC_CONTACT_ATTRIBUTE = "value";
static const char* OIC_ON_OFF_ATTRIBUTE = "value";

PIPlugin_Zigbee ** gPlugin = NULL;

typedef enum
{
    ZB_NULL,   // No Data
    ZB_8_BIT,  // 1 byte
    ZB_16_BIT, // 2 bytes
    ZB_24_BIT, // 3 bytes
    ZB_32_BIT, // 4 bytes
    ZB_40_BIT, // 5 bytes
    ZB_48_BIT, // 6 bytes
    ZB_56_BIT, // 7 bytes
    ZB_64_BIT, // 8 bytes
    ZB_BOOL,   // boolean
    ZB_8_BITMAP,
    ZB_16_BITMAP,
    ZB_24_BITMAP,
    ZB_32_BITMAP,
    ZB_40_BITMAP,
    ZB_48_BITMAP,
    ZB_56_BITMAP,
    ZB_64_BITMAP,
    ZB_16_SINT,
    ZB_8_UINT,
    ZB_16_UINT
} ZigBeeAttributeDataType;

char * getZBDataTypeString(ZigBeeAttributeDataType attrType);
OCEntityHandlerResult ProcessEHRequest(PIPluginBase * plugin, OCEntityHandlerRequest *ehRequest,
        OCRepPayload **payload);

typedef enum
{
    OIC_ATTR_NULL,
    OIC_ATTR_INT,
    OIC_ATTR_DOUBLE,
    OIC_ATTR_BOOL,
    OIC_ATTR_STRING
} OICAttributeType;

typedef struct
{
    char                *oicAttribute;
    char                *zigBeeAttribute;
    OICAttributeType    oicType;
    ZigBeeAttributeDataType zigbeeType;
    union
    {
        int64_t i;
        double d;
        bool b;
        char* str;
    } val;

} OICZigBeeAttributePair;

typedef enum
{
    CIE_RON_OFF         = 1 << 1,
    CIE_MOVE_TO_LEVEL   = 1 << 2

} CIECommandMask;

typedef struct
{
    uint32_t count;
    CIECommandMask CIEMask;
    OICZigBeeAttributePair list[MAX_ATTRIBUTES];
} AttributeList;

const char* ZigBeeClusterIDToOICResourceType(const char * clusterID);

OCStackResult getZigBeeAttributesForOICResource(const char * OICResourceType,
                                                    AttributeList *attributeList);

bool getZigBeeAttributesIfValid(const char * OICResourceType,
                                    AttributeList *attributeList,
                                    OCRepPayload *payload);

const char * getResourceTypeForIASZoneType(TWDevice *device)
{
    if(!device)
    {
        return NULL;
    }
    char *IASZoneType = NULL;
    const char *resourceType = NULL;
    uint8_t length = 0;

    OCStackResult ret = TWGetAttribute(
        NULL,
        device->nodeId,
        device->endpointOfInterest->endpointId,
        ZB_IAS_ZONE_CLUSTER,
        ZB_IAS_ZONE_TYPE_ATTRIBUTE_ID,
        &IASZoneType,
        &length
    );

    if (ret != OC_STACK_OK || !IASZoneType)
    {
        OC_LOG_V (ERROR, TAG, "Error %u getting IAS Zone Type", ret);
        return NULL;
    }

    if (strcmp (IASZoneType, IAS_ZONE_TYPE_CONTACT_SENSOR) == 0)
    {
        resourceType = OIC_CONTACT_SENSOR;
    }
    else if (strcmp (IASZoneType, IAS_ZONE_TYPE_MOTION_SENSOR) == 0)
    {
        resourceType = OIC_MOTION_SENSOR;
    }
    else if (strcmp (IASZoneType, IAS_ZONE_TYPE_WATER_SENSOR) == 0)
    {
        resourceType = OIC_WATER_SENSOR;
    }
    else
    {
        OC_LOG_V (ERROR, TAG, "Unsupported Zone Type %s", IASZoneType);
        resourceType = NULL;
    }

    OICFree(IASZoneType);

    return resourceType;
}

OCStackResult buildURI(char ** output,
                       const char * prefix,
                       const char * nodeId,
                       const char * endpointId,
                       const char * clusterId)
{
    if(!output || !prefix || !nodeId || !endpointId || !clusterId)
    {
        return OC_STACK_INVALID_PARAM;
    }
    const char LEN_SEPARATOR[] = "/";
    size_t lenSeparatorSize = sizeof(LEN_SEPARATOR) - 1;
    size_t newUriSize = strlen(prefix) + lenSeparatorSize +
                        strlen(nodeId) + lenSeparatorSize +
                        strlen(endpointId) + lenSeparatorSize +
                        strlen(clusterId)
                        + 1; // NULL Terminator
    *output = (char *) OICCalloc(1, newUriSize);

    if (!*output)
    {
        OC_LOG (ERROR, TAG, "Out of memory");
        return OC_STACK_NO_MEMORY;
    }

    char * temp = OICStrcpy(*output, newUriSize, prefix);
    if(temp != *output)
    {
        goto exit;
    }
    temp = OICStrcat(*output, newUriSize, LEN_SEPARATOR);
    if(temp != *output)
    {
        goto exit;
    }
    temp = OICStrcat(*output, newUriSize, nodeId);
    if(temp != *output)
    {
        goto exit;
    }
    temp = OICStrcat(*output, newUriSize, LEN_SEPARATOR);
    if(temp != *output)
    {
        goto exit;
    }
    temp = OICStrcat(*output, newUriSize, endpointId);
    if(temp != *output)
    {
        goto exit;
    }
    temp = OICStrcat(*output, newUriSize, LEN_SEPARATOR);
    if(temp != *output)
    {
        goto exit;
    }
    temp = OICStrcat(*output, newUriSize, clusterId);
    if(temp != *output)
    {
        goto exit;
    }

    return OC_STACK_OK;

exit:
    OICFree(*output);
    *output = NULL;
    return OC_STACK_NO_MEMORY;
}

void foundZigbeeCallback(TWDevice *device)
{
    if(!device)
    {
        OC_LOG(ERROR, TAG, "foundZigbeeCallback: Invalid parameter.");
        return;
    }
    int count = device->endpointOfInterest->clusterList->count;
    for(int i=0; i < count; i++)
    {
        PIResource_Zigbee *piResource = (PIResource_Zigbee *) OICMalloc(sizeof(*piResource));
        if (!piResource)
        {
            OC_LOG (ERROR, TAG, "Out of memory");
            return;
        }
        piResource->header.plugin = (PIPluginBase *)gPlugin;

        OCStackResult result = buildURI(&piResource->header.piResource.uri,
                                PI_ZIGBEE_PREFIX,
                                device->nodeId,
                                device->endpointOfInterest->endpointId,
                                device->endpointOfInterest->clusterList->clusterIds[i].clusterId);

        if(result != OC_STACK_OK)
        {
            OICFree(piResource);
            return;
        }

        char * foundClusterID =
            device->endpointOfInterest->clusterList->clusterIds[i].clusterId;

        if (strcmp (foundClusterID, ZB_IAS_ZONE_CLUSTER) == 0)
        {
            piResource->header.piResource.resourceTypeName
                = getResourceTypeForIASZoneType (device);

            OCStackResult ret = TWListenForStatusUpdates (device->nodeId,
                                      device->endpointOfInterest->endpointId);

            if (ret != OC_STACK_OK)
            {
                // Just log it and move on if this fails?
                // or not create this resource at all?
                OC_LOG (ERROR, TAG, "Command to listen for status updates failed");
            }
        }
        else
        {
            piResource->header.piResource.resourceTypeName =
                    (char *) ZigBeeClusterIDToOICResourceType(foundClusterID);
        }

        if(piResource->header.piResource.resourceTypeName == NULL)
        {
            OC_LOG_V (ERROR, TAG, "unsupported clusterId : %s",
                device->endpointOfInterest->clusterList->clusterIds[i].clusterId);
            OICFree(piResource->header.piResource.uri);
            OICFree(piResource);
            continue;
        }
        piResource->header.piResource.resourceInterfaceName =
                            OC_RSRVD_INTERFACE_DEFAULT;

        piResource->header.piResource.callbackParam = NULL;
        piResource->header.piResource.resourceProperties = 0;
        piResource->eui = OICStrdup(device->eui);
        piResource->nodeId = OICStrdup(device->nodeId);
        piResource->endpointId = OICStrdup(device->endpointOfInterest->endpointId);
        piResource->clusterId =
            OICStrdup(device->endpointOfInterest->clusterList->clusterIds[i].clusterId);
        (*gPlugin)->header.NewResourceFoundCB(&(*gPlugin)->header, &piResource->header);
    }
}

void zigbeeZoneStatusUpdate(TWUpdate * update)
{
    if(!update)
    {
        return;
    }

    char * uri = NULL;
    OCStackResult result = buildURI(&uri,
                                    PI_ZIGBEE_PREFIX,
                                    update->nodeId,
                                    update->endpoint,
                                    ZB_IAS_ZONE_CLUSTER);
    if(result != OC_STACK_OK || !uri)
    {
        OC_LOG_V(ERROR, TAG, "Failed to build URI with result: %d", result);
        return;
    }

    (*gPlugin)->header.ObserveNotificationUpdate((PIPluginBase *)*gPlugin, uri);
    OICFree(uri);
}

OCStackResult ZigbeeInit(const char * comPort, PIPlugin_Zigbee ** plugin,
                         PINewResourceFound newResourceCB,
                         PIObserveNotificationUpdate observeNotificationUpdate)
{
    if(!plugin)
    {
        return OC_STACK_INVALID_PARAM;
    }
    *plugin = (PIPlugin_Zigbee *) OICMalloc(sizeof(PIPlugin_Zigbee) + sizeof(PIPluginBase));
    if(!*plugin)
    {
        return OC_STACK_NO_MEMORY;
    }
    ((*plugin)->header).type = PLUGIN_ZIGBEE;
    ((*plugin)->header).comPort = comPort;
    ((*plugin)->header).NewResourceFoundCB = newResourceCB;
    ((*plugin)->header).ObserveNotificationUpdate = observeNotificationUpdate;
    ((*plugin)->header).next = NULL;
    ((*plugin)->header).resourceList = NULL;
    ((*plugin)->header).processEHRequest = ProcessEHRequest;

    gPlugin = plugin;
    OCStackResult result = TWInitialize(comPort);
    if(result != OC_STACK_OK)
    {
        return result;
    }

    return TWSetStatusUpdateCallback(zigbeeZoneStatusUpdate);
}

OCStackResult ZigbeeDiscover(PIPlugin_Zigbee * plugin)
{
    OCStackResult result = OC_STACK_ERROR;
    (void)plugin;
    TWSetDiscoveryCallback(foundZigbeeCallback);
    result = TWDiscover(NULL);
    OC_LOG_V (DEBUG, TAG, "ZigbeeDiscover : Status = %d\n", result);

    return result;
}

OCStackResult ZigbeeStop(PIPlugin_Zigbee * plugin)
{
    free(plugin);
    return TWUninitialize();
}

OCStackResult ZigbeeProcess(PIPlugin_Zigbee * plugin)
{
    (void)plugin;
    return TWProcess();
}

// Function returns an OIC Smart Home resource Type
// from the cluster ID. If the cluster is not supported, null is
// returned.
// NOTE: The returned string is NOT malloc'ed.
const char* ZigBeeClusterIDToOICResourceType(const char * clusterID) //Discovery/CreateResource
{
    if (strcmp(clusterID, ZB_TEMPERATURE_CLUSTER) == 0)
    {
        return OIC_TEMPERATURE_SENSOR;
    }
    else if (strcmp(clusterID, ZB_LEVEL_CONTROL_CLUSTER) == 0)
    {
        return OIC_DIMMABLE_LIGHT;
    }
    else if (strcmp(clusterID, ZB_IAS_ZONE_CLUSTER) == 0)
    {
        return OIC_CONTACT_SENSOR;
    }
    else if (strcmp(clusterID, ZB_ON_OFF_CLUSTER) == 0)
    {
        return OIC_BINARY_SWITCH;
    }
    else
    {
        return NULL;
    }
}

const char* OICResourceToZigBeeClusterID(char *oicResourceType)
{
    if (strcmp(oicResourceType, OIC_TEMPERATURE_SENSOR) == 0)
    {
        return ZB_TEMPERATURE_CLUSTER;
    }
    else if (strcmp(oicResourceType, OIC_DIMMABLE_LIGHT) == 0)
    {
        return ZB_LEVEL_CONTROL_CLUSTER;
    }
    else if (strcmp(oicResourceType, OIC_CONTACT_SENSOR) == 0)
    {
        return ZB_IAS_ZONE_CLUSTER;
    }
    else if (strcmp(oicResourceType, OIC_BINARY_SWITCH) == 0)
    {
        return ZB_ON_OFF_CLUSTER;
    }
    else if (strcmp(oicResourceType, OIC_BINARY_SWITCH) == 0)
    {
        return ZB_INDICATOR_CLUSTER;
    }
    else
    {
        return NULL;
    }
}

OCStackResult getZigBeeAttributesForOICResource(const char * OICResourceType,
                                                    AttributeList *attributeList) // GET
{
    if (strcmp (OICResourceType, OIC_TEMPERATURE_SENSOR) == 0)
    {
        attributeList->count = 1;
        attributeList->list[0].oicAttribute = OICStrdup(OIC_TEMPERATURE_ATTRIBUTE);
        attributeList->list[0].zigBeeAttribute = ZB_TEMPERATURE_ATTRIBUTE_ID;
        attributeList->list[0].oicType = OIC_ATTR_DOUBLE;
        attributeList->list[0].zigbeeType = ZB_16_SINT;
        return OC_STACK_OK;
    }
    else if (strcmp (OICResourceType, OIC_DIMMABLE_LIGHT) == 0)
    {
        attributeList->count = 1;
        attributeList->list[0].oicAttribute = OICStrdup(OIC_DIMMING_ATTRIBUTE);
        attributeList->list[0].zigBeeAttribute = ZB_CURRENT_LEVEL_ATTRIBUTE_READONLY;
        attributeList->list[0].oicType = OIC_ATTR_INT;
        attributeList->list[0].zigbeeType = ZB_8_UINT;
        return OC_STACK_OK;
    }
    else if (strcmp (OICResourceType, OIC_CONTACT_SENSOR) == 0)
    {
        attributeList->count = 1;
        attributeList->list[0].oicAttribute = OICStrdup(OIC_CONTACT_ATTRIBUTE);
        attributeList->list[0].zigBeeAttribute = ZB_IAS_ZONE_STATUS_ATTRIBUTE_ID;
        attributeList->list[0].oicType = OIC_ATTR_BOOL;
        attributeList->list[0].zigbeeType = ZB_BOOL;
        return OC_STACK_OK;
    }
    else if (strcmp (OICResourceType, OIC_BINARY_SWITCH) == 0)
    {
        attributeList->count = 1;
        attributeList->list[0].oicAttribute = OICStrdup(OIC_ON_OFF_ATTRIBUTE);
        attributeList->list[0].zigBeeAttribute = ZB_ON_OFF_ATTRIBUTE_ID;
        attributeList->list[0].oicType = OIC_ATTR_BOOL;
        attributeList->list[0].zigbeeType = ZB_BOOL;
        return OC_STACK_OK;
    }

    return OC_STACK_ERROR;
}

bool getZigBeeAttributesIfValid(const char * OICResourceType,
                                    AttributeList *attributeList,
                                    OCRepPayload *payload) // Put
{
    if(!OICResourceType)
    {
        return false;
    }
    if(strcmp(OICResourceType, OIC_TEMPERATURE_SENSOR) == 0)
    {
        // Cant really PUT on the temp sensor, but the code is still there.
        int64_t temperature = 0;

        // TODO: This if should only look for attributes it supports and ignore the rest
        // or examine every attribute in the payload and complain about unsupported attributes?
        if(OCRepPayloadGetPropInt(payload, OIC_TEMPERATURE_ATTRIBUTE, &temperature))
        {
            attributeList->count = 1;
            attributeList->list[0].oicAttribute = OICStrdup(OIC_TEMPERATURE_ATTRIBUTE);
            attributeList->list[0].zigBeeAttribute = ZB_TEMPERATURE_ATTRIBUTE_ID;
            attributeList->list[0].oicType = OIC_ATTR_DOUBLE;
            attributeList->list[0].val.d = temperature;
            attributeList->list[0].zigbeeType = ZB_16_SINT;
            attributeList->CIEMask = (CIECommandMask) 0;

            return true;
        }
    }
    else if (strcmp (OICResourceType, OIC_DIMMABLE_LIGHT) == 0)
    {
        int64_t onLevel = 0;

        if(OCRepPayloadGetPropInt(payload, OIC_DIMMING_ATTRIBUTE, &onLevel))
        {
            attributeList->count = 1;
            attributeList->list[0].oicAttribute = OICStrdup(OIC_DIMMING_ATTRIBUTE);
            attributeList->list[0].zigBeeAttribute = ZB_ON_LEVEL_ATTRIBUTE;
            attributeList->list[0].oicType = OIC_ATTR_INT;
            attributeList->list[0].val.i = onLevel;
            attributeList->list[0].zigbeeType = ZB_8_UINT;

            // Level control cluster is dealing with level in the PUT payload.
            attributeList->CIEMask = attributeList->CIEMask | CIE_MOVE_TO_LEVEL;
            return true;
        }
    }
    else if (strcmp (OICResourceType, OIC_CONTACT_SENSOR) == 0)
    {
        int64_t value = 0;

        if(OCRepPayloadGetPropInt(payload, OIC_CONTACT_ATTRIBUTE, &value))
        {
            attributeList->count = 1;
            attributeList->list[0].oicAttribute = OICStrdup(OIC_CONTACT_ATTRIBUTE);
            attributeList->list[0].zigBeeAttribute = ZB_IAS_ZONE_STATUS_ATTRIBUTE_ID;
            attributeList->list[0].oicType = OIC_ATTR_BOOL;
            attributeList->list[0].val.i = value;
            attributeList->list[0].zigbeeType = ZB_BOOL;
            attributeList->CIEMask = (CIECommandMask) 0;

            return true;
        }
    }
    else if (strcmp (OICResourceType, OIC_BINARY_SWITCH) == 0)
    {
        bool value = 0;

        if(OCRepPayloadGetPropBool(payload, OIC_ON_OFF_ATTRIBUTE, &value))
        {
            attributeList->count = 1;
            attributeList->list[0].oicAttribute = OICStrdup(OIC_ON_OFF_ATTRIBUTE);
            attributeList->list[0].zigBeeAttribute = ZB_ON_OFF_ATTRIBUTE_ID;
            attributeList->list[0].oicType = OIC_ATTR_BOOL;
            attributeList->list[0].val.b = value;
            attributeList->list[0].zigbeeType = ZB_BOOL;

            attributeList->CIEMask = attributeList->CIEMask | CIE_RON_OFF;
            return true;
        }
    }
    return false;
}

OCEntityHandlerResult getDoubleValueFromString (const char *str, double *outDouble)
{
    size_t hexOutValSize = strlen(HexPrepend) + strlen(str) + 1;
    char * hexOutVal = (char *) OICCalloc(1, hexOutValSize);
    if(!hexOutVal)
    {
        return OC_EH_ERROR;
    }
    OICStrcpy(hexOutVal, hexOutValSize, HexPrepend);
    OICStrcat(hexOutVal, hexOutValSize, str);

    char *endPtr;
    errno = 0;
    double value = strtod(hexOutVal, &endPtr);

    if(errno != 0 || *endPtr != 0 || value == HUGE_VALF || value == HUGE_VALL)
    {
        OICFree(hexOutVal);
        return OC_EH_ERROR;
    }

    OICFree(hexOutVal);
    *outDouble = value;
    return OC_EH_OK;

}

OCEntityHandlerResult processGetRequest (PIPluginBase * plugin,
        OCEntityHandlerRequest *ehRequest, OCRepPayload **payload)
{
    if (!plugin || !ehRequest || !payload)
    {
        return OC_EH_ERROR;
    }
    uint32_t attributeListIndex = 0;
    OCStackResult stackResult = OC_STACK_OK;
    PIResource_Zigbee * piResource = NULL;

    AttributeList attributeList = { 0, (CIECommandMask) 0,
        .list[0] = { NULL, NULL, OIC_ATTR_NULL, ZB_NULL, { .i = 0 } } };
    stackResult = GetResourceFromHandle(plugin, (PIResource**) (&piResource),
                        ehRequest->resource);
    if (stackResult != OC_STACK_OK)
    {
        OC_LOG (ERROR, TAG, "Failed to get resource from handle");
        return OC_EH_ERROR;
    }
    stackResult = getZigBeeAttributesForOICResource (
        piResource->header.piResource.resourceTypeName, &attributeList);
    if(stackResult != OC_STACK_OK)
    {
        OC_LOG_V (ERROR, TAG, "Failed to fetch attributes for %s",
            piResource->header.piResource.resourceTypeName);
        return OC_EH_ERROR;
    }

    *payload = OCRepPayloadCreate();
    if(!payload)
    {
        OC_LOG(ERROR, TAG, PCF("Failed to allocate Payload"));
        return OC_EH_ERROR;
    }
    bool boolRes = OCRepPayloadSetUri(*payload, piResource->header.piResource.uri);
    if (boolRes == false)
    {
        OCRepPayloadDestroy (*payload);
        return OC_EH_ERROR;
    }
    for(uint32_t i = 0; i<attributeList.count; i++)
    {
        char * outVal = NULL;
        uint8_t outValLength = 0;

        stackResult = TWGetAttribute(piResource->eui,
                                     piResource->nodeId,
                                     piResource->endpointId,
                                     piResource->clusterId,
                                     attributeList.list[i].zigBeeAttribute,
                                     &outVal,
                                     &outValLength);

        if (stackResult != OC_STACK_OK || !outVal)
        {
            stackResult = OC_EH_ERROR;
            OCRepPayloadDestroy (*payload);
            goto exit;
        }
        if (attributeList.list[i].oicType == OIC_ATTR_INT)
        {
            char *endPtr = NULL;
            // Third arg is 16 as outVal is a hex Number
            uint64_t value = strtol (outVal, &endPtr, 16);

            if (*endPtr != 0)
            {
                return OC_EH_ERROR;
            }
            if (strcmp(attributeList.list[i].oicAttribute, OIC_DIMMING_ATTRIBUTE) == 0)
            {
                // OIC Dimming operates between 0-100, while Zigbee operates
                // between 0-254 (ie. 0xFE).
                if (value > 0xFE)
                {
                    value = 0xFE;
                }
                if (value <= 0xFE)
                {
                    value = value / 2.54;
                }
            }
            boolRes = OCRepPayloadSetPropInt(*payload,
                                             attributeList.list[i].oicAttribute,
                                             (uint64_t) value);
        }
        else if (attributeList.list[i].oicType == OIC_ATTR_DOUBLE)
        {
            double value = 0;

            if (getDoubleValueFromString (outVal, &value) != OC_EH_OK)
            {
                return OC_EH_ERROR;
            }
            if (strcmp(piResource->clusterId, ZB_TEMPERATURE_CLUSTER) == 0)
            {
                // Divide by 100 as temperature readings have a resolution of
                // 0.01 or one hundreth of a degree celsius.
                value = value/100;
            }
            boolRes = OCRepPayloadSetPropDouble(*payload,
                                                attributeList.list[i].oicAttribute,
                                                value);
        }
        else if (attributeList.list[i].oicType == OIC_ATTR_STRING)
        {
            boolRes = OCRepPayloadSetPropString(*payload,
                                                attributeList.list[i].oicAttribute,
                                                outVal);
        }
        else if (attributeList.list[i].oicType == OIC_ATTR_BOOL)
        {
            char *endPtr = NULL;
            errno = 0;
            // Third arg is 16 as outVal is a hex Number
            uint64_t value = strtol (outVal, &endPtr, 16);

            if (errno != 0 || *endPtr != 0)
            {
                return OC_EH_ERROR;
            }
            // value COULD be a bit mask and the LSB indicates boolean true/false.
            // If not a bit mask, it'll be plain 0 or 1.
            value = value & 1;
            boolRes = OCRepPayloadSetPropBool(*payload,
                                              attributeList.list[i].oicAttribute,
                                              value);
        }

        OICFree (outVal);
    }

    if (boolRes == false)
    {
        stackResult = OC_EH_ERROR;
        goto exit;
    }

exit:
    for(; attributeListIndex < attributeList.count; attributeListIndex++)
    {
        OICFree(attributeList.list[attributeListIndex].oicAttribute);
    }
    return stackResult;
}

OCEntityHandlerResult processPutRequest(PIPluginBase * plugin,
    OCEntityHandlerRequest *ehRequest, OCRepPayload **payload)
{
    if (!plugin || !ehRequest || !payload)
    {
        return OC_EH_ERROR;
    }
    OCStackResult stackResult = OC_STACK_OK;
    PIResource_Zigbee *piResource = NULL;
    AttributeList attributeList = {
        0,
        (CIECommandMask) 0,
        .list[0] = { NULL, NULL, OIC_ATTR_NULL, ZB_NULL, { .i = 0 } }
    };

    stackResult = GetResourceFromHandle(plugin,
                                        ((PIResource **) (&piResource)),
                                        ehRequest->resource);
    if (stackResult != OC_STACK_OK)
    {
        OC_LOG (ERROR, TAG, "Failed to get resource from handle");
        return OC_EH_ERROR;
    }

    bool boolRes = getZigBeeAttributesIfValid (
                        piResource->header.piResource.resourceTypeName,
                        &attributeList, *payload);
    if(boolRes == false)
    {
        OC_LOG_V (ERROR, TAG, "Failed to fetch attributes for %s",
            piResource->header.piResource.resourceTypeName);
        return OC_EH_ERROR;
    }

    uint32_t i = 0;
    for(; i<attributeList.count; i++)
    {
        if (attributeList.list[i].oicType == OIC_ATTR_INT)
        {
            char value[MAX_STRLEN_INT] = {};
            if (attributeList.CIEMask || CIE_MOVE_TO_LEVEL)
            {
                int64_t rangeDiff = 0;
                // OIC Dimming operates between 0-100, while Zigbee
                // operates between 0-254 (ie. 0xFE).
                rangeDiff = attributeList.list[i].val.i * 0xFE/100;
                if (rangeDiff > 0xFE)
                {
                    rangeDiff = 0xFE;
                }
                if (rangeDiff < 0)
                {
                    rangeDiff = 0;
                }
                if (rangeDiff <= 0xFE)
                {
                    snprintf(value, sizeof(value), "%02x", (unsigned int) rangeDiff);
                }
                stackResult = TWMoveToLevel(piResource->nodeId, piResource->endpointId,
                                    DEFAULT_MOVETOLEVEL_MODE, value, DEFAULT_TRANS_TIME);
            }
            else
            {
                snprintf(value, sizeof(value), "%"PRId64, attributeList.list[i].val.i);
                stackResult = TWSetAttribute(piResource->eui,
                    piResource->nodeId, piResource->endpointId,
                    piResource->clusterId, attributeList.list[i].zigBeeAttribute,
                    getZBDataTypeString(attributeList.list[i].zigbeeType), value);
            }
            if (stackResult != OC_STACK_OK)
            {
                return OC_EH_ERROR;
            }
        }
        else if (attributeList.list[i].oicType == OIC_ATTR_DOUBLE)
        {
            char value[MAX_STRLEN_DOUBLE] = {};
            snprintf(value, sizeof(value), "%f", attributeList.list[i].val.d);
            stackResult = TWSetAttribute(piResource->eui,
                piResource->nodeId, piResource->endpointId,
                 piResource->clusterId, attributeList.list[i].zigBeeAttribute,
                 getZBDataTypeString(attributeList.list[i].zigbeeType), value);
        }
        else if (attributeList.list[i].oicType == OIC_ATTR_STRING)
        {
            stackResult = TWSetAttribute(piResource->eui,
                piResource->nodeId, piResource->endpointId,
                piResource->clusterId, attributeList.list[i].zigBeeAttribute,
                getZBDataTypeString(attributeList.list[i].zigbeeType),
                attributeList.list[i].val.str);
            if (stackResult != OC_STACK_OK)
            {
                return OC_EH_ERROR;
            }
        }
        else if (attributeList.list[i].oicType == OIC_ATTR_BOOL)
        {
            char * value = attributeList.list[i].val.b ? "1" : "0";
            if (attributeList.CIEMask || CIE_RON_OFF)
            {
                stackResult = TWSwitchOnOff(piResource->nodeId, piResource->endpointId, value);
            }
            else
            {
                stackResult = TWSetAttribute(piResource->eui,
                    piResource->nodeId, piResource->endpointId,
                    piResource->clusterId, attributeList.list[i].zigBeeAttribute,
                    getZBDataTypeString(attributeList.list[i].zigbeeType),
                    value);
            }
            if (stackResult != OC_STACK_OK)
            {
                return OC_EH_ERROR;
            }
        }
        else
        {
            continue;
        }
    }

    return processGetRequest(plugin, ehRequest, payload);
}

OCEntityHandlerResult ProcessEHRequest(PIPluginBase * plugin,
    OCEntityHandlerRequest *ehRequest, OCRepPayload **payload)
{
    if(!ehRequest || !payload)
    {
        return OC_EH_ERROR;
    }
    if(ehRequest->method == OC_REST_GET)
    {
        return processGetRequest(plugin, ehRequest, payload);
    }
    else if(ehRequest->method == OC_REST_PUT)
    {
        return processPutRequest(plugin, ehRequest, payload);
    }
    else
    {
        return OC_EH_FORBIDDEN;
    }
}

char * getZBDataTypeString(ZigBeeAttributeDataType attrType)
{
    switch (attrType)
    {
        case ZB_NULL:
            return ZB_DATA_TYPE_NULL;
        case ZB_8_BIT:
            return ZB_DATA_TYPE_1_BYTE;
        case ZB_16_BIT:
            return ZB_DATA_TYPE_2_BYTE;
        case ZB_24_BIT:
            return ZB_DATA_TYPE_3_BYTE;
        case ZB_32_BIT:
            return ZB_DATA_TYPE_4_BYTE;
        case ZB_40_BIT:
            return ZB_DATA_TYPE_5_BYTE;
        case ZB_48_BIT:
            return ZB_DATA_TYPE_6_BYTE;
        case ZB_56_BIT:
            return ZB_DATA_TYPE_7_BYTE;
        case ZB_64_BIT:
            return ZB_DATA_TYPE_8_BYTE;
        case ZB_BOOL:
            return ZB_DATA_TYPE_BOOL;
        case ZB_8_BITMAP:
            return ZB_DATA_TYPE_1_BYTE;
        case ZB_16_BITMAP:
            return ZB_DATA_TYPE_2_BYTE;
        case ZB_24_BITMAP:
            return ZB_DATA_TYPE_3_BYTE;
        case ZB_32_BITMAP:
            return ZB_DATA_TYPE_4_BYTE;
        case ZB_40_BITMAP:
            return ZB_DATA_TYPE_5_BYTE;
        case ZB_48_BITMAP:
            return ZB_DATA_TYPE_6_BYTE;
        case ZB_56_BITMAP:
            return ZB_DATA_TYPE_7_BYTE;
        case ZB_64_BITMAP:
            return ZB_DATA_TYPE_8_BYTE;
        case ZB_16_SINT:
            return ZB_DATA_TYPE_SIGNED_INT_16;
        case ZB_8_UINT:
            return ZB_DATA_TYPE_UNSIGNED_INT_8;
        case ZB_16_UINT:
            return ZB_DATA_TYPE_UNSIGNED_INT_16;
        default:
            return ZB_DATA_TYPE_NULL;
    }
}
