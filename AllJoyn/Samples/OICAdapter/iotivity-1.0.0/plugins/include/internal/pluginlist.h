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
 * This file contains the accessors and setters for the PluginList
 */

#ifndef PLUGINLIST_H_
#define PLUGINLIST_H_

#include "plugintranslatortypes.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

OCStackResult AddPlugin(PIPluginBase * plugin);

OCStackResult DeletePlugin(PIPluginBase * plugin);

OCStackResult DeletePluginList();

OCStackResult GetResourceFromHandle(PIPluginBase * plugin, PIResource ** piResource,
                                    OCResourceHandle * resourceHandle);

OCStackResult GetResourceFromURI(PIPluginBase * plugin, PIResource ** piResource,
                                    const char * uri);

OCStackResult AddResourceToPlugin(PIPluginBase * plugin, PIResourceBase * resource);

OCStackResult DeleteResource(PIPluginBase * plugin, PIResourceBase * resource);

OCStackResult DeleteResourceList(PIPluginBase * plugin);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* PLUGINLIST_H_ */
