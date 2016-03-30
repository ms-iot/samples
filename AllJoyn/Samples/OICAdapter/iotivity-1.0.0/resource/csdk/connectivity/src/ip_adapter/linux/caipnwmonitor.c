/******************************************************************
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

#include "caipinterface.h"

#include <sys/types.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "caadapterutils.h"
#include "logger.h"
#include "oic_malloc.h"
#include "oic_string.h"

#define TAG "IP_MONITOR"

CAResult_t CAIPStartNetworkMonitor()
{
    return CA_STATUS_OK;
}

CAResult_t CAIPStopNetworkMonitor()
{
    return CA_STATUS_OK;
}

int CAGetPollingInterval(int interval)
{
    return interval;
}

CAInterface_t *CAFindInterfaceChange()
{
    return NULL;
}

u_arraylist_t *CAIPGetInterfaceInformation(int desiredIndex)
{
    if (desiredIndex < 0)
    {
        OIC_LOG_V(ERROR, TAG, "invalid index : %d", desiredIndex);
        return NULL;
    }

    u_arraylist_t *iflist = u_arraylist_create();
    if (!iflist)
    {
        OIC_LOG_V(ERROR, TAG, "Failed to create iflist: %s", strerror(errno));
        return NULL;
    }

    struct ifaddrs *ifp = NULL;
    if (-1 == getifaddrs(&ifp))
    {
        OIC_LOG_V(ERROR, TAG, "Failed to get ifaddrs: %s", strerror(errno));
        u_arraylist_destroy(iflist);
        return NULL;
    }
    OIC_LOG(DEBUG, TAG, "Got ifaddrs");

    struct ifaddrs *ifa = NULL;
    for (ifa = ifp; ifa; ifa = ifa->ifa_next)
    {
        if (!ifa->ifa_addr)
        {
            continue;
        }
        int family = ifa->ifa_addr->sa_family;
        if ((ifa->ifa_flags & IFF_LOOPBACK) || (AF_INET != family && AF_INET6 != family))
        {
            continue;
        }

        int ifindex = if_nametoindex(ifa->ifa_name);
        if (desiredIndex && (ifindex != desiredIndex))
        {
            continue;
        }

        int length = u_arraylist_length(iflist);
        int already = false;
        for (int i = length-1; i >= 0; i--)
        {
            CAInterface_t *ifitem = (CAInterface_t *)u_arraylist_get(iflist, i);

            if (ifitem
                && (int)ifitem->index == ifindex
                && ifitem->family == (uint16_t)family)
            {
                already = true;
                break;
            }
        }
        if (already)
        {
            continue;
        }

        CAInterface_t *ifitem = (CAInterface_t *)OICCalloc(1, sizeof(CAInterface_t));
        if (!ifitem)
        {
            OIC_LOG(ERROR, TAG, "Malloc failed");
            goto exit;
        }

        OICStrcpy(ifitem->name, INTERFACE_NAME_MAX, ifa->ifa_name);
        ifitem->index = ifindex;
        ifitem->family = family;
        ifitem->ipv4addr = ((struct sockaddr_in *)(ifa->ifa_addr))->sin_addr.s_addr;
        ifitem->flags = ifa->ifa_flags;

        bool result = u_arraylist_add(iflist, ifitem);
        if (!result)
        {
            OIC_LOG(ERROR, TAG, "u_arraylist_add failed.");
            goto exit;
        }

        OIC_LOG_V(DEBUG, TAG, "Added interface: %s (%d)", ifitem->name, family);
    }

    freeifaddrs(ifp);
    return iflist;

exit:
    freeifaddrs(ifp);
    u_arraylist_destroy(iflist);
    return NULL;
}
