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
#include "caipadapterutils_eth.h"

#include <Arduino.h>
#include <Ethernet.h>
#include <socket.h>
#include <w5100.h>
#include <EthernetUdp.h>
#include <IPAddress.h>

#include "logger.h"
#include "cacommon.h"
#include "caadapterinterface.h"
#include "caadapterutils.h"

#define TAG "IPU"

CAResult_t CAArduinoGetAvailableSocket(int *sockID)
{
    VERIFY_NON_NULL(sockID, TAG, "sockID");
    uint8_t state;
    //Is any socket available to work with ?
    *sockID = 0;
    for (int i = 1; i < MAX_SOCK_NUM; i++)
    {
        state = W5100.readSnSR(i);
        if (state == SnSR::CLOSED || state == SnSR::FIN_WAIT)
        {
            *sockID = i;
            break;
        }
    }

    if (*sockID == 0)
    {
        OIC_LOG(ERROR, TAG, "sockID 0");
        return CA_SOCKET_OPERATION_FAILED;
    }

    return CA_STATUS_OK;
}

CAResult_t CAArduinoInitUdpSocket(uint16_t *port, int *socketID)
{
    OIC_LOG(DEBUG, TAG, "IN");
    VERIFY_NON_NULL(port, TAG, "port");
    VERIFY_NON_NULL(socketID, TAG, "socketID");

    CAResult_t ret = CAArduinoGetAvailableSocket(socketID);
    if (ret != CA_STATUS_OK)
    {
        OIC_LOG(ERROR, TAG, "get sock fail");
        return ret;
    }

    //Create a datagram socket on which to recv/send.
    if (!socket(*socketID, SnMR::UDP, *port, 0))
    {
        OIC_LOG(ERROR, TAG, "sock fail");
        return CA_STATUS_FAILED;
    }

    OIC_LOG_V(DEBUG, TAG, "socketId:%d", *socketID);
    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CAArduinoInitMulticastUdpSocket(const char *mcastAddress,
                                           uint16_t mport,
                                           uint16_t lport, int *socketID)
{
    OIC_LOG(DEBUG, TAG, "IN");
    VERIFY_NON_NULL(mcastAddress, TAG, "address");
    VERIFY_NON_NULL(socketID, TAG, "socket");

    uint8_t mcastMacAddr[] = { 0x01, 0x00, 0x5E, 0x00, 0x00, 0x00};
    uint8_t ipAddr[4] = { 0 };
    uint16_t parsedPort = 0;
    if (CAParseIPv4AddressInternal(mcastAddress, ipAddr, sizeof(ipAddr),
                                   &parsedPort) != CA_STATUS_OK)
    {
        OIC_LOG(ERROR, TAG, "parse fail");
        return CA_STATUS_FAILED;
    }

    *socketID = 0;
    CAResult_t ret = CAArduinoGetAvailableSocket(socketID);
    if (ret != CA_STATUS_OK)
    {
        OIC_LOG(ERROR, TAG, "sock fail");
        return ret;
    }

    //Calculate Multicast MAC address
    mcastMacAddr[3] = ipAddr[1] & 0x7F;
    mcastMacAddr[4] = ipAddr[2];
    mcastMacAddr[5] = ipAddr[3];
    W5100.writeSnDIPR(*socketID, (uint8_t *)ipAddr);
    W5100.writeSnDHAR(*socketID, mcastMacAddr);
    W5100.writeSnDPORT(*socketID, mport);

    //Create a datagram socket on which to recv/send.
    if (!socket(*socketID, SnMR::UDP, lport, SnMR::MULTI))
    {
        OIC_LOG(ERROR, TAG, "sock fail");
        return CA_SOCKET_OPERATION_FAILED;
    }

    OIC_LOG_V(DEBUG, TAG, "socketId:%d", *socketID);
    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

