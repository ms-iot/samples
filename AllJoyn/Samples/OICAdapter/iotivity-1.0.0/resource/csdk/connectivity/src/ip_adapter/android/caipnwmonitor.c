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
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <linux/if.h>

#include "caadapterutils.h"
#include "logger.h"
#include "oic_malloc.h"
#include "oic_string.h"
#include "org_iotivity_ca_CaIpInterface.h"

#define TAG "IP_MONITOR"

static CAInterface_t *CANewInterfaceItem(int index, const char *name, int family,
                                         uint32_t addr, int flags);

static CAResult_t CAAddInterfaceItem(u_arraylist_t *iflist, int index,
                            const char *name, int family, uint32_t addr, int flags);

CAResult_t CAIPJniInit();

#define MAX_INTERFACE_INFO_LENGTH 1024 // allows 32 interfaces from SIOCGIFCONF

CAResult_t CAIPStartNetworkMonitor()
{
    return CAIPJniInit();
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
    char buf[MAX_INTERFACE_INFO_LENGTH] = { 0 };
    struct ifconf ifc  = { .ifc_len = MAX_INTERFACE_INFO_LENGTH, .ifc_buf = buf };

    int s = caglobals.ip.u6.fd != -1 ? caglobals.ip.u6.fd : caglobals.ip.u4.fd;
    if (ioctl(s, SIOCGIFCONF, &ifc) < 0)
    {
        OIC_LOG_V(ERROR, TAG, "SIOCGIFCONF failed: %s", strerror(errno));
        return NULL;
    }

    CAInterface_t *foundNewInterface = NULL;

    struct ifreq* ifr = ifc.ifc_req;
    size_t interfaces = ifc.ifc_len / sizeof (ifc.ifc_req[0]);
    size_t ifreqsize = ifc.ifc_len;

    CAIfItem_t *previous = (CAIfItem_t *)OICMalloc(ifreqsize);
    if (!previous)
    {
        OIC_LOG(ERROR, TAG, "OICMalloc failed");
        return NULL;
    }

    memcpy(previous, caglobals.ip.nm.ifItems, ifreqsize);
    size_t numprevious = caglobals.ip.nm.numIfItems;

    if (ifreqsize > caglobals.ip.nm.sizeIfItems)
    {

        CAIfItem_t *items = (CAIfItem_t *)OICRealloc(caglobals.ip.nm.ifItems, ifreqsize);
        if (!items)
        {
            OIC_LOG(ERROR, TAG, "OICRealloc failed");
            OICFree(previous);
            return NULL;
        }
        caglobals.ip.nm.ifItems = items;
        caglobals.ip.nm.sizeIfItems = ifreqsize;
    }

    caglobals.ip.nm.numIfItems = 0;
    for (size_t i = 0; i < interfaces; i++)
    {
        struct ifreq* item = &ifr[i];
        char *name = item->ifr_name;
        struct sockaddr_in *sa4 = (struct sockaddr_in *)&item->ifr_addr;
        uint32_t ipv4addr = sa4->sin_addr.s_addr;

        if (ioctl(s, SIOCGIFFLAGS, item) < 0)
        {
            OIC_LOG_V(ERROR, TAG, "SIOCGIFFLAGS failed: %s", strerror(errno));
            continue;
        }
        int16_t flags = item->ifr_flags;
        if ((flags & IFF_LOOPBACK) || !(flags & IFF_RUNNING))
        {
            continue;
        }
        if (ioctl(s, SIOCGIFINDEX, item) < 0)
        {
            OIC_LOG_V(ERROR, TAG, "SIOCGIFINDEX failed: %s", strerror(errno));
            continue;
        }

        int ifIndex = item->ifr_ifindex;
        caglobals.ip.nm.ifItems[i].ifIndex = ifIndex;  // refill interface list
        caglobals.ip.nm.numIfItems++;

        if (foundNewInterface)
        {
            continue;   // continue updating interface list
        }

        // see if this interface didn't previously exist
        bool found = false;
        for (size_t j = 0; j < numprevious; j++)
        {
            if (ifIndex == previous[j].ifIndex)
            {
                found = true;
                break;
            }
        }
        if (found)
        {
            OIC_LOG_V(INFO, TAG, "Interface found: %s", name);
            continue;
        }

        foundNewInterface = CANewInterfaceItem(ifIndex, name, AF_INET, ipv4addr, flags);
    }

    OICFree(previous);
    return foundNewInterface;
}

