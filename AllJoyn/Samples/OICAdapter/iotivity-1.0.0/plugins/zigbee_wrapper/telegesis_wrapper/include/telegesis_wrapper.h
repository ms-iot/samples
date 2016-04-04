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
 * This API only works with:
 *      Telegesis ETRX357
 *      CICIE R310 B110615
 *
 */

#ifndef TELEGESISWRAPPER_H_
#define TELEGESISWRAPPER_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "octypes.h"
#include "twtypes.h"

#include <stdint.h>
#include <time.h>
#include <stdbool.h>

/**
 *
 * Defines a cluster id.
 *
 */
typedef struct
{
    char clusterId[SIZE_CLUSTERID];
} TWClusterId;

/**
 *
 * Defines a list of ZigBee device's clusters.
 *
 */
typedef struct
{
    TWClusterId* clusterIds;
    int count;
} TWClusterList;

/**
 *
 * Defines an endpoint of a ZigBee device.
 *
 */
typedef struct
{
    char endpointId[SIZE_ENDPOINTID];
    TWClusterList* clusterList;
} TWEndpoint;

/**
 *
 * Defines a list of endpoints of a ZigBee device.
 *
 */
typedef struct
{
    TWEndpoint* endpoints;
    int count;
} TWEndpointList;

/**
 *
 * Defines a discovered ZigBee device.
 *
 */
typedef struct
{
    char eui[SIZE_EUI];
    char nodeId[SIZE_NODEID];
    TWEndpoint* endpointOfInterest;
} TWDevice;

/**
 *
 * Defines a ZigBee device list.
 *
 */
typedef struct
{
    TWDevice* deviceList;
    int count;
} TWDeviceList;

/**
 *
 * Defines a ZigBee device update.
 *
 */
typedef struct
{
    char nodeId[SIZE_NODEID];
    char endpoint[SIZE_ENDPOINTID];
    char status[SIZE_ZONESTATUS];
    char extendedStatus[SIZE_ZONESTATUS_EXTENDED];
    char zoneId[SIZE_ZONEID];               //optional
    char delay[SIZE_UPDATE_DELAY_TIME];     //optional - amount of time in quarter of seconds
} TWUpdate;

typedef struct
{
    char zoneId[SIZE_ZONEID];
    char zoneType[SIZE_ZONETYPE];
    char eui[SIZE_EUI];
} TWEnrollee;

typedef void (*TWDeviceFoundCallback)(TWDevice* device);
typedef void (*TWEnrollmentSucceedCallback)(TWEnrollee* enrollee);
typedef void (*TWDeviceStatusUpdateCallback)(TWUpdate* update);
typedef void (*TWInitCompleteCallback)(bool status);

/**
 *
 * Initializes the Telegesis module.
 *
 * @param[in] deviceDevPath The device path at which this Telegesis module exists.
 *
 */
OCStackResult TWInitialize(const char* deviceDevPath);

/**
 *
 * Initiates necessary operations to find ZigBee devices.
 *
 */
OCStackResult TWDiscover();

/**
 *
 * Gets a value at the specified parameters.
 *
 * @param[in] extendedUniqueId The extended unique id of the device.
 * @param[in] nodeId The node id of the device.
 * @param[in] endpointId The endpoint id from which the attribute belongs.
 * @param[in] clusterId The cluster id from which the attribute belongs.
 * @param[in] attributeId The attribute id from which the attribute belongs.
 * @param[out] outValue The value at the specified attribute.
 * @param[out] outValueLength The length of the value.
 *
 */
OCStackResult TWGetAttribute(char* extendedUniqueId, char* nodeId, char* endpointId,
                             char* clusterId, char* attributeId,
                             char** outValue, uint8_t* outValueLength);

/**
 *
 * Sets a value at the specified parameters.
 *
 * @param[in] extendedUniqueId The extended unique id of the device.
 * @param[in] nodeId The node id of the device.
 * @param[in] endpointId The endpoint id from which the attribute belongs.
 * @param[in] clusterId The cluster id from which the attribute belongs.
 * @param[in] attributeId The attribute id from which the attribute belongs.
 * @param[in] attributeType The attribute type of the attribute.
 * @param[in] newValue The value to set at the specified attribute.
 *
 */
OCStackResult TWSetAttribute(char* extendedUniqueId, char* nodeId, char* endpointId,
                             char* clusterId, char* attributeId, char* attributeType,
                             char* newValue);

/**
 *
 * Switches a device to On/Off.
 *
 * @param[in] nodeId The node id of the device.
 * @param[in] endpointId The endpoint id from which the attribute belongs.
 * @param[in] newState Use "0" for OFF, "1" for ON, NULL for toggling.
 *
 */
OCStackResult TWSwitchOnOff(char* nodeId, char* endpointId, char* newState);

/**
 *
 * Move to a level. All parameters are null-terminated.
 *
 * @param[in] nodeId The node id of the device.
 * @param[in] endpointId The endpoint id from which the attribute belongs.
 * @param[in] onOfState A boolean type number represents if the command
 *                      is used with On/Off. If it is set to 0, it means the command is
 *                      implemented as Move to Level command. If it is set to 1, it
 *                      means the command will be implemented Move to Level
 *                      (with On/Off) command.
 *
 * @param[in] level The The meaning of ‘level’ is device dependent
 *                  e.g. for a light it may mean brightness level.
 *
 * @param[in] transTime The time taken to move to the new level
 *
 */
OCStackResult TWMoveToLevel(char* nodeId, char* endpointId,
                            char* onOffState, char* level, char* transTime);

/**
 *
 * Switches door lock state.
 *
 * @param[in] nodeId The node id of the device.
 * @param[in] endpointId The endpoint id from which the attribute belongs.
 * @param[in] newState Use "0" to Unlock, "1" to Lock ON.
 *
 */
OCStackResult TWSwitchDoorLockState(char* nodeId, char* endpointId, char* newState);

/**
 *
 * Move Color Temperature
 *
 * @param[in] nodeId The node id of the device.
 * @param[in] endpointId The endpoint id from which the attribute belongs.
 * @param[in] colorTemperature 16 bit hexadecimal number.
 * @param[in] transTime The time taken to move to the new level.
 *
 */
OCStackResult TWColorMoveToColorTemperature(char* nodeId, char* endpointId,
                                            char* colorTemperature, char* transTime);

/**
 *
 * Sets discovery callback.
 * This callback will be called when TWDiscover() discovers ZigBee device(s).
 *
 */
OCStackResult TWSetDiscoveryCallback(const TWDeviceFoundCallback callback);
/**
 *
 * Sets status update callback.
 * This callback will be called when there is an update on remote ZigBee devices.
 *
 */
OCStackResult TWSetStatusUpdateCallback(TWDeviceStatusUpdateCallback callback);

/**
 *
 * Attempts to listen to status change updates of a remote zone device.
 * A callback TWEnrollSucceedCallback will be invoked when this registration is fulfill.
 * @param[in] nodeId The node id of the remote zone device.
 * @param[in] endpointId The node id of the remote zone device.
 *
 *
 */
OCStackResult TWListenForStatusUpdates(char* nodeId, char* endpointId);

/**
 *
 * Process TWEntry.
 *
 */
OCStackResult TWProcess();

/**
 *
 * Uninitializes the Telegesis module.
 *
 */
OCStackResult TWUninitialize();


#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* TELEGESISWRAPPER_H_ */
