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
 * This file contains APIs for Plugin Interface module to be implemented.
 */

#include "plugininterface.h"
#include "plugintranslatortypes.h"
#include "pluginlist.h"
#include "zigbee_wrapper.h"
#include "oic_string.h"
#include "oic_malloc.h"
#include "ocstack.h"
#include "ocpayload.h"
#include "logger.h"

#include <string.h>
#include <stdlib.h>

#define TAG PCF("pluginInterface")

/**
 * Entity handler callback that fills the resPayload of the entityHandlerRequest.
 */
OCEntityHandlerResult PluginInterfaceEntityHandler(OCEntityHandlerFlag flag,
                                                   OCEntityHandlerRequest * entityHandlerRequest,
                                                   void* callbackParam)
{
    if (!entityHandlerRequest)
    {
        OC_LOG (ERROR, TAG, "Invalid request pointer");
        return OC_EH_ERROR;
    }

    OCEntityHandlerResult ehResult = OC_EH_ERROR;
    OCStackResult result = OC_STACK_ERROR;
    PIPluginBase * plugin = (PIPluginBase *) callbackParam;

    OCEntityHandlerResponse * response =
                        (OCEntityHandlerResponse *) OICCalloc(1, sizeof(*response));

    if (!response)
    {
        return OC_EH_ERROR;
    }

    OCRepPayload* payload = (OCRepPayload *) entityHandlerRequest->payload;

    if (flag & OC_REQUEST_FLAG)
    {
        if (plugin->processEHRequest)
        {
            ehResult = plugin->processEHRequest(plugin, entityHandlerRequest, &payload);
        }
    }

    // If the result isn't an error or forbidden, send response
    if (!((ehResult == OC_EH_ERROR) || (ehResult == OC_EH_FORBIDDEN)))
    {
        // Format the response.  Note this requires some info about the request
        response->requestHandle = entityHandlerRequest->requestHandle;
        response->resourceHandle = entityHandlerRequest->resource;
        response->ehResult = ehResult;
        response->payload = (OCPayload*) payload;
        // Indicate that response is NOT in a persistent buffer
        response->persistentBufferFlag = 0;

        result = OCDoResponse(response);
        if (result != OC_STACK_OK)
        {
            OC_LOG_V(ERROR, TAG, "Error sending response %u", result);
            ehResult = OC_EH_ERROR;
        }
    }
    else
    {
        OC_LOG_V(ERROR, TAG, "Error handling request %u", ehResult);
    }

    OCPayloadDestroy(response->payload);
    OICFree(response);
    return ehResult;
}

void piNewResourceCB(PIPluginBase * p_plugin, PIResourceBase * r_newResource)
{
    if (!p_plugin || !r_newResource)
    {
        return;
    }

    r_newResource->piResource.resourceProperties = OC_DISCOVERABLE | OC_OBSERVABLE;
    OCStackResult result = OCCreateResource(&r_newResource->piResource.resourceHandle,
                                            r_newResource->piResource.resourceTypeName,
                                            r_newResource->piResource.resourceInterfaceName,
                                            r_newResource->piResource.uri,
                                            PluginInterfaceEntityHandler,
                                            (void *) p_plugin,
                                            r_newResource->piResource.resourceProperties);
    if (result != OC_STACK_OK)
    {
        OICFree (r_newResource->piResource.uri);
        OICFree (r_newResource);
        return;
    }
    OC_LOG_V(INFO, TAG, "Created resource of type: %s\n",
        r_newResource->piResource.resourceTypeName);

    result = AddResourceToPlugin(p_plugin, r_newResource);
}

void piObserveNotificationUpdate(PIPluginBase * plugin, const char * uri)
{
    if(!plugin || !uri)
    {
        return;
    }
    PIResource * piResource = NULL;

    OCStackResult result = GetResourceFromURI(plugin, &piResource, uri);
    if(result != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "Failed to find a matching URI based on observe notification update.");
        return;
    }

    result = OCNotifyAllObservers(piResource->resourceHandle, OC_LOW_QOS);
    if(result != OC_STACK_OK && result != OC_STACK_NO_OBSERVERS)
    {
        OC_LOG_V(ERROR, TAG, "Failed to notify observers of update. Result: %d", result);
    }
}

OCStackResult PIStartPlugin(const char * comPort, PIPluginType pluginType, PIPlugin ** plugin)
{
    if (!plugin || !comPort || strlen(comPort) == 0)
    {
        return OC_STACK_INVALID_PARAM;
    }
    OCStackResult result = OC_STACK_ERROR;
    if (pluginType == PLUGIN_ZIGBEE)
    {
        result = ZigbeeInit(comPort,
                            (PIPlugin_Zigbee **) plugin,
                            piNewResourceCB,
                            piObserveNotificationUpdate);
        if (result != OC_STACK_OK)
        {
            return result;
        }
        if (!*plugin)
        {
            return OC_STACK_ERROR;
        }
        result = AddPlugin((PIPluginBase *) *plugin);
        if (result == OC_STACK_OK)
        {
            result = ZigbeeDiscover((PIPlugin_Zigbee *) plugin);
        }
    }
    return result;
}

OCStackResult PIStopPlugin(PIPlugin * plugin)
{
    if (!plugin)
    {
        return OC_STACK_INVALID_PARAM;
    }

    return DeletePlugin((PIPluginBase *) plugin);
}

OCStackResult PIStopAll()
{
    return DeletePluginList();
}

OCStackResult PIProcess(PIPlugin * p_plugin)
{
    PIPluginBase * plugin = (PIPluginBase *) p_plugin;
    if (!plugin)
    {
        return OC_STACK_INVALID_PARAM;
    }
    OCStackResult result = OC_STACK_ERROR;
    if (plugin->type == PLUGIN_ZIGBEE)
    {
        result = ZigbeeProcess((PIPlugin_Zigbee *)plugin);
    }
    return result;
}