u_arraylist_t *CAIPGetInterfaceInformation(int desiredIndex)
{
    u_arraylist_t *iflist = u_arraylist_create();
    if (!iflist)
    {
        OIC_LOG_V(ERROR, TAG, "Failed to create iflist: %s", strerror(errno));
        return NULL;
    }

    char buf[MAX_INTERFACE_INFO_LENGTH] = { 0 };
    struct ifconf ifc = { .ifc_len = MAX_INTERFACE_INFO_LENGTH, .ifc_buf = buf };

    int s = caglobals.ip.u6.fd != -1 ? caglobals.ip.u6.fd : caglobals.ip.u4.fd;
    if (ioctl(s, SIOCGIFCONF, &ifc) < 0)
    {
        OIC_LOG_V(ERROR, TAG, "SIOCGIFCONF failed: %s", strerror(errno));
        u_arraylist_destroy(iflist);
        return NULL;
    }

    struct ifreq* ifr = ifc.ifc_req;
    size_t interfaces = ifc.ifc_len / sizeof (ifc.ifc_req[0]);
    size_t ifreqsize = ifc.ifc_len;

    if (ifreqsize > caglobals.ip.nm.sizeIfItems)
    {
        CAIfItem_t *items = (CAIfItem_t *)OICRealloc(caglobals.ip.nm.ifItems, ifreqsize);
        if (!items)
        {
            OIC_LOG(ERROR, TAG, "OICRealloc failed");
            goto exit;
        }
        caglobals.ip.nm.ifItems = items;
        caglobals.ip.nm.sizeIfItems = ifreqsize;
    }

    caglobals.ip.nm.numIfItems = 0;
    for (size_t i = 0; i < interfaces; i++)
    {
        CAResult_t result = CA_STATUS_OK;
        struct ifreq* item = &ifr[i];
        char *name = item->ifr_name;
        struct sockaddr_in *sa4 = (struct sockaddr_in *)&item->ifr_addr;
        uint32_t ipv4addr = sa4->sin_addr.s_addr;

        if (ioctl(s, SIOCGIFFLAGS, item) < 0)
        {
            OIC_LOG_V(ERROR, TAG, "SIOCGIFFLAGS failed: %s", strerror(errno));
            continue;
        }
        int16_t flags = item->ifr_flags;
        if ((flags & IFF_LOOPBACK) || !(flags & IFF_RUNNING))
        {
            continue;
        }
        if (ioctl(s, SIOCGIFINDEX, item) < 0)
        {
            OIC_LOG_V(ERROR, TAG, "SIOCGIFINDEX failed: %s", strerror(errno));
            continue;
        }

        int ifindex = item->ifr_ifindex;
        caglobals.ip.nm.ifItems[i].ifIndex = ifindex;
        caglobals.ip.nm.numIfItems++;

        if (desiredIndex && (ifindex != desiredIndex))
        {
            continue;
        }

        // Add IPv4 interface
        result = CAAddInterfaceItem(iflist, ifindex, name, AF_INET, ipv4addr, flags);
        if (CA_STATUS_OK != result)
        {
            goto exit;
        }

        // Add IPv6 interface
        result = CAAddInterfaceItem(iflist, ifindex, name, AF_INET6, ipv4addr, flags);
        if (CA_STATUS_OK != result)
        {
            goto exit;
        }
    }
    return iflist;

exit:
    u_arraylist_destroy(iflist);
    return NULL;
}

static CAResult_t CAAddInterfaceItem(u_arraylist_t *iflist, int index,
                            const char *name, int family, uint32_t addr, int flags)
{
    CAInterface_t *ifitem = CANewInterfaceItem(index, name, family, addr, flags);
    if (!ifitem)
    {
        return CA_STATUS_FAILED;
    }
    bool result = u_arraylist_add(iflist, ifitem);
    if (!result)
    {
        OIC_LOG(ERROR, TAG, "u_arraylist_add failed.");
        OICFree(ifitem);
        return CA_STATUS_FAILED;
    }

    return CA_STATUS_OK;
}

