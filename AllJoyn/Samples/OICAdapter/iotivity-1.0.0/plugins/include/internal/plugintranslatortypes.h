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
//******************************************************************


/**
 * @file
 *
 * This file contains the definition, types and APIs for all operations
 * required to translate plugin's respective devices to an OCResource.
 */

#ifndef PLUGINTRANSLATORTYPES_H_
#define PLUGINTRANSLATORTYPES_H_

#include "plugintypes.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define PI_ZIGBEE_PREFIX "/zb"

// Forward definitions to support inter-linking between structs in this file
// and the following callback.
// Note: If there are issues with either of the following two structs, please
//       manually check these structs and their uses for valid operation.
struct PIPluginBase;
struct PIResourceBase;

/**
 *
 * This callback will be called when a new resource is created by radio wrapper.
 *
 */
typedef void (* PINewResourceFound)(struct PIPluginBase * plugin,
                                    struct PIResourceBase *newResource);

/**
 *
 * This callback will be called when a resource' representation has changed.
 *
 */
typedef void (* PIObserveNotificationUpdate)(struct PIPluginBase * plugin,
                                            const char * uri);

/**
 *
 * This function type is used by the radio's mapping implementation against IoTivity.
 * The mapping implementation must implement this function to handle GET & PUT requests.
 *
 */
typedef OCEntityHandlerResult (* PIProcessRequest) (struct PIPluginBase * plugin,
                                                    OCEntityHandlerRequest *ehRequest,
                                                    OCRepPayload **payload);

/**
 * Parameter list for a plugin.
 */
typedef struct PIPluginBase
{
    /** The type of plugin this represents. */
    PIPluginType type;

    /** The file location which represents the interface of the plugin.  */
    const char * comPort;

    /** Linked list of plugins. */
    struct PIPluginBase * next;

    /** Callback to be used when a new resource has been found. */
    PINewResourceFound NewResourceFoundCB;

    /** Callback to be used when an Observation update has occurred. */
    PIObserveNotificationUpdate ObserveNotificationUpdate;

    /** Function Pointer to be invoked upon an incoming IoTivity request. */
    PIProcessRequest processEHRequest;

    /** All resources which exist within the context of this plugin. */
    struct PIResourceBase * resourceList;

    // Any other common internal properties between plugins can be placed here.
} PIPluginBase;

/**
 * The inherite plugin type to be associated with the ZigBee radio and its
 * implementation.
 */
// Note: Although ZigBee has no new members for it's Plugin Type, other radio
// implementations should follow this paradigm where each radio type has
// inherited from the PIPluginBase type.
typedef struct
{
    PIPluginBase header;
} PIPlugin_Zigbee;

/**
 * Parameter list for a new OCResource. This will be handed up in the
 * PINewResource callback.
 */
typedef struct
{
    OCResourceHandle resourceHandle;
    const char *resourceTypeName;
    const char *resourceInterfaceName;
    char *uri;
    OCEntityHandler entityHandler;
    void* callbackParam;
    uint8_t resourceProperties;
} PIResource;

/**
 *  Header for all PIResources.
 */
typedef struct PIResourceBase
{
    PIResource piResource;
    struct PIResourceBase * next; // Linked list of resources.
    PIPluginBase * plugin; // Context this resource exists.
} PIResourceBase;

typedef struct
{
// Todo: This needs to map nicely to a struct that's defined in Zigbee_wrapper
    uint8_t placeholder;
// Todo: This struct will be refactored once Zigbee_Wrapper is finished.
} PIZigbeeProfile;

/**
 * Parameter list for a resource. Abstraction of PIResource.
 */
typedef struct
{
    PIResourceBase header;
    char * eui;
    char * nodeId;
    char * endpointId;
    char * clusterId;
} PIResource_Zigbee;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* PLUGINTRANSLATORTYPES_H_ */
