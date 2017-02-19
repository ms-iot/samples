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

#include <WinSock2.h>
#include <Ws2tcpip.h>

#include <sys/types.h>
#include <string.h>
#include <errno.h>

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
    u_arraylist_t *iflist = u_arraylist_create();
    if (!iflist)
    {
        OIC_LOG_V(ERROR, TAG, "Failed to create iflist: %s", strerror(errno));
        return NULL;
    }

    struct hostent *pHost = gethostbyname("");
    int i = 0;
    if(pHost->h_addrtype == AF_INET)
    {
        while(pHost->h_addr_list[i] != 0)
        {
            struct in_addr inaddr = { 0 };
            inaddr.s_addr = *(u_long *)pHost->h_addr_list[i];

            CAInterface_t *ifitem = (CAInterface_t *)OICCalloc(1, sizeof(CAInterface_t));;
            OICStrcpy(ifitem->name, INTERFACE_NAME_MAX, inet_ntoa(inaddr));
            ifitem->index = 0;
            ifitem->family = AF_INET; //we support ipv4 only
            ifitem->ipv4addr = inaddr.s_addr;
            ifitem->flags = IFF_UP;

            bool result = u_arraylist_add(iflist, ifitem);
            if (!result)
            {
                OIC_LOG(ERROR, TAG, "u_arraylist_add failed.");
                goto exit;
            }
            OIC_LOG_V(ERROR, TAG, "Added interface: %s (%d)", ifitem->name, ifitem->family);
            i++;
        }
        if (iflist->length > 0)
        {
            return iflist;
        }
    }

exit:
    u_arraylist_destroy(iflist);
    return NULL;
}