static CAInterface_t *CANewInterfaceItem(int index, const char *name, int family,
                                         uint32_t addr, int flags)
{
    CAInterface_t *ifitem = (CAInterface_t *)OICCalloc(1, sizeof (CAInterface_t));
    if (!ifitem)
    {
        OIC_LOG(ERROR, TAG, "Malloc failed");
        return NULL;
    }

    OICStrcpy(ifitem->name, sizeof (ifitem->name), name);
    ifitem->index = index;
    ifitem->family = family;
    ifitem->ipv4addr = addr;
    ifitem->flags = flags;

    return ifitem;
}

CAResult_t CAIPJniInit()
{
    OIC_LOG(DEBUG, TAG, "CAIPJniInit_IN");

    JavaVM *jvm = CANativeJNIGetJavaVM();
    if (!jvm)
    {
        OIC_LOG(ERROR, TAG, "Could not get JavaVM pointer");
        return CA_STATUS_FAILED;
    }

    jobject context = CANativeJNIGetContext();
    if (!context)
    {
        OIC_LOG(ERROR, TAG, "unable to get application context");
        return CA_STATUS_FAILED;
    }

    JNIEnv* env;
    if ((*jvm)->GetEnv(jvm, (void**) &env, JNI_VERSION_1_6) != JNI_OK)
    {
        OIC_LOG(ERROR, TAG, "Could not get JNIEnv pointer");
        return CA_STATUS_FAILED;
    }

    jclass cls_Context = (*env)->FindClass(env, "android/content/Context");
    if (!cls_Context)
    {
        OIC_LOG(ERROR, TAG, "Could not get context object class");
        return CA_STATUS_FAILED;
    }

    jmethodID mid_getApplicationContext = (*env)->GetMethodID(env, cls_Context,
                                                                "getApplicationContext",
                                                                "()Landroid/content/Context;");
    if (!mid_getApplicationContext)
    {
        OIC_LOG(ERROR, TAG, "Could not get getApplicationContext method");
        return CA_STATUS_FAILED;
    }

    jobject jApplicationContext = (*env)->CallObjectMethod(env, context,
                                                           mid_getApplicationContext);
    if (!jApplicationContext)
    {
        OIC_LOG(ERROR, TAG, "Could not get application context");
        return CA_STATUS_FAILED;
    }

    jclass cls_CaIpInterface = (*env)->FindClass(env, "org/iotivity/ca/CaIpInterface");
    if (!cls_CaIpInterface)
    {
        OIC_LOG(ERROR, TAG, "Could not get CaIpInterface class");
        return CA_STATUS_FAILED;
    }

    jmethodID mid_CaIpInterface_ctor = (*env)->GetMethodID(env, cls_CaIpInterface, "<init>",
                                                                   "(Landroid/content/Context;)V");
    if (!mid_CaIpInterface_ctor)
    {
        OIC_LOG(ERROR, TAG, "Could not get CaIpInterface constructor method");
        return CA_STATUS_FAILED;
    }

    (*env)->NewObject(env, cls_CaIpInterface, mid_CaIpInterface_ctor, jApplicationContext);
    OIC_LOG(DEBUG, TAG, "Create CaIpInterface instance, success");

    OIC_LOG(DEBUG, TAG, "CAIPJniInit_OUT");
    return CA_STATUS_OK;
}

JNIEXPORT void JNICALL
Java_org_iotivity_ca_CaIpInterface_caIpStateEnabled(JNIEnv *env, jclass class)
{
    (void)env;
    (void)class;
    OIC_LOG(DEBUG, TAG, "caIpStateEnabled");

    CAWakeUpForChange();
}

JNIEXPORT void JNICALL
Java_org_iotivity_ca_CaIpInterface_caIpStateDisabled(JNIEnv *env, jclass class)
{
    (void)env;
    (void)class;
    OIC_LOG(DEBUG, TAG, "caIpStateDisabled");

    u_arraylist_t *iflist = CAIPGetInterfaceInformation(0);
    if (!iflist)
    {
        OIC_LOG_V(ERROR, TAG, "get interface info failed");
        return;
    }
    u_arraylist_destroy(iflist);
}
