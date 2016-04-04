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

#include "plugintranslatortypes.h"

/**
 * @file
 *
 * This file contains the interface for the ZigBee Radio.
 */

/**
 * Initializes the specified ZigBee radio at location comPort.
 *
 * @param[in] comPort The location the ZigBee radio is located at.
 *
 * @param[out] plugin A pointer to the plugin that has been started.
 *
 * @param[in] newResourceCB A function pointer to the callback that will be
 *                          invoked when a ZigBee cluster is found that matches
 *                          a valid OIC resource.
 *
 * @param[in] observeNotificationUpdate A function pointer to the callback that will be
 *                                      invoked when a Zigbee Zone IAS update occurs that
 *                                      should be mapped to IoTivity's observe functionality.
 */
OCStackResult ZigbeeInit(const char * comPort, PIPlugin_Zigbee ** plugin,
                         PINewResourceFound newResourceCB,
                         PIObserveNotificationUpdate observeNotificationUpdate);

/**
 * Initiates the discovery operation associated with this ZigBee radio.
 *
 * @param[in] plugin A pointer to the current ZigBee radio context.
 */
OCStackResult ZigbeeDiscover(PIPlugin_Zigbee * plugin);

/**
 * De-Initializes the specified ZigBee radio.
 *
 * @param[in] plugin A pointer to the current ZigBee radio context.
 */
OCStackResult ZigbeeStop(PIPlugin_Zigbee * plugin);

/**
 * Called from upper layer. Gives cycles for internal operation.
 *
 * @param[in] plugin A pointer to the current ZigBee radio context.
 */
OCStackResult ZigbeeProcess(PIPlugin_Zigbee * plugin);
