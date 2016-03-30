/* ****************************************************************
 *
 * Copyright 2014 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/

/**
 * @file
 *
 * This file provides helper functions for EDR adapter.
 */

#ifndef CA_EDR_UTILS_H_
#define CA_EDR_UTILS_H_

#include <stdbool.h>
#include <string.h>

#include "cacommon.h"
#include "oic_malloc.h"

/**
 * Logging tag for module name.
 */
#ifndef EDR_ADAPTER_TAG
#define EDR_ADAPTER_TAG "CA_EDR_ADAPTER"
#endif //EDR_ADAPTER_TAG

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Checks if the specified list of service UUIDs contains OIC service UUID.
 *
 * @param[in]  serviceUUID   Array of service UUIDs.
 * @param[in]  serviceCount  Size of the service UUIDs array.
 * @param[in]  matchService  Service UUID to be checked in the given array of service UUIDs.
 *
 * @return  true if match service UUID found otherwise false.
 *
 */
bool CAEDRIsServiceSupported(const char **serviceUUID, int32_t serviceCount,
                            const char *matchService);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CA_EDR_UTILS_H_ */


