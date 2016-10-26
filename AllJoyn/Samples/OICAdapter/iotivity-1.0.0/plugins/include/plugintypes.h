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
 * This file contains the definition, types and APIs for resource(s) be
 * implemented.
 */

#ifndef PLUGINTYPES_H_
#define PLUGINTYPES_H_

#include "octypes.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * Types of plugins.
 */
typedef enum
{
    PLUGIN_UNKNOWN = 0,
    PLUGIN_ZIGBEE = 1

} PIPluginType;


/**
 * Handle to a plugin.
 */
typedef struct PIPlugin {} PIPlugin;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* PLUGINTYPES_H_ */
