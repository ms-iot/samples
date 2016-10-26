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

#include <Arduino.h>
#include <Ethernet.h>
#include <socket.h>
#include <w5100.h>
#include <EthernetUdp.h>
#include <IPAddress.h>

#include "logger.h"
#include "cacommon.h"
#include "caadapterinterface.h"
#include "caipadapter.h"
#include "caipadapterutils_eth.h"
#include "caadapterutils.h"
#include "oic_malloc.h"
#include "oic_string.h"

#define TAG "IPC"

static int g_sockID = 0;

/**
 * @var g_unicastPort
 * @brief Unicast Port
 */
static uint16_t g_unicastPort = 0;

#define IPv4_MULTICAST     "224.0.1.187"

void CAIPSetUnicastSocket(int socketID)
{
    OIC_LOG(DEBUG, TAG, "IN");
    if (0 < socketID)
    {
        g_sockID = socketID;
    }
    else
    {
        OIC_LOG(ERROR, TAG, "sock err");
    }

    OIC_LOG(DEBUG, TAG, "OUT");
    return;
}

void CAIPSetUnicastPort(uint16_t port)
{
    OIC_LOG(DEBUG, TAG, "IN");
    g_unicastPort = port;
    OIC_LOG(DEBUG, TAG, "OUT");
    return;
}

void CAIPSendData(CAEndpoint_t *endpoint, const void *buf,
                  uint32_t bufLen, bool isMulticast)
{
    if (!isMulticast && 0 == g_unicastPort)
    {
        OIC_LOG(ERROR, TAG, "port 0");
        return;
    }

    VERIFY_NON_NULL_VOID(endpoint, TAG, "endpoint");

    int socketID = 0;
    uint16_t port = endpoint->port;
    if (isMulticast)
    {
        port = CA_COAP;
        OICStrcpy(endpoint->addr, sizeof(endpoint->addr), IPv4_MULTICAST);
        if (CAArduinoInitMulticastUdpSocket(endpoint->addr, port,
                                            g_unicastPort, &socketID) != CA_STATUS_OK)
        {
            OIC_LOG(ERROR, TAG, "init mcast err");
            return;
        }
        OIC_LOG_V(DEBUG, TAG, "MPORT:%u", port);
        OIC_LOG_V(DEBUG, TAG, "LPORT:%u", g_unicastPort);
        OIC_LOG_V(DEBUG, TAG, "SOCKET ID:%d", socketID);
    }
    else
    {
        if (0 == g_sockID)
        {
            if (CAArduinoInitUdpSocket(&port, &socketID) != CA_STATUS_OK)
            {
                OIC_LOG(ERROR, TAG, "init ucast err");
                return;
            }
        }
        else
        {
            socketID = g_sockID;
        }
    }

    uint32_t ret;
    uint8_t ipAddr[4] = { 0 };
    uint16_t parsedPort = 0;
    if (CAParseIPv4AddressInternal(endpoint->addr, ipAddr, sizeof(ipAddr),
                                   &parsedPort) != CA_STATUS_OK)
    {
        OIC_LOG(ERROR, TAG, "parse fail");
        return;
    }

    if (bufLen > 65535) // Max value for uint16_t
    {
        // This will never happen as max buffer size we are dealing with is COAP_MAX_PDU_SIZE
        OIC_LOG(ERROR, TAG, "Size exceeded");
        return;
    }

    ret = sendto(socketID, (const uint8_t *)buf, (uint16_t)bufLen, ipAddr, port);
    if (ret <= 0)
    {
        OIC_LOG_V(ERROR, TAG, "SendData failed: %d", ret);
    }
    if (g_sockID != socketID)
    {
        close(socketID);
    }

    OIC_LOG(DEBUG, TAG, "OUT");
}

