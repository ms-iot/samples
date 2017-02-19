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
#include "cainterface.h"
#include "caadapterinterface.h"
#include "caipadapter.h"
#include "caipadapterutils_eth.h"
#include "caadapterutils.h"
#include "oic_malloc.h"
#include "oic_string.h"

#define TAG "IPS"

// Length of the IP address decimal notation string
#define IPNAMESIZE (16)

/** Multicast IP address.*/
#define IPv4_MULTICAST      "224.0.1.187"

/** Multicast Port.*/
#define IPv4_MULTICAST_PORT 5683

CAResult_t CAIPStartUnicastServer(const char *localAddress, uint16_t *port,
                                        const bool forceStart, int32_t *serverFD);
static CAResult_t CAArduinoRecvData(int32_t sockFd);
static void CAArduinoCheckData();
static void CAPacketReceivedCallback(const char *ipAddress, const uint16_t port,
                              const void *data, const uint32_t dataLength);

static CAIPPacketReceivedCallback g_packetReceivedCallback = NULL;
static int g_unicastSocket = 0;
static int g_multicastSocket = 0;

/**
 * @var g_isMulticastServerStarted
 * @brief Flag to check if multicast server is started
 */
static bool g_isMulticastServerStarted = false;

/**
 * @var g_unicastPort
 * @brief Unicast Port
 */
static uint16_t g_unicastPort = 0;

CAResult_t CAIPInitializeServer(const ca_thread_pool_t threadPool)
{
    return CA_STATUS_OK;
}

void CAIPTerminateServer(void)
{
    return;
}

uint16_t CAGetServerPortNum(const char *ipAddress, bool isSecured)
{
    return g_unicastPort;
}

