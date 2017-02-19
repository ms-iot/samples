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

#include "pluginlist.h"

#include <stdlib.h>
#include <string.h>

#include "zigbee_wrapper.h"
#include "utlist.h"
#include "oic_malloc.h"
#include "ocstack.h"
#include "logger.h"

#define TAG "pluginlist"

static PIPluginBase * pluginList = NULL;

OCStackResult AddPlugin (PIPluginBase * plugin)
{
    if (!plugin)
    {
        return OC_STACK_INVALID_PARAM;
    }

    LL_APPEND(pluginList, plugin);
    return OC_STACK_OK;
}

OCStackResult DeletePlugin(PIPluginBase * plugin)
{
    OCStackResult result = OC_STACK_ERROR;
    if (!plugin)
    {
        return OC_STACK_INVALID_PARAM;
    }
    DeleteResourceList(plugin);
    LL_DELETE(pluginList, plugin);
    if (plugin->type == PLUGIN_ZIGBEE)
    {
        result = ZigbeeStop((PIPlugin_Zigbee *) plugin);
    }
    return result;
}


OCStackResult DeletePluginList()
{
    OCStackResult result = OC_STACK_OK;
    PIPluginBase * out = NULL;
    PIPluginBase * tmp = NULL;
    LL_FOREACH_SAFE(pluginList, out, tmp)
    {
        result = DeletePlugin(out);
        if (result != OC_STACK_OK)
        {
            break;
        }
    }
    if (result == OC_STACK_OK)
    {
        pluginList = NULL;
    }
    return result;
}

OCStackResult GetResourceFromHandle(PIPluginBase * plugin, PIResource ** piResource,
                                    OCResourceHandle * resourceHandle)
{
    if (!plugin || !resourceHandle || !piResource)
    {
        return OC_STACK_INVALID_PARAM;
    }
    PIResourceBase * out = NULL;
    PIResourceBase * tmp = NULL;
    LL_FOREACH_SAFE(plugin->resourceList, out, tmp)
    {
        if (out->piResource.resourceHandle == resourceHandle)
        {
            *piResource = (PIResource *) out;
            return OC_STACK_OK;
        }
    }
    return OC_STACK_NO_RESOURCE;
}

OCStackResult GetResourceFromURI(PIPluginBase * plugin, PIResource ** piResource,
                                    const char * uri)
{
    if (!plugin || !piResource || !uri)
    {
        return OC_STACK_INVALID_PARAM;
    }
    PIResourceBase * out = NULL;
    PIResourceBase * tmp = NULL;
    size_t checkUriLength = strlen(uri);
    size_t indexUriLength = 0;
    size_t minLength = 0;
    LL_FOREACH_SAFE(plugin->resourceList, out, tmp)
    {
        indexUriLength = strlen(out->piResource.uri);
        minLength = indexUriLength > checkUriLength ? checkUriLength : indexUriLength;
        if ((checkUriLength == indexUriLength) &&
            memcmp(out->piResource.uri, uri, minLength + 1) == 0)
        {
            *piResource = (PIResource *) out;
            return OC_STACK_OK;
        }
    }
    *piResource = NULL;
    return OC_STACK_NO_RESOURCE;
}

OCStackResult AddResourceToPlugin (PIPluginBase * plugin, PIResourceBase * resource)
{
    if (!plugin || !resource)
    {
        return OC_STACK_INVALID_PARAM;
    }

    LL_APPEND(plugin->resourceList, resource);

    return OC_STACK_NO_MEMORY;
}

OCStackResult DeleteResource(PIPluginBase * plugin, PIResourceBase * resource)
{
    if (!plugin || !resource)
    {
        return OC_STACK_INVALID_PARAM;
    }

    //Todo: Free all of resource allocations.
    PIResourceBase * resourceList = ((PIResourceBase *) plugin->resourceList);

    LL_DELETE(resourceList, resource);

    OCStackResult result = OCDeleteResource(resource->piResource.resourceHandle);
    if(result != OC_STACK_OK)
    {
        OC_LOG_V(ERROR, TAG, "Failed to delete resource with error: %d", result);
        return result;
    }

    OICFree (resource->piResource.uri);
    if (plugin->type == PLUGIN_ZIGBEE)
    {
        OICFree (((PIResource_Zigbee *)resource)->eui);
        OICFree (((PIResource_Zigbee *)resource)->nodeId);
        OICFree (((PIResource_Zigbee *)resource)->endpointId);
        OICFree (((PIResource_Zigbee *)resource)->clusterId);
    }
    OICFree (resource);
    return OC_STACK_OK;
}

OCStackResult DeleteResourceList(PIPluginBase * plugin)
{
    if (!plugin)
    {
        return OC_STACK_INVALID_PARAM;
    }

    OCStackResult result = OC_STACK_OK;
    PIResourceBase * out = NULL;
    PIResourceBase * tmp = NULL;

    LL_FOREACH_SAFE(plugin->resourceList, out, tmp)
    {
        result = DeleteResource(plugin, out);
        if (result != OC_STACK_OK)
        {
            break;
        }
    }
    if (result == OC_STACK_OK)
    {
        plugin->resourceList = NULL;
    }
    return result;
}
