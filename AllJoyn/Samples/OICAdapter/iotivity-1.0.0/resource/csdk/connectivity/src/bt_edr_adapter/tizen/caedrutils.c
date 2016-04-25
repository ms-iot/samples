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

#include "caedrutils.h"

#include <bluetooth.h>

#include "logger.h"

bool CAEDRIsServiceSupported(const char **serviceUUID, int32_t serviceCount,
                                 const char *matchService)
{
    OIC_LOG(DEBUG, EDR_ADAPTER_TAG, "IN");

    if (NULL == serviceUUID || 0 == serviceCount || NULL == matchService)
    {
        OIC_LOG(DEBUG, EDR_ADAPTER_TAG, "Invalid input");
        return false;
    }

    for (int i = 0; i < serviceCount; i++)
    {
        if (!strcasecmp(serviceUUID[i], matchService))
        {
            OIC_LOG(DEBUG, EDR_ADAPTER_TAG, "Service found !");
            return true;
        }
    }

    OIC_LOG(DEBUG, EDR_ADAPTER_TAG, "OUT");
    return false;
}



