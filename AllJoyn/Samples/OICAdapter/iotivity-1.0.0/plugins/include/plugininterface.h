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
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

/**
 * @file
 *
 * This file contains APIs for PIPlugin module to be implemented.
 */

#ifndef PLUGININTERFACE_H_
#define PLUGININTERFACE_H_

#include "plugintypes.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 *
 * Makes any required calls to instantiate plugin's radio.
 *
 * @param[in]  comPort The com port which this plugin is located.
 * @param[in]  pluginType The type of plugin to start.
 * @param[out] plugin The plugin handle that will be started.
 *
 */
OCStackResult PIStartPlugin(const char * comPort, PIPluginType pluginType, PIPlugin ** plugin);

/**
 *
 * Makes any required calls to stop plugin.
 *
 * @param[in] plugin The plugin to be stopped.
 *
 */
OCStackResult PIStopPlugin(PIPlugin * plugin);

/**
 *
 * Makes any required calls to stop all plugins.
 *
 */
OCStackResult PIStopAll();

/**
 *
 * Called in main loop of application. Gives cycles for Plugin Interface'
 * internal operation.
 *
 * @param[in] plugin The plugin to get cycles from this function's invocation.
 *
 */
OCStackResult PIProcess(PIPlugin * plugin);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* PLUGININTERFACE_H_ */