CAResult_t CAIPStartUnicastServer(const char *localAddress, uint16_t *port,
                                  bool secured)
{
    OIC_LOG(DEBUG, TAG, "IN");
    VERIFY_NON_NULL(port, TAG, "port");

    uint8_t rawIPAddr[4];
    char address[16];
    W5100.getIPAddress(rawIPAddr);
    sprintf(address, "%d.%d.%d.%d", rawIPAddr[0], rawIPAddr[1], rawIPAddr[2], rawIPAddr[3]);
    OIC_LOG_V(DEBUG, TAG, "address:%s", address);
    int serverFD = 1;
    if (CAArduinoInitUdpSocket(port, &serverFD) != CA_STATUS_OK)
    {
        OIC_LOG(DEBUG, TAG, "failed");
        return CA_STATUS_FAILED;
    }

    g_unicastPort = *port;
    g_unicastSocket = serverFD;
    CAIPSetUnicastSocket(g_unicastSocket);
    CAIPSetUnicastPort(g_unicastPort);
    caglobals.ip.u4.port =  *port;

    OIC_LOG_V(DEBUG, TAG, "g_unicastPort: %u", g_unicastPort);
    OIC_LOG_V(DEBUG, TAG, "g_unicastSocket: %d", g_unicastSocket);
    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CAIPStartMulticastServer(const char *localAddress, const char *multicastAddress,
                                    uint16_t multicastPort)
{
    OIC_LOG(DEBUG, TAG, "IN");
    if (g_isMulticastServerStarted == true)
    {
        OIC_LOG(ERROR, TAG, "Already Started!");
        return CA_SERVER_STARTED_ALREADY;
    }
    int serverFD = 1;
    if (CAArduinoInitMulticastUdpSocket(multicastAddress, multicastPort, multicastPort,
                                        &serverFD) != CA_STATUS_OK)
    {
        OIC_LOG(DEBUG, TAG, "failed");
        return CA_STATUS_FAILED;
    }

    g_multicastSocket = serverFD;
    g_isMulticastServerStarted = true;

    OIC_LOG_V(DEBUG, TAG, "gMulticastPort: %d", multicastPort);
    OIC_LOG_V(DEBUG, TAG, "g_multicastSocket: %d", g_multicastSocket);
    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CAIPStartServer()
{
    uint16_t unicastPort = 55555;

    CAResult_t ret = CAIPStartUnicastServer("0.0.0.0", &unicastPort, false);
    if (CA_STATUS_OK != ret)
    {
        OIC_LOG_V(ERROR, TAG, "Start unicast server failed[%d]", ret);
        return ret;
    }
    ret = CAIPStartMulticastServer("0.0.0.0", IPv4_MULTICAST, IPv4_MULTICAST_PORT);
    if (CA_STATUS_OK != ret)
    {
        OIC_LOG_V(ERROR, TAG, "Start multicast failed[%d]", ret);
    }
    return ret;
}

CAResult_t CAIPStopUnicastServer()
{
    OIC_LOG(DEBUG, TAG, "IN");
    close(g_unicastSocket);
    g_unicastSocket = 0;
    caglobals.ip.u4.port =  0;
    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CAIPStopMulticastServer()
{
    OIC_LOG(DEBUG, TAG, "IN");
    close(g_multicastSocket);
    g_multicastSocket = 0;
    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CAIPStartListenServer()
{
    OIC_LOG(DEBUG, TAG, "IN");
    CAResult_t ret = CAIPStartMulticastServer("0.0.0.0", IPv4_MULTICAST, IPv4_MULTICAST_PORT);
    if (CA_STATUS_OK != ret)
    {
        OIC_LOG_V(ERROR, TAG, "Start multicast failed[%d]", ret);
    }
    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CAIPStopListenServer()
{
    OIC_LOG(DEBUG, TAG, "IN");
    CAResult_t ret = CAIPStopMulticastServer();
    if (CA_STATUS_OK != ret)
    {
        OIC_LOG_V(ERROR, TAG, "Stop multicast failed[%d]", ret);
    }
    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

void CAIPStopServer()
{
    OIC_LOG(DEBUG, TAG, "IN");
    CAResult_t result = CAIPStopUnicastServer();
    if (CA_STATUS_OK != result)
    {
        OIC_LOG_V(ERROR, TAG, "stop ucast srv fail:%d", result);
        return;
    }
    CAIPSetUnicastSocket(-1);
    CAIPSetUnicastPort(0);

    result = CAIPStopMulticastServer();
    if (CA_STATUS_OK != result)
    {
        OIC_LOG_V(ERROR, TAG, "stop mcast srv fail:%d", result);
    }
    OIC_LOG(DEBUG, TAG, "OUT");
}

void CAPacketReceivedCallback(const char *ipAddress, const uint16_t port,
                              const void *data, const uint32_t dataLength)
{
    OIC_LOG(DEBUG, TAG, "IN");
    if (g_packetReceivedCallback)
    {
        CASecureEndpoint_t sep =
        {.endpoint = {.adapter = CA_ADAPTER_IP, .flags = CA_IPV4, .port = port}};
        OICStrcpy(sep.endpoint.addr, sizeof(sep.endpoint.addr), ipAddress);
        g_packetReceivedCallback(&sep, data, dataLength);
    }
    OIC_LOG(DEBUG, TAG, "OUT");
}

void CAArduinoCheckData()
{
    if (g_unicastSocket)
    {
        if (CAArduinoRecvData(g_unicastSocket) != CA_STATUS_OK)
        {
            OIC_LOG(ERROR, TAG, "rcv fail");
            CAIPStopUnicastServer();
        }
    }

    if (g_multicastSocket)
    {
        if (CAArduinoRecvData(g_multicastSocket) != CA_STATUS_OK)
        {
            OIC_LOG(ERROR, TAG, "rcv fail");
            CAIPStopMulticastServer();
        }
    }
}

/** Retrieve any available data from UDP socket and call callback.
 *  This is a non-blocking call.
 */
CAResult_t CAArduinoRecvData(int32_t sockFd)
{
    /**Bug : When there are multiple UDP packets in Wiznet buffer, W5100.getRXReceivedSize
     * will not return correct length of the first packet.
     * Fix : Use the patch provided for arduino/libraries/Ethernet/utility/socket.cpp
     */

    void *data = NULL;
    uint8_t senderAddr[4] = { 0 };
    char addr[IPNAMESIZE] = {0};
    uint16_t senderPort = 0;

    uint16_t recvLen = W5100.getRXReceivedSize(sockFd);
    if (recvLen == 0)
    {
        // No data available on socket
        return CA_STATUS_OK;
    }

    OIC_LOG_V(DEBUG, TAG, "rcvd %d", recvLen);
    recvLen = recvLen > COAP_MAX_PDU_SIZE ? COAP_MAX_PDU_SIZE:recvLen;

    data = OICCalloc(recvLen + 1, 1);
    if (NULL == data)
    {
        OIC_LOG(DEBUG, TAG, "Out of memory!");
        return CA_MEMORY_ALLOC_FAILED;
    }

    // Read available data.
    int32_t ret = recvfrom(sockFd, (uint8_t *)data, recvLen + 1, senderAddr, &senderPort);
    if (ret < 0)
    {
        OIC_LOG(ERROR, TAG, "rcv fail");
        OICFree(data);
        return CA_STATUS_FAILED;
    }
    else if (ret > 0)
    {
        OIC_LOG(DEBUG, TAG, "data recvd");
        snprintf(addr, sizeof(addr), "%d.%d.%d.%d", senderAddr[0], senderAddr[1], senderAddr[2],
                 senderAddr[3]);
        CAPacketReceivedCallback(addr, senderPort, data, ret);
    }

    OICFree(data);

    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

void CAIPSetPacketReceiveCallback(CAIPPacketReceivedCallback callback)
{
    OIC_LOG(DEBUG, TAG, "IN");
    g_packetReceivedCallback = callback;
    OIC_LOG(DEBUG, TAG, "OUT");
}

void CAIPSetExceptionCallback(CAIPExceptionCallback callback)
{
    // TODO
}

void CAIPSetErrorHandleCallback(CAIPErrorHandleCallback ipErrorCallback)
{
    OIC_LOG(DEBUG, TAG, "IN");
    OIC_LOG(DEBUG, TAG, "OUT");
}

void CAIPPullData()
{
    CAArduinoCheckData();
}

CAResult_t CAGetIPInterfaceInformation(CAEndpoint_t **info, uint32_t *size)
{
    OIC_LOG(DEBUG, TAG, "IN");

    VERIFY_NON_NULL(info, TAG, "info is NULL");
    VERIFY_NON_NULL(size, TAG, "size is NULL");

    u_arraylist_t *iflist = CAIPGetInterfaceInformation(0);
    if (!iflist)
    {
        OIC_LOG(ERROR, TAG, "get interface info failed");
        return CA_STATUS_FAILED;
    }

    uint32_t len = u_arraylist_length(iflist);

    CAEndpoint_t *eps = (CAEndpoint_t *)OICCalloc(len, sizeof (CAEndpoint_t));
    if (!eps)
    {
        OIC_LOG(ERROR, TAG, "Malloc Failed");
        u_arraylist_destroy(iflist);
        return CA_MEMORY_ALLOC_FAILED;
    }

    for (uint32_t i = 0, j = 0; i < len; i++)
    {
        CAInterface_t *ifitem = (CAInterface_t *)u_arraylist_get(iflist, i);
        if(!ifitem)
        {
            continue;
        }
        unsigned char *addr=  (unsigned char *) &(ifitem->ipv4addr);
        snprintf(eps[j].addr, MAX_ADDR_STR_SIZE_CA, "%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);

        eps[j].flags = CA_IPV4;
        eps[j].adapter = CA_ADAPTER_IP;
        eps[j].interface = 0;
        eps[j].port = caglobals.ip.u4.port;
        j++;
    }

    *info = eps;
    *size = len;

    u_arraylist_destroy(iflist);

    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

