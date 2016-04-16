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
 * This file is to keep design in sync with other platforms.  Right now
 * there is no api for network monitoring in arduino.
 */

#include "caipinterface.h"

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <utility/server_drv.h>
#include <utility/wifi_drv.h>
#include <IPAddress.h>

#include "logger.h"
#include "cacommon.h"
#include "caipadapter.h"
#include "caadapterutils.h"
#include "oic_malloc.h"
#include "oic_string.h"

#define TAG "IPNW"

// Since the CA abstraction expects a value for "family", AF_INET will be
// defined & used (as-is defined in the linux socket headers).
#define AF_INET (2)

CAResult_t CAIPStartNetworkMonitor()
{
    return CA_STATUS_OK;
}

CAResult_t CAIPStopNetworkMonitor()
{
    return CA_STATUS_OK;
}

/// Retrieves the IP address assigned to Arduino WiFi shield
void CAArduinoGetInterfaceAddress(uint32_t *address)
{
    OIC_LOG(DEBUG, TAG, "IN");
    if (WiFi.status() != WL_CONNECTED)
    {
        OIC_LOG(DEBUG, TAG, "No WIFI");
        return;
    }

    VERIFY_NON_NULL_VOID(address, TAG, "Invalid address");

    IPAddress ip = WiFi.localIP();
    *address = (uint32_t) ip;

    OIC_LOG_V(DEBUG, TAG, "Wifi shield address is: %d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    OIC_LOG(DEBUG, TAG, "OUT");
    return;
}

u_arraylist_t *CAIPGetInterfaceInformation(int desiredIndex)
{
    bool result = true;

    u_arraylist_t *iflist = u_arraylist_create();
    if (!iflist)
    {
        OIC_LOG(ERROR, TAG, "Failed to create iflist");
        return NULL;
    }

    CAInterface_t *ifitem = (CAInterface_t *)OICCalloc(1, sizeof(CAInterface_t));
    if (!ifitem)
    {
        OIC_LOG(ERROR, TAG, "Malloc failed");
        goto exit;
    }

    // Since Arduino currently only supports one interface, the next 4 lines are sufficient.
    OICStrcpy(ifitem->name, INTERFACE_NAME_MAX, "WIFI");
    ifitem->index = 1;
    ifitem->family = AF_INET;
    ifitem->flags = 0;
    CAArduinoGetInterfaceAddress(&ifitem->ipv4addr);

    result = u_arraylist_add(iflist, ifitem);
    if (!result)
    {
        OIC_LOG(ERROR, TAG, "u_arraylist_add failed.");
        goto exit;
    }

    OIC_LOG_V(DEBUG, TAG, "Added interface: %s (%d)", ifitem->name, ifitem->family);

    return iflist;

exit:
    u_arraylist_destroy(iflist);
    return NULL;
}

